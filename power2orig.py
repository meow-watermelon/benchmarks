#!/usr/bin/env python3

'''
find Kaprekar numbers
'''

import time

init = 1000
start_time = time.time()

while True:
    if len(str(init)) % 2 != 0:
        init = init * 10

    splitter = int(len(str(init)) / 2)
    fore = str(init)[:splitter]
    post = str(init)[splitter:]

    if (int(fore) + int(post)) ** 2 == init:
        print('{0} {1} {2} {3:.6f}'.format(fore, post, init, time.time() - start_time))

    init = init + 1
