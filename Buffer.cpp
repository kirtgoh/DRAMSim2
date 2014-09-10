//=====================================================================================
// Buffer.cpp
//
// Copyright (C) 2014 Kirt Goh <kirtgoh@gmail.com>
//=====================================================================================

#include <Buffer.h>
#include <PrintMacros.h>

using namespace DRAMSim;

void Block::print(unsigned maskOfseg)
{
    unsigned shiftBits = dramsim_log2(maskOfseg + 1);
    unsigned row, seg;

    row = tag >> shiftBits;
    seg = tag & maskOfseg;
    switch(state)
    {
    case BLOCK_VALID:
        DEBUGN("[" << row << ":" << seg << "]" );
        // DEBUGN("[" << tag << "]" );
        break;
    case BLOCK_INVALID:
        DEBUGN("[inva]");
        break;
    case BLOCK_MODIFIED:
        DEBUGN("[" << row << ":" << seg << "*]" );
        // DEBUGN("[" << tag << "*]" );
        break;
    case BLOCK_VALID | BLOCK_MODIFIED:
        DEBUGN("[" << row << ":" << seg << "**]" );
        // DEBUGN("[" << tag << "**]" );
        break;
    default:
        ERROR("Trying to print unkown kind of block");
        exit(-1);
	}
}

//=====================================================================================
// Buffer methods
//
Buffer::Buffer(ostream &dramsim_log_):
    dramsim_log(dramsim_log_),
    nextRead(0),
    nextWrite(0)
{
	// Block size must be at least one data bus burst length, i.e., 64 bytes
	if (BUFFER_BLOCK_SIZE < 64 || !isPowerOfTwo(BUFFER_BLOCK_SIZE)) {
		ERROR("Buffer Block size "<< BUFFER_BLOCK_SIZE <<" config error");
		abort();
	}

	// Ways of buffer must be a power of two
	if (!isPowerOfTwo(BUFFER_WAY_COUNT)) {
		ERROR("Buffer Ways number " << BUFFER_WAY_COUNT << " config error");
		abort();
	}

    size_t numOfsets = BUFFER_STORAGE / (BUFFER_WAY_COUNT * BUFFER_BLOCK_SIZE);

	if (!isPowerOfTwo(numOfsets)) {
		ERROR("Buffer Sets number "<< numOfsets <<" config error");
		abort();
	}

    sets = vector<BufferSet>(numOfsets);

    // Since the column address increments internally on bursts, the bottom n
    // bits of the column (colLow) have to be zero in order to account
    // for the total size of the transaction. These n bits should be shifted
    // off the address and also subtracted from the total column width. 
    //
    // I am having a hard time explaining the reasoning here, but it comes down
    // this: for a 64 byte transaction, the bottom 6 bits of the address must be 
    // zero. These zero bits must be made up of the byte offset (3 bits) and also
    // from the bottom bits of the column 
    //
    // For example: cowLowBits = log2(64bytes) - 3 bits = 3 bits
    //
    shift2set = dramsim_log2(BUFFER_BLOCK_SIZE / 64);
    shift2seg = shift2set + dramsim_log2(numOfsets);

    maskOfset = numOfsets - 1;
	maskOfseg = (1 << (dramsim_log2(NUM_COLS) - 3 - shift2seg)) -1;
}

void Buffer::bufferAccess(BusPacket *packet)
{
  	unsigned tag, set;
    decodeAddress(packet->row, packet->column, tag, set);

	switch(packet->busPacketType)
	{
		case FETCH:
            // make sure data not in there
            if(sets[set].probe(tag))
            {
                ERROR("Fetch duplicated!");
                exit(-1);
            }
            sets[set].fetch(tag);
			break;
        case READ_B:
            // make sure data in there
            if(!sets[set].probe(tag))
            {
                ERROR("buffer does not exits requesting data!");
                exit(-1);
            }

            sets[set].read(tag);
            break;
		case WRITE_B:
            // make sure data in there
			if (!sets[set].probe(tag))
			{
				ERROR("there is no hit buffer, when a write_b issued");
				exit(-1);
			}
			
            sets[set].write(tag);
			break;
		case RESTORE:
            // inner check restored block is in modified state
            sets[set].restore(); 
			break;
		default:
			ERROR("Unknown buffer access command!");
	}
}

bool Buffer::probe(unsigned row,  unsigned col)
{
    unsigned tag, set;
    decodeAddress(row, col, tag, set);

    if (sets[set].probe(tag))
        return true;

    return false;
}

bool Buffer::isFetchSafe(unsigned fetchRow, unsigned fetchCol, unsigned &restoreRow)
{
    unsigned tag, set;
    decodeAddress(fetchRow, fetchCol, tag, set);

    // reuse tag to get restore row address 
    if(!sets[set].isFetchSafe(tag))
    {
        unsigned shiftBits = dramsim_log2(maskOfseg + 1);
        restoreRow = tag >> shiftBits;

        return false;
    }

    return true;
}

void Buffer::print()
{
    for ( size_t i = 0; i < sets.size(); ++i )
    {
        sets[i].print(maskOfseg);
    }
}

void Buffer::decodeAddress(unsigned row, unsigned col, unsigned &tag, unsigned &set)
{
    // segment address bits 
    unsigned shiftBits = dramsim_log2(maskOfseg + 1);

    // get segment address
    unsigned seg = (col >> shift2seg);

    // get set address
    set = (col >> shift2set) & maskOfset;

    // shift and add segment address
    tag = (row << shiftBits) | seg;

}

//=====================================================================================
// BufferSet methods
//
BufferSet::BufferSet():
    bufferlist(BUFFER_WAY_COUNT,Block(-1))
{
}

void BufferSet::read(unsigned tag)
{
    updateWayList(tag);
}

void BufferSet::write(unsigned tag)
{
    updateWayList(tag);
    hash[tag]->state |= BLOCK_MODIFIED;
}

void BufferSet::fetch(unsigned tag)
{
    if (BUFFER_WAY_COUNT == bufferlist.size())
    {
        if (bufferlist.back().state & BLOCK_MODIFIED)
        {
            ERROR("Fetch unrestored block!");
            exit(-1);
        }

        hash.erase(bufferlist.back().tag);
        bufferlist.pop_back();
    }
    bufferlist.push_front(Block(tag));
    hash[tag] = bufferlist.begin();
    hash[tag]->state = BLOCK_VALID;
}

void BufferSet::restore()
{
    if (!(bufferlist.back().state & BLOCK_MODIFIED))
    {
        ERROR("Restore unmodified block!");
        exit(-1);
    }

    bufferlist.back().state = BLOCK_VALID;
}

bool BufferSet::isFetchSafe(unsigned &tag)
{
    if (bufferlist.back().state & BLOCK_MODIFIED)
    {
        tag = bufferlist.back().tag;
        return false;
    }

    return true;
}

bool BufferSet::probe(unsigned tag)
{
    if (hash.find(tag) != hash.end())
    {
        return true;
    }

    return false;
}

void BufferSet::print(unsigned maskOfseg)
{
    // print bufferlist
    list<Block>::iterator itr = bufferlist.begin();
    while(itr != bufferlist.end())
    {
        itr->print(maskOfseg);
        itr++;
	    DEBUGN(" ");
    }
	DEBUG("");
}

void BufferSet::updateWayList(uint32_t tag)
{
    switch(bufferPolicy)
    {
        case FIFO:
        case LRU:
            bufferlist.splice(bufferlist.begin(), bufferlist, hash[tag]);
            break;
        case RANDOM:
            // TODO: do something 
            break;
        default:
            ERROR("Unknown Buffer policy!");
    }

    hash[tag] = bufferlist.begin();
}
