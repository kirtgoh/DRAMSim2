#ifndef ROWBUFFERBUFFER_H
#define ROWBUFFERBUFFER_H

#include "SystemConfiguration.h"
#include "BusPacket.h"

namespace DRAMSim
{
enum CurrentBufferState
{
	Invalid,
	Valid,
	Modified
};

class RowBufferBuffer
{
	ostream &dramsim_log;
public:
	//Fields
	CurrentBufferState currentBufferState;
	unsigned openRowAddress;
	uint64_t nextRead;
	uint64_t nextWrite;

	BusPacketType lastCommand;
	unsigned stateChangeCountdown;

	//Functions
	RowBufferBuffer(ostream &dramsim_log_);

//	unsigned access();
	bool isHit(BusPacket *packet);


};






}

#endif
