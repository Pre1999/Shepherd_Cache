/*
 * Copyright (c) 2012-2014 ARM Limited
 * All rights reserved.
 *
 * The license below extends only to copyright in the software and shall
 * not be construed as granting a license to any other intellectual
 * property including but not limited to intellectual property relating
 * to a hardware implementation of the functionality of the software
 * licensed hereunder.  You may use the software subject to the license
 * terms below provided that you ensure that this notice is replicated
 * unmodified and in its entirety in all distributions of the software,
 * modified or unmodified, in source code or in binary form.
 *
 * Copyright (c) 2003-2005,2014 The Regents of The University of Michigan
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file
 * Definitions of a conventional tag store.
 */

#include "mem/cache/tags/sc_set_assoc.hh"

#include <string>

#include "base/intmath.hh"

namespace gem5
{



SCSetAssoc::SCSetAssoc(const Params &p)
    :BaseTags(p), allocAssoc(p.assoc), blks(p.size / p.block_size),
     sequentialAccess(p.sequential_access),
     replacementPolicy(p.replacement_policy)
{
    // There must be a indexing policy
    fatal_if(!p.indexing_policy, "An indexing policy is required");

    // Check parameters
    if (blkSize < 4 || !isPowerOf2(blkSize)) {
        fatal("Block size must be at least 4 and a power of 2");
    }
}

void
SCSetAssoc::tagsInit()
{
    // Initialize all blocks
    for (unsigned blk_index = 0; blk_index < numBlocks; blk_index++) {
        // Locate next cache block
        CacheBlk* blk = &blks[blk_index];

        // Link block to indexing policy
        indexingPolicy->setEntry(blk, blk_index);

        // Associate a data chunk to the block
        blk->data = &dataBlks[blkSize*blk_index];

        // Associate a replacement data entry to the block
        blk->replacementData = replacementPolicy->instantiateEntry();
        
        // Initializing the Shepherd Cache and Main Cache
        if(blk->getWay() < 4)
        {
            blk->SC_flag = 1;
            blk->SC_ptr = blk->getWay();
        }
        else
        {
            blk->SC_flag = 0;
            blk->SC_ptr = blk->getWay();
        }
    }
}

void
SCSetAssoc::invalidate(CacheBlk *blk)
{
    BaseTags::invalidate(blk);

    // Decrease the number of tags in use
    stats.tagsInUse--;

    // Invalidate replacement data
    replacementPolicy->invalidate(blk->replacementData);
}

void
SCSetAssoc::moveBlock(CacheBlk *src_blk, CacheBlk *dest_blk)
{
    BaseTags::moveBlock(src_blk, dest_blk);

    // Since the blocks were using different replacement data pointers,
    // we must touch the replacement data of the new entry, and invalidate
    // the one that is being moved.
    replacementPolicy->invalidate(src_blk->replacementData);
    replacementPolicy->reset(dest_blk->replacementData);
}

CacheBlk*
SCSetAssoc::findBlock(Addr addr, bool is_secure) const
{
    // Extract block tag
    Addr tag = extractTag(addr);
    uint32_t set_sc = 0;
    static int print_var, temp_var;

    // Find possible entries that may contain the given address
    const std::vector<ReplaceableEntry*> entries =
        indexingPolicy->getPossibleEntries(addr);

    // std::cout<<"--> Number of Ways: "<<entries.size()<<"\n";         -- Verified : Ways = 16

    // Search for block
    for (const auto& location : entries) 
    {
        CacheBlk* blk = static_cast<CacheBlk*>(location);
        if (blk->matchTag(tag, is_secure)) 
        {
            set_sc = blk->getSet();
            for(uint32_t i = 0; i<4; i++)
            {
                if(entries[i]->used == 1)
                {
                    temp_var = blk->nvc[i];
                    blk->CM_entry[i] = temp_var;
                    // std::cout<<"-> Block NVC: "<<temp_var<<"\n";
                    print_var = blk->CM_entry[i];
                    // std::cout<<"-> Block CM Entry: "<<print_var<<"\n";
                    if(blk->nvc[i] <= 15)
                    {
                        print_var = blk->nvc[i];
                        blk->nvc[i] = print_var + 1;
                        for(int k=0; k<16; k++)
                        {
                            temp_var = blk->nvc[i];
                            entries[k]->nvc[i] = temp_var;
                        }
                    }
                }
            }

            // std::cout<<"-> Block Set Number: "<<set_sc<<"\n";
            
            // std::cout<<"-> Entries Used: \n";
            // for(int k=0; k<16; k++)
            // {
            //     print_var = entries[k]->used; 
            //     std::cout<<print_var<<" ";
            // }
            // std::cout<<"\n";

            // std::cout<<"-> SC Entries: \n";
            // for(int k=0; k<16; k++)
            // {
            //     print_var = entries[k]->SC_flag;
            //     std::cout<<print_var<<" ";
            // }
            // std::cout<<"\n";   

            // std::cout<<"-> NVC Vals: \n";
            // for(int k=0; k<4; k++)
            // {
            //     print_var = blk->nvc[k];
            //     std::cout<<print_var<<" ";
            // }
            // std::cout<<"\n";        

            // std::cout<<"-> Blk Count Matrix: \n";
            // for(int k=0; k<4; k++)
            // {
            //     print_var = blk->CM_entry[k]; 
            //     std::cout<<print_var<<" ";
            // }
            // std::cout<<"\n";        

            // std::cout<<"-> Entries Count Matrix: \n";
            // for(int i=0; i<16; i++)
            // {
            //     for(int k=0; k<4; k++)
            //     {
            //         temp_var = entries[i]->CM_entry[k]; 
            //         std::cout<<temp_var<<" ";
            //     }
            //     std::cout<<"\n";
            // }
            // std::cout<<"\n";

            return blk;
        }
    }

    // Did not find block
    return nullptr;
}

/*CacheBlk*
SCSetAssoc::findBlock(Addr addr, bool is_secure) const
{
    // Extract block tag
    Addr tag = extractTag(addr);
    uint32_t set_sc = 0;

    // Find possible entries that may contain the given address
    const std::vector<ReplaceableEntry*> entries =
        indexingPolicy->getPossibleEntries(addr);

    // std::cout<<"--> Number of Ways: "<<entries.size()<<"\n";         -- Verified : Ways = 16

    // Search for block
    for (const auto& location : entries) 
    {
        CacheBlk* blk = static_cast<CacheBlk*>(location);
        if (blk->matchTag(tag, is_secure)) 
        {
            set_sc = blk->getSet();
            for(uint32_t i = 0; i<16; i++)
            {
                if(entries[i]->SC_flag == 1 && entries[i]->used == 1)
                {
                    blk->CM_entry[entries[i]->SC_ptr] = nvc[set_sc][entries[i]->SC_ptr];
                    if(nvc[set_sc][entries[i]->SC_ptr] <= 15)
                        nvc[set_sc][entries[i]->SC_ptr] = nvc[set_sc][entries[i]->SC_ptr] + 1;
                }
            }
            return blk;
        }
    }

    // Did not find block
    return nullptr;
}*/

} // namespace gem5
