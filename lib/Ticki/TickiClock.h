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
    Clock() : timezone_(DST, STD), config_{DEFAULT_CONFIG} {}

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
        String alarm_string = "";
        bool alarm_on = false;

        // handle case if alarm is later today
        if (config_.alarm_days[today] && ((hour(t) < config_.alarm_hour) ||
                                          (hour(t) == config_.alarm_hour &&
                                           minute(t) < config_.alarm_min))) {
            alarm_string += WEEKDAYS[today];
            alarm_on = true;
        } else {
            // handle case if alarm is any other weekday
            for (int day = today + 1; day != today; ++day %= 7) {
                if (config_.alarm_days[day]) {
                    alarm_string += WEEKDAYS[day];
                    alarm_on = true;
                    break;
                }
            }
        }

        if (alarm_on) {
            char buf[10];
            sprintf(buf, " %.2d:%.2d", config_.alarm_hour, config_.alarm_min);
            alarm_string += String(buf);
        } else {
            alarm_string = "off";
        }

        Serial.println("Alarm: " + alarm_string);
        return alarm_string;
    }

    void setConfig(const TickiConfig& config) { config_ = config; }

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

    bool checkAlarm() {
        time_t t = getLocalTime();
        if (config_.alarm_days[getWeekday(t)]) {
            if (hour(t) == config_.alarm_hour &&
                minute(t) == config_.alarm_min) {
                Serial.println("ALARM!!!!!!!!!!");
                return true;
            }
        }
        return false;
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
    struct TickiConfig config_;
};

}  // namespace tick
