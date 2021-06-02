//
// Created by SiriusNEO on 2021/4/26.
//

#ifndef TICKETSYSTEM_2021_MAIN_CMDPROCESSOR_HPP
#define TICKETSYSTEM_2021_MAIN_CMDPROCESSOR_HPP

#include "../lib/mytools.hpp"
#include "../lib/timetype.hpp"
#include "../lib/bst.hpp"
#include <algorithm>

namespace Sirius {
    constexpr int Argc_Max = 24, CmdTypeNum_Max = 17;
    constexpr int UserID_Max = 21, Password_Max = 31, Name_Max = 16, MailAddr_Max = 31, UserNum_Max = 5000321; //Username = UserID
    constexpr int TrainID_Max = 21, StationNum_Max = 101, StationName_Max = 31;
    constexpr int Pool_Max = 10005;

    typedef FixedStr<UserID_Max> uidType;
    typedef FixedStr<Password_Max> pwdType;
    typedef FixedStr<Name_Max> uNameType;
    typedef FixedStr<MailAddr_Max> addrType;
    typedef FixedStr<TrainID_Max> tidType;
    typedef FixedStr<StationName_Max> staNameType;

    const tidType TrainIDStr_Max = "~~~~~~~~~~~~~~~~~~~~";

    const std::string CMD[CmdTypeNum_Max] = {"add_user", "login", "logout", "query_profile", "modify_profile", "add_train",
                                            "release_train", "query_train", "delete_train", "query_ticket", "query_transfer",
                                            "buy_ticket", "query_order", "refund_ticket", "clean", "exit", "query_privilege"
                                            };

    struct cmdType {
        int cmdNo, argNum;
        std::string args[26];
        cmdType() : cmdNo(0), argNum(0), args() {}
    };

    //翻译命令字符串，以及简单的合法性检查
    std::pair<cmdType, bool> parse(const std::string& cmdStr) {
        cmdType ret;
        int argc = 0, len = cmdStr.size();
        bool valid = false;
        std::string argv[Argc_Max];
        while (cmdStr[len-1] == ' ' || cmdStr[len-1] == '\r' || cmdStr[len-1] == '\n') --len; //过滤尾部无用字符
        split(cmdStr.substr(0, len), argv, argc, ' ');
        for (int i = 0; i < CmdTypeNum_Max; ++i) { //找到对应函数的编号
            if (argv[0] == CMD[i]) {
                ret.cmdNo = i; ret.argNum = ((argc-1)>>1);
                valid = true;
                break;
            }
        }
        if (!(argc & 1)) valid = false;
        for (int i = 1; i < argc; i += 2) { //从字符串中记录参数
            if (argv[i][0] != '-' || argv[i].size() != 2 || argv[i][1] < 'a' || argv[i][1] > 'z') valid = false;
            else ret.args[int(argv[i][1]-'a')] = argv[i+1];
        }
        return std::make_pair(ret, valid);
    }
}

#endif //TICKETSYSTEM_2021_MAIN_CMDPROCESSOR_HPP
