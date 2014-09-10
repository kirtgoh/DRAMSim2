//=====================================================================================
// Buffer.h - A Victim Buffer implementation based on unordered_map
//
// Copyright (C) 2014 Kirt Goh <kirtgoh@gmail.com>
//=====================================================================================

#ifndef VICTIM_BUFFER_H
#define VICTIM_BUFFER_H

#include <list>
#include <vector>
#include <unordered_map>
#include <SystemConfiguration.h>    // enum BufferPolicy
#include <BusPacket.h>

using namespace std;

namespace DRAMSim
{

enum BlockState
{
	BLOCK_INVALID = 0,
	BLOCK_VALID,
	BLOCK_MODIFIED,
};

struct Block {
    int tag;    // row and column segment address
    int state;

    Block(int t): tag(t), state(BLOCK_INVALID) {}
    void print(unsigned maskOfseg);
};

class BufferSet;

//=====================================================================================
// Buffer class definition
//
class Buffer
{
    ostream &dramsim_log;
public:
    // Initial parameters passed from system.ini file
    Buffer(ostream &dramsim_log_);

    // bufferAccess (packet) - handle FETCH/RESTORE/READ_B/WRITE_B protocol
    void bufferAccess(BusPacket *packet);

    // probe (row, col) - Probe the tag address of the packet if the data exists
    // in the block, otherwise return false.
    bool probe(unsigned row, unsigned col);

    size_t size() const { return sets.size();}
    size_t storage() const { return BUFFER_STORAGE;}

    // isFetchSafe (fetchRow, fetchCol, restoreRow) - FETCH iussing safely
    // check. If back of buffer block is Modified, get its row for RESTORE,
    // otherwise return ture.
    bool isFetchSafe(unsigned fetchRow, unsigned fetchCol, unsigned &restoreRow);

    void print();

    // Fields 
    uint64_t nextRead;
    uint64_t nextWrite;

private:
    // decodeAddress (row, col, &tag, &set) - pick set and tag address of buffer
    // block with row and col address of bank
    void decodeAddress(unsigned row, unsigned col, unsigned &tag, unsigned &set);

    // buffer data
    vector<BufferSet> sets;

    // for buffer address mapping 
    uint32_t shift2seg;
    uint32_t shift2set;

    uint32_t maskOfset;
    uint32_t maskOfseg;
};

typedef list<Block>::iterator itBlock;

//=====================================================================================
// BufferSet class definition
//
class BufferSet
{
public:
    // Initial parameters passed from system.ini file
    BufferSet();

    // read (tag) - Read the value (not actually read) of the tag if the tag 
    // exists in the block, and update its location according buffer policy.
    void read(unsigned tag);

    // write (tag) - Write data from row buffer addressed by tag (row and
    // segments) to buffer block, and update its location according policy.
    void write(unsigned tag);

    // fetch (tag) - Fetch data from row to buffer block. Initialize to
    // BLOCK_VALID state.
    void fetch(unsigned tag);

    // restore () - Restore block back to bank row.
    void restore();

    // isFetchSafe (tag) - FETCH iussing safely check. If back of buffer block
    // is Modified, get its tag for restoreRow, otherwise return ture.
    bool isFetchSafe(unsigned &tag);

    // probe (tag) - Check if the data of the tag exists in the Buffer,
    // otherwise return false;
    bool probe(unsigned tag);

    size_t size() const { return BUFFER_WAY_COUNT;}

    void print(unsigned maskOfseg);

private:
    // updateWayList (tag) - Update the block position of tag in the buffer list
    // according buffer policy.
    void updateWayList(unsigned tag);

    list<Block> bufferlist;
    unordered_map<int , list<Block>::iterator> hash;
};

}

#endif /* VICTIM_BUFFER_H */
