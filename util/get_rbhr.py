#!/usr/bin/python
# get_rbhr.py - handle vis file and get rbhr

import os
import re
import sys 
import tempfile
import math
from Graphs import *

from optparse import OptionParser, OptionGroup

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
			rbhr.append(pattern.group(1))

	# calc average
	for r in rbhr:
		sum += r;

	print sum / len(rbhr)
	# return sum / len(rbhr)

if __name__ == '__main__':
	# SEARCH_PAT = re.compile(r'RBHR\s*=\s*(0.\d+)')
	# # src_line = 'io=8192.0MB, bw=24407KB/s, iops=6101 , runt=343698msec'
	# src_line = 'RBHR =0.423581'
    #
	# rbhr = []
	# pat_search = SEARCH_PAT.search(src_line)
	# if pat_search != None:
	# 	print 'Done.'
	# 	rbhr.append(pat_search.group(1))
    #
	# for i in rbhr:
	# 	print i
	line = 'RBHR =0.423581'

	pattern = re.compile(r'RBHR\s*=\s*(0.\d+)')

	rbhr = []
	# for line in fp:
	match = re.search(pattern, line)
	if match:
		rbhr.append(match.group(1))

	for i in rbhr:
		print i
