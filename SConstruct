# Top level SConstruct file for lib_mptlsim

import os

# List of subdirectories where we have source code
dirs = ['gtest']

# Now get list of .cpp files
src_files = Glob('*.cpp')

# Setup the environment

env = Environment(
	CXXFLAGS = "-DNO_STORAGE -Wall -DDEBUG_BUILD" 
)

debug = ARGUMENTS.get('DEBUG',0)
if int(debug) == 1:
	env.Append(CXXFLAGS = ' -O0 -g')
else:
	env.Append(CXXFLAGS = ' -O3')

env['CPPPATH'] = ['.']
objs = env.Object(src_files)
env.Program('DRAMSim',objs)
