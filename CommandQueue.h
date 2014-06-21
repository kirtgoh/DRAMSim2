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






#ifndef CMDQUEUE_H
#define CMDQUEUE_H

//CommandQueue.h
//
//Header
//

#include "BusPacket.h"
#include "BankState.h"
#include "Transaction.h"
#include "SystemConfiguration.h"
#include "SimulatorObject.h"

#ifdef VICTIMBUFFER
#include "Buffer.h"
#endif

using namespace std;

namespace DRAMSim
{
class CommandQueue : public SimulatorObject
{
	CommandQueue();
	ostream &dramsim_log;
public:
	//typedefs
	typedef vector<BusPacket *> BusPacket1D;
	typedef vector<BusPacket1D> BusPacket2D;
	typedef vector<BusPacket2D> BusPacket3D;

	//functions
#ifdef VICTIMBUFFER
	CommandQueue(vector< vector<BankState> > &states, vector< vector<Buffer> > &buffers, ostream &dramsim_log);
	bool isIssuableVRB(BusPacket *busPacket);
#else
	CommandQueue(vector< vector<BankState> > &states, ostream &dramsim_log);
#endif
	virtual ~CommandQueue(); 

	void enqueue(BusPacket *newBusPacket);
	bool pop(BusPacket **busPacket);
	bool hasRoomFor(unsigned numberToEnqueue, unsigned rank, unsigned bank);
	bool isIssuable(BusPacket *busPacket);
	bool isEmpty(unsigned rank);
	void needRefresh(unsigned rank);
	void print();
	//MOD: kgoh for cmdq verfiy Wed 18 Jun 2014 08:39:22 PM CST
	void print(uint64_t currentClockCycle);
	void needFetch(unsigned rank, unsigned bank);
	void needRestore(unsigned rank, unsigned bank, unsigned col);
	//END_MOD
	void update(); //SimulatorObject requirement
	vector<BusPacket *> &getCommandQueue(unsigned rank, unsigned bank);

	//fields
	
	BusPacket3D queues; // 3D array of BusPacket pointers
	vector< vector<BankState> > &bankStates;

#ifdef VICTIMBUFFER
	vector < vector<Buffer> > &bankBuffers;
#endif

private:
	void nextRankAndBank(unsigned &rank, unsigned &bank);
	//fields
	unsigned nextBank;
	unsigned nextRank;

	unsigned nextBankPRE;
	unsigned nextRankPRE;

	unsigned refreshRank;
	bool refreshWaiting;

	//kgoh
	unsigned fetchRank;
	unsigned fetchBank;
	bool fetchWaiting;

	unsigned restoreRank;
	unsigned restoreBank;
	bool restoreWaiting;
	//end

	vector< vector<unsigned> > tFAWCountdown;
	vector< vector<unsigned> > rowAccessCounters;

	bool sendAct;
};
}

#endif

