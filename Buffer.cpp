//=====================================================================================
// Buffer.cpp - A Buffer Class Implementation 
//
// Copyright (C) 2014 Kirt Goh <kirtgoh@gmail.com>
//=====================================================================================

#include "Buffer.h"

using namespace std;
using namespace DRAMSim;

#define	BLOCK_TAG(addr)	((addr) >> shift_tag)
#define BLOCK_SET(addr)	(((addr) >> shift_set) & mask_set)

Buffer::Buffer(ostream &dramsim_log_):
	dramsim_log(dramsim_log_),
	nextRead(0),
	nextWrite(0),
	hitBlock(NULL)
{}

void Buffer::init()
{
	
	sizeOfBuffer = BUFFER_STORAGE;

	// check system.ini parameters
	// block size must be at least one datum large, i.e., 64 bytes
	lenOfBlk = BUFFER_BLOCK_SIZE;
	if (lenOfBlk < 64 || !isPowerOfTwo(lenOfBlk)) {
		ERROR("Buffer Block size "<<lenOfBlk<<" config error");
		abort();
	}

	// ways of buffer must be a power of two
	numOfWays = BUFFER_WAY_COUNT;
	if (numOfWays <= 0 || !isPowerOfTwo(numOfWays)) {
		ERROR("Buffer Ways number " << numOfWays << " config error");
		abort();
	}

	numOfSets = sizeOfBuffer / (numOfWays * lenOfBlk);
	if (numOfSets <= 0 || !isPowerOfTwo(numOfSets)) {
		ERROR("Buffer Sets number "<<numOfSets <<" config error");
		abort();
	}

	policy = bufferPolicy;

	// allocate the Buffer structure
	for (size_t i = 0; i < numOfSets; i++) {
			BufferSet *set = new BufferSet(numOfWays);
			Sets.push_back(set);
	}

	// BusPacket->column only have column high bits, 64B span.
	shift_set = dramsim_log2(lenOfBlk / 64); 
	shift_tag = shift_set + dramsim_log2(numOfSets);

	mask_set = numOfSets - 1;
	// NUM_COLS refer to 8 device chip bits and 8 depth bits
	mask_tag = (1 << (dramsim_log2(NUM_COLS) - 3 - shift_tag)) -1;
}

//FIXME: Segment Fault, before Buffer init , deconstruct happens.
/* Buffer::~Buffer()
 * {
 * 	for (size_t i = 0; i < numOfSets; i++) {
 * 		Sets[i]->clear();
 * 		delete Sets[i]; 
 * 	}
 * }
 * 
 */

unsigned Buffer::getTag(unsigned a)
{
	return BLOCK_TAG(a);
}

void BufferSet::insert(Block *blk)
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

void BufferSet::update_way_list(Block *blk, BufferPolicy policy)
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
/* commandQueue.pop() test if cmd is buffer hitted before popped
 * if hitted, keey hit buffer used to test
 */
bool Buffer::isHit(BusPacket *packet)
{
	// just need column address to mapping
  	uint32_t tag = BLOCK_TAG(packet->column);
  	uint32_t set = BLOCK_SET(packet->column);

  	// current block used to detect
  	Block *blk = NULL;		

  	// linear search the way list
  	for (blk = Sets[set]->way_head; blk; blk = blk->way_next)
  	{
  	  if (blk->row == packet->row && blk->tag == tag && (blk->state & BLOCK_VALID)
			  && (packet->busPacketType == READ ||
				  packet->busPacketType == READ_B ||
				  packet->busPacketType == ACTIVATE ||
				  packet->busPacketType == WRITE ||
				  packet->busPacketType == WRITE_B)) {

		  hitBlock = blk;
  		  return true;
	  }
	}

	hitBlock = NULL;
	return false;
}

int Buffer::handle_hit(BusPacket *packet) 
{
	hits++;

  	uint32_t set = BLOCK_SET(packet->column);

	if (hitBlock == NULL)
		ERROR("hit buffer should not be empty!\n");
	// update dirty status
	if (packet->busPacketType == WRITE_B)
		hitBlock->state |= BLOCK_MODIFIED;


	// if LRU replacement and this is not the first element of list, reorder
	if (hitBlock->way_prev && policy == LRU)
	{
	  // move this block to head of the way (MRU) list
	  Sets[set]->update_way_list(hitBlock, LRU);
	}
	return 0;

}

int Buffer::handle_pre(BusPacket *packet)
{

  	uint32_t tag = BLOCK_TAG(packet->column);
  	uint32_t set = BLOCK_SET(packet->column);

	// select the appropriate block to replace, and re-link this entry to
	// the appropriate place in the way list
	Block *repl = NULL;

	// find a free buffer slot
	switch (policy)
	{
		case LRU:
		case FIFO:
			repl = this->Sets[set]->way_tail;
			this->Sets[set]->update_way_list(repl, policy);
			break;
		case RANDOM:
			// FIXME: do something 
			break;
		default:
			ERROR("bogus replacement policy");
	}

	// replace if neccessory
	if (repl->state & BLOCK_VALID)
	{
		replacements++;

		if (repl->state & BLOCK_MODIFIED)
		{
		  // write back the cache block
		  writebacks++;
		}
	}
	//FIXME: just row ?
	//repl->tag = tag;
	repl->row = packet->row;
	repl->tag = tag;
	repl->state = BLOCK_VALID;

	//TODO: handle write ?
/* 	if (packet->busPacketType == WRITE)
 * 		repl->state |= BLOCK_MODIFIED;
 */

	return 0;
}

