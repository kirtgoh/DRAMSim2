#ifndef BUFFER_H
#define BUFFER_H

#include "SystemConfiguration.h"
#include "BusPacket.h"

using namespace std;

namespace DRAMSim
{

enum CurrentBlockState
{
	BLOCK_INVALID = 0,
	BLOCK_VALID,
	BLOCK_MODIFIED,
};

typedef struct _Block	// per-block size is N times of cacheline
{
	struct _Block *way_next;
	struct _Block *way_prev;

	uint32_t row;
	uint32_t tag;
	unsigned state;

} Block;

class BufferSet
{
	public:
		BufferSet(size_t n)
			:way_head(NULL),way_tail(NULL),lenOfList(0)
		{
			Block *p = NULL;
			for (size_t i = 0; i < n; i++)
			{
				//p = (Buffer *)malloc(sizeof(Buffer));
				p = new Block;
				p->row = 0;
				p->tag = 0;
				p->state = BLOCK_INVALID;

				insert(p);
			}
		}

		BufferSet()
			:way_head(NULL),way_tail(NULL),lenOfList(0)
		{}

		~BufferSet() { clear(); }


		int size() {
			return lenOfList;
		}

		void insert(Block *blk);
		void update_way_list(Block *blk, BufferPolicy policy);

		void clear()
		{
			Block *prev;
			for (Block *p = way_tail; p; p = prev)
			{
				prev = p->way_prev;
				delete p;
			}
			way_head = way_tail = NULL;
		}

		Block *way_head;
		Block *way_tail;
	private:
		size_t lenOfList;
};

class Buffer
{
	ostream &dramsim_log;
public:
	//Functions
 	Buffer(ostream &dramsim_log_);
	void init();

//	~Buffer();

	int handle_hit(BusPacket *p);
	int handle_pre(BusPacket *p);

	// return Size of Buffer in bytes
	int get_size() const {
		return sizeOfBuffer;
	}

	int get_way_count() const {
		return numOfWays;
	}

	int get_set_count() const {
		return numOfSets;
	}

	int get_block_size() const {
		return lenOfBlk;
	}
	unsigned getTag(unsigned);
	vector< BufferSet *> Sets;

	// parameters of this Buffer 
	unsigned lenOfBlk;
	unsigned numOfWays;
	unsigned numOfSets;
	unsigned sizeOfBuffer;

	// used to decode request's set and tag address
	unsigned shift_set;
	unsigned shift_tag;

	uint32_t mask_set;			// use after shift
	uint32_t mask_tag;			// use after shift


	// FIFO, LRU or RANDOM
	BufferPolicy policy;

	// Buffer stats 
	uint64_t hits;				// total number of hits
	uint64_t misses;			// total number of misses
	uint64_t replacements;		// total number of replacements at misses
	uint64_t writebacks;		// total number of writebacks at misses

	//
	uint64_t nextRead;
	uint64_t nextWrite;


	//Fields
	Block *hitBlock;

	bool isHit(BusPacket *packet);


};






}

#endif
