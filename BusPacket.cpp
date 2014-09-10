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








//BusPacket.cpp
//
//Class file for bus packet object
//

#include "BusPacket.h"

using namespace DRAMSim;
using namespace std;

BusPacket::BusPacket(BusPacketType packtype, uint64_t physicalAddr, 
		unsigned col, unsigned rw, unsigned r, unsigned b, void *dat, 
		ostream &dramsim_log_) :
	dramsim_log(dramsim_log_),
	busPacketType(packtype),
	column(col),
	row(rw),
	bank(b),
	rank(r),
	physicalAddress(physicalAddr),
	data(dat)
{}

void BusPacket::print(uint64_t currentClockCycle, bool dataStart)
{
	if (this == NULL)
	{
		return;
	}

	if (dataStart)
	{
		cmd_verify_out << currentClockCycle << ": error! data bus collapse!"<<endl;
	}
	if (VERIFICATION_OUTPUT)
	{
		switch (busPacketType)
		{
		case READ:
			cmd_verify_out << currentClockCycle << ": read (0x"<<hex<<physicalAddress<<dec<<","<<rank<<","<<bank<<","<<row<<","<<column<<",0);"<<endl;
			break;
		case READ_P:
			cmd_verify_out << currentClockCycle << ": read ("<<rank<<","<<bank<<","<<column<<",1);"<<endl;
			break;
#ifdef VICTIMBUFFER
		case READ_B:
			cmd_verify_out << currentClockCycle << ": read_b (0x"<<hex<<physicalAddress<<dec<<","<<rank<<","<<bank<<","<<row<<","<<column<<",2);"<<endl;
			break;
		case RESTORE:
			cmd_verify_out << currentClockCycle << ": restore (0x"<<hex<<physicalAddress<<dec<<","<<rank<<","<<bank<<","<<row<<","<<column<<",2);"<<endl;
			break;
		case FETCH:
			cmd_verify_out << currentClockCycle << ": fetch (0x"<<hex<<physicalAddress<<dec<<","<<rank<<","<<bank<<","<<row<<","<<column<<",2);"<<endl;
			break;
		case WRITE_B:
			cmd_verify_out << currentClockCycle << ": write_b (0x"<<hex<<physicalAddress<<dec<<","<<rank<<","<<bank<<","<<column<<",0 , 0, 'h0);"<<endl;
			break;
#endif
		case WRITE:
			cmd_verify_out << currentClockCycle << ": write (0x"<<hex<<physicalAddress<<dec<<","<<rank<<","<<bank<<","<<column<<",0 , 0, 'h0);"<<endl;
			break;
		case WRITE_P:
			cmd_verify_out << currentClockCycle << ": write ("<<rank<<","<<bank<<","<<column<<",1, 0, 'h0);"<<endl;
			break;
		case ACTIVATE:
			cmd_verify_out << currentClockCycle <<": activate (" << rank << "," << bank << "," << row <<");"<<endl;
			break;
		case PRECHARGE:
			cmd_verify_out << currentClockCycle <<": precharge (" << rank << "," << bank << "," << row <<");"<<endl;
			break;
		case REFRESH:
			cmd_verify_out << currentClockCycle <<": refresh (" << rank << ");"<<endl;
			break;
		case DATA:
			//TODO: data verification?
			cmd_verify_out << currentClockCycle << ": date (0x"<<hex<<physicalAddress<<dec<<","<<rank<<","<<bank<<","<<row<<","<<column<<");"<<endl;
			break;
		default:
			ERROR("Trying to print unknown kind of bus packet");
			exit(-1);
		}
	}
}
void BusPacket::print()
{
	if (this == NULL) //pointer use makes this a necessary precaution
	{
		return;
	}
	else
	{
		switch (busPacketType)
		{
		case READ:
			DEBUG("BP [READ] pa[0x"<<hex<<physicalAddress<<dec<<"] r["<<rank<<"] b["<<bank<<"] row["<<row<<"] col["<<column<<"]");
			break;
		case READ_P:
			DEBUG("BP [READ_P] pa[0x"<<hex<<physicalAddress<<dec<<"] r["<<rank<<"] b["<<bank<<"] row["<<row<<"] col["<<column<<"]");
			break;
		case WRITE:
			DEBUG("BP [WRITE] pa[0x"<<hex<<physicalAddress<<dec<<"] r["<<rank<<"] b["<<bank<<"] row["<<row<<"] col["<<column<<"]");
			break;
		case WRITE_P:
			DEBUG("BP [WRITE_P] pa[0x"<<hex<<physicalAddress<<dec<<"] r["<<rank<<"] b["<<bank<<"] row["<<row<<"] col["<<column<<"]");
			break;
		case ACTIVATE:
			DEBUG("BP [ACT] pa[0x"<<hex<<physicalAddress<<dec<<"] r["<<rank<<"] b["<<bank<<"] row["<<row<<"] col["<<column<<"]");
			break;
		case PRECHARGE:
			DEBUG("BP [PRE] pa[0x"<<hex<<physicalAddress<<dec<<"] r["<<rank<<"] b["<<bank<<"] row["<<row<<"] col["<<column<<"]");
			break;
		case REFRESH:
			DEBUG("BP [REF] pa[0x"<<hex<<physicalAddress<<dec<<"] r["<<rank<<"] b["<<bank<<"] row["<<row<<"] col["<<column<<"]");
			break;
		case DATA:
			DEBUG("BP [DATA] pa[0x"<<hex<<physicalAddress<<dec<<"] r["<<rank<<"] b["<<bank<<"] row["<<row<<"] col["<<column<<"] data["<<data<<"]=");
			printData();
			DEBUG("");
			break;
#ifdef VICTIMBUFFER
		case READ_B:
			DEBUG("BP [READ_B] pa[0x"<<hex<<physicalAddress<<dec<<"] r["<<rank<<"] b["<<bank<<"] row["<<row<<"] col["<<column<<"]");
			break;
		case WRITE_B:
			DEBUG("BP [WRITE_B] pa[0x"<<hex<<physicalAddress<<dec<<"] r["<<rank<<"] b["<<bank<<"] row["<<row<<"] col["<<column<<"]");
			break;
		case RESTORE:
			DEBUG("BP [RSTORE] pa[0x"<<hex<<physicalAddress<<dec<<"] r["<<rank<<"] b["<<bank<<"] row["<<row<<"] col["<<column<<"]");
			break;
		case FETCH:
			DEBUG("BP [FETCH] pa[0x"<<hex<<physicalAddress<<dec<<"] r["<<rank<<"] b["<<bank<<"] row["<<row<<"] col["<<column<<"]");
			break;
#endif
		default:
			ERROR("Trying to print unknown kind of bus packet");
			exit(-1);
		}
	}
}

void BusPacket::printData() const 
{
	if (data == NULL)
	{
		DEBUGN("NO DATA");
		return;
	}
	DEBUGN("'" << hex);
	for (int i=0; i < 4; i++)
	{
		DEBUGN(((uint64_t *)data)[i]);
	}
	DEBUGN("'" << dec);
}
