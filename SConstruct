# Top level SConstruct file for lib_mptlsim

import os

# Colored Output of Compilation
import sys

colors = {}
colors['cyan']   = '\033[96m'
colors['purple'] = '\033[95m'
colors['blue']   = '\033[94m'
colors['green']  = '\033[92m'
colors['yellow'] = '\033[93m'
colors['red']    = '\033[91m'
colors['end']    = '\033[0m'

#If the output is not a terminal, remove the colors
if not sys.stdout.isatty():
   for key, value in colors.iteritems():
      colors[key] = ''

compile_source_message = '%sCompiling %s:: %s$SOURCE ==> $TARGET%s' % \
   (colors['blue'], colors['purple'], colors['yellow'], colors['end'])

create_header_message = '%sCreating %s==> %s$TARGET%s' % \
   (colors['green'], colors['purple'], colors['yellow'], colors['end'])

compile_shared_source_message = '%sCompiling shared %s==> %s$SOURCE%s' % \
   (colors['blue'], colors['purple'], colors['yellow'], colors['end'])

link_program_message = '%sLinking Program %s==> %s$TARGET%s' % \
   (colors['red'], colors['purple'], colors['yellow'], colors['end'])

link_library_message = '%sLinking Static Library %s==> %s$TARGET%s' % \
   (colors['red'], colors['purple'], colors['yellow'], colors['end'])

ranlib_library_message = '%sRanlib Library %s==> %s$TARGET%s' % \
   (colors['red'], colors['purple'], colors['yellow'], colors['end'])

link_shared_library_message = '%sLinking Shared Library %s==> %s$TARGET%s' % \
   (colors['red'], colors['purple'], colors['yellow'], colors['end'])


pretty_printing=ARGUMENTS.get('pretty',1)

# Base Environment used to compile Marss code (QEMU and PTLSIM both)
if int(pretty_printing) :
    base_env = Environment(
            CXXCOMSTR = compile_source_message,
            CREATECOMSTR = create_header_message,
            CCCOMSTR = compile_source_message,
            SHCCCOMSTR = compile_shared_source_message,
            SHCXXCOMSTR = compile_shared_source_message,
            ARCOMSTR = link_library_message,
            RANLIBCOMSTR = ranlib_library_message,
            SHLINKCOMSTR = link_shared_library_message,
            LINKCOMSTR = link_program_message,
            )
else:
    base_env = Environment()

# Setup the environment
env = base_env.Clone()
env.Decider('MD5-timestamp')

env['CXXFLAGS'] = '-DNO_STORAGE -Wall -DDEBUG_BUILD'
env['CPPPATH'] = []
env['CPPPATH'].append(os.getcwd())

# List of subdirectories where we have source code
dirs = ['lib']

# Now get list of .cpp files
src_files = Glob('*.cpp')

# Debug enable/disable
debug = ARGUMENTS.get('debug',0)

if int(debug) == 1:
	env.Append(CXXFLAGS = ' -O0 -g')
	# Enable tests
	env.Append(CCFLAGS = '-DENABLE_TESTS')
	dirs.append('tests')
	# Add gtest directory in CPPPATH
	env['CPPPATH'].append(os.getcwd() + "/lib/gtest/include")
	env['CPPPATH'].append(os.getcwd() + "/lib/gtest")
	env['tests'] = True
else:
	env.Append(CXXFLAGS = ' -O3')
	env['tests'] = False

# RowBufferCache enable/disable
cache = ARGUMENTS.get('cache',0)

if int(cache) == 1:
	env.Append(CCFLAGS = '-DROWBUFFERCACHE')

# Include all the subdirectories into the CCFLAGS
for dir in dirs:
    env['CPPPATH'].append(os.getcwd() + "/" + dir)

# libdramsim.so Builder
lib_bld_action = "$CXX -g -shared -Wl,-soname,$TARGET -o $TARGET $SOURCES"
lib_bld = Builder(action = Action(lib_bld_action, cmdstr="$SHLINKCOMSTR"))
env['BUILDERS']['LIB_BLD'] = lib_bld

Export('env')

# Now call the SConscript in all subdirectories to build object files
# , lib/gtest and tests folders are included
tobjs = []
for dir in dirs:
    o = SConscript('%s/SConscript' % dir)
    if type(o) == list:
        tobjs.append(o[0])
    else:
        tobjs.append(o)

# generate *DRAMSim* or *libdramsim.so* 
lib = ARGUMENTS.get('lib',0)

if int(lib) >= 1: #libdramsim.so
	env.Append(CXXFLAGS = ' -DLOG_OUTPUT -fPIC')
	pobjs = env.Object(src_files)
	env.LIB_BLD('libdramsim.so', pobjs)
else: #DRAMSim
	objs = env.Object(src_files)
	env.Program('DRAMSim',objs+tobjs,LIBS='pthread')
