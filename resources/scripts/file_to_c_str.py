#!/usr/bin/python

import sys

if len(sys.argv) == 2:
    with open(sys.argv[1]) as file:
        line = file.readline()
        while (line):
            print('"', end='')
            for c in line:
                if (c != '\n'):
                    print(c, sep='', end='')
            print('\\n"')
            
            line = file.readline()