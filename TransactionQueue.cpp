#include "TransactionQueue.h"
#include "AddressMapping.h"

using namespace DRAMSim;

TransactionQueue::TransactionQueue()
{
	index = new Item* **[NUM_RANKS];
	for (size_t r=0; r < NUM_RANKS; r++)
	{
		index[r] = new Item* *[NUM_BANKS];
		for ( size_t b = 0; b < NUM_BANKS; b++) 
			index[r][b] = new Item*[TRANS_QUEUE_DEPTH + 1];
	}

    for (size_t r=0; r < NUM_RANKS; r++)
    {   
        for ( size_t b = 0; b < NUM_BANKS; b++) 
            for ( size_t l = 0; l <= TRANS_QUEUE_DEPTH; l++) 
            index[r][b][l] = 0;
    } 

	m_used = 0;
	m_vecpTransQueue = vector<Transaction *>(TRANS_QUEUE_DEPTH + 1);
	
	m_nLastRank = -1;
	m_nLastBank = -1;
}


void TransactionQueue::insertTrans(Transaction * trans)
{
    unsigned chan, rank, bank, row ,col;

	// used to init index item
    addressMapping(trans->address, chan, rank, bank, row, col);

	unsigned i = 1;
	for (; i <= TRANS_QUEUE_DEPTH; i++)
		if (m_vecpTransQueue[i] == 0)
		{
			m_vecpTransQueue[i] = trans;
			break;
		}

	m_used++;

//	trans->timeAdded = time;

	Item * it = new Item();
	it->Row = row;
	it->Cycle = trans->timeAdded;

	index[rank][bank][i] = it;

}

/*
unsigned TransactionQueue::find_first_of(unsigned rank, unsigned bank)
{
	uint64_t posLeastCycle = 0;
	uint64_t posRowLeastCycle = 0;	//same row , lestCycle
	Item* check;
	Item* lastPop = index[rank][bank][0];		//last pop one 
	
	unsigned i;
	for (i = 1; i <= TRANS_QUEUE_DEPTH; i++)
	{
		check = index[rank][bank][i];
		if (check)
		{
			// need to judge wheather row is the same 
			if(lastPop) {
				if(check->Row == lastPop->Row) {
					if(posRowLeastCycle)    
						posRowLeastCycle = (check->Cycle < index[rank][bank][posRowLeastCycle]->Cycle) ? i : posRowLeastCycle;
					else
						posRowLeastCycle = i;
				}
			}

			if(posLeastCycle) 
			    posLeastCycle = (check->Cycle < index[rank][bank][posLeastCycle]->Cycle) ? i : posLeastCycle;
			else
				posLeastCycle = i; 
		}
		
	}

	if (posRowLeastCycle)
		return posRowLeastCycle;
	else if (posLeastCycle)
		return posLeastCycle;

	return 0;
}
*/
// no schedule
unsigned TransactionQueue::find_first_of(unsigned rank, unsigned bank)
{
	uint64_t posLeastCycle = 0;
	//uint64_t posRowLeastCycle = 0;	//same row , lestCycle
	Item* check;
	//Item* lastPop = index[rank][bank][0];		//last pop one 
	
	unsigned i;
	for (i = 1; i <= TRANS_QUEUE_DEPTH; i++)
	{
		check = index[rank][bank][i];
		if (check)
		{
			if(posLeastCycle) 
			    posLeastCycle = (check->Cycle < index[rank][bank][posLeastCycle]->Cycle) ? i : posLeastCycle;
			else
				posLeastCycle = i; 
		}
		
	}

	if (posLeastCycle)
		return posLeastCycle;

	return 0;
}

/* poped , need to change m_vec and index */
Transaction* TransactionQueue::popTrans(unsigned rank, unsigned bank, unsigned pos, bool& isHit)
{
	if (index[rank][bank][0])
	{
		// for row buffer hit rate statistic
		if (index[rank][bank][0]->Row == index[rank][bank][pos]->Row)
			isHit = true;
		else
			isHit = false;

		delete index[rank][bank][0];
	}

	index[rank][bank][0] = index[rank][bank][pos];
	index[rank][bank][pos] = 0;

	m_used--;

	Transaction *p = m_vecpTransQueue[pos];
	m_vecpTransQueue[pos] = 0;
	return p;
	
}

/*
memory:update()
{
	if (transQueue.size())
	{
		//from last travel position, travel a cycle
		for (r = 0; r  ;r++)
			for(b = 0; b < m_nLastBank; b++)
			{
				pos = b + m_nLastBank % len
				if (found) , hasroom : pop 
					break;
				else 
					next q

	}
}*/
