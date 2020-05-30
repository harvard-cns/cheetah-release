#!/usr/bin/python3

# To check if I send and receive the correct result for my filter query

import csv
import sys

if __name__ == '__main__':
    with open(sys.argv[1], 'r') as f:
        reader = csv.reader(f)
        count = 0
        for row in reader:
            if int(row[-1]) < 50:
                count += 1

        print(count)
