//
// Created by SiriusNEO on 2021/4/26.
//

#ifndef TICKETSYSTEM_2021_MAIN_SYSTEMCORE_HPP
#define TICKETSYSTEM_2021_MAIN_SYSTEMCORE_HPP

#include "cmdprocessor.hpp"
#include "../db/filemanager.hpp"

namespace Sirius {
    constexpr int HashSeed = 131;

    enum orderStatusType {SUCCESS, PENDING, REFUNDED};

    class System {

    private:
    /*  User  */
    struct User {
        pwdType password;
        uNameType name;
        addrType mailAddr;
        int privilege;
    };
    FileManager<uidType, User> userDatabase; //uid -> user

    /* Train
     * 0 -> 1 -> 2 -> 3 -> ... -> n
     * for route 1->2->3, the total price is: price[2]+price[3], that is: priceSum[3] - priceSum[1]
     * the min seat is: min(seat[1], seat[2]), that is: querySeat(1, 3-1)
     * the total time is: arriving[3] - leaving[1]
    */

    struct Train {
        tidType trainID;
        int stationNum;
        staNameType stations[StationNum_Max]; //0-based
        int totalSeatNum, priceSum[StationNum_Max];
                                                //price i-1->i, seat i->i+1
                                                //price[0]: ? -> 0, === 0
        TimeType startTime, arrivingTimes[StationNum_Max], leavingTimes[StationNum_Max], startSaleDate, endSaleDate;
                                                //arrive / leave i, 0-based
                                                //arrive[0] === 0, leaving[0] = startTime, leaving[final] === Int_Max，保证起点不做终点站，终点不做起点站
        char type;
        bool isReleased;
    };
    FileManager<tidType, Train> trainDatabase; //tid -> train

    struct DayTrain { //某一天发站的 trainID 火车上的座位情况. 优化：线段树
        int seatNum[StationNum_Max];
        int querySeat(int l, int r) {
            int ret = Int_Max;
            for (int i = l; i <= r; ++i) ret = std::min(ret, seatNum[i]);
            return ret;
        }
        void modifySeat(int l, int r, int val) {
            for (int i = l; i <= r; ++i) seatNum[i] += val;
        }
    };
    FileManager<std::pair<TimeType, tidType>, DayTrain> dayTrainDatabase; //(startDay, tid) -> dayTrain

    struct Station { //属于某个车次的站
        tidType trainID;
        int index{}, priceSum{}; //方便查座位用
        TimeType startSaleDate, endSaleDate, arrivingTime, leavingTime; //精简版信息，不用去查 trainDatabase
        Station() = default;
    };
    FileManager<std::pair<staNameType, tidType>, Station> stationDatabase; //(staName, tid) -> 特定车次的 station

    struct Ticket {
        tidType trainID;
        int s, t, time, cost;
        Ticket():trainID(), s(0), t(0), time(Int_Max), cost(Int_Max){}
        Ticket(tidType _trainID, int _s, int _t, int _time, int _cost):trainID(_trainID), s(_s), t(_t), time(_time), cost(_cost){}
    };
    static bool timeCmp(const Ticket& obj1, const Ticket& obj2) {return (obj1.time == obj2.time) ? obj1.trainID < obj2.trainID : obj1.time < obj2.time;}
    static bool costCmp(const Ticket& obj1, const Ticket& obj2) {return (obj1.cost == obj2.cost) ? obj1.trainID < obj2.trainID : obj1.cost < obj2.cost;}

    /* Order */
    struct Order {
        tidType trainID;
        uidType userID;
        int fromIndex, toIndex;
        staNameType from, to;
        TimeType startDay, leavingTime, arrivingTime;
        int orderID, price, num;
        orderStatusType status;
    };
    FileManager<std::pair<uidType, int>, Order> orderDatabase; // (uid, oid) -> order
    FileManager<std::pair<std::pair<TimeType, tidType>, int>, Order > orderQueue;// (startDay, tid, oid) -> order

    Sirius::HashTable<UserID_Max, UserNum_Max, HashSeed> loggedUser; //a set based on HashTable

    std::string (System::*Interfaces[CmdTypeNum_Max])(const cmdType&) = {&System::add_user, &System::login, &System::logout, &System::query_profile, &System::modify_profile,
                                                                 &System::add_train, &System::release_train, &System::query_train, &System::delete_train, &System::query_ticket,
                                                                 &System::query_transfer, &System::buy_ticket, &System::query_order, &System::refund_ticket, &System::clean,
                                                                 &System::exit
                                                                 };

    public:
        System():userDatabase("user.bin"), loggedUser(),trainDatabase("train.bin"), dayTrainDatabase("daytrain.bin"), stationDatabase("station.bin"),
                 orderDatabase("order.bin"), orderQueue("queue.bin"){}

        std::pair<bool, std::string> response(const std::string &cmdStr) { // false::quit
            auto info = parse(cmdStr);
            if (info.second) {
                std::string result = (this->* Interfaces[info.first.cmdNo])(info.first);
                return std::make_pair(result != "bye", result);
            }
            return std::make_pair(false, "parse error!");
        }

        std::string add_user(const cmdType& info) {
            if (info.argNum != 6) return "-1";
            uidType curUserID = info.args['c'-'a'], targetUserID = info.args['u'-'a'];
            if (userDatabase.size()) {
                if (!loggedUser.find(curUserID)) return "-1";
                auto curUser = userDatabase.find(curUserID);
                if (curUser.first.privilege <= stringToInt(info.args['g'-'a'])) return "-1";
                if (userDatabase.find(targetUserID).second) return "-1";
                userDatabase.insert(targetUserID, (User){info.args['p'-'a'], info.args['n'-'a'], info.args['m'-'a'], stringToInt(info.args['g'-'a'])});
                return "0";
            }
            userDatabase.insert(targetUserID, (User){info.args['p'-'a'], info.args['n'-'a'], info.args['m'-'a'], 10});
            return "0";
        }

        std::string login(const cmdType& info) {
            if (info.argNum != 2) return "-1";
            auto targetUser = userDatabase.find(info.args['u'-'a']);
            if (!targetUser.second || loggedUser.find(info.args['u'-'a'])) return "-1";
            if (targetUser.first.password != FixedStr<Password_Max>(info.args['p'-'a'])) return "-1";
            loggedUser.insert(info.args['u'-'a']);
            return "0";
        }

        std::string logout(const cmdType& info) {
            if (info.argNum != 1) return "-1";
            if (!loggedUser.find(info.args['u'-'a'])) return "-1";
            loggedUser.del(info.args['u'-'a']);
            return "0";
        }

        std::string query_profile(const cmdType& info) {
            if (info.argNum != 2) return "-1";
            uidType curUserID = info.args['c'-'a'];
            if (!loggedUser.find(curUserID)) return "-1";
            auto curUser = userDatabase.find(curUserID);
            auto targetUser = userDatabase.find(info.args['u'-'a']);
            if (!targetUser.second) return "-1";
            if (curUser.first.privilege <= targetUser.first.privilege && info.args['c'-'a'] != info.args['u'-'a']) return "-1";

            return info.args['u'-'a'] + " " + std::string(targetUser.first.name.str) + " " + std::string(targetUser.first.mailAddr.str) + " " + intToString(targetUser.first.privilege);
        }

        std::string modify_profile(const cmdType& info) {
            if (info.argNum < 2 || info.argNum > 6) return "-1";
            if (!loggedUser.find(info.args['c'-'a'])) return "-1";
            auto curUser = userDatabase.find(info.args['c'-'a']);
            uidType targetID = info.args['u'-'a'];
            auto targetUser = userDatabase.find(targetID);
            if (!targetUser.second) return "-1";
            if (curUser.first.privilege <= targetUser.first.privilege && info.args['c'-'a'] != info.args['u'-'a']) return "-1"; //权限大于或是同名，取反变成与
            if (stringToInt(info.args['g'-'a']) >= curUser.first.privilege) return "-1"; //-g 低于 -c

            auto oldPassword = (info.args['p'-'a'].empty()) ? targetUser.first.password : info.args['p'-'a'];
            auto oldName = (info.args['n'-'a'].empty()) ? targetUser.first.name : info.args['n'-'a'];
            auto oldMailAddr = (info.args['m'-'a'].empty()) ? targetUser.first.mailAddr : info.args['m'-'a'];
            auto oldPrivilege = (info.args['g'-'a'].empty()) ? targetUser.first.privilege : stringToInt(info.args['g'-'a']);

            userDatabase.modify(targetID, (User){oldPassword, oldName, oldMailAddr, oldPrivilege});
            return info.args['u'-'a'] + " " + std::string(oldName.str) + " " + std::string(oldMailAddr.str) + " " + intToString(oldPrivilege);
        }

        std::string add_train(const cmdType& info) {
            if (info.argNum != 10) return "-1";
            tidType id = info.args['i'-'a'];
            if (trainDatabase.find(id).second) return "-1";

            Train newTrain = (Train){id, stringToInt(info.args['n'-'a'])};
            newTrain.totalSeatNum = stringToInt(info.args['m'-'a']);
            int tempStorageNum = 0;
            std::string tempStorage1[StationNum_Max], tempStorage2[StationNum_Max];
            split(info.args['s'-'a'], tempStorage1, tempStorageNum, '|');
            for (int i = 0; i < tempStorageNum; ++i) newTrain.stations[i] = tempStorage1[i];
            split(info.args['p'-'a'], tempStorage1, tempStorageNum, '|');
            for (int i = 1; i <= tempStorageNum; ++i)
                newTrain.priceSum[i] = stringToInt(tempStorage1[i-1]) + newTrain.priceSum[i-1];

            newTrain.startTime = "01-01 " + info.args['x'-'a'];
            split(info.args['o'-'a'], tempStorage2, tempStorageNum, '|');
            split(info.args['t'-'a'], tempStorage1, tempStorageNum, '|');
            for (int i = 0; i < newTrain.stationNum; ++i) {
                if (i > 0) newTrain.arrivingTimes[i] = newTrain.leavingTimes[i-1] + stringToInt(tempStorage1[i-1]);
                if (i < newTrain.stationNum-1) {
                    if (i > 0) newTrain.leavingTimes[i] = newTrain.arrivingTimes[i] + stringToInt(tempStorage2[i-1]);
                    else newTrain.leavingTimes[0] = newTrain.startTime;
                }
                else newTrain.leavingTimes[i] = Int_Max;
            }
            split(info.args['d'-'a'], tempStorage1, tempStorageNum, '|');
            newTrain.startSaleDate = tempStorage1[0] + " 00:00", newTrain.endSaleDate = tempStorage1[1] + " 00:00";
            newTrain.type = info.args['y'-'a'][0];
            trainDatabase.insert(newTrain.trainID, newTrain);
            return "0";
        }

        std::string release_train(const cmdType& info) {
            if (info.argNum != 1) return "-1";
            tidType id = info.args['i'-'a'];
            auto targetTrain = trainDatabase.find(id);
            if (!targetTrain.second || targetTrain.first.isReleased) return "-1";
            for (auto i = targetTrain.first.startSaleDate; i <= targetTrain.first.endSaleDate; i += 24*60) {
                DayTrain dayTrain{};
                for (int j = 0; j < targetTrain.first.stationNum; ++j) dayTrain.seatNum[j] = targetTrain.first.totalSeatNum;
                dayTrainDatabase.insert(std::make_pair(i, id), dayTrain);
            }
            for (int i = 0; i < targetTrain.first.stationNum; ++i) {
                 stationDatabase.insert(std::make_pair(targetTrain.first.stations[i], id),
                 (Station){id, i, targetTrain.first.priceSum[i], targetTrain.first.startSaleDate, targetTrain.first.endSaleDate, targetTrain.first.arrivingTimes[i], targetTrain.first.leavingTimes[i]});
            }
            targetTrain.first.isReleased = true;
            trainDatabase.modify(id, targetTrain.first);
            return "0";
        }

        std::string query_train(const cmdType& info) {
            if (info.argNum != 2) return "-1";
            tidType id = info.args['i'-'a'];
            auto targetTrain = trainDatabase.find(id);
            TimeType day(info.args['d'-'a'] + " 00:00");

            if (!targetTrain.second) return "-1";
            if (!(targetTrain.first.startSaleDate <= day && day <= targetTrain.first.endSaleDate)) return "-1";
            auto dayTrain = dayTrainDatabase.find(std::make_pair(day, id));
            std::string ret = std::string(targetTrain.first.trainID.str) + " " + targetTrain.first.type + "\n";
            for (int i = 0; i < targetTrain.first.stationNum; ++i) {
                ret += std::string(targetTrain.first.stations[i].str) + " ";
                if (i == 0) {
                    ret += "xx-xx xx:xx -> " + (day+targetTrain.first.leavingTimes[0]).toFormatString() + " 0 ";
                    if (!targetTrain.first.isReleased) ret += intToString(targetTrain.first.totalSeatNum) + "\n";
                    else ret += intToString(dayTrain.first.seatNum[0]) + "\n";
                }
                else if (i == targetTrain.first.stationNum-1){
                    ret += (day+targetTrain.first.arrivingTimes[i]).toFormatString() + " -> xx-xx xx:xx " + intToString(targetTrain.first.priceSum[i]) + " x";
                }
                else {
                   ret += (day+targetTrain.first.arrivingTimes[i]).toFormatString() + " -> " + (day+targetTrain.first.leavingTimes[i]).toFormatString() + " " + intToString(targetTrain.first.priceSum[i]) + " ";
                   if (!targetTrain.first.isReleased) ret += intToString(targetTrain.first.totalSeatNum) + "\n";
                   else ret += intToString(dayTrain.first.seatNum[i]) + "\n";
                }
            }
            return ret;
        }

        std::string delete_train(const cmdType& info) {
            if (info.argNum != 1) return "-1";
            tidType id = info.args['i'-'a'];
            auto targetTrain = trainDatabase.find(id);
            if (!targetTrain.second || targetTrain.first.isReleased) return "-1";
            trainDatabase.del(id);
            return "0";
        }

        std::string query_ticket(const cmdType& info) {
            if (info.argNum < 3 || info.argNum > 4) return "-1";
            TimeType day = info.args['d'-'a'] + " 00:00";
            staNameType s = info.args['s'-'a'], t = info.args['t'-'a'];
            auto sList = stationDatabase.rangeFind(std::make_pair(s, ""), std::make_pair(s, TrainIDStr_Max));
            auto tList = stationDatabase.rangeFind(std::make_pair(t, ""), std::make_pair(t, TrainIDStr_Max));
            if (sList.empty() || tList.empty()) return "0";
            auto si = sList.begin(), ti = tList.begin();
            Ticket tickets[trainDatabase.size()+2];
            int ticketCnt = 0;
            while (si != sList.end() && ti != tList.end()) {
                if (si->trainID < ti->trainID) si++;
                else if (si->trainID > ti->trainID) ti++;
                else {
                    TimeType startDay = day - si->leavingTime.getDate(); //要在day这一天上车，对应的发站时间
                    if (si->startSaleDate <= startDay && startDay <= ti->endSaleDate && si->leavingTime < ti->arrivingTime && si->index < ti->index)
                        //售卖时间范围内每天都有车.同一辆车，arr和lea可以直接比. 比两个更鲁棒
                        tickets[ticketCnt++] = Ticket(si->trainID, si->index,ti->index, ti->arrivingTime-si->leavingTime, ti->priceSum-si->priceSum);
                    si++, ti++;
                }
            }
            if (!ticketCnt) return "0";
            if (info.argNum == 4 && info.args['p'-'a'] == "cost") qsort(tickets, tickets+ticketCnt-1, costCmp);
            else qsort(tickets, tickets+ticketCnt-1, timeCmp);
            std::string ret = intToString(ticketCnt);
            for (int i = 0; i < ticketCnt; ++i) {
                auto train = trainDatabase.find(tickets[i].trainID);
                TimeType startDay = day - train.first.leavingTimes[tickets[i].s].getDate();
                auto dayTrain = dayTrainDatabase.find(std::make_pair(startDay, tickets[i].trainID));
                std::string from = train.first.stations[tickets[i].s].str, to = train.first.stations[tickets[i].t].str,
                            lea = (startDay + train.first.leavingTimes[tickets[i].s]).toFormatString(),
                            arr = (startDay + train.first.arrivingTimes[tickets[i].t]).toFormatString();
                std::string seat = intToString(dayTrain.first.querySeat(tickets[i].s, tickets[i].t-1));
                ret += "\n" + std::string(tickets[i].trainID.str) + " " + from + " " + lea + " -> " + to + " " + arr + " " + intToString(tickets[i].cost) + " " + seat;
            }
            return ret;
        }

        std::string query_transfer(const cmdType& info) {
            if (info.argNum < 3 || info.argNum > 4) return "-1";
            TimeType day = info.args['d'-'a'] + " 00:00";
            staNameType s = info.args['s'-'a'], t = info.args['t'-'a'];
            auto sList = stationDatabase.rangeFind(std::make_pair(s, ""), std::make_pair(s, TrainIDStr_Max));
            auto tList = stationDatabase.rangeFind(std::make_pair(t, ""), std::make_pair(t, TrainIDStr_Max));
            if (sList.empty() || tList.empty()) return "0";
            int ans = Int_Max, firstTime = Int_Max;
            std::string ret;
            for (auto si : sList) {
                TimeType startDay1 = day - si.leavingTime.getDate();
                if (!(si.startSaleDate <= startDay1 && startDay1 <= si.endSaleDate)) continue;
                auto trainS = trainDatabase.find(si.trainID);
                for (auto ti : tList) {
                    if (ti.trainID == si.trainID) continue;
                    auto trainT = trainDatabase.find(ti.trainID);
                    if (ti.endSaleDate < day) continue; //出发后第二车就已经无票
                    for (int k = si.index + 1; k < trainS.first.stationNum; ++k)
                        for (int l = 0; l < ti.index; ++l) {
                            if (trainS.first.stations[k] == trainT.first.stations[l]) {
                                TimeType fastestStartDay2;
                                if (trainS.first.arrivingTimes[k].getClock() <= trainT.first.leavingTimes[l].getClock())
                                    fastestStartDay2 = (startDay1 + trainS.first.arrivingTimes[k]).getDate() - trainT.first.leavingTimes[l].getDate();
                                else
                                    fastestStartDay2 = (startDay1 + trainS.first.arrivingTimes[k]).getDate() + 24*60 - trainT.first.leavingTimes[l].getDate();
                                //第一辆车发车时间，第二辆车最快发车时间（保证第二辆车 上车时间为第一辆车到达当天）
                                if (ti.endSaleDate < fastestStartDay2) continue; //最快还是赶不上第二辆车卖完，不行
                                TimeType startDay2 = std::max(fastestStartDay2, ti.startSaleDate); //如果能最快发车就最快，否则从第二辆车第一次发车就上车
                                bool updated = false;
                                if (info.argNum == 4 && info.args['p' - 'a'] == "cost") {
                                    if ((ans > trainS.first.priceSum[k] - si.priceSum + ti.priceSum - trainT.first.priceSum[l]) ||
                                    (ans == trainS.first.priceSum[k] - si.priceSum + ti.priceSum - trainT.first.priceSum[l] && firstTime > trainS.first.arrivingTimes[k] - si.leavingTime)) {
                                        ans = trainS.first.priceSum[k] - si.priceSum + ti.priceSum - trainT.first.priceSum[l];
                                        firstTime = trainS.first.arrivingTimes[k] - si.leavingTime;
                                        updated = true;
                                    }
                                }
                                else if (ans > (startDay2 + ti.arrivingTime) - (startDay1 + si.leavingTime) ||
                                (ans == (startDay2 + ti.arrivingTime) - (startDay1 + si.leavingTime) && firstTime > trainS.first.arrivingTimes[k] - si.leavingTime)) {
                                    ans = (startDay2 + ti.arrivingTime) - (startDay1 + si.leavingTime);
                                    firstTime = trainS.first.arrivingTimes[k] - si.leavingTime;
                                    updated = true;
                                }
                                if (updated) {
                                    ret.clear();
                                    auto dayTrainS = dayTrainDatabase.find(std::make_pair(startDay1, trainS.first.trainID));
                                    auto dayTrainT = dayTrainDatabase.find(std::make_pair(startDay2, trainT.first.trainID));
                                    ret += std::string(si.trainID.str) + " " + std::string(trainS.first.stations[si.index].str) + " " + (startDay1 + si.leavingTime).toFormatString()
                                           + " -> " + std::string(trainS.first.stations[k].str) + " " + (startDay1 + trainS.first.arrivingTimes[k]).toFormatString() + " " +
                                           intToString(trainS.first.priceSum[k] - si.priceSum) + " " + intToString(dayTrainS.first.querySeat(si.index, k - 1)) + "\n";
                                    ret += std::string(trainT.first.trainID.str) + " " + std::string(trainT.first.stations[l].str) + " " + (startDay2 + trainT.first.leavingTimes[l]).toFormatString()
                                           + " -> " + std::string(trainT.first.stations[ti.index].str) + " " + (startDay2 + ti.arrivingTime).toFormatString() + " " +
                                           intToString(ti.priceSum - trainT.first.priceSum[l]) + " " + intToString(dayTrainT.first.querySeat(l, ti.index - 1));
                                }
                            }
                        }
                }
            }
            return (ans == Int_Max) ? "0" : ret;
        }

        std::string buy_ticket(const cmdType& info) {
            if (info.argNum < 6 || info.argNum > 7) return "-1";
            uidType uid = info.args['u'-'a'];
            if (!loggedUser.find(uid)) return "-1";
            TimeType day = info.args['d'-'a'] + " 00:00";
            tidType id = info.args['i'-'a'];
            auto train = trainDatabase.find(id);
            int buyNum = stringToInt(info.args['n'-'a']);
            if (!train.first.isReleased || buyNum > train.first.totalSeatNum) return "-1";
            int f = -1, t = -1;
            for (int i = 0; i < train.first.stationNum && (f == -1 || t == -1); ++i) {
                if (train.first.stations[i] == info.args['f'-'a']) f = i;
                else if (train.first.stations[i] == info.args['t'-'a']) t = i;
            }
            if (f == -1 || t == -1 || f >= t) return "-1";
            TimeType startDay = day - train.first.leavingTimes[f].getDate();
            if (!(train.first.startSaleDate <= startDay && startDay <= train.first.endSaleDate)) return "-1";
            auto dayTrain = dayTrainDatabase.find(std::make_pair(startDay, id));
            int remainSeat = dayTrain.first.querySeat(f, t-1);
            if ((info.argNum != 7 || info.args['q'-'a'] == "false") && remainSeat < buyNum) return "-1";
            int price = train.first.priceSum[t]-train.first.priceSum[f], oid = orderDatabase.size();
            Order order = (Order){id, uid, f, t, train.first.stations[f], train.first.stations[t], startDay, train.first.leavingTimes[f], train.first.arrivingTimes[t], oid, price, buyNum};
            if (remainSeat >= buyNum) {
                dayTrain.first.modifySeat(f, t-1, -buyNum);
                dayTrainDatabase.modify(std::make_pair(startDay, id), dayTrain.first);
                order.status = SUCCESS;
                orderDatabase.insert(std::make_pair(uid, oid), order);
                return intToString(price*buyNum);
            }
            order.status = PENDING;
            orderDatabase.insert(std::make_pair(uid, oid), order);
            orderQueue.insert(std::make_pair(std::make_pair(startDay, train.first.trainID), oid), order);
            return "queue";
        }

        std::string query_order(const cmdType& info) {
            if (info.argNum != 1) return "-1";
            std::string ret;
            uidType uid = info.args['u'-'a'];
            if (!loggedUser.find(uid)) return "-1";
            auto orders = orderDatabase.rangeFind(std::make_pair(uid, 0), std::make_pair(uid, Int_Max));
            if (orders.empty()) return "0";
            int orderCnt = 0;
            ret += intToString(orders.size()) + "\n";
            for (auto it = orders.rbegin(); it != orders.rend(); it++) {
                switch (it->status) {
                    case SUCCESS:ret += "[success] ";break;
                    case PENDING:ret += "[pending] ";break;
                    case REFUNDED:ret += "[refunded] ";
                }
                ret += std::string(it->trainID.str) + " " + std::string(it->from.str) + " " + (it->startDay+it->leavingTime).toFormatString() + " -> "
                        + std::string(it->to.str) + " " + (it->startDay+it->arrivingTime).toFormatString() + " " + intToString(it->price) + " " + intToString(it->num);
                ++orderCnt;
                if (orderCnt != orders.size()) ret += "\n";
            }
            return ret;
        }

        std::string refund_ticket(const cmdType& info) {
            if (info.argNum < 1 || info.argNum > 2) return "-1";
            auto uid = info.args['u'-'a'];
            if (!loggedUser.find(uid)) return "-1";
            auto orders = orderDatabase.rangeFind(std::make_pair(uid, 0), std::make_pair(uid, Int_Max));
            int n = (info.args['n'-'a'].empty()) ? 1 : stringToInt(info.args['n'-'a']);
            if (n > orders.size()) return "-1";
            auto it = orders.end() - n;
            if (it->status == REFUNDED) return "-1";
            auto oldStatus = it->status;
            it->status = REFUNDED;
            orderDatabase.modify(std::make_pair(uid, it->orderID), *it);
            if (oldStatus == PENDING) {
                orderQueue.del(std::make_pair(std::make_pair(it->startDay, it->trainID), it->orderID));
                return "0";
            }
            auto dayTrain = dayTrainDatabase.find(std::make_pair(it->startDay, it->trainID));
            dayTrain.first.modifySeat(it->fromIndex, it->toIndex-1, it->num);
            auto refundOrders = orderQueue.rangeFind(std::make_pair(std::make_pair(it->startDay, it->trainID), 0), std::make_pair(std::make_pair(it->startDay, it->trainID), Int_Max));
            for (auto& i : refundOrders) {
                if (i.fromIndex > it->toIndex || i.toIndex < it->fromIndex) continue;
                if (dayTrain.first.querySeat(i.fromIndex, i.toIndex-1) >= i.num) {
                    dayTrain.first.modifySeat(i.fromIndex, i.toIndex-1, -i.num);
                    i.status = SUCCESS;
                    orderQueue.del(std::make_pair(std::make_pair(i.startDay, i.trainID), i.orderID));
                    orderDatabase.modify(std::make_pair(i.userID, i.orderID), i);
                }
            }
            dayTrainDatabase.modify(std::make_pair(it->startDay, it->trainID), dayTrain.first);
            return "0";
        }

        std::string clean(const cmdType& info) {
            /*
                loggedUser.clear();
                userDatabase.clear();
                trainDatabase.clear();
                dayTrainDatabase.clear();
                stationDatabase.clear();
                orderDatabase.clear();
                orderQueue.clear();
            */
            return "0";
        }
        std::string exit(const cmdType& info) {
            return "bye";
        }
    };
}

#endif //TICKETSYSTEM_2021_MAIN_SYSTEMCORE_HPP
