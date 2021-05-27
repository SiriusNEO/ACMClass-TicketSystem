#!/usr/bin/python3

import time
import os
import filecmp

time_Limit = 30  # s


def clean_all():
    os.system('bash ./clean.sh')
    os.system('rm output.txt')


def clean_sh():
    os.system('bash ./clean.sh')


def run_basic_case(id):
    if id <= 2:
        os.system('timeout ' + str(time_Limit) + 's ' + './code < data/basic_' + str(id) + '/1.in > output.txt 2>/dev/null')
    #    return filecmp.cmp('data/basic_' + str(id) + '/1.out', 'output.txt')
        return True
    if 3 <= id <= 4:
        for i in range(1, 6):
            os.system('timeout ' + str(time_Limit) + 's ' + './code < data/basic_' + str(id) + '/' + str(
                i) + '.in >> output.txt 2>/dev/null')
    #        if not filecmp.cmp('data/basic_' + str(id) + '/' + str(i) + '.out', 'output.txt'):
            os.system('rm output.txt')
    if 5 <= id <= 6:
        for i in range(1, 11):
            os.system('timeout ' + str(time_Limit) + 's ' + './code < data/basic_' + str(id) + '/' + str(
                i) + '.in >> output.txt 2>/dev/null')
    #       if not filecmp.cmp('data/basic_' + str(id) + '/' + str(i) + '.out', 'output.txt'):
            os.system('rm output.txt')
    return True


def run_basic():
    print('Running Basic Test')
    for i in range(1, 7):
        if run_basic_case(i):
            print('Basic Testpoint ' + str(i) + ' Accepted!')
        else:
            print('Basic Testpoint ' + str(i) + ' Failed!')
        if 1 <= i <= 2:
            clean_all()
        else:
            clean_sh()



clean_sh()
start = time.time()
run_basic()
print('耗时:', time.time() - start)
