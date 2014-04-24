/*********************************************************************************
*  Copyright (c) 2010-2011, Elliott Cooper-Balis
*                             Paul Rosenfeld
*                             Bruce Jacob
*                             University of Maryland 
*                             dramninjas [at] gmail [dot] com
*  All rights reserved.
*  
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions are met:
*  
*     * Redistributions of source code must retain the above copyright notice,
*        this list of conditions and the following disclaimer.
*  
*     * Redistributions in binary form must reproduce the above copyright notice,
*        this list of conditions and the following disclaimer in the documentation
*        and/or other materials provided with the distribution.
*  
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
*  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
*  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
*  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
*  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
*  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
*  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
*  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
*  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*********************************************************************************/
#include "SystemConfiguration.h"
#include "AddressMapping.h"

namespace DRAMSim
{

void addressMapping(uint64_t physicalAddress, unsigned &newTransactionChan, unsigned &newTransactionRank, unsigned &newTransactionBank, unsigned &newTransactionRow, unsigned &newTransactionColumn)
{
	uint64_t tempA, tempB;
	unsigned transactionSize = (JEDEC_DATA_BUS_BITS/8)*BL; 
	uint64_t transactionMask =  transactionSize - 1; //ex: (64 bit bus width) x (8 Burst Length) - 1 = 64 bytes - 1 = 63 = 0x3f mask
	unsigned channelBitWidth = dramsim_log2(NUM_CHANS);
	unsigned	rankBitWidth = dramsim_log2(NUM_RANKS);
	unsigned	bankBitWidth = dramsim_log2(NUM_BANKS);
	unsigned	rowBitWidth = dramsim_log2(NUM_ROWS);
	unsigned	colBitWidth = dramsim_log2(NUM_COLS);
	// this forces the alignment to the width of a single burst (64 bits = 8 bytes = 3 address bits for DDR parts)
	unsigned	byteOffsetWidth = dramsim_log2((JEDEC_DATA_BUS_BITS/8));
	// Since we're assuming that a request is for BL*BUS_WIDTH, the bottom bits
	// of this address *should* be all zeros if it's not, issue a warning

	if ((physicalAddress & transactionMask) != 0)
	{
		DEBUG("WARNING: address 0x"<<std::hex<<physicalAddress<<std::dec<<" is not aligned to the request size of "<<transactionSize); 
	}

	// each burst will contain JEDEC_DATA_BUS_BITS/8 bytes of data, so the bottom bits (3 bits for a single channel DDR system) are
	// 	thrown away before mapping the other bits
	physicalAddress >>= byteOffsetWidth;

	// The next thing we have to consider is that when a request is made for a
	// we've taken into account the granulaity of a single burst by shifting 
	// off the bottom 3 bits, but a transaction has to take into account the
	// burst length (i.e. the requests will be aligned to cache line sizes which
	// should be equal to transactionSize above). 
	//
	// Since the column address increments internally on bursts, the bottom n 
	// bits of the column (colLow) have to be zero in order to account for the 
	// total size of the transaction. These n bits should be shifted off the 
	// address and also subtracted from the total column width. 
	//
	// I am having a hard time explaining the reasoning here, but it comes down
	// this: for a 64 byte transaction, the bottom 6 bits of the address must be 
	// zero. These zero bits must be made up of the byte offset (3 bits) and also
	// from the bottom bits of the column 
	// 
	// For example: cowLowBits = log2(64bytes) - 3 bits = 3 bits 
	unsigned colLowBitWidth = dramsim_log2(transactionSize) - byteOffsetWidth;

	physicalAddress >>= colLowBitWidth;
	unsigned colHighBitWidth = colBitWidth - colLowBitWidth; 
	if (DEBUG_ADDR_MAP)
	{
		DEBUG("Bit widths: ch:"<<channelBitWidth<<" r:"<<rankBitWidth<<" b:"<<bankBitWidth
				<<" row:"<<rowBitWidth<<" colLow:"<<colLowBitWidth
				<< " colHigh:"<<colHighBitWidth<<" off:"<<byteOffsetWidth 
				<< " Total:"<< (channelBitWidth + rankBitWidth + bankBitWidth + rowBitWidth + colLowBitWidth + colHighBitWidth + byteOffsetWidth));
	}

	//perform various address mapping schemes
	if (addressMappingScheme == Scheme1)
	{
		//chan:rank:row:col:bank
		tempA = physicalAddress;
		physicalAddress = physicalAddress >> bankBitWidth;
		tempB = physicalAddress << bankBitWidth;
		newTransactionBank = tempA ^ tempB;

		tempA = physicalAddress;
		physicalAddress = physicalAddress >> colHighBitWidth;
		tempB = physicalAddress << colHighBitWidth;
		newTransactionColumn = tempA ^ tempB;

		tempA = physicalAddress;
		physicalAddress = physicalAddress >> rowBitWidth;
		tempB = physicalAddress << rowBitWidth;
		newTransactionRow = tempA ^ tempB;

		tempA = physicalAddress;
		physicalAddress = physicalAddress >> rankBitWidth;
		tempB = physicalAddress << rankBitWidth;
		newTransactionRank = tempA ^ tempB;

		tempA = physicalAddress;
		physicalAddress = physicalAddress >> channelBitWidth;
		tempB = physicalAddress << channelBitWidth;
		newTransactionChan = tempA ^ tempB;

	}
	else if (addressMappingScheme == Scheme2)
	{
		//chan:row:col:bank:rank
		tempA = physicalAddress;
		physicalAddress = physicalAddress >> rankBitWidth;
		tempB = physicalAddress << rankBitWidth;
		newTransactionRank = tempA ^ tempB;

		tempA = physicalAddress;
		physicalAddress = physicalAddress >> bankBitWidth;
		tempB = physicalAddress << bankBitWidth;
		newTransactionBank = tempA ^ tempB;

		tempA = physicalAddress;
		physicalAddress = physicalAddress >> colHighBitWidth;
		tempB = physicalAddress << colHighBitWidth;
		newTransactionColumn = tempA ^ tempB;

		tempA = physicalAddress;
		physicalAddress = physicalAddress >> rowBitWidth;
		tempB = physicalAddress << rowBitWidth;
		newTransactionRow = tempA ^ tempB;

		tempA = physicalAddress;
		physicalAddress = physicalAddress >> channelBitWidth;
		tempB = physicalAddress << channelBitWidth;
		newTransactionChan = tempA ^ tempB;

	}
	else if (addressMappingScheme == Scheme3)
	{
		//chan:rank:bank:col:row
		tempA = physicalAddress;
		physicalAddress = physicalAddress >> rowBitWidth;
		tempB = physicalAddress << rowBitWidth;
		newTransactionRow = tempA ^ tempB;

		tempA = physicalAddress;
		physicalAddress = physicalAddress >> colHighBitWidth;
		tempB = physicalAddress << colHighBitWidth;
		newTransactionColumn = tempA ^ tempB;

		tempA = physicalAddress;
		physicalAddress = physicalAddress >> bankBitWidth;
		tempB = physicalAddress << bankBitWidth;
		newTransactionBank = tempA ^ tempB;

		tempA = physicalAddress;
		physicalAddress = physicalAddress >> rankBitWidth;
		tempB = physicalAddress << rankBitWidth;
		newTransactionRank = tempA ^ tempB;

		tempA = physicalAddress;
		physicalAddress = physicalAddress >> channelBitWidth;
		tempB = physicalAddress << channelBitWidth;
		newTransactionChan = tempA ^ tempB;

	}
	else if (addressMappingScheme == Scheme4)
	{
		//chan:rank:bank:row:col
		tempA = physicalAddress;
		physicalAddress = physicalAddress >> colHighBitWidth;
		tempB = physicalAddress << colHighBitWidth;
		newTransactionColumn = tempA ^ tempB;

		tempA = physicalAddress;
		physicalAddress = physicalAddress >> rowBitWidth;
		tempB = physicalAddress << rowBitWidth;
		newTransactionRow = tempA ^ tempB;

		tempA = physicalAddress;
		physicalAddress = physicalAddress >> bankBitWidth;
		tempB = physicalAddress << bankBitWidth;
		newTransactionBank = tempA ^ tempB;

		tempA = physicalAddress;
		physicalAddress = physicalAddress >> rankBitWidth;
		tempB = physicalAddress << rankBitWidth;
		newTransactionRank = tempA ^ tempB;

		tempA = physicalAddress;
		physicalAddress = physicalAddress >> channelBitWidth;
		tempB = physicalAddress << channelBitWidth;
		newTransactionChan = tempA ^ tempB;

	}
	else if (addressMappingScheme == Scheme5)
	{
		//chan:row:col:rank:bank

		tempA = physicalAddress;
		physicalAddress = physicalAddress >> bankBitWidth;
		tempB = physicalAddress << bankBitWidth;
		newTransactionBank = tempA ^ tempB;

		tempA = physicalAddress;
		physicalAddress = physicalAddress >> rankBitWidth;
		tempB = physicalAddress << rankBitWidth;
		newTransactionRank = tempA ^ tempB;

		tempA = physicalAddress;
		physicalAddress = physicalAddress >> colHighBitWidth;
		tempB = physicalAddress << colHighBitWidth;
		newTransactionColumn = tempA ^ tempB;

		tempA = physicalAddress;
		physicalAddress = physicalAddress >> rowBitWidth;
		tempB = physicalAddress << rowBitWidth;
		newTransactionRow = tempA ^ tempB;

		tempA = physicalAddress;
		physicalAddress = physicalAddress >> channelBitWidth;
		tempB = physicalAddress << channelBitWidth;
		newTransactionChan = tempA ^ tempB;


	}
	else if (addressMappingScheme == Scheme6)
	{
		//chan:row:bank:rank:col

		tempA = physicalAddress;
		physicalAddress = physicalAddress >> colHighBitWidth;
		tempB = physicalAddress << colHighBitWidth;
		newTransactionColumn = tempA ^ tempB;

		tempA = physicalAddress;
		physicalAddress = physicalAddress >> rankBitWidth;
		tempB = physicalAddress << rankBitWidth;
		newTransactionRank = tempA ^ tempB;

		tempA = physicalAddress;
		physicalAddress = physicalAddress >> bankBitWidth;
		tempB = physicalAddress << bankBitWidth;
		newTransactionBank = tempA ^ tempB;

		tempA = physicalAddress;
		physicalAddress = physicalAddress >> rowBitWidth;
		tempB = physicalAddress << rowBitWidth;
		newTransactionRow = tempA ^ tempB;

		tempA = physicalAddress;
		physicalAddress = physicalAddress >> channelBitWidth;
		tempB = physicalAddress << channelBitWidth;
		newTransactionChan = tempA ^ tempB;


	}
	// clone of scheme 5, but channel moved to lower bits
	else if (addressMappingScheme == Scheme7)
	{
		//row:col:rank:bank:chan
		tempA = physicalAddress;
		physicalAddress = physicalAddress >> channelBitWidth;
		tempB = physicalAddress << channelBitWidth;
		newTransactionChan = tempA ^ tempB;

		tempA = physicalAddress;
		physicalAddress = physicalAddress >> bankBitWidth;
		tempB = physicalAddress << bankBitWidth;
		newTransactionBank = tempA ^ tempB;

		tempA = physicalAddress;
		physicalAddress = physicalAddress >> rankBitWidth;
		tempB = physicalAddress << rankBitWidth;
		newTransactionRank = tempA ^ tempB;

		tempA = physicalAddress;
		physicalAddress = physicalAddress >> colHighBitWidth;
		tempB = physicalAddress << colHighBitWidth;
		newTransactionColumn = tempA ^ tempB;

		tempA = physicalAddress;
		physicalAddress = physicalAddress >> rowBitWidth;
		tempB = physicalAddress << rowBitWidth;
		newTransactionRow = tempA ^ tempB;

	}
	// a real Nehalem CPU address mapping
	else if (addressMappingScheme == Nehalem)
	{
/* 
 *        |         row remain                    | b2    |   row[3:0]    | c1| b1| rank  |          Col[8:3]     |ch |colLowBits |ChipOffset |
 *   -+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 *    | 33| 32| 31| 30| 29| 28| 27| 26| 25| 24| 23| 22| 21| 20| 19| 18| 17| 16| 15| 14| 13| 12| 11| 10|  9|  8|  7|  6|  5|  4|  3|  2|  1|  0|
 *   -+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 */
		//chan:rank:bank:row:col
		unsigned col6;
		unsigned col1;
		unsigned col_remain;
		unsigned bit16;
		
		tempA = physicalAddress;
		bit16 = (physicalAddress & 0x400) >> 10;
		physicalAddress = physicalAddress >> channelBitWidth;
		tempB = physicalAddress << channelBitWidth;
		// FIXME: when NUM_CHANS = 1, chan > 0 occurs
//		newTransactionChan = (tempA ^ tempB) ^ bit16;	// need to xor bit 16
		newTransactionChan = (tempA ^ tempB);

		tempA = physicalAddress;
		physicalAddress = physicalAddress >> 6;
		tempB = physicalAddress << 6;
		col6 = tempA ^ tempB;

		tempA = physicalAddress;
		physicalAddress = physicalAddress >> rankBitWidth;
		tempB = physicalAddress << rankBitWidth;
		newTransactionRank = tempA ^ tempB;

		unsigned b1;
		unsigned b2;

		tempA = physicalAddress;
		physicalAddress = physicalAddress >> 1;
		tempB = physicalAddress << 1;
		b1 = tempA ^ tempB;

		tempA = physicalAddress;
		physicalAddress = physicalAddress >> 1;
		tempB = physicalAddress << 1;
		col1 = tempA ^ tempB;

		unsigned row4;
		unsigned row_remain;

		tempA = physicalAddress;
		physicalAddress = physicalAddress >> 4;
		tempB = physicalAddress << 4;
		row4 = tempA ^ tempB;

		tempA = physicalAddress; // need to change b_remain instead of 2
		physicalAddress = physicalAddress >> (bankBitWidth - 1);
		tempB = physicalAddress << (bankBitWidth - 1);
		b2 = tempA ^ tempB;

		tempA = physicalAddress;
		physicalAddress = physicalAddress >> (rowBitWidth - 4);
		tempB = physicalAddress << (rowBitWidth - 4);
		row_remain = tempA ^ tempB;

		tempA = physicalAddress;
		physicalAddress = physicalAddress >> (colHighBitWidth - 7);
		tempB = physicalAddress << (colHighBitWidth - 7);
		col_remain = tempA ^ tempB;

		newTransactionBank = b1 + (b2 << 1) ;
		newTransactionColumn = col6 + (col1 << 6) + (col_remain << 7);
		newTransactionRow = row4 +  (row_remain << 4);

	}
	else
	{
		ERROR("== Error - Unknown Address Mapping Scheme");
		exit(-1);
	}
	if (DEBUG_ADDR_MAP)
	{
		DEBUG("Mapped Ch="<<newTransactionChan<<" Rank="<<newTransactionRank
				<<" Bank="<<newTransactionBank<<" Row="<<newTransactionRow
				<<" Col="<<newTransactionColumn<<"\n"); 
	}

}
};
