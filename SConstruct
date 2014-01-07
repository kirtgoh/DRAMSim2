# Top level SConstruct file for lib_mptlsim

import os

# List of subdirectories where we have source code
dirs = ['gtest']

# Now get list of .cpp files
src_files = Glob('*.cpp')

# Setup the environment
env = Environment(
	CXXFLAGS = '-DNO_STORAGE -Wall -DDEBUG_BUILD',
	CPPPATH = '.'
)

# debug enable/disable
debug = ARGUMENTS.get('debug',0)

if int(debug) == 1:
	env.Append(CXXFLAGS = ' -O0 -g')
else:
	env.Append(CXXFLAGS = ' -O3')

# libdramsim.so Builder
lib_bld_action = "$CXX -g -shared -Wl,-soname,$TARGET -o $TARGET $SOURCES"
lib_bld = Builder(action = Action(lib_bld_action))
env['BUILDERS']['LIB_BLD'] = lib_bld

# scons lib=0/1, 
lib = ARGUMENTS.get('lib',0)

if int(lib) >= 1: #libdramsim.so
	env.Append(CXXFLAGS = ' -DLOG_OUTPUT -fPIC')
	pobjs = env.Object(src_files)
	env.LIB_BLD('libdramsim.so', pobjs)
else: #DRAMSim
	objs = env.Object(src_files)
	env.Program('DRAMSim',objs)
