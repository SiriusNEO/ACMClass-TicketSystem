//
// Created by SiriusNEO on 2021/4/29.
//

#ifndef TICKETSYSTEM_2021_MAIN_TIMETYPE_HPP
#define TICKETSYSTEM_2021_MAIN_TIMETYPE_HPP

#include "mytools.hpp"

namespace Sirius {
    const int PerMonthDay[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
            MonthDaySum[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};

    //注意：对于纯日期，要 + " 00:00"，对于纯时刻，要 "01-01 "
    class TimeType {
    private:
        int minute; //Month - Day - Hour - Minute
    public:
        TimeType():minute(0) {}
        TimeType(int _minute):minute(_minute){}
        TimeType(const std::string& timeStr):minute(0) { //07-01 19:19
            minute = ((MonthDaySum[stringToInt(timeStr.substr(0, 2))-1] + stringToInt(timeStr.substr(3, 2)) - 1) * 24
                      + stringToInt(timeStr.substr(6, 2))) * 60
                     + stringToInt(timeStr.substr(9, 2));
        }

        TimeType& operator = (const std::string& timeStr) {
            minute = ((MonthDaySum[stringToInt(timeStr.substr(0, 2))-1] + stringToInt(timeStr.substr(3, 2)) - 1) * 24
                      + stringToInt(timeStr.substr(6, 2))) * 60
                     + stringToInt(timeStr.substr(9, 2));
            return *this;
        }

        std::pair<int, int> getDatePair() const {
            int month = 0, day = 0, l = 1, r = 13;
            while (r - l != 1) {
                int mid = (l + r) >> 1;
                if (MonthDaySum[mid-1]*24*60 > minute) r = mid;
                else l = mid;
            }
            month = l;
            l = 1, r = PerMonthDay[month] + 1;
            while (r - l != 1) {
                int mid = (l + r) >> 1;
                if ((MonthDaySum[month-1]+mid-1)*24*60 > minute) r = mid;
                else l = mid;
            }
            day = l;
            return std::make_pair(month, day);
        }

        TimeType getDate() const {
            return TimeType(minute-minute%(24*60));
        }

        TimeType getClock() const {
            return TimeType(*this-this->getDate());
        }

        std::string toFormatString() const {
            auto date = getDatePair();
            int res = minute - (MonthDaySum[date.first-1] + date.second - 1) * 24 * 60,
            min = res % 60, hour = (res - min) / 60;
            return dateFormat(date.first)+"-"+dateFormat(date.second)+" "+dateFormat(hour)+":"+dateFormat(min);
        }

        TimeType operator + (const TimeType& obj) const {
            return TimeType(minute + obj.minute);
        }
        TimeType operator += (const TimeType& obj) {
            minute += obj.minute;
            return *this;
        }
        TimeType operator + (int obj) const {
            return TimeType(minute + obj);
        }
        TimeType operator += (int obj) {
            minute += obj;
            return *this;
        }
        int operator - (const TimeType& obj) const {
            return minute - obj.minute;
        }
        bool operator == (const TimeType& obj) const {
            return minute == obj.minute;
        }
        bool operator < (const TimeType& obj) const {
            return minute < obj.minute;
        }
        bool operator <= (const TimeType& obj) const {
            return minute <= obj.minute;
        }
        bool operator > (const TimeType& obj) const {
            return minute > obj.minute;
        }
        bool operator >= (const TimeType& obj) const {
            return minute >= obj.minute;
        }
        friend std::ostream& operator << (std::ostream& os, const TimeType& obj) {
            return os << obj.toFormatString();
        }
    };
}

#endif //TICKETSYSTEM_2021_MAIN_TIMETYPE_HPP
