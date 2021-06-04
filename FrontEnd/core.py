import subprocess

class Courier:
    def __init__(self, backend):
        self.backend = backend
        self.pipe = subprocess.Popen(backend, bufsize=1024, stdin=subprocess.PIPE, stdout=subprocess.PIPE)

    def __del__(self):
        self.write("exit")

    def read(self):
        ret = ''
        print('read...')
        while True:
            nowline = self.pipe.stdout.readline().decode('UTF-8')
            print(nowline)
            if nowline == "UAG\n":
                break
            else:
                ret += nowline
        return ret

    def write(self, str):
        print(str)
        self.pipe.stdin.write((str + '\n').encode('UTF-8'))
        self.pipe.stdin.flush()

    def cmd(self, str):
        self.write(str)
        ret = self.read()
        print("bomb")
        return ret

    def login(self, userid, passwd):
        self.write('login ' + '-u ' + userid + ' -p ' + passwd)
        ret = self.read()
        retList = ret.split('\n')
        print(retList)
        if retList[0] == "wrong_passwd" or retList[0] == "user_not_found":
           return False, retList[0], userid, passwd
        elif retList[0] == "0":
           return True, retList[1], userid, passwd
        return True, retList[1], userid, passwd #重复提交表单

    def logout(self, userid):
        self.write('logout -u ' + userid)
        ret = self.read()
        print(ret)

    def add_user(self, curuserid, userid, passwd, name, mailAddr, privilege):
        self.write('add_user -c ' + curuserid + ' -u ' + userid + ' -p ' + passwd + ' -n ' + name + ' -m ' + mailAddr + ' -g ' + privilege)
        ret = self.read()
        retList = ret.split('\n')
        if retList[0] == "not_logged" or retList[0] == "privilege_error" or retList[0] == "uid_duplicate":
            return False, retList[0]
        return True, userid

    def query_profile(self, curuserid, userid):
        self.write('query_profile -c ' + curuserid + ' -u ' + userid)
        ret = self.read()
        retList = ret.split('\n')
        if retList[0] == "not_logged" or retList[0] == "privilege_error" or retList[0] == "user_not_found":
            return False, retList[0]
        return True, retList[0].split(' ')

    def modify_profile(self, curuserid, userid, passwd, name, mailAddr, privilege):
        str = 'modify_profile -c ' + curuserid + ' -u ' + userid
        if passwd:
            str += ' -p ' + passwd
        if name:
            str += ' -n ' + name
        if mailAddr:
            str += ' -m ' + mailAddr
        if privilege:
            str += ' -g ' + privilege
        self.write(str)
        ret = self.read()
        retList = ret.split('\n')
        if retList[0] == "not_logged" or retList[0] == "privilege_error" or retList[0] == "user_not_found":
            return False, retList[0]
        return True, retList[0].split(' ')

    def query_ticket(self, s, t, date, p):
        self.write('query_ticket -s ' + s + ' -t ' + t + ' -d ' + date + ' -p ' + p)
        ret = self.read()
        retList = ret.split('\n')
        for i in range(1, len(retList)):
            retList[i] = retList[i].split(' ')
        return retList

    def query_transfer(self, s, t, date, p):
        self.write('query_transfer -s ' + s + ' -t ' + t + ' -d ' + date + ' -p ' + p)
        ret = self.read()
        retList = ret.split('\n')
        if retList[0] == "0":
            return False, retList
        for i in range(len(retList)):
            retList[i] = retList[i].split(' ')
        print(retList)
        return True, retList

    def buy_ticket(self, userid, trainid, date, f, t, n, q):
        self.write('buy_ticket -u ' + userid + ' -i ' + trainid + ' -d ' + date + ' -f ' + f + ' -t ' + t + ' -n ' + n + ' -q ' + q)
        ret = self.read()
        retList = ret.split('\n')
        if retList[0] == "not_logged" or retList[0] == "train_not_found" or retList[0] == "not_released" or retList[0] == "day_error" or retList[0] == "seat_not_enough":
            return False, retList[0]
        return True, retList[0]

    def query_order(self, userid):
        self.write('query_order -u ' + userid)
        ret = self.read()
        retList = ret.split('\n')
        if retList[0] == "not_logged":
            return False, retList
        for i in range(1, len(retList)):
            retList[i] = retList[i].split(' ')
        return True, retList

    def refund_ticket(self, userid, n):
        self.write('refund_ticket -u ' + userid + ' -n ' + n)
        ret = self.read()
        retList = ret.split('\n')
        return retList[0] == "0"

    def add_train(self, train_str):
        self.write('add_train' + train_str)
        ret = self.read()
        retList = ret.split('\n')
        if retList[0] == "0":
            return True, retList[0]
        return False, retList[0]

    def query_train(self, trainid, date):
        self.write('query_train -i ' + trainid + ' -d ' + date)
        ret = self.read()
        retList = ret.split('\n')
        if retList[0] == "train_not_found" or retList[0] == "day_error":
            return False, retList[0], date
        for i in range(len(retList)):
            retList[i] = retList[i].split(' ')
        print(retList)
        return True, retList, date

    def release_train(self, trainid):
        self.write('release_train -i ' + trainid)
        ret = self.read()
        retList = ret.split('\n')
        if retList[0] == "train_not_found" or retList[0] == "has_released":
            return False, retList[0]
        return True, retList[0]

    def delete_train(self, trainid):
        self.write('delete_train -i ' + trainid)
        ret = self.read()
        retList = ret.split('\n')
        if retList[0] == "train_not_found" or retList[0] == "has_released":
            return False, retList[0]
        return True, retList[0]