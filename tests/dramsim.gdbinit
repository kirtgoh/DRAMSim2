
# filename: .gdbinit
# gdb will read it when starting 

# gdb logging file
set logging file gdb_log

echo ## Welcome the TraceBasedSim.gdbinit File ##\n

echo ## Loading file DRAMSim ##\n
file DRAMSim

#set args -s sysini/system.Nehalem.ini -d ini/DDR3_micron_16M_8B_x8_sg15.ini -t traces/read2read/mase_srsb_rmiss.trc 
#set args -s sysini/system.Nehalem.ini -d ini/DDR3_micron_16M_8B_x8_sg15.ini -t traces/write2read/mase_srsb_wrhit.trc 
#set args -s sysini/system.Nehalem.ini -d ini/DDR3_micron_16M_8B_x8_sg15.ini -t traces/read2write/mase_srsb_rw.trc 
#set args -s sysini/system.Nehalem.ini -d ini/DDR3_micron_16M_8B_x8_sg15.ini -t traces/read2read/mase_srsb_rmiss.trc 
#set args -s sysini/system.Nehalem.ini -d ini/DDR3_micron_16M_8B_x8_sg15.ini -t traces/mase_art.trc
#set args -s sysini/system.Nehalem.ini -d ini/DDR3_micron_16M_8B_x8_sg15.ini -t mase_run_out.trc
set args -s sysini/system.Nehalem.ini -d ini/DDR3_micron_16M_8B_x8_sg15.ini -t traces/k6_pending.trc

#b MemoryController.cpp: DRAMSim::MemoryController::update if poppedBusPacket->busPacketType == ACTIVATE
