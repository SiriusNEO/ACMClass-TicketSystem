#!/usr/bin/python3

import os


def clean():
    os.system('bash ./clean.sh')


def clean_output():
    os.system('rm output*')


def run_basic_case(id):
    if id <= 2:
        os.system('./code < data/basic_' + str(id) + '/1.in > output.txt 2>/dev/null')
    if 3 <= id <= 4:
        for i in range(1, 6):
            os.system('./code < data/basic_' + str(id) + '/' + str(i) + '.in >> output' + str(i) + '.txt 2>/dev/null')
    if 5 <= id <= 6:
        for i in range(1, 11):
            os.system('./code < data/basic_' + str(id) + '/' + str(i) + '.in >> output' + str(i) + '.txt 2>/dev/null')


def run_basic():
    clean()
    print('Input Basic Test ID you want to run:')
    id = int(input())
    run_basic_case(id)


run_basic()
