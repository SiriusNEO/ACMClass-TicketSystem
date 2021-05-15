//TicketSystem main.cpp for test

#include "systemcore.hpp"
#include <algorithm>
#include <set>
#include <assert.h>

Sirius::System ticketSystem;

void HelloTicketSystem() {
    srand(time(nullptr));
    Sirius::FixedStr<233> hello("Hello, TicketSystem!");
    std::cout << hello << '\n';
}

void SplitTest() {
    std::string cmd;
    std::cin >> cmd;
    cmd[2] = 'f';
    std::cout << cmd << '\n';
    std::string cmdcmd[10];
    int cc;
    Sirius::split(cmd, cmdcmd, cc, 'c');
    for (int i = 0; i < cc; ++i) std::cout << cmdcmd[i] << '\n';
    MINE(cc)
}

void TimeTypeTest() {
    std::cout << Sirius::stringToInt("666") << '\n';
    Sirius::TimeType test("09-30 15:38");
    auto tmp = Sirius::TimeType("01-01 02:30");
    test = test + tmp;
    std::cout << test.getDatePair().first << ' ' << test.getDatePair().second << '\n';
    MINE(test.toFormatString())
    MINE(test.getDate())
    MINE(test.getClock())
}

void HashTableTest() {
    Sirius::HashTable<30, 1226959, 131> mySet;

    std::set<std::string> stdTest;
    {
        STANDINGBY()
        MINE("DataSet: 20000")
        for (int i = 1; i <= 20000; ++i) {
            std::string tmp = Sirius::randString(29);
            if (!mySet.find(tmp)) mySet.insert(tmp);
            else mySet.del(tmp);
        }
        COMPLETE("Sirius::HashTable")
    }

    {
        STANDINGBY()
        for (int i = 1; i <= 20000; ++i) {
            std::string tmp = Sirius::randString(29);
            if (stdTest.count(tmp) == 0) stdTest.insert(tmp);
            else stdTest.erase(tmp);
        }
        COMPLETE("std::set")
    }
}

void ParseTest() {
    std::string str;
    getline(std::cin, str, '\n');
    Sirius::parse(str);
}

void QSortTest() {
    int a[10000002], b[10000002];
    for (int i = 1; i <= 10000000; ++i) a[i] = b[i] = rand();
    {
        STANDINGBY()
        Sirius::qsort<int>(a+1, a+10000000, [](const int& x, const int& y)->bool {return x < y;});
        //for (int i = 1; i <= 1000000; ++i) std::printf("%d ", a[i]);printf("%d\n");
        COMPLETE("Datasize: 10000000\nmy qsort")
    }

    {
        STANDINGBY()
        std::qsort(b, 10000000, sizeof(int), [](const void *a,const void *b)
        {
            return *(int*)b-*(int*)a;
        });
        COMPLETE("std::qsort")
    }
}

void UserQuerySpeedTest() {
    ticketSystem.response("add_user -c cur -u I_am_the_admin -p awsl -n 奥斯卡 -m foo@bar.com -g 10");
    ticketSystem.response("login -u I_am_the_admin -p awsl");
    STANDINGBY()
    for (int i = 1; i <= 1000000; ++i) {
        auto tmp = Sirius::randString(17);
        ticketSystem.response("add_user -c I_am_the_admin -u " + tmp + " -p awsl -n 奥斯卡 -m foo@bar.com -g 1");
        ticketSystem.response("login -u " + tmp + " -p awsl");
    }
    COMPLETE("User System with HashTable")
}

void CORE() {
    //STANDINGBY()
    std::string cmd;
    //std::ios::sync_with_stdio(false);
    //std::cin.tie(0);
    //std::cout.tie(0);
    while (getline(std::cin, cmd, '\n')) {
        //std::cout << cmd << '\n';
        auto res = ticketSystem.response(cmd);
        std::cout << res.second << '\n';
        if (!res.first) break;
        cmd.clear();
    }
    //COMPLETE("core system time use")
}

void FILEOPEN(std::string no) {
    freopen(("../data/basic_"+no+"/"+"1.in").c_str(), "r", stdin);
    freopen(("../data/basic_"+no+"/my.out").c_str(), "w", stdout);
}

void TestBlockList() {
    Sirius::BlockList<int, 20> blockList("233.bin");
    blockList.insert(1, 2);
    std::cout << blockList.find(1).first << '\n';
    blockList.del(1);
    blockList.Print();
}

void TestFM() {
    Sirius::FileManager<Sirius::FixedStr<20>, Sirius::FixedStr<20>> fm("test.bin");
    /*
    for (int i = 1; i <= 10; ++i) fm.insert(Sirius::intToString(i)+"BOMB!", i*100);
    fm.dataStructure.Print();
    auto ret = fm.rangeFind("1", "3");
    for (auto i : ret) std::cout << i << '\n';
    std::cout << fm.siz << '\n';
    std::cout << fm.modify("1BOMB!", 666) << '\n';
    std::cout << fm.find("1BOMB!").first << '\n';
    BOMB
    std::cout << fm.del("2BOMB!") << '\n';
    std::cout << fm.find("2BOMB!").second << '\n';
    fm.insert("2BOMB!", 233);
    std::cout << fm.find("2BOMB!").first << '\n';
    fm.modify("2BOMB!", 666);
    std::cout << fm.find("2BOMB!").first << '\n';
    */
    fm.insert("中院", "rocket");
    fm.dataStructure.Print();
    std::cout << fm.find("中院").first << '\n';
    fm.del("中院");
    std::cout << fm.find("中院").second << '\n';
}

int main() {
    //freopen("../data/basic_2/test.in", "r", stdin);
    //FILEOPEN("6");
    CORE();
    //TestFM();
    //TestBlockList();
    return 0;
}
