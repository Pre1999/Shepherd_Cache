/**
 * Copyright (c) 2018-2020 Inria
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

#include "mem/cache/replacement_policies/sc_rp.hh"
// #include "mem/cache/nvc.hh"
#include "mem/cache/tags/sc_set_assoc.hh"

#include <cassert>
#include <memory>

#include "params/SCRP.hh"
#include "sim/cur_tick.hh"

namespace gem5
{

// uint8_t nvc[2048][4];

GEM5_DEPRECATED_NAMESPACE(ReplacementPolicy, replacement_policy);
namespace replacement_policy
{

// uint8_t nvc[2048][4];

SC::SC(const Params &p)
  : Base(p)
{
}

void
SC::invalidate(const std::shared_ptr<ReplacementData>& replacement_data)
{
    // Reset last touch timestamp
    std::static_pointer_cast<SCReplData>(
        replacement_data)->lastTouchTick = Tick(0);
    std::static_pointer_cast<SCReplData>(
        replacement_data)->tickInserted = Tick(0);
}

void
SC::touch(const std::shared_ptr<ReplacementData>& replacement_data) const
{
    // Update last touch timestamp
    std::static_pointer_cast<SCReplData>(
        replacement_data)->lastTouchTick = curTick();
}

void
SC::reset(const std::shared_ptr<ReplacementData>& replacement_data) const
{
    // Set last touch timestamp
    std::static_pointer_cast<SCReplData>(
        replacement_data)->lastTouchTick = curTick();
    std::static_pointer_cast<SCReplData>(
        replacement_data)->tickInserted = curTick();
}

/*ReplaceableEntry*
SC::getVictim(const ReplacementCandidates& candidates) const
{
    // There must be at least one replacement candidate
    assert(candidates.size() > 0);

	// std::cout<<"--> Number of Candidates: "<<candidates.size()<<"\n";		-- Verified : Candidates = 16

    // Visit all candidates to find victim
    ReplaceableEntry* victim = candidates[2];
    ReplaceableEntry* temp_victim = candidates[0];
    ReplaceableEntry* temp_sc = candidates[0]; 
    int temp_index, temp_index_loop, temp_way, temp_cm, print_var;
    uint32_t set_sc;
    temp_index = 0;
    temp_index_loop = 0;
    temp_way = 0;
    temp_cm = 0;
    set_sc = 0;

    //for (const auto& candidate : candidates) 

    // Update victim entry if necessary
	for(int i=4; i<16; i++)
	{
		if(candidates[i]->used == 0)
		{ 
			victim = candidates[i];
			candidates[i]->used = 1;
			candidates[i]->SC_flag = 0;
			candidates[i]->SC_ptr = 0;
			for(int j=0; j<16; j++){
				candidates[j]->CM_entry[0] = 20; // To signify empty flag in CM entry
				candidates[j]->CM_entry[1] = 20; // To signify empty flag in CM entry
				candidates[j]->CM_entry[2] = 20; // To signify empty flag in CM entry
				candidates[j]->CM_entry[3] = 20; // To signify empty flag in CM entry
			}
			return victim;
		}
	}

    // It means all MC are filled. Going to SCs.
    for(int i=0; i<4; i++)
	{
		if(candidates[i]->used == 0)
		{
			victim = candidates[i];
			candidates[i]->used = 1;
			candidates[i]->SC_flag = 1;
			candidates[i]->SC_ptr = i;
			for(int j=0; j<16; j++)
			{
				candidates[j]->CM_entry[i] = 20; // To signify empty flag in CM entry
			}
			for(int k=0; k<4; k++)
			{
				if(i!=k && candidates[k]->used==1)
				{
					candidates[i]->CM_entry[k] = 0;
				}
			}
			set_sc = victim->getSet();
			// std::cout<<"-> Victim Set No: "<<set_sc<<"\n";

			victim->nvc[i] = 0;
			// print_var = victim->nvc[i];
			// std::cout<<"||-> NVC Val:"<<print_var<<"\n";

			// std::cout<<"-> NVC Vals: \n";
            // for(int k=0; k<4; k++)
            //     std::cout<<victim->nvc[k]<<" ";
            // std::cout<<"\n";

			for(int k=0; k<16; k++)
                candidates[k]->nvc[i] = victim->nvc[i];
			// std::cout<<"-> NVC Vals: \n";
            // for(int k=0; k<16; k++)
			// {
			// 	for(int l=0; l<4; l++)
            //     	std::cout<<candidates[k]->nvc[l]<<" ";
			// 	std::cout<<"\n";
			// }
            // std::cout<<"\n";
			return victim;
		}
    }

    // This means that SCs are also filled already
	for(int i=0; i<4; i++)
	{
		if (std::static_pointer_cast<SCReplData>(
				candidates[i]->replacementData)->lastTouchTick <
				std::static_pointer_cast<SCReplData>(
				temp_sc->replacementData)->lastTouchTick) 
		{
			temp_sc = candidates[i];
			temp_way = i;
		}
	}

	// std::cout<<"Count Matrix: \n";
	// for(int i=0; i<16; i++)
	// {
	// 	for(int k=0; k<4; k++)
	// 		std::cout<<candidates[i]->CM_entry[k]<<" ";
	// 	std::cout<<"\n";
	// }
	// std::cout<<"\n";	

	set_sc = temp_sc->getSet();

	// std::cout<<"----> NVC Vals: \n";
	// for(int k=0; k<4; k++)
	// 	std::cout<<nvc[set_sc][k]<<" ";
	// std::cout<<"\n"; 

	for(int k=0; k<16; k++)
        candidates[k]->nvc[temp_way] = 0;

	temp_victim = temp_sc;
	if(temp_sc->CM_entry[temp_way] == 20)
		temp_cm = 0;
	else
		temp_cm = temp_sc->CM_entry[temp_way];
	
	for(int j=4; j<16; j++)
	{
		if(candidates[j]->CM_entry[temp_way] != 20)
		{
			if(candidates[j]->CM_entry[temp_way] > temp_cm)
			{
				temp_cm = candidates[j]->CM_entry[temp_way];
				temp_victim = candidates[j];
				temp_index_loop = j;
			}
		}
	}
	
	if(temp_cm > temp_sc->CM_entry[temp_way])
		temp_index = temp_index_loop;
	else
		temp_index = temp_way;
	
	if(temp_index != temp_way)
	{	
        for(int i=0; i<4; i++)
		{
			candidates[temp_index]->CM_entry[i] = temp_sc->CM_entry[i];
			temp_sc->CM_entry[i] = 0;
			const auto& location1 = candidates[temp_index];
			CacheBlk* blk1 = static_cast<CacheBlk*>(location1);
			const auto& location2 = temp_sc;
			CacheBlk* blk2 = static_cast<CacheBlk*>(location2);
			blk1->data = blk2->data;
			// *(blk1->data) = *(blk2->data);
			// for (const auto& location : candidates) 
			// {
			// 	CacheBlk* blk = static_cast<CacheBlk*>(location);
			// }
		}
	}	
	
	for(int j=0; j<16; j++)
	{
		candidates[j]-> CM_entry[temp_way] = 20; // To signify empty flag in CM entry
	}
	victim = temp_sc;

	return victim;
}*/

ReplaceableEntry*
SC::getVictim(const ReplacementCandidates& candidates) const
{
    // There must be at least one replacement candidate
    assert(candidates.size() > 0);

	// std::cout<<"--> Number of Candidates: "<<candidates.size()<<"\n";		-- Verified : Candidates = 16

    // Visit all candidates to find victim
    ReplaceableEntry* victim = candidates[2];
    ReplaceableEntry* temp_victim = candidates[0];
    ReplaceableEntry* temp_sc = candidates[0];
    int temp_index, temp_index_loop, temp_way, temp_cm, sc_index;
    uint32_t set_sc;
    temp_index = 0;
    temp_index_loop = 0;
    temp_way = 0;
    temp_cm = 0;
    set_sc = 0;

    //for (const auto& candidate : candidates) 

    // Update victim entry if necessary
	for(int i=0; i<16; i++)
	{
		if(candidates[i]->SC_flag == 0 && candidates[i]->used == 0)
		{ 
			victim = candidates[i];
			candidates[i]->used = 1;
			// candidates[i]->SC_flag = 0;
			// candidates[i]->SC_ptr = 0;
			for(int j=0; j<16; j++){
				candidates[j]->CM_entry[0] = 20; // To signify empty flag in CM entry
				candidates[j]->CM_entry[1] = 20; // To signify empty flag in CM entry
				candidates[j]->CM_entry[2] = 20; // To signify empty flag in CM entry
				candidates[j]->CM_entry[3] = 20; // To signify empty flag in CM entry
			}
			return victim;
		}
	}

    // It means all MC are filled. Going to SCs.
    for(int i=0; i<16; i++)
	{
		if(candidates[i]->SC_flag == 1 && candidates[i]->used == 0)
		{
			victim = candidates[i];
			candidates[i]->used = 1;
			// candidates[i]->SC_flag = 1;
			// candidates[i]->SC_ptr = i;
			for(int j=0; j<16; j++)
			{
				candidates[j]->CM_entry[candidates[i]->SC_ptr] = 20; // To signify empty flag in CM entry
			}
			// for(int k=0; k<4; k++)
			// {
			// 	if(candidates[i]->SC_ptr!=k )
			// 	{
			// 		candidates[i]->CM_entry[k] = 0;
			// 	}
			// }
			set_sc = victim->getSet();
			nvc[set_sc][candidates[i]->SC_ptr] = 0;
			return victim;
		}
    }

	// Re-initializing temp_sc to first SC entry found
	for(int i=0; i<16; i++)
	{
		if (candidates[i]->SC_flag == 1)
		{
			temp_sc = candidates[i];
			break;
		}
	}

    // This means that SCs are also filled already
	for(int i=0; i<16; i++)
	{
		if (candidates[i]->SC_flag == 1)
		{
			if (std::static_pointer_cast<SCReplData>(
					candidates[i]->replacementData)->tickInserted <
					std::static_pointer_cast<SCReplData>(
					temp_sc->replacementData)->tickInserted) 
			{
				temp_sc = candidates[i];
				sc_index = i;
				temp_way = candidates[i]->SC_ptr;
			}
		}
	}
		// -- Issue Might be here
	// std::cout<<"Count Matrix: \n";
	for(int i=0; i<16; i++)
	{
		for(int k=0; k<4; k++)
			std::cout<<candidates[i]->CM_entry[k]<<" ";
		std::cout<<"\n";
	}
	std::cout<<"\n";

	set_sc = temp_sc->getSet();
	nvc[set_sc][temp_way] = 0;
	temp_victim = temp_sc;
	
	if(temp_sc->CM_entry[temp_way] == 20)
		temp_cm = 0;
	else
		temp_cm = temp_sc->CM_entry[temp_way];
	
	for(int j=0; j<16; j++)
	{
		if(candidates[j]->SC_flag == 0 && candidates[j]->CM_entry[temp_way] != 20)
		{
			if(candidates[j]->CM_entry[temp_way] > temp_cm)
			{
				temp_cm = candidates[j]->CM_entry[temp_way];
				temp_victim = candidates[j];
				temp_index_loop = j;
			}
		}
	}

	if(temp_cm > temp_sc->CM_entry[temp_way])
		temp_index = temp_index_loop;
	else
		temp_index = sc_index;
	
	if(temp_index != sc_index)
	{	
		candidates[sc_index]->SC_flag = 0;						// Becomes MC
		candidates[temp_index]->SC_flag = 1;					// Becomes SC
		candidates[temp_index]->SC_ptr = temp_sc->SC_ptr;
        for(int i=0; i<4; i++)
		{
			candidates[temp_index]->CM_entry[i] = 0;
		}
		victim = candidates[temp_index];
	}
	else
		victim = temp_sc;
	
	for(int j=0; j<16; j++)
	{
		candidates[j]->CM_entry[temp_way] = 20; // To signify empty flag in CM entry
	}

	return victim;
}

std::shared_ptr<ReplacementData>
SC::instantiateEntry()
{
    return std::shared_ptr<ReplacementData>(new SCReplData());
}

} // namespace replacement_policy
} // namespace gem5
