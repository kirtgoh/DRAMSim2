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

# Base plugin Metaclass
class PluginBase(type):
    """
    A Metaclass for reader and write plugins.
    """

    def __init__(self, class_name, bases, namespace):
        if not hasattr(self, 'plugins'):
            self.plugins = []
        else:
            self.plugins.append(self)

    def __str__(self):
        return self.__name__

    def get_plugins(self, *args, **kwargs):
        return sorted(self.plugins, key=lambda x: x.order)

    # add plugins's options, should check set_options attr
    def set_opt_parser(self, parser):
        for plugin in self.plugins:
            assert hasattr(plugin, 'set_options')
            p = plugin()
            p.set_options(parser)

# Reader Plugin Base Class
class Readers(object):
    """
    Base class for all Reader plugins.
    """
    __metaclass__ = PluginBase
    order = 0

    def read(options, args):
        stats = []
        for plugin in Readers.get_plugins():
            print plugin
            p = plugin()
            st = p.read(options, args)
            if type(st) == list:
                stats.extend(st)
            elif st:
                stats.append(st)

        print stats
        return stats

    read = staticmethod(read)

# Writer Plugin Base Class
class Writers(object):
    """
    Base class for all Writer plugins.
    """
    __metaclass__ = PluginBase
    order = 0

    def write(stats, options):
        for plugin in Writers.get_plugins():
            p = plugin()
            p.write(stats, options)

        return stats
    write = staticmethod(write)

# YAML Stats Reader Class
class YAMLReader(Readers):
    """
    Read the input file as YAML format.
    """

    def __init__(self):
        pass

    def set_options(self, parser):
        """ Add options to parser"""
        parser.add_option("-y", "--yaml", action="store_true", default=False,
                dest="yaml_file",
                help="Treat arguments as input YAML files")

    def load_yaml(self, file):
        docs = []

        for doc in yaml.load_all(file, Loader=Loader):
            doc['_file'] = file.name
            doc['_name'] = os.path.splitext(file.name)[0]
            docs.append(doc)

        return docs

    def read(self, options, args):
        """ Read yaml file if user give that option"""
        if options.yaml_file == True:
            docs = []
            for yf in args:
                l = lambda x: [ doc for doc in yaml.load_all(x, Loader=Loader)]
                with open(yf, 'r') as st_f:
                    docs += self.load_yaml(st_f)
            return docs

# VIS Stats Reader Class
class VISReader(Readers):
    """
    Read the input file as VIS format.
    """

    def __init__(self):
        pass

    def set_options(self, parser):
        """ Add options to parser"""
        parser.add_option("-v", "--vis", action="store_true", default=False,
                dest="vis_file",
                help="Treat arguments as input vis files")

    def load_vis(self, file):
        docs = []

        for doc in yaml.load_all(file, Loader=Loader):
            doc['_file'] = file.name
            doc['_name'] = os.path.splitext(file.name)[0]
            docs.append(doc)

        return docs

    def read(self, options, args):
        """ Read vis file if user give that option"""
        if options.vis_file == True:
			for fn in args:
				output = tempfile.NamedTemporaryFile(delete=False)
				print output.name

				# open the vis file
				fp = open(fn, "r")
				line = 'deadbeef';

				startCopying = False
				while line:
					line = fp.readline();
					if line.startswith("!!EPOCH_DATA"):
						startCopying = True
						# print "yes find !!EPOCH_DATA"
						continue
					elif line.endswith("!!HISTOGRAM_DATA\n"):
						# print "yes find !!HISTOGRAM_DATA"
						break
					if startCopying:
						output.write(line)
						# print "input=%s output=%s" % (filename, output.name)
				output.close()
					# 	return DataTable(output.name)


def setup_options():
  opt = OptionParser("usage: %prog [options] args")

  # y is class of Readers, Filters, Process or Writers
  opt_setup = lambda x,y: y.set_opt_parser(x) or opt.add_option_group(x)

  read_opt = OptionGroup(opt, "Input Options")
  opt_setup(read_opt, Readers)

  filter_opt = OptionGroup(opt, "Stats Filtering Options")
  # opt_setup(filter_opt, Filters)

  process_opt = OptionGroup(opt, "PostProcess Options")
  # opt_setup(process_opt, Process)

  write_opt = OptionGroup(opt, "Output Options")
  # opt_setup(write_opt, Writers)

  return opt 

def execute(options, args):
    """ Run this script with given options to generate user specific output"""

    # First read in all the stats
    stats = Readers.read(options, args)

    # stats = Filters.filter(stats, options)
    #
    # stats = Process.process(stats, options)
    #
    # Writers.write(stats, options)

def load_plugins():
    exec_dir = os.path.dirname(os.path.realpath(sys.argv[0]))
    sys.path.append(exec_dir)
    path = "%s/mstats_plugins" % (exec_dir)
    for root, dirs, files in os.walk(path):
        for name in files:
            if name.endswith(".py") and not name.startswith("__"):
                path = os.path.join("mstats_plugins", name)
                path = path [1:] if path[0] == '/' else path
                print path
                plugin_name = path.rsplit('.',1)[0].replace('/','.')
                try:
                    __import__(plugin_name)
                except Exception as e:
                    debug("Unable to load plugin: %s" % plugin_name)
                    debug("Exception %s" % str(e))
                    pass
def get_files_path(path, suffix=".vis"):
	file_list = []
	for root, dirs, files in os.walk(path):
		for name in files:
			if name.endswith(suffix):
				filepath = os.path.join(root, name)
				file_list.append(filepath)
				# os.remove(name)
				# file_list.append(name)

	return file_list 

def print_files(path, filelist):
	filetmp = filelist
	print "Change folder"
	os.chdir(path)
	print "The dirname is: "
	print os.getcwd()
	print "Got the list now: "

	for i in os.listdir(path):
		print '\n',i,'\t\t',len(i)

	for i in filelist:
		os.remove(i)

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

def gen_header(filename):
	# (machine) (checkpoint) (scheme)
	pattern= re.compile(out_dir + '(.+)_(.+)\.(.+)/DDR.*')
	match = re.search(pattern, filename)
	if match:
		machine = match.group(1)
		checkpoint = match.group(2)
		scheme = match.group(3)

	print "checkpoint:\t%s" % checkpoint

if __name__ == '__main__':

	# realpath = out_dir
  # -stats %(out_dir)s/%(machine_tag)s_%(bench)s.stats
  # -dramsim-results-dir-name parsec-2.1/%(machine_tag)s_%(bench)s.%(scheme)s
  #
	# findout machine_tag,checkpoints, shceme, 
	vis_file = '/home/kgoh/DRAMSim2/results/spec2006/xeon-4-1_1perl4c.Nehalem/DDR3_micron_16M_8B_x8_sg15/2GB.2Ch.2R.Nehalem.open_page.32TQ.4CQ.RtB.pRankpBank.vis'
	stats_file = '/home/kgoh/DRAMSim2/results/spec2006/xeon-4-1_1perl4c.Nehalem/xeon-4-1_1perl4c.Nehalem.stats'
	out_dir = '/home/kgoh/DRAMSim2/results/spec2006/'

	# print out_dir + '(s+)_*'

	bench = 'spec2006'
	path = "/home/kgoh/DRAMSim2/results/%s" % (bench)

	# get all the vis file
	lists = get_files_path(path)

	chan1 = 'Aggregate_Bandwidth[0]'
	chan2 = 'Aggregate_Bandwidth[1]'
	for fn in lists:
		gen_header(fn)
		dt = vis_file_to_datatable(fn)
		bd = calc_bandwidth(dt)
		print "BW:\t" + str(round(bd * 1000, 2)) + "MB/s"

