//
// Created by SiriusNEO on 2021/4/26.
//

#ifndef TICKETSYSTEM_2021_MAIN_SYSTEMCORE_HPP
#define TICKETSYSTEM_2021_MAIN_SYSTEMCORE_HPP

#include "cmdprocessor.hpp"
#include "../db/bpt.hpp"

namespace Sirius {
    enum orderStatusType {SUCCESS, PENDING, REFUNDED};

    class System {

    public:
        /*  User  */
        struct User {
            pwdType password;
            uNameType name;
            addrType mailAddr;
            int privilege;
        };
        Bptree<hashCode, User> userDatabase; //uid -> user
        Sirius::BinarySearchTree<hashCode> loggedUser;

        /* Train
         * 0 -> 1 -> 2 -> 3 -> ... -> n
         * for route 1->2->3, the total price is: price[2]+price[3], that is: priceSum[3] - priceSum[1]
         * the min seat is: min(seat[1], seat[2]), that is: querySeat(1, 3-1)
         * the total time is: arriving[3] - leaving[1]
         * arrive[0] === 0, leaving[0] = startTime, leaving[final] === Int_Max，保证起点不做终点站，终点不做起点站
        */
        struct Train {
            bool isReleased;
            tidType trainID;
            int stationNum;
            staNameType stations[StationNum_Max]; //0-based
            int totalSeatNum, priceSum[StationNum_Max];
            TimeType startTime, arrivingTimes[StationNum_Max], leavingTimes[StationNum_Max], startSaleDate, endSaleDate;
            char type;
        };
        Bptree<hashCode, Train> trainDatabase; //tid -> train

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
        Bptree<std::pair<TimeType, hashCode>, DayTrain> dayTrainDatabase; //(startDay, tid) -> dayTrain

        struct Station { //属于某个车次的站
            tidType trainID;
            staNameType station;
            hashCode tidHash;
            int index, priceSum; //方便查座位用
            TimeType startSaleDate, endSaleDate, arrivingTime, leavingTime; //精简版信息，不用去查 trainDatabase
            Station() = default;
        };
        Bptree<std::pair<hashCode, hashCode>, Station> stationDatabase; //(staName, tid) -> 特定车次的 station

        struct Ticket {
            Station s, t;
            Ticket() = default;
            Ticket(const Station& _s, const Station& _t):s(_s), t(_t){}
            inline int time() const {
                return t.arrivingTime - s.leavingTime;
            }
            inline int cost() const {
                return t.priceSum - s.priceSum;
            }
        };
        static bool timeCmp(const Ticket& obj1, const Ticket& obj2) {
            return (obj1.time() == obj2.time()) ? obj1.s.trainID < obj2.s.trainID : obj1.time() < obj2.time();
        }
        static bool costCmp(const Ticket& obj1, const Ticket& obj2) {
            return (obj1.cost() == obj2.cost()) ? obj1.s.trainID < obj2.s.trainID : obj1.cost() < obj2.cost();
        }
        typedef std::pair<staNameType, int> stationPair;
        static bool stationCmp(const stationPair& obj1, const stationPair& obj2) {return obj1.first < obj2.first;}

        /* Order */
        struct Order {
            orderStatusType status;
            tidType trainID;
            uidType userID;
            int fromIndex, toIndex;
            staNameType from, to;
            TimeType startDay, leavingTime, arrivingTime;
            int orderID, price, num;
        };
        struct PendingOrder {
            hashCode tidHash, uidHash;
            int fromIndex, toIndex, orderID, num;
            TimeType startDay;
        };
        Bptree<std::pair<hashCode, int>, Order> orderDatabase; // (uid, oid) -> order
        Bptree<std::pair<std::pair<TimeType, hashCode>, int>, PendingOrder> pendingQueue;// (startDay, tid, oid) -> order

        std::string (System::*Interfaces[CmdTypeNum_Max])(const cmdType&) = {&System::add_user, &System::login, &System::logout, &System::query_profile, &System::modify_profile,
                                                                     &System::add_train, &System::release_train, &System::query_train, &System::delete_train, &System::query_ticket,
                                                                     &System::query_transfer, &System::buy_ticket, &System::query_order, &System::refund_ticket, &System::clean,
                                                                     &System::exit, &System::query_privilege
        };
        Station sList[Pool_Max], tList[Pool_Max];
        Ticket tickets[Pool_Max];
        Order orders[Pool_Max];
        PendingOrder refundOrders[Pool_Max];

    public:
        System():userDatabase("user.bin", "user1.bin"), loggedUser(),trainDatabase("train.bin", "train1.bin"), dayTrainDatabase("daytrain.bin", "daytrain1.bin"),
                 stationDatabase("station.bin", "station1.bin"), orderDatabase("order.bin", "order1.bin"), pendingQueue("queue.bin", "queue1.bin"){}

        std::pair<bool, std::string> response(const std::string &cmdStr) { // false::quit
            auto info = parse(cmdStr);
            if (info.second) {
                std::string result = (this->* Interfaces[info.first.cmdNo])(info.first);
                return std::make_pair(result != "bye", result);
            }
            return std::make_pair(false, "parse_error");
        }

        std::string add_user(const cmdType& info) {
            if (info.argNum != 6) return "argnum_error";
            hashCode uidHash = hash(info.args['u'-'a'].c_str());
            if (userDatabase.size()) { //非第一次添加用户
                int curUserPriv = loggedUser.find(hash(info.args['c'-'a'].c_str()));
                if (curUserPriv == -1) return "not_logged"; //-c未登录
                int g = stringToInt(info.args['g'-'a']);
                if (curUserPriv <= g) return "privilege_error"; //-g权限大等于-c
                if (userDatabase.find(uidHash).second) return "uid_duplicate"; //id已有
                userDatabase.insert(uidHash, (User){info.args['p'-'a'], info.args['n'-'a'], info.args['m'-'a'], g});
                return "0";
            }
            //创建第一个用户，直接插入，权限为10
            userDatabase.insert(uidHash, (User){info.args['p'-'a'], info.args['n'-'a'], info.args['m'-'a'], 10});
            return "0";
        }

        std::string login(const cmdType& info) {
            if (info.argNum != 2) return "argnum_error";
            hashCode uidHash = hash(info.args['u'-'a'].c_str());
            auto targetUser = userDatabase.find(uidHash);
            if (!targetUser.second) return "user_not_found"; //无此用户、重复登陆
            if (loggedUser.find(uidHash) != -1) return "logged_duplicate\n"+std::string(targetUser.first.name.str);
            if (targetUser.first.password != pwdType(info.args['p'-'a'])) return "wrong_passwd"; //密码错误
            loggedUser.insert(uidHash, targetUser.first.privilege);
            return "0\n"+std::string(targetUser.first.name.str);
        }

        std::string logout(const cmdType& info) {
            if (info.argNum != 1) return "argnum_error";
            hashCode uidHash = hash(info.args['u'-'a'].c_str());
            if (loggedUser.find(uidHash) == -1) return "not_logged"; //未登录
            loggedUser.del(uidHash);
            return "0";
        }

        std::string query_profile(const cmdType& info) {
            if (info.argNum != 2) return "argnum_error";
            int curUserPriv = loggedUser.find(hash(info.args['c'-'a'].c_str()));
            if (curUserPriv == -1) return "not_logged"; //-c未登录
            auto targetUser = userDatabase.find(hash(info.args['u'-'a'].c_str()));
            if (!targetUser.second) return "user_not_found"; //-u 无此用户
            if (curUserPriv <= targetUser.first.privilege && info.args['c'-'a'] != info.args['u'-'a']) return "privilege_error";
            //-c权限小等于-u权限，且-c和-u不同
            return info.args['u'-'a'] + " " + std::string(targetUser.first.name.str) + " " + std::string(targetUser.first.mailAddr.str) + " " + std::to_string(targetUser.first.privilege);
        }

        std::string query_privilege(const cmdType& info) {
            if (info.argNum != 1) return "argnum_error";
            auto targetUser = userDatabase.find(hash(info.args['u'-'a'].c_str()));
            if (!targetUser.second) return "user_not_found"; //-u 无此用户
            return std::to_string(targetUser.first.privilege);
        }

        std::string modify_profile(const cmdType& info) {
            if (info.argNum < 2 || info.argNum > 6) return "argnum_error";
            int curUserPriv = loggedUser.find(hash(info.args['c'-'a'].c_str()));
            if (curUserPriv == -1) return "not_logged"; //-c 未登录
            hashCode uidHash = hash(info.args['u'-'a'].c_str());
            auto targetUser = userDatabase.find(uidHash);
            if (!targetUser.second) return "user_not_found"; //-u 无此用户
            if (curUserPriv <= targetUser.first.privilege && info.args['c'-'a'] != info.args['u'-'a']) return "privilege_error"; //权限大等于或是同名，取反变成与
            if (stringToInt(info.args['g'-'a']) >= curUserPriv) return "privilege_error"; //-g 低于 -c

            auto oldPassword = (info.args['p'-'a'].empty()) ? targetUser.first.password : info.args['p'-'a'];
            auto oldName = (info.args['n'-'a'].empty()) ? targetUser.first.name : info.args['n'-'a'];
            auto oldMailAddr = (info.args['m'-'a'].empty()) ? targetUser.first.mailAddr : info.args['m'-'a'];
            auto oldPrivilege = (info.args['g'-'a'].empty()) ? targetUser.first.privilege : stringToInt(info.args['g'-'a']);

            if (!info.args['g'-'a'].empty() && loggedUser.find(uidHash) != -1) loggedUser.insert(uidHash, oldPrivilege); //修改权限
            userDatabase.modify(uidHash, (User){oldPassword, oldName, oldMailAddr, oldPrivilege});
            return info.args['u'-'a'] + " " + std::string(oldName.str) + " " + std::string(oldMailAddr.str) + " " + std::to_string(oldPrivilege);
        }

        std::string add_train(const cmdType& info) {
            if (info.argNum != 10) return "argnum_error";
            tidType id = info.args['i'-'a'];
            hashCode idHash = hash(id.str);
            if (trainDatabase.find(idHash).second) return "tid_duplicate"; //tid已有

            Train newTrain = (Train){false, id, stringToInt(info.args['n'-'a'])};
            newTrain.totalSeatNum = stringToInt(info.args['m'-'a']);
            int tempStorageNum = 0;
            std::string tempStorage1[StationNum_Max], tempStorage2[StationNum_Max];
            split(info.args['s'-'a'], tempStorage1, tempStorageNum, '|');
            for (int i = 0; i < tempStorageNum; ++i) newTrain.stations[i] = tempStorage1[i];
            split(info.args['p'-'a'], tempStorage1, tempStorageNum, '|');
            for (int i = 1; i <= tempStorageNum; ++i)
                newTrain.priceSum[i] = stringToInt(tempStorage1[i-1]) + newTrain.priceSum[i-1];

            newTrain.startTime = "01-01 " + info.args['x'-'a'];
            split(info.args['o'-'a'], tempStorage2, tempStorageNum, '|'); //stopoverTime
            split(info.args['t'-'a'], tempStorage1, tempStorageNum, '|'); //travelTime
            for (int i = 0; i < newTrain.stationNum; ++i) {
                if (i > 0) newTrain.arrivingTimes[i] = newTrain.leavingTimes[i-1] + stringToInt(tempStorage1[i-1]);
                if (i < newTrain.stationNum-1) {
                    if (i > 0) newTrain.leavingTimes[i] = newTrain.arrivingTimes[i] + stringToInt(tempStorage2[i-1]); //只有两站，不会管
                    else newTrain.leavingTimes[0] = newTrain.startTime;
                }
                else newTrain.leavingTimes[i] = Int_Max; //终点站leavingTime无穷
            }
            split(info.args['d'-'a'], tempStorage1, tempStorageNum, '|');
            newTrain.startSaleDate = tempStorage1[0] + " 00:00", newTrain.endSaleDate = tempStorage1[1] + " 00:00";
            newTrain.type = info.args['y'-'a'][0];
            trainDatabase.insert(idHash, newTrain);
            return "0";
        }

        std::string release_train(const cmdType& info) {
            if (info.argNum != 1) return "argnum_error";
            tidType id = info.args['i'-'a'];
            hashCode idHash = hash(id.str);
            auto targetTrain = trainDatabase.find(idHash);
            if (!targetTrain.second) return "train_not_found";
            if (targetTrain.first.isReleased) return "has_released"; //找不到或已released
            for (auto i = targetTrain.first.startSaleDate; i <= targetTrain.first.endSaleDate; i += 24*60) {
                DayTrain dayTrain{};
                for (int j = 0; j < targetTrain.first.stationNum; ++j) dayTrain.seatNum[j] = targetTrain.first.totalSeatNum;
                dayTrainDatabase.insert(std::make_pair(i, idHash), dayTrain);
            }
            for (int i = 0; i < targetTrain.first.stationNum; ++i) {
                stationDatabase.insert(std::make_pair(hash(targetTrain.first.stations[i].str), idHash),
                                       (Station){id, targetTrain.first.stations[i], idHash, i, targetTrain.first.priceSum[i], targetTrain.first.startSaleDate, targetTrain.first.endSaleDate, targetTrain.first.arrivingTimes[i], targetTrain.first.leavingTimes[i]});
            }
            targetTrain.first.isReleased = true;
            trainDatabase.modify_info(idHash, true, 0);
            return "0";
        }

        std::string query_train(const cmdType& info) {
            if (info.argNum != 2) return "argnum_error";
            tidType id = info.args['i'-'a'];
            hashCode idHash = hash(id.str);
            auto targetTrain = trainDatabase.find(idHash);
            TimeType day(info.args['d'-'a'] + " 00:00");

            if (!targetTrain.second) return "train_not_found"; //无此车
            if (!(targetTrain.first.startSaleDate <= day && day <= targetTrain.first.endSaleDate)) return "day_error"; //这里的day是发车时间
            const auto& dayTrain = dayTrainDatabase.find(std::make_pair(day, idHash));
            std::string ret = std::string(targetTrain.first.trainID.str) + " " + targetTrain.first.type + " " + std::to_string(targetTrain.first.isReleased) + "\n";
            for (int i = 0; i < targetTrain.first.stationNum; ++i) {
                ret += std::string(targetTrain.first.stations[i].str) + " ";
                if (i == 0) {
                    ret += "xx-xx xx:xx -> " + (day+targetTrain.first.leavingTimes[0]).toFormatString() + " 0 ";
                    if (!targetTrain.first.isReleased) ret += std::to_string(targetTrain.first.totalSeatNum) + "\n";
                    else ret += std::to_string(dayTrain.first.seatNum[0]) + "\n";
                }
                else if (i == targetTrain.first.stationNum-1){
                    ret += (day+targetTrain.first.arrivingTimes[i]).toFormatString() + " -> xx-xx xx:xx " + std::to_string(targetTrain.first.priceSum[i]) + " x";
                }
                else {
                    ret += (day+targetTrain.first.arrivingTimes[i]).toFormatString() + " -> " + (day+targetTrain.first.leavingTimes[i]).toFormatString() + " " + std::to_string(targetTrain.first.priceSum[i]) + " ";
                    if (!targetTrain.first.isReleased) ret += std::to_string(targetTrain.first.totalSeatNum) + "\n";
                    else ret += std::to_string(dayTrain.first.seatNum[i]) + "\n";
                }
            }
            return ret;
        }

        std::string delete_train(const cmdType& info) {
            if (info.argNum != 1) return "argnum_error";
            tidType id = info.args['i'-'a'];
            hashCode idHash = hash(id.str);
            auto targetTrain = trainDatabase.find(idHash);
            if (!targetTrain.second) return "train_not_found";
            if (targetTrain.first.isReleased) return "has_released"; //无此车或已发行
            trainDatabase.erase(idHash);
            return "0";
        }

        std::string query_ticket(const cmdType& info) {
            if (info.argNum < 3 || info.argNum > 4) return "argnum_error";
            TimeType day = info.args['d'-'a'] + " 00:00";
            staNameType s = info.args['s'-'a'], t = info.args['t'-'a'];
            hashCode sHash = hash(s.str), tHash = hash(t.str);
            if (s == t) return "0"; //起终相同，直接判掉
            int sLen = 0, tLen = 0;
            stationDatabase.range_find(std::make_pair(sHash, 0), std::make_pair(sHash, LL_Max), sList, sLen);
            stationDatabase.range_find(std::make_pair(tHash, 0), std::make_pair(tHash, LL_Max), tList, tLen);
            if (!sLen || !tLen) return "0"; //无票
            auto si = sList, ti = tList;
            int ticketCnt = 0;
            while (si != sList+sLen && ti != tList+tLen) {
                if (si->tidHash < ti->tidHash) si++;
                else if (si->tidHash > ti->tidHash) ti++;
                else if (si->index >= ti->index) si++, ti++;
                else {
                    TimeType startDay = day - si->leavingTime.getDate(); //要在day这一天上车，对应的发站时间
                    if (si->startSaleDate <= startDay && startDay <= si->endSaleDate)
                        //售卖时间范围内每天都有车.同一辆车，arr和lea可以直接比. 比两个更鲁棒
                        tickets[ticketCnt++] = Ticket(*si, *ti);
                    si++, ti++;
                }
            }
            if (!ticketCnt) return "0";
            if (info.argNum == 4 && info.args['p'-'a'] == "cost") qsort(tickets, tickets+ticketCnt-1, costCmp);
            else qsort(tickets, tickets+ticketCnt-1, timeCmp);
            std::string ret = std::to_string(ticketCnt);
            for (int i = 0; i < ticketCnt; ++i) {
                hashCode idHash = hash(tickets[i].s.trainID.str);
                TimeType startDay = day - tickets[i].s.leavingTime.getDate();
                auto dayTrain = dayTrainDatabase.find(std::make_pair(startDay, idHash));
                std::string from = tickets[i].s.station.str, to = tickets[i].t.station.str,
                        lea = (startDay + tickets[i].s.leavingTime).toFormatString(),
                        arr = (startDay + tickets[i].t.arrivingTime).toFormatString();
                std::string seat = std::to_string(dayTrain.first.querySeat(tickets[i].s.index, tickets[i].t.index-1));
                ret += "\n" + std::string(tickets[i].s.trainID.str) + " " + from + " " + lea + " -> " + to + " " + arr + " " + std::to_string(tickets[i].cost()) + " " + seat;
            }
            return ret;
        }

        std::string query_transfer(const cmdType& info) {
            if (info.argNum < 3 || info.argNum > 4) return "argnum_error";
            TimeType day = info.args['d'-'a'] + " 00:00";
            staNameType s = info.args['s'-'a'], t = info.args['t'-'a'];
            hashCode sHash = hash(s.str), tHash = hash(t.str);
            if (s == t) return "0";
            int sLen = 0, tLen = 0;
            stationDatabase.range_find(std::make_pair(sHash, 0), std::make_pair(sHash, LL_Max), sList, sLen);
            stationDatabase.range_find(std::make_pair(tHash, 0), std::make_pair(tHash, LL_Max), tList, tLen);
            if (!sLen || !tLen) return "0"; //无票
            int ans = Int_Max, firstTime = Int_Max;
            std::string ret;
            for (auto si = sList; si != sList+sLen; si++) {
                TimeType startDay1 = day - si->leavingTime.getDate();
                if (!(si->startSaleDate <= startDay1 && startDay1 <= si->endSaleDate)) continue;
                auto trainS = trainDatabase.find(hash(si->trainID.str));
                for (auto ti = tList; ti != tList+tLen; ti++) {
                    if (ti->trainID == si->trainID) continue;
                    auto trainT = trainDatabase.find(hash(ti->trainID.str));
                    stationPair kList[StationNum_Max+2], lList[StationNum_Max+2];
                    int kLen = 0, lLen = 0;
                    for (int k = si->index + 1; k < trainS.first.stationNum; ++k) kList[kLen++] = std::make_pair(trainS.first.stations[k], k);
                    for (int l = 0; l < ti->index; ++l) lList[lLen++] = std::make_pair(trainT.first.stations[l], l);
                    if (!kLen || !lLen) continue;
                    qsort(kList, kList+kLen-1, stationCmp);
                    qsort(lList, lList+lLen-1, stationCmp);
                    stationPair *ptr1 = kList, *ptr2 = lList;
                    while (ptr1 != kList+kLen && ptr2 != lList+lLen) {
                        if (ptr1->first < ptr2->first) ptr1++;
                        else if (ptr1->first > ptr2->first) ptr2++;
                        else {
                            int k = ptr1->second, l = ptr2->second;
                            ptr1++, ptr2++;
                            TimeType fastestStartDay2;
                            if (trainS.first.arrivingTimes[k].getClock() <=
                                trainT.first.leavingTimes[l].getClock())
                                fastestStartDay2 = (startDay1 + trainS.first.arrivingTimes[k]).getDate() -
                                                   trainT.first.leavingTimes[l].getDate();
                            else
                                fastestStartDay2 =
                                        (startDay1 + trainS.first.arrivingTimes[k]).getDate() + 24 * 60 -
                                        trainT.first.leavingTimes[l].getDate();
                            //第一辆车发车时间，第二辆车最快发车时间（保证第二辆车 上车时间为第一辆车到达当天）
                            if (ti->endSaleDate < fastestStartDay2) continue; //最快还是赶不上第二辆车卖完，不行
                            TimeType startDay2 = std::max(fastestStartDay2,
                                                          ti->startSaleDate); //如果能最快发车就最快，否则从第二辆车第一次发车就上车
                            bool updated = false;
                            if (info.argNum == 4 && info.args['p' - 'a'] == "cost") {
                                if ((ans > trainS.first.priceSum[k] - si->priceSum + ti->priceSum -
                                           trainT.first.priceSum[l]) ||
                                    (ans == trainS.first.priceSum[k] - si->priceSum + ti->priceSum -
                                            trainT.first.priceSum[l] &&
                                     firstTime > trainS.first.arrivingTimes[k] - si->leavingTime)) {
                                    ans = trainS.first.priceSum[k] - si->priceSum + ti->priceSum -
                                          trainT.first.priceSum[l];
                                    firstTime = trainS.first.arrivingTimes[k] - si->leavingTime;
                                    updated = true;
                                }
                            } else if (ans > (startDay2 + ti->arrivingTime) - (startDay1 + si->leavingTime) ||
                                       (ans == (startDay2 + ti->arrivingTime) - (startDay1 + si->leavingTime) &&
                                        firstTime > trainS.first.arrivingTimes[k] - si->leavingTime)) {
                                ans = (startDay2 + ti->arrivingTime) - (startDay1 + si->leavingTime);
                                firstTime = trainS.first.arrivingTimes[k] - si->leavingTime;
                                updated = true;
                            }
                            if (updated) {
                                ret.clear();
                                auto dayTrainS = dayTrainDatabase.find(
                                        std::make_pair(startDay1, hash(trainS.first.trainID.str)));
                                auto dayTrainT = dayTrainDatabase.find(
                                        std::make_pair(startDay2, hash(trainT.first.trainID.str)));
                                ret += std::string(si->trainID.str) + " " +
                                       std::string(trainS.first.stations[si->index].str) + " " +
                                       (startDay1 + si->leavingTime).toFormatString()
                                       + " -> " + std::string(trainS.first.stations[k].str) + " " +
                                       (startDay1 + trainS.first.arrivingTimes[k]).toFormatString() + " " +
                                       std::to_string(trainS.first.priceSum[k] - si->priceSum) + " " +
                                       std::to_string(dayTrainS.first.querySeat(si->index, k - 1)) + "\n";
                                ret += std::string(trainT.first.trainID.str) + " " +
                                       std::string(trainT.first.stations[l].str) + " " +
                                       (startDay2 + trainT.first.leavingTimes[l]).toFormatString()
                                       + " -> " + std::string(trainT.first.stations[ti->index].str) + " " +
                                       (startDay2 + ti->arrivingTime).toFormatString() + " " +
                                       std::to_string(ti->priceSum - trainT.first.priceSum[l]) + " " +
                                       std::to_string(dayTrainT.first.querySeat(l, ti->index - 1));
                            }
                        }
                    }
                }
            }
            if (ans != Int_Max) return ret;
            return "0";
        }

        std::string buy_ticket(const cmdType& info) {
            if (info.argNum < 6 || info.argNum > 7) return "argnum_error";
            uidType uid = info.args['u'-'a'];
            hashCode uidHash = hash(uid.str);
            if (loggedUser.find(uidHash) == -1) return "not_logged"; //未登录
            TimeType day = info.args['d'-'a'] + " 00:00";
            tidType id = info.args['i'-'a'];
            hashCode idHash = hash(id.str);
            auto train = trainDatabase.find(idHash);
            int buyNum = stringToInt(info.args['n'-'a']);
            if (!train.second) return "train_not_found";
            if (!train.first.isReleased) return "not_released";
            if (buyNum > train.first.totalSeatNum) return "seat_not_enough";
            int f = -1, t = -1;
            for (int i = 0; i < train.first.stationNum && (f == -1 || t == -1); ++i) {
                if (train.first.stations[i] == info.args['f'-'a']) f = i;
                if (train.first.stations[i] == info.args['t'-'a']) t = i;
            }
            if (f == -1 || t == -1 || f >= t) return "unknow_wrong";
            TimeType startDay = day - train.first.leavingTimes[f].getDate();
            if (!(train.first.startSaleDate <= startDay && startDay <= train.first.endSaleDate)) return "day_error";
            auto dayTrain = dayTrainDatabase.find(std::make_pair(startDay, idHash));
            int remainSeat = dayTrain.first.querySeat(f, t-1);
            if ((info.argNum != 7 || info.args['q'-'a'] == "false") && remainSeat < buyNum) return "seat_not_enough";
            int price = train.first.priceSum[t]-train.first.priceSum[f], oid = orderDatabase.size();
            Order order = (Order){SUCCESS, id, uid, f, t, train.first.stations[f], train.first.stations[t], startDay, train.first.leavingTimes[f], train.first.arrivingTimes[t], oid, price, buyNum};
            if (remainSeat >= buyNum) {
                dayTrain.first.modifySeat(f, t-1, -buyNum);
                dayTrainDatabase.modify(std::make_pair(startDay, idHash), dayTrain.first);
                orderDatabase.insert(std::make_pair(uidHash, oid), order);
                long long ret = (long long)price*buyNum;
                return std::to_string(ret);
            }
            order.status = PENDING;
            orderDatabase.insert(std::make_pair(uidHash, oid), order);
            pendingQueue.insert(std::make_pair(std::make_pair(startDay, idHash), oid), (PendingOrder){idHash, uidHash, f, t, oid, buyNum, startDay});
            return "queue";
        }

        std::string query_order(const cmdType& info) {
            if (info.argNum != 1) return "argnum_error";
            uidType uid = info.args['u'-'a'];
            hashCode uidHash = hash(uid.str);
            if (loggedUser.find(uidHash) == -1) return "not_logged";
            int orderLen = 0;
            orderDatabase.range_find(std::make_pair(uidHash, 0), std::make_pair(uidHash, Int_Max), orders, orderLen);
            if (!orderLen) return "0";
            std::string ret;
            ret += std::to_string(orderLen);
            for (int i = orderLen-1; i >= 0; --i) {
                ret += "\n";
                auto it = orders+i;
                switch (it->status) {
                    case SUCCESS:ret += "[success] ";break;
                    case PENDING:ret += "[pending] ";break;
                    case REFUNDED:ret += "[refunded] ";
                }
                ret += std::string(it->trainID.str) + " " + std::string(it->from.str) + " " + (it->startDay+it->leavingTime).toFormatString() + " -> "
                       + std::string(it->to.str) + " " + (it->startDay+it->arrivingTime).toFormatString() + " " + std::to_string(it->price) + " " + std::to_string(it->num);
            }
            return ret;
        }

        std::string refund_ticket(const cmdType& info) {
            if (info.argNum < 1 || info.argNum > 2) return "argnum_error";
            uidType uid = info.args['u'-'a'];
            hashCode uidHash  = hash(uid.str);
            if (loggedUser.find(uidHash) == -1) return "not_logged";
            int orderLen = 0;
            orderDatabase.range_find(std::make_pair(uidHash, 0), std::make_pair(uidHash, Int_Max), orders, orderLen);
            int n = (info.args['n'-'a'].empty()) ? 1 : stringToInt(info.args['n'-'a']);
            if (n > orderLen) return "order_num_overflow";
            auto it = orders + orderLen - n;
            if (it->status == REFUNDED) return "refund_duplicate";
            orderDatabase.modify_info(std::make_pair(uidHash, it->orderID), REFUNDED, 0);
            if (it->status == PENDING) {
                pendingQueue.erase(std::make_pair(std::make_pair(it->startDay, hash(it->trainID.str)), it->orderID));
                return "0";
            }
            hashCode idHash = hash(it->trainID.str);
            auto dayTrain = dayTrainDatabase.find(std::make_pair(it->startDay, idHash));
            dayTrain.first.modifySeat(it->fromIndex, it->toIndex-1, it->num);
            int roLen = 0;
            pendingQueue.range_find(std::make_pair(std::make_pair(it->startDay, hash(it->trainID.str)), 0), std::make_pair(std::make_pair(it->startDay, hash(it->trainID.str)), Int_Max), refundOrders, roLen);
            for (auto i = refundOrders; i != refundOrders + roLen; i++) {
                if (i->fromIndex > it->toIndex || i->toIndex < it->fromIndex) continue;
                if (dayTrain.first.querySeat(i->fromIndex, i->toIndex-1) >= i->num) {
                    dayTrain.first.modifySeat(i->fromIndex, i->toIndex-1, -i->num);
                    pendingQueue.erase(std::make_pair(std::make_pair(i->startDay, i->tidHash), i->orderID));
                    orderDatabase.modify_info(std::make_pair(i->uidHash, i->orderID), SUCCESS, 0);
                }
            }
            dayTrainDatabase.modify(std::make_pair(it->startDay, idHash), dayTrain.first);
            return "0";
        }

        std::string clean(const cmdType& info) {
            loggedUser.clear();
            userDatabase.clear();
            trainDatabase.clear();
            dayTrainDatabase.clear();
            stationDatabase.clear();
            orderDatabase.clear();
            pendingQueue.clear();
            return "0";
        }

        std::string exit(const cmdType& info) {
            return "bye";
        }
    };
}

#endif //TICKETSYSTEM_2021_MAIN_SYSTEMCORE_HPP