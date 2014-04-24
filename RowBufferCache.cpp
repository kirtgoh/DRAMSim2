//=====================================================================================
// RowBufferCache.cpp - A RowBuffer Cache Class Implementation
//
// Copyright (C) 2014 Kirt Goh <kirtgoh@gmail.com>
//=====================================================================================

#include "RowBufferCache.h"

using namespace DRAMSim;

#define	CACHE_TAG(addr) ((addr) >> shift_tag)
#define CACHE_SET(addr)	(((addr) >> shift_set) & mask_set)

//=====================================================================================
// Class CacheSet
//-------------------------------------------------------------------------------------
// insert - insert a pointed cacheline into head of way list
//
void CacheSet::insert(CacheLine *blk)
{
	blk->way_next = way_head;
	blk->way_prev = NULL;

	// if set is empty, head and tail both point to this cacheline
	// otherwise, only head pointer needs to change
	if (!way_tail && !way_head) {
		way_tail = blk;
		way_head = blk;
	} else {
		way_head->way_prev = blk; 
		way_head = blk;
	}

	lenOfList++;
}

//-------------------------------------------------------------------------------------
// update_way_list - update pointed cacheline to location of way list, according cache policy
//
void CacheSet::update_way_list(CacheLine *blk, CachePolicy policy)
{
	switch(policy) {
		case FIFO:
		case LRU:
			// unlink entry from the way list
			if (!blk->way_prev && !blk->way_next)
			{
			  // only one entry in list (direct-mapped), no action
			  // Head/Tail order already
			  return;
			}
			// else, more than one element in the list
			else if (!blk->way_prev)
			{
			  // already there
			  return;
			}
			else if (!blk->way_next)
			{
			  // end of list (and not front of list)
			  way_tail = blk->way_prev;
			  blk->way_prev->way_next = NULL;
			}
			else
			{
			  // middle of list (and not front or end of list)
			  blk->way_prev->way_next = blk->way_next;
			  blk->way_next->way_prev = blk->way_prev;
			}

			// link BLK back into the list
			// link to the head of the way list
			blk->way_next = way_head;
			blk->way_prev = NULL;
			way_head->way_prev = blk;
			way_head = blk;
			break;
		case RANDOM:
			break;
		default:
			ERROR("policy error!\n");
	}
}

//=====================================================================================
// Class RowBufferCache
//-------------------------------------------------------------------------------------
// constructor 
//
RowBufferCache::RowBufferCache (unsigned kilosOfCache,
								unsigned ways,
								unsigned linelen,
								enum CachePolicy repl):
	// initialize cache stats
	hits(0), misses(0), replacements(0), writebacks(0)	
{
	// check all cache parameters!
	// cacheline must be at least one datum large, i.e., 64 bytes for common
	if (linelen < 8) {
		ERROR("Cache block size (in bytes) " << linelen << " must be 8 or greater");
		abort();
	} else if (!isPowerOfTwo(linelen)) {
		ERROR("Cache block size (in bytes) " << linelen << " must be a power of two");
		abort();
	}

	// ways of cache must be a power of two
	if (ways <= 0) {
		ERROR("Cache associativity " << ways << " must be a positive value");
		abort();
	} else if (!isPowerOfTwo(ways)) {
		ERROR("Cache associativity " << ways << " must be a power of two");
		abort();
	}

	// initialize user parameters
	lenOfLine = linelen;
	numOfWays = ways;

	// cache size is SETS * WAYS * LINE_SIZE, so
	numOfSets = kilosOfCache * 1024 / (numOfWays * lenOfLine);

	if (!isPowerOfTwo(numOfSets)) {
		ERROR("Cache size (in sets)"<<numOfSets <<"is not a power of two");
		abort();
	}

	// allocate the cache structure
	Sets = vector< CacheSet *>(numOfSets);
	for (size_t i = 0; i < numOfSets; i++) {
			Sets[i] = new CacheSet(numOfWays);
	}
	policy = repl;

	shift_set = dramsim_log2(lenOfLine);				// shift to the set address, strip the cacheline address
	shift_tag = shift_set + dramsim_log2(numOfSets);	// shift to the tag address

	mask_set = numOfSets - 1;							// binary mask used to obtain cache sets address
	mask_tag = (1 << (32 - shift_tag)) - 1;				// strip set and block bits, remain is tag address

}

//-------------------------------------------------------------------------------------
// deconstructor 
//
RowBufferCache::~RowBufferCache()
{
	Sets = vector< CacheSet *>(numOfSets);
	for (size_t i = 0; i < numOfSets; i++) {
			delete Sets[i]; 
	}
}

//-------------------------------------------------------------------------------------
// handle_hit - handle READ/WRITE hit.
//
// @blk: point to the hit cacheline
// @cmd: if WRITE, change cacheline state to LINE_MODIFIED
// @set: which sets hit happen
//
int RowBufferCache::handle_hit(CacheLine *blk, BusPacketType cmd, unsigned set)
{
	hits++;

	// update dirty status
	if (cmd == WRITE)
		blk->state |= LINE_MODIFIED;

	// if LRU replacement and this is not the first element of list, reorder
	if (blk->way_prev && policy == LRU)
	{
	  // move this block to head of the way (MRU) list
	  Sets[set]->update_way_list(blk, LRU);
	}
	return 0;

}

//-------------------------------------------------------------------------------------
// handle_miss - handle READ/WRITE miss.
//
// @tag: tag address to access
// @set: set address to access
// @cmd: if WRITE, change cacheline state to LINE_MODIFIED
//
int RowBufferCache::handle_miss(uint32_t tag, uint32_t set,  BusPacketType cmd)
{
	misses++;

	// select the appropriate block to replace, and re-link this entry to
	// the appropriate place in the way list
	CacheLine *repl = NULL;

	switch (policy)
	{
		case LRU:
		case FIFO:
			repl = Sets[set]->way_tail;
			Sets[set]->update_way_list(repl, policy);
			break;
		case RANDOM:
			// FIXME: do something 
			break;
		default:
			ERROR("bogus replacement policy");
	}

	if (repl->state & LINE_VALID)
	{
		replacements++;

		if (repl->state & LINE_MODIFIED)
		{
		  // write back the cache block
		  writebacks++;
		}
	}

	repl->tag = tag;
	repl->state = LINE_VALID;

	if (cmd == WRITE)
		repl->state |= LINE_MODIFIED;

	return 0;
}

//-------------------------------------------------------------------------------------
// access - access a cache, perform a CMD operation on cache at address ADDR
//
unsigned RowBufferCache::access(BusPacketType cmdType, uint32_t addr)
{
	// decode address infomation
	uint32_t tag = CACHE_TAG(addr);
	uint32_t set = CACHE_SET(addr);

	// current block used to detect
	CacheLine *blk = NULL;		

	// linear search the way list
	for (blk = Sets[set]->way_head; blk; blk = blk->way_next)
	{
	  if (blk->tag == tag && (blk->state & LINE_VALID)) {
		  handle_hit(blk, cmdType, set);
		  return 1;
	  }
	}

	// cache block not found
	handle_miss(tag, set, cmdType);
	
	return 0;
}
