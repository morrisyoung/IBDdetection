import os
import math
import sys


# set the parameters, used to predict
n = 1000
l = 1
N = 2000
m = 0.01
filename = ""


if __name__ == "__main__":

	
	#if len(sys.argv[1:]) != 3:
	#	print "Please enter \"CLENGTH\", \"CUTOFF\" and \"filename\" in order ... "
	#	print "default values:"
	#	print "100,000,000 1,000,000 xxx"
	#	sys.exit()
	#CLENGTH = int(sys.argv[1])
	#CUTOFF = int(sys.argv[2])
	filename = sys.argv[1]



	h = {}
	for i in range(1, n + 1):
		for j in range(i+1, n + 1):
			key = str(i) + '-' + str(j)
			h[key] = []

	file = open(os.getcwd() + "/" + filename, "r")
	rep = {}
	count = 0
	while 1:
		count += 1
		#if count % 100 == 0:	
		#	print count
		line = file.readline()
		if not line:
			break
		line = line.strip()
		line = line.split(" ")
		line[1] = line[1][1:-1].split(",")
		line[1] = int(line[1][1]) - int(line[1][0])
		#print line[0]
		#print line[1]
		key = line[0] #the key, or pair name
		value = line[1] #the interval
		h[key].append(value)
	file.close()


	ns = 0
	fs = 0
	for key in h:
		ns += len(h[key])
		fs += sum(h[key])
	ns = float(ns) / float(n * (n-1) / 2)
	fs = float(fs) / float( (n * (n-1) / 2) * l * 100000000 )
	print "ns fs"
	print "real data:"
	print ns,
	print fs
	print "theory:"
	ns1 = (2 * N * l / math.pow((1 + 2 * m * N) , 2))
	print ns1,
	fs1 = (1 + 4 * m * N) / math.pow((1 + 2 * m * N) , 2)
	print fs1


	'''
	file = open(os.getcwd() + "/result.txt", "w")
	file.write("ns fs\nreal data:\n")
	file.write(str(ns) + " " + str(fs) + "\n")
	file.write("theory:\n")
	file.write(str(ns1) + " " + str(fs1) + "\n")
	file.close()
	'''
