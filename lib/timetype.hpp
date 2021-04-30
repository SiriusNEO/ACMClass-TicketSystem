//
// Created by SiriusNEO on 2021/4/29.
//

#ifndef TICKETSYSTEM_2021_MAIN_TIMETYPE_HPP
#define TICKETSYSTEM_2021_MAIN_TIMETYPE_HPP

#include "toolfunctions.hpp"

namespace Sirius {
    const int PerMonthDay[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
            MonthDaySum[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};

    class TimeType {
    private:
        int minute; //Month - Day - Hour - Minute
    public:
        TimeType():minute(0) {}
        explicit TimeType(int _minute):minute(_minute){}
        explicit TimeType(const std::string& timeStr):minute(0) { //07-01 19:19
            minute = ((MonthDaySum[stringToInt(timeStr.substr(0, 2))-1] + stringToInt(timeStr.substr(3, 2)) - 1) * 24
                      + stringToInt(timeStr.substr(6, 2))) * 60
                     + stringToInt(timeStr.substr(9, 2));
        }

        std::pair<int, int> getDate() const {
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

        std::string toDateString() const {
            auto date = getDate();
            int res = minute - (MonthDaySum[date.first-1] + date.second - 1) * 24 * 60,
            min = res % 60, hour = (res - min) / 60;
            return dateFormat(date.first)+"-"+dateFormat(date.second)+" "+dateFormat(hour)+":"+dateFormat(min);
        }

        TimeType operator + (const TimeType& obj) const {
            return TimeType(minute + obj.minute);
        }
        TimeType operator - (const TimeType& obj) const {
            return TimeType(minute - obj.minute);
        }

    };
}

#endif //TICKETSYSTEM_2021_MAIN_TIMETYPE_HPP
