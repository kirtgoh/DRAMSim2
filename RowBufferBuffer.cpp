//=====================================================================================
// RowBufferBuffer.cpp -  
//
// Copyright (C) 2014 Kirt Goh <kirtgoh@gmail.com>
//=====================================================================================

#include "RowBufferBuffer.h"

using namespace std;
using namespace DRAMSim;

//ALl buffers start invalid
RowBufferBuffer::RowBufferBuffer(ostream &dramsim_log_):
	dramsim_log(dramsim_log_),
	currentBufferState(Invalid),
	openRowAddress(0),
	nextRead(0),
	nextWrite(0),
	lastCommand(READ),
	stateChangeCountdown(0)
{}

/* unsigned RowBufferBuffer::access() {
 * 
 * }
 */
unsigned RowBufferBuffer::isHit(BusPacket *packet)
{
	if (packet->row == openRowAddress)
		return 1;	
	else
		return 0;
}
