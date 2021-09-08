/**
 * @file main.cpp
 * @author Moritz Zimmermann
 * @brief ticki main
 * @date 2021-09-07
 *
 * copyright Copyright 2021 Moritz Zimmermann. All rights reserved.
 */

#include <WiFi.h>

#include "TickiClock.h"
#include "TickiDisplay.h"
#include "TickiRadio.h"
#include "TickiWeather.h"
#include "TickiWeb.h"
#include "common.h"
#include "private.h"

// const char *ssid = "SSID";
// const char *password = "ENTER HERE!";

time_t current_t = 0;
time_t prev_config_t = 0;
time_t prev_timeupd_t = 0;
time_t prev_weather_t = 0;

time_t config_interval =
    500;  // check if config has been changed via web interface
time_t timeupd_interval = 10 * 1000;  // check if alarm should go off
time_t weather_interval = 60 * 1000;  // update weather report

tick::Clock clock_;
tick::Display display_;
tick::Radio radio_;
tick::WebInterface web_;
tick::TickiConfig web_config_;
tick::Weather weather_;
String weather_string_ = "";
String weather_symbol_ = "";
bool led_on_ = false;

#define LED_BUILTIN 2

void setup() {
    Serial.begin(115200);
    SPI.begin();

    display_.setup(150);

    // setup wifi
    {
        Serial.printf("Connecting to %s ", ssid);
        WiFi.begin(ssid, password);
        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
        }
        Serial.printf("WiFi connected, IP address: %s\n",
                      WiFi.localIP().toString().c_str());
    }

    clock_.setup();
    radio_.setup();
    web_.setup();
    weather_.setup();
    weather_.updateWeatherReport(clock_.getUnixTime());
    weather_.printMinMaxTemp();
    radio_.start();

    Serial.println("Finished setup!");
}

void loop() {
    current_t = millis();

    if (current_t - prev_config_t >= config_interval) {
        web_config_ = web_.getConfig();
        clock_.setConfig(web_config_);
        radio_.setConfig(web_config_);
        prev_config_t = current_t;
    }

    if (current_t - prev_timeupd_t >= timeupd_interval) {
        String time_string = clock_.printTime();
        String alarm_string = clock_.printAlarm();
        display_.setText(time_string, weather_symbol_, weather_string_,
                         alarm_string,
                         "♥");  // TODO: Radiosender instead of heart

        if (clock_.checkAlarm()) {
            radio_.start();
        } else {
            if (!web_config_.radio_on) {
                radio_.stop();
            }
        }

        web_.setAlarmString(alarm_string);
        prev_timeupd_t = current_t;
    }

    radio_.playRadioChunk();

    // update weather report
    if (current_t - prev_weather_t >= weather_interval) {
        // do not update while radio is running
        if (!web_config_.radio_on) {
            weather_.updateWeatherReport(clock_.getUnixTime());
            weather_string_ = weather_.printMinMaxTemp();
            weather_symbol_ = weather_.printWeatherSymbol();
            prev_weather_t = current_t;
        }
    }
}