#!/usr/bin/python
# calc_bandwidth.py

#
# This is a helper script to manipulate Marss statistics (YAML, time based
# etc.). Please run --help to list all the options.
#
# This script is provided under LGPL licence.
#
# Author: Avadh Patel (avadh4all@gmail.com) Copyright 2011
#


"""
Goal: this script will genearte spec2006/[m:xeon-4-1]_[checkpoints].[Scheme]/[DDR]/*.vis

graph and power and bandiwth and row buffer hit rates

folder
spec2006/xeon-4-1_1perl4c.Nehalem/DDR3
provides a way to look at vis files that come out of the time based stats in a
column format. Otherwise, it's hard to line up what number belongs to what
column header

step-1: according otipns, print all the vis filename
"""

import os
import re
import sys 
import tempfile
import math
from Graphs import *

from optparse import OptionParser, OptionGroup

try:
    import yaml
except (ImportError, NotImplementedError):
    path = os.path.dirname(sys.argv[0])
    a_path = os.path.abspath(path)
    sys.path.append("%s/../ptlsim/lib/python" % a_path)
    import yaml

try:
    import Graphs
    graphs_supported = True
except (ImportError, NotImplementedError):
    graphs_supported = False

try:
    from yaml import CLoader as Loader
except:
    from yaml import Loader

# Standard Logging and Error reporting functions
def log(msg):
    print(msg)

def debug(msg):
    print("[DEBUG] : %s" % msg)

def error(msg):
    print("[ERROR] : %s" % msg)
    sys.exit(-1)
''' 
# should use another methods open *.vis 
f = file('single_core_smt2_threads-64.vis', 'r') # open for 'w'riting
 # if no mode is specified, 'r'ead mode is assumed by default

bandwidth = 0.0 

begin = 64
end = 17589

for line in f.readlines()[begin:end]:
    strdata = line.split(',')[-2]
    bandwidth += float(strdata) 
print bandwidth / (end - begin)
f.close()
'''
def calc_average_value(dt, y_col_name):
	if dt.contains_column(y_col_name):
		data = dt.get_data(y_col_name)
		print y_col_name
		print sum(data) / len(data)


# datatable = vis_file_to_datatable(options.vis_filename)
# calc_average_value(datatable,options.y_col_name)
'''
while True:
     line = f.readlines()[64:66]
     if len(line) == 0: # Zero length indicates EOF
         break
     print line,
     # Notice comma to avoid automatic newline added by Python
f.close() # close the file 

            for line in weights.readlines():
                sp = line.strip().split(' ')
                assert(len(sp) == 2)
                weight = float(sp[0])
                id = int(sp[1])
                w[id] = weight
'''

def vis_file_to_datatable(filename):
	if not os.path.exists(filename):
		print "ERROR: vis file %s not found" % filename
		exit()

	output = tempfile.NamedTemporaryFile(delete=False)
	fp = open(filename, "r")

	line = 'deadbeef';
	startCopying = False
	while line:
		line = fp.readline();
		if line.startswith("!!EPOCH_DATA"):
			startCopying = True;
			continue
		elif line.endswith("!!HISTOGRAM_DATA\n"):
			break
		if startCopying:
			output.write(line)
	output.close()
	return DataTable(output.name)

def calc_bandwidth(datatable):
	c1 = 'Aggregate_Bandwidth[0]'
	c2 = 'Aggregate_Bandwidth[1]'

	if datatable.contains_column(c1):
		data = datatable.get_data(c1)
		chan1 = sum(data) / len(data)

	if datatable.contains_column(c2):
		data = datatable.get_data(c2)
		chan2 = sum(data) / len(data)
	
	return chan1 + chan2

def get_files_list(path, suffix=".vis"):
	file_list = []
	for root, dirs, files in os.walk(path):
		for name in files:
			if name.endswith(suffix):
				filepath = os.path.join(root, name)
				file_list.append(filepath)
				# os.remove(name)
				# file_list.append(name)

	return file_list 


# input should be vis file
def get_rbhr_from_vis(filename):
	if not os.path.exists(filename):
		print "ERROR: vis file %s not found" % filename
		exit()

	fp = open(filename, "r")

	pattern = re.compile(r'RBHR\s*=\s*(0.\d+)')

	rbhr = []
	for line in fp:
		match  = re.search(pattern, line)
		if match:
			rbhr.append(match.group(1))

	# calc average
	sum = 0.0
	for r in rbhr:
		sum += float(r);

	return sum / len(rbhr)

# First check if user has provided directory to save all results files
opt_parser = OptionParser("Usage: %prog [options] run_config")

# input file as vis file
opt_parser.add_option("-f", "--file", dest="vis_filename", help="vis filename with which to generage a y_col_description average value", metavar="FILE")
# indicate stats file's column name
opt_parser.add_option("-y", "--y_col_name", dest="y_col_name", help="vis filename with which to generage a y_col_description average value")
opt_parser.add_option("-i", "--ipc", action="store_true", default=False,
		dest="ipc_stats",
		help="Present IPC stats")
opt_parser.add_option("-r", "--rbhr", action="store_true", default=False,
		dest="rbhr_stats",
		help="Present RBHR stats")
opt_parser.add_option("-m", "--mpki", action="store_true", default=False,
		dest="mpki_stats",
		help="Present mpki stats")
opt_parser.add_option("-b", "--bdw", action="store_true", default=False,
		dest="band_stats",
		help="Present bandwidth stats")

def get_threads_ipc(log_filename):
	if not os.path.exists(log_filename):
		error("ERROR: vis file %s not found" % log_filename)

	fp = open(log_filename, "r")

	pat_total = re.compile(r'insns\/cyc:\s*(\d.\d+)')
	pat_xeon = re.compile(r'total.base_machine.(.*).thread0.commit.ipc\s*=\s*(\d.\d+)')

	ipcs = []
	for line in fp:
		match_total  = re.search(pat_total, line)
		match_xeon = re.search(pat_xeon, line) 

		if match_total:
			ipcs.append(match_total.group(1))

		if match_xeon:
			ipcs.append(match_xeon.group(2))


	# if not match:
	# 	error("Can not match IPC")
	
	# print match.group()
	# return match.group(1)
	return ipcs

# stats file should be handled by mstats.py
def get_mpki_from_stats(stats_filename, num_insns):
	# generate cache file
	target_file = stats_filename + '.cache'
	cmd_line = "./mstats.py -y --yaml-out -t total --cache-summary %s > %s" % (stats_filename, target_file)

	if not os.path.exists(target_file):
		os.system(cmd_line)


	# handle generate cache summary file
	fp = open(target_file, "r")

	# L2_0:
	#   Total Hits:
	#   Total Miss:
	# counter indicate Total Miss line offset is 1 + 2
	counter = 0

	p1 = re.compile(r'L2_0:')
	p2 = re.compile(r'\s*Total\s*Miss\s*:\s*(\d+)')
	pattern = p1

	miss = 0
	for line in fp:
		counter += 1
		match  = re.search(pattern, line)
		if counter == 3 and match:
			miss = float(match.group(1))
			break
		elif counter == 3 and not match:
			pattern = p1
		elif match: 		# match L2_0:
			pattern = p2
			# print match.group()
			# print counter
			counter = 1

	return (miss * 1000) / float(num_insns)

def gen_tags(vis_filename):
	# (machine) (checkpoint) (scheme)

	# findout machine_tag,checkpoints, shceme, 
	# eg., :
	# vis_file = '/home/kgoh/DRAMSim2/results/spec2006/xeon-4-1_1perl4c.Nehalem/DDR3_micron_16M_8B_x8_sg15/2GB.2Ch.2R.Nehalem.open_page.32TQ.4CQ.RtB.pRankpBank.vis'
	# stats_file = '/home/kgoh/DRAMSim2/results/spec2006/xeon-4-1_1perl4c.Nehalem/xeon-4-1_1perl4c.Nehalem.stats'
	# out_dir = '/home/kgoh/DRAMSim2/results/spec2006/'

	pattern= re.compile(path + '\/(.+)_(.+)\.(.+)/DDR.*')
	match = re.search(pattern, vis_filename)
	if match:
		tag = dict(
			machine = match.group(1),
			checkpoint = match.group(2),
			scheme = match.group(3)
		)
	else:
		error("can not get visfile's tags")

	# print "checkpoint:\t%s" % tag['checkpoint']
	return tag

def get_num_insns(logfile):
	if not os.path.exists(logfile):
		error("ERROR: log file %s not found" % logfile)

	fp = open(logfile, "r")
	
	num = 0
	pattern = re.compile(r'(\d+)\s*instructions')
	for line in fp:
		# print line
		match = re.search(pattern, line)
		if match:
			num = match.group(1)
	
	return num

def gen_run_configs(path):
	# path is util like spec2006
    # For each checkpoint create a run_config dict and add to list

	run_cfgs = []

	# first get first sub folders and 
	# get first sub folders
	sub_dirs= os.listdir(path)
	
	for result_fold in sub_dirs:
		p = os.path.join(path, result_fold)
		# walk through every sub result folder
		run_cfg = {}
		for root, dirs, files in os.walk(p):
			for name in files:
				# vis file
				if name.endswith(".vis"):
					filepath = os.path.join(root, name)
					run_cfg['vis_file'] = filepath
					# get # (machine) (checkpoint) (scheme)
					tag = gen_tags(filepath)
					run_cfg['machine'] = tag['machine']
					run_cfg['checkpoint'] = tag['checkpoint']
					run_cfg['scheme'] = tag['scheme']
				# stats file
				if name.endswith(".stats"):
					filepath = os.path.join(root, name)
					run_cfg['stats_file'] = filepath
				# log file
				if name.endswith(".log"):
					filepath = os.path.join(root, name)
					run_cfg['log_file'] = filepath
					run_cfg['num_insns'] = get_num_insns(filepath)

		run_cfgs.append(run_cfg)

	return run_cfgs

if __name__ == '__main__':

	# parse options
	(options, args) = opt_parser.parse_args()
	if args == None or args == []:
		opt_parser.print_help()
		sys.exit(-1)


	# bench = 'spec2006'
	for bench in args:
		# gen target path
		path = "/home/kgoh/DRAMSim2/results/%s" % (bench)
		if not os.path.exists(path):
			error("Target is not exists")
			

    	# For each checkpoint create a run_config dict and add to list
		run_cfgs = []

		# get all the vis file, list =['abs1','abs2']
		run_cfgs.extend (gen_run_configs(path))

		print " == Bench folder: %s" % bench
		print "Total checkpoints : %d" % len(run_cfgs)
		for cfgs in run_cfgs:
			print " -- %s: " % cfgs['checkpoint']

			# Output mpki
			if options.mpki_stats == True:
				print "\tMPKI: \t\t%.2f" % get_mpki_from_stats(cfgs['stats_file'], cfgs['num_insns'])

			# Output Bandwidth
			if options.band_stats == True:
				dt = vis_file_to_datatable(cfgs['vis_file'])
				bd = calc_bandwidth(dt) * 1000 # MB/s
				# DDR3-1333 and 2 channals
				full_bd = (128 * 2 * 666.0) / 8
				print "\tBW:\t\t%.2fMB/s\t%.2f%%" % (bd, ((bd / full_bd) * 100))

			# Output rbhr
			if options.rbhr_stats == True:
				print "\tRBHR: \t\t%.2f%%" % (get_rbhr_from_vis(cfgs['vis_file'])*100)
				print ""

			# Output ipc
			if options.ipc_stats == True:
				ipcs = get_threads_ipc(cfgs['log_file'])

				print "\tIPC_all: \t%f" % (float(ipcs[0]))

				xeon_num = len(ipcs)
				for i in range(1, xeon_num):
					print "\tIPC_xeon_%d: \t%f" % (i-1, float(ipcs[i]))
				
			
