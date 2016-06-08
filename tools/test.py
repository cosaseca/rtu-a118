#!/usr/bin/python

import os,sys

for i in range(int(sys.argv[2])):
    os.system("./rtu_a118 client tcp " + sys.argv[1]+ "&")
