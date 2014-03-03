//=====================================================================================
// RowBufferCache.h - RowBuffer Cache API 
//
// Copyright (C) 2014 Kirt Goh <kirtgoh@gmail.com>
//=====================================================================================

#ifndef _ROWBUFFERCACHE_H_ 
#define _ROWBUFFERCACHE_H_ 

#include <vector>
#include <stdint.h>
#include "SystemConfiguration.h"
#include "BusPacket.h"

using namespace std;

namespace DRAMSim {

//-------------------------------------------------------------------------------------
// generic defines and data types
//
enum CachePolicy	// cache replacement policy 
{
	LRU,		// replace least recently used block (perfect LRU)
	RANDOM,		// replace a random block
	FIFO		// replace the oldest block in the set
};

enum CacheState		// cache line state
{
	LINE_INVALID = 0,
	LINE_VALID,
	LINE_MODIFIED,
};

typedef struct _CacheLine	// cache line (or block) definition
{
	struct _CacheLine *way_next;
	struct _CacheLine *way_prev;

	uint32_t tag;
	unsigned state;

} CacheLine;

//-------------------------------------------------------------------------------------
// Class CacheSet definition - one or more lines sharing the same set index
//
class CacheSet
{
	public:
		CacheSet(size_t n)
			:way_head(NULL),way_tail(NULL),lenOfList(0)
		{
			CacheLine *p = NULL;
			for (size_t i = 0; i < n; i++)
			{
				p = new CacheLine;
				insert(p);
			}
		}

		CacheSet()
			:way_head(NULL),way_tail(NULL),lenOfList(0)
		{}

		~CacheSet() { clear(); }


		int size() {
			return lenOfList;
		}

		void insert(CacheLine *line);
		void update_way_list(CacheLine *blk, CachePolicy policy);

		void clear()
		{
			CacheLine *prev;
			for (CacheLine *p = way_tail; p; p = prev)
			{
				prev = p->way_prev;
				delete p;
			}
			way_head = way_tail = NULL;
		}

		CacheLine *way_head;
		CacheLine *way_tail;
	private:
		size_t lenOfList;
};

//-------------------------------------------------------------------------------------
// Class RowBufferCache definition - Per Rank
//
class RowBufferCache
{
	public:

		// constructor
		RowBufferCache(unsigned kilosOfCache,
				unsigned ways,
				unsigned linelen,
				CachePolicy repl);

		// deconstructor
		~RowBufferCache();
		
		unsigned access(BusPacketType cmdType, uint32_t addr);

		int handle_hit(CacheLine *blk, BusPacketType cmd, unsigned set);
		int handle_miss(uint32_t tag, uint32_t set,  BusPacketType cmd);

		// return Size of Cache in bytes
		int get_size() const {
			return (numOfWays * numOfSets * lenOfLine);
		}

		int get_way_count() const {
			return numOfWays;
		}

		int get_set_count() const {
			return numOfSets;
		}

		int get_line_size() const {
			return lenOfLine;
		}
		vector< CacheSet *> Sets;

		// parameters of this cache
		unsigned lenOfLine;
		unsigned numOfWays;
		unsigned numOfSets;

		// used to decode request's set and tag address
		unsigned shift_set;
		unsigned shift_tag;

		uint32_t mask_set;			// use after shift
		uint32_t mask_tag;			// use after shift


		// FIFO, LRU or RANDOM
		CachePolicy policy;

		// per-cache stats 
		uint64_t hits;				// total number of hits
		uint64_t misses;			// total number of misses
		uint64_t replacements;		// total number of replacements at misses
		uint64_t writebacks;		// total number of writebacks at misses
};

}

#endif // _ROWBUFFERCACHE_H_
