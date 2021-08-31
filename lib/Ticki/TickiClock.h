/**
 *
 * @file TickiClock.h
 * @author Moritz Zimmermann
 * @brief
 * @date 2021-08-21
 *
 * copyright Copyright 2021 Moritz Zimmermann. All rights reserved.
 */

#pragma once

#include <Timezone.h>  // https://github.com/JChristensen/Timezone

#include "common.h"

namespace tick {

#define NTP_ADDRESS "pool.ntp.org"

// US Eastern Time Zone (New York, Detroit)
TimeChangeRule DST = {"CEST", Last, Sun,
                      Mar,    2,    +120};  // Daylight time = UTC +2 hours
TimeChangeRule STD = {"CET", Last, Sun,
                      Oct,   2,    +60};  // Standard time = UTC +1 hours

class Clock {
   public:
    Clock()
        : timezone_(DST, STD), alarm_days_{true,  false, true, false,
                                           false, false, false} {}

    void setup() {
        configTime(0, 0, NTP_ADDRESS);
        Serial.println("Connecting to NTP server...");
        while (utc_ < 1598000000) {
            utc_ = time(nullptr);
            delay(500);
        }
    }

    // format and print a time_t value, with a time zone appended.
    void printDateTime() {
        time_t t = getLocalTime();
        char buf[32];
        char m[4];  // temporary storage for month string (DateStrings.cpp uses
                    // shared buffer)
        strcpy(m, monthShortStr(month(t)));
        sprintf(buf, "%.2d:%.2d:%.2d %s %.2d %s %d %s", hour(t), minute(t),
                second(t), dayShortStr(weekday(t)), day(t), m, year(t),
                tcr_ptr_->abbrev);
        Serial.println(buf);
    }

    // format and print a time_t value
    String printTime() {
        time_t t = getLocalTime();
        char buf[10];
        sprintf(buf, "%.2d:%.2d", hour(t), minute(t));
        String time_string(buf);
        Serial.println("Time: " + time_string);
        return time_string;
    }

    String printAlarm() {
        time_t t = getLocalTime();
        uint8_t today = getWeekday(t);
        String alarm_string = "\u23F0 ";
        bool alarm_on = false;

        // handle case if alarm is later today
        if (alarm_days_[today] &&
            ((hour(t) < alarm_hour_) ||
             (hour(t) == alarm_hour_ && minute(t) < alarm_min_))) {
            alarm_string += WEEKDAYS[today];
            alarm_on = true;
        } else {
            // handle case if alarm is any other weekday
            for (int day = today + 1; day != today; day++) {
                day %= 7;
                if (alarm_days_[day]) {
                    alarm_string += WEEKDAYS[day];
                    alarm_on = true;
                    break;
                }
            }
        }

        if (alarm_on) {
            char buf[10];
            sprintf(buf, " %.2d:%.2d", alarm_hour_, alarm_min_);
            alarm_string += String(buf);
        } else {
            alarm_string = "\u23F0 off";
        }

        Serial.println("Alarm: " + alarm_string);
        return alarm_string;
    }

    void setAlarm(int hour, int min) {
        if (hour != alarm_hour_ || min != alarm_min_) {
            alarm_hour_ = hour;
            alarm_min_ = min;
            Serial.print("Set alarm time: ");
            Serial.print(alarm_hour_);
            Serial.print(":");
            Serial.println(alarm_min_);
        }
    }

    void setAlarmDays(std::array<bool, 7> days) { alarm_days_ = days; }

    /**
     * @brief Get weekday as int, monday == 0, sunday == 6
     *
     * @param t
     * @return uint8_t
     */
    uint8_t getWeekday(time_t t) {
        uint8_t weekday_sundayfirst = weekday(t);
        if (weekday_sundayfirst > 1) {
            return weekday_sundayfirst - 2;
        } else {
            return weekday_sundayfirst + 5;
        }
    }

    void checkAlarm() {
        time_t t = getLocalTime();
        if (alarm_days_[getWeekday(t)]) {
            if (hour(t) == alarm_hour_ && minute(t) == alarm_min_) {
                Serial.println("ALARM!!!!!!!!!!");
            }
        }
    }

    time_t getLocalTime() {
        utc_ = time(nullptr);
        time_t time = timezone_.toLocal(utc_, &tcr_ptr_);
        return time;
    }

    time_t getUnixTime() { return utc_; }

   private:
    Timezone timezone_;
    TimeChangeRule* tcr_ptr_;
    time_t utc_;
    int alarm_hour_;
    int alarm_min_;
    std::array<bool, 7> alarm_days_;
};

}  // namespace tick
