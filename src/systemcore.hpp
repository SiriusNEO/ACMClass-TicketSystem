//
// Created by SiriusNEO on 2021/4/26.
//

#ifndef TICKETSYSTEM_2021_MAIN_SYSTEMCORE_HPP
#define TICKETSYSTEM_2021_MAIN_SYSTEMCORE_HPP

#include "cmdprocessor.hpp"
#include <map> //for test

namespace Sirius {
    constexpr int HashSeed = 131;

    class System {

    #define database std::map

    private:
    struct User {
        FixedStr<Username_Max> username;
        FixedStr<Password_Max> password;
        FixedStr<Name_Max> name;
        FixedStr<MailAddr_Max> mailAddr;
        int privilege;
    };

    struct Train {
        FixedStr<TrainID_Max> trainID;
        int stationNum;
        FixedStr<Stationname_Max> stations[StationNum_Max];
        int seatNum, prices[StationNum_Max];
        TimeType startTime, travelTimes[StationNum_Max], stopoverTimes[StationNum_Max];
        char type;
    };
    database<FixedStr<Username_Max>, User> userDatabase;
    Sirius::HashTable<Username_Max, UserNum_Max, HashSeed> loggedUser; //as a set, the second para is no used.
    int (System::*Interfaces[CmdTypeNum_Max])(const cmdType&) = {&System::add_user, &System::login, &System::logout, &System::query_profile, &System::modify_profile,
                                                                 &System::add_train, &System::release_train, &System::query_train, &System::delete_train, &System::query_ticket,
                                                                 &System::query_transfer, &System::buy_ticket, &System::query_order, &System::refund_ticket, &System::clean,
                                                                 &System::exit
                                                                 };

    public:
        System():userDatabase(), loggedUser(){}

        bool response(const std::string &cmdStr) { // false::quit
            auto info = parse(cmdStr);
            if (info.second) {
                int result = (this->* Interfaces[info.first.cmdNo])(info.first); //0 1 -1 2
                if (result == 0 || result == -1) printf("%d\n", result);
                return result != 2;
            }
            return false;
        }

        int add_user(const cmdType& info) {
            if (info.argNum != 6) return -1;
            if (!userDatabase.empty()) {
                if (!loggedUser.find(info.args['c'-'a'])) return -1;
                auto curUser = userDatabase.find(info.args['c'-'a']);
                if (curUser->second.privilege <= stringToInt(info.args['g'-'a'])) return -1;
                userDatabase.insert(std::make_pair(info.args['u'-'a'], (User){info.args['u'-'a'], info.args['p'-'a'], info.args['n'-'a'], info.args['m'-'a'], stringToInt(info.args['g'-'a'])}));
                return 0;
            }
            userDatabase.insert(std::make_pair(info.args['u'-'a'], (User){info.args['u'-'a'], info.args['p'-'a'], info.args['n'-'a'], info.args['m'-'a'], 10}));
            return 0;
        }

        int login(const cmdType& info) {
            if (info.argNum != 2) return -1;
            auto targetUser = userDatabase.find(info.args['u'-'a']);
            if (targetUser == userDatabase.end() || loggedUser.find(info.args['u'-'a'])) return -1;
            if (targetUser->second.password != FixedStr<Password_Max>(info.args['p'-'a'])) return -1;
            loggedUser.insert(info.args['u'-'a']);
            return 0;
        }

        int logout(const cmdType& info) {
            if (info.argNum != 1) return -1;
            if (!loggedUser.find(info.args['u'-'a'])) return -1;
            loggedUser.del(info.args['u'-'a']);
            return 0;
        }

        int query_profile(const cmdType& info) {
            if (info.argNum != 2) return -1;
            if (!loggedUser.find(info.args['c'-'a'])) return -1;
            auto curUser = userDatabase.find(info.args['c'-'a']);
            auto targetUser = userDatabase.find(info.args['u'-'a']);
            if (targetUser == userDatabase.end()) return -1;
            if (curUser->second.privilege <= targetUser->second.privilege && info.args['c'-'a'] != info.args['u'-'a']) return -1;

            printf("%s %s %s %d\n", targetUser->second.username.str, targetUser->second.name.str, targetUser->second.mailAddr.str, targetUser->second.privilege);
            return 1;
        }

        int modify_profile(const cmdType& info) {
            if (info.argNum < 2 || info.argNum > 6) return -1;
            if (!loggedUser.find(info.args['c'-'a'])) return -1;
            auto curUser = userDatabase.find(info.args['c'-'a']);
            auto targetUser = userDatabase.find(info.args['u'-'a']);
            if (targetUser == userDatabase.end()) return -1;
            if (curUser->second.privilege <= targetUser->second.privilege && info.args['c'-'a'] != info.args['u'-'a'] && stringToInt(info.args['g'-'a']) >= curUser->second.privilege) return -1;

            auto oldPassword = targetUser->second.password;
            auto oldName = targetUser->second.name;
            auto oldMailAddr = targetUser->second.mailAddr;
            auto oldPrivilege = targetUser->second.privilege;
            if (info.args['p'-'a'] != "") oldPassword = info.args['p'-'a'];
            if (info.args['n'-'a'] != "") oldName = info.args['n'-'a'];
            if (info.args['m'-'a'] != "") oldMailAddr = info.args['m'-'a'];
            if (info.args['g'-'a'] != "") oldPrivilege = stringToInt(info.args['g'-'a']);
            printf("%s %s %s %d\n", info.args['u'-'a'].c_str(), oldName.str, oldMailAddr.str, oldPrivilege);
            userDatabase.erase(targetUser);
            userDatabase.insert(std::make_pair(info.args['u'-'a'], (User){info.args['u'-'a'], oldPassword, oldName, oldMailAddr, oldPrivilege}));
            return 1;
        }

        int add_train(const cmdType& info) {
            //TODO
            return 0;
        }
        int release_train(const cmdType& info) {
            //TODO
            return 0;
        }
        int query_train(const cmdType& info) {
            //TODO
            return 0;
        }
        int delete_train(const cmdType& info) {
            //TODO
            return 0;
        }
        int query_ticket(const cmdType& info) {
            //TODO
            return 0;
        }
        int query_transfer(const cmdType& info) {
            //TODO
            return 0;
        }
        int buy_ticket(const cmdType& info) {
            //TODO
            return 0;
        }
        int query_order(const cmdType& info) {
            //TODO
            return 0;
        }
        int refund_ticket(const cmdType& info) {
            //TODO
            return 0;
        }

        int clean(const cmdType& info) {
            //TODO
            return 0;
        }
        int exit(const cmdType& info) {
            printf("bye\n");
            return 2;
        }
    };

}

#endif //TICKETSYSTEM_2021_MAIN_SYSTEMCORE_HPP
