//TicketSystem main.cpp for test

#include "systemcore.hpp"
#include <algorithm>
#include <set>
#include <assert.h>

int a[10000005], b[10000005];

int compare(const void *a,const void *b)
{
    return *(int*)b-*(int*)a;
}

int main() {
    /*
    std::cout << Sirius::stringToInt("666") << '\n';
    Sirius::TimeType test("09-30 15:38");
    auto tmp = Sirius::TimeType("01-01 02:30");
    test = test + tmp;
    std::cout << test.getDate().first << ' ' << test.getDate().second << '\n';
    MINE(test.toDateString())
    */

    freopen("../data/basic_1/1.in", "r", stdin);
    freopen("../data/basic_1/my.out", "w", stdout);

    Sirius::System ticketSystem;
    std::string cmd;
    while (getline(std::cin, cmd, '\n') && ticketSystem.response(cmd));

    /*
    srand(time(nullptr));
    Sirius::FixedStr<233> hello("Hello, TicketSystem!");
    std::cout << hello << '\n';
    */

    /*
    Sirius::HashTable<30, 1226959, 131> mySet;

    std::set<std::string> stdTest;
    clock_t st = clock();
    MINE("DataSet: 20000")
    for (int i = 1; i <= 20000; ++i) {
        std::string tmp = Sirius::randString(29);
        if (!mySet.find(tmp)) mySet.insert(tmp);
        else mySet.del(tmp);
    }
    printf("Sirius::HashTable: %.6lf\n", (clock()-st)/(double)CLOCKS_PER_SEC);

    st = clock();
    for (int i = 1; i <= 20000; ++i) {
        std::string tmp = Sirius::randString(29);
        if (stdTest.count(tmp) == 0) stdTest.insert(tmp);
        else stdTest.erase(tmp);
    }
    printf("std::set: %.6lf\n", (clock()-st)/(double)CLOCKS_PER_SEC);
    */

    /*
    std::string str;
    getline(std::cin, str, '\n');
    Sirius::parse(str);
    */

    /*
    for (int i = 1; i <= 10000000; ++i) a[i] = b[i] = rand();
    clock_t st = clock();
    Sirius::qsort<int>(a+1, a+10000000, [](const int& x, const int& y)->bool {return x < y;});
    //for (int i = 1; i <= 1000000; ++i) std::printf("%d ", a[i]);printf("%d\n");
    printf("Datasize: 10000000\nmy qsort: %.6lf\n", (clock()-st)/(double)CLOCKS_PER_SEC);
    st = clock();
    std::qsort(b, 10000000, sizeof(int), compare);
    printf("std::qsort: %.6lf\n", (clock()-st)/(double)CLOCKS_PER_SEC);
     */

    /*
    Sirius::System ticketSystem;
    ticketSystem.response("add_user -c cur -u I_am_the_admin -p awsl -n 奥斯卡 -m foo@bar.com -g 10");
    ticketSystem.response("login -u I_am_the_admin -p aswl");
    ticketSystem.response("login -u I_am_the_admin -p awsl");
    ticketSystem.response("login -u I_am_the_admin -p awsl");
    ticketSystem.response("query_profile -u I_am_the_admin -c I_am_the_admin");
    ticketSystem.response("modify_profile -c I_am_the_admin -u I_am_the_admin -p hahaha -m siriusneo@sjtu.edu.cn -g 5 -n 小小天狼星");
    ticketSystem.response("logout -u I_am_the_admin");
    ticketSystem.response("login -u I_am_the_admin -p awsl");
    ticketSystem.response("login -u I_am_the_admin -p hahaha");
    ticketSystem.response("query_profile -u I_am_the_admin -c I_am_the_admin");
    */

    /*
    ticketSystem.response("add_user -c cur -u I_am_the_admin -p awsl -n 奥斯卡 -m foo@bar.com -g 10");
    ticketSystem.response("login -u I_am_the_admin -p awsl");
    STANDINGBY()
    for (int i = 1; i <= 100000; ++i) {
        auto tmp = Sirius::randString(17);
        ticketSystem.response("add_user -c I_am_the_admin -u " + tmp + " -p awsl -n 奥斯卡 -m foo@bar.com -g 1");
        ticketSystem.response("login -u " + tmp + " -p awsl");
    }
    COMPLETE("User System with HashTable")
    */
    return 0;
}
