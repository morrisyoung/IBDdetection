import os
import sys


if __name__ == '__main__':
	filename_std = sys.argv[1]
	filename_tester = sys.argv[2]

	file_std = open(os.getcwd() + "/" + filename_std, "r")
	file_tester = open(os.getcwd() + "/" + filename_tester, "r")

	rep_tester = {}
	lines = file_tester.readlines()
	for line in lines:
		line = line.strip()
		rep_tester[line] = 1

	rep_std = {}
	lines = file_std.readlines()
	for line in lines:
		line = line.strip()
		rep_std[line] = 1


	print "Now testing... (start from Tester's rep)"
	for item in rep_tester:
		if item in rep_std:
			rep_std[item] = 0
		else:
			print "Tester has his own:",
			print item

	print "Now let's see the rep of Std:"
	for item in rep_std:
		if rep_std[item] == 1:
			print "Std has his own:",
			print item

	print "testing done!"
