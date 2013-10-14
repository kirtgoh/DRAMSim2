/*********************************************************************************
*********************************************************************************/

#ifndef TRANSACTIONQUEUE_H
#define TRANSACTIONQUEUE_H

#include<vector>
#include<stdint.h>
#include "Transaction.h"
//TransactionQueue.h
//
//Header file for transaction queue object

extern unsigned TRANS_QUEUE_DEPTH;
extern unsigned NUM_RANKS;
extern unsigned NUM_BANKS;


namespace DRAMSim
{
using namespace std;

class TransactionQueue
{
	// trans 's item in the index table
	typedef struct _Item
	{
		unsigned Row;
		uint64_t Cycle;		// insert cycle
	} Item;

public:
	TransactionQueue();
	unsigned size(){return m_used;};
	void insertTrans(Transaction * trans);
	Transaction* popTrans(unsigned rank, unsigned bank, unsigned pos, bool& isHit);
	unsigned find_first_of(unsigned rank, unsigned bank);

	Item* ***index;		//3D array of Item pointers
	unsigned getDepth(){ return TRANS_QUEUE_DEPTH;};

	unsigned m_nLastBank;	// round-robin
	unsigned m_nLastRank;	// round-robin
	vector<Transaction *> m_vecpTransQueue;
private:

	unsigned m_used;

};

}

#endif

