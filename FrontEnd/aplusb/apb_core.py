import sys
import os.path
import subprocess

class Calculator:
    def __init__(self, backend):
        self.pipe = subprocess.Popen(backend, bufsize=1024, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
        self.backend = backend

    def __del__(self):
        self.write('exit -1 -1')
        self.read()

    def read(self):
        ret = ''
        print('read...')
        ret += self.pipe.stdout.readline().decode()
        self.pipe.stdout.flush()
        print(ret)
        return ret

    def write(self, str):
        print(str)
        self.pipe.stdin.write((str + '\n').encode())
        self.pipe.stdin.flush()

    def add(self, a, b):
        self.write('add ' + str(a) + ' ' + str(b))
        ret = self.read()
        print("bomb")
        return ret