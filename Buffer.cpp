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
	hitBlock(NULL),
	nextRead(0),
	nextWrite(0)
{}

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

void Buffer::buffer_access(BusPacket *packet)
{
	// block address mapping
  	uint32_t tag = BLOCK_TAG(packet->column);
  	uint32_t set = BLOCK_SET(packet->column);

	Block *repl = NULL;
	switch(packet->busPacketType)
	{
		// select the appropriate block to replace, and re-link this entry to
		// the appropriate place in the way list
		case FETCH:
			repl = Sets[set]->way_tail;
			// (re)fill this block
			if (repl->state & BLOCK_MODIFIED)
			{
				ERROR("not restore modified block, when a fetch issued");
				exit(-1);
			}
			repl->row = packet->row;
			repl->tag = tag;
			repl->state = BLOCK_VALID;

			Sets[set]->update_way_list(repl, policy);
			break;
		case RESTORE:
			repl = Sets[set]->way_tail;
			// restore this block
			if (!(repl->state & BLOCK_MODIFIED))
			{
				ERROR("Set: " << set << "state: " << repl->state << " rwo: " << repl->row << " tag: " << repl->tag << "not modified block, when a restore issued");
				exit(-1);
			}
			repl->state = BLOCK_VALID;
			break;
		case READ_B:
			if (!hitBlock)
			{
				ERROR("there is no hit buffer, when a read_b issued");
				exit(-1);
			}

			hits++;
			// move this block to head of the way (MRU) list
			Sets[set]->update_way_list(hitBlock, policy);
			break;
		case WRITE_B:
			if (!hitBlock)
			{
				ERROR("there is no hit buffer, when a write_b issued");
				exit(-1);
			}
			
			hits++;
			// handle write_b 
 	 		hitBlock->state |= BLOCK_MODIFIED;
			Sets[set]->update_way_list(hitBlock, policy);
			break;
		default:
			ERROR("unknown buffer access command!\n");
	}
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
			  // DEBUG("update_way_list");
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

unsigned Buffer::getTag(unsigned a)
{
	return BLOCK_TAG(a);
}

void BufferSet::insert(Block *blk)
{
	blk->way_next = way_head;
	blk->way_prev = NULL;

	// if set is empty, head and tail both point to this block 
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

void Buffer::print()
{
	if (this == NULL)
	{
		return;
	}
	else
	{
		for (size_t i=0; i < numOfSets; i++)
		{
			for (Block *p = Sets[i]->way_tail; p; p = p->way_prev)
				p->print();
			PRINT("");
		}
	}
}

void Block::print()
{
// 	if (this == NULL)
// 	{
// 		return;
// 	}
// 	else
// 	{
// 		switch(state)
// 		{
// 		case BLOCK_VALID:
// 			PRINTN("[" << row << ":" << tag << "]" );
// 			break;
// 		case BLOCK_INVALID:
// 			PRINTN("[invalid]");
// 			break;
// 		case BLOCK_MODIFIED:
// 			PRINTN("[" << row << ":" << tag << "*]" );
// 			break;
// 		case BLOCK_VALID | BLOCK_MODIFIED:
// 			PRINTN("[" << row << ":" << tag << "**]" );
// 			break;
// 		default:
// 			ERROR("Trying to print unkown kind of block");
// 			exit(-1);
//
// 		}
// 	}
}

Block* Buffer::get_repl(unsigned col)
{

  	uint32_t set = BLOCK_SET(col);
	return Sets[set]->way_tail;
}

