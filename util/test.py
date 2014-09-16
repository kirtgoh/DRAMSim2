#!/usr/bin/python

import os

def loadSWF2(lis,extdsname):
	litmp=[]
	for i in lis:
		if i.endswith(extdsname):
			litmp.append(i)
	return litmp         

def getFoundfiles(strpath):
	print "in getFoundfiles"
	for root, dirs, files in os.walk(strpath):
		listt=files
		return listt

def goodremove(ipath,filelist):

	filetmp=filelist
	print 'Now we had better change the path!'
	os.chdir(ipath)

	print '\nThe dirname is :'
	print os.getcwd()

	print '\nGot the fileslist now:\n'

	for i in os.listdir(ipath):
		print '\n',i,'\t\t',len(i),

	for i in filelist:
	   os.remove(i)        

def testmain():
	strpath='tmp'
	lis1=getFoundfiles(strpath)
	print lis1
	lis2=loadSWF2(lis1,'.swf')

	# '.extendsname'
	goodremove(strpath,lis2)
	for i in lis2:
		if i not in os.listdir(strpath):
			print '%-15s\t' % i,'has removed now!\n'

if __name__=='__main__':
	testmain() 
