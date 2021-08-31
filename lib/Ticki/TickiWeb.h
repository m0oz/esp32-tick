/**
 *
 * @file webserver.h
 * @author Moritz Zimmermann
 * @brief
 * @date 2021-08-21
 *
 * copyright Copyright 2021 Moritz Zimmermann. All rights reserved.
 */

#pragma once

#include <ArduinoJson.h>
#include <AsyncJson.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>

#include "common.h"
#include "index.html"

namespace tick {

class WebInterface {
   public:
    WebInterface()
        : server_(80),
          alarm_hour_{7},
          alarm_min_{0},
          alarm_string_{""},
          alarm_days_{false, false, false, false, false, false, false} {}

    void setup() {
        if (!SPIFFS.begin()) {
            Serial.println("An Error has occurred while mounting SPIFFS");
            return;
        }

        serveImages();

        server_.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
            request->send(200, "text/html", INDEX_HTML);
        });

        server_.on(
            "/checkconnection", HTTP_GET, [](AsyncWebServerRequest *request) {
                request->send(200, "text/html", "ESP32 radio connected!");
            });

        server_.on("/gettime", HTTP_GET,
                   [this](AsyncWebServerRequest *request) {
                       Serial.println("Get time");
                       AsyncResponseStream *response =
                           request->beginResponseStream("application/json");
                       DynamicJsonDocument json(512);
                       json["hour"] = alarm_hour_;
                       json["min"] = alarm_min_;
                       serializeJson(json, *response);
                       request->send(response);
                   });

        server_.on(
            "/settime", HTTP_GET, [this](AsyncWebServerRequest *request) {
                String hour;
                String min;
                if (request->hasParam("hour") && request->hasParam("min")) {
                    alarm_hour_ = request->getParam("hour")->value().toInt();
                    alarm_min_ = request->getParam("min")->value().toInt();
                } else {
                    Serial.println("Failed to set alarm time!");
                    request->send(400, "text/plain", "Arguments incomplete!");
                    return;
                }

                Serial.print("hour: ");
                Serial.print(alarm_hour_);
                Serial.print(" min: ");
                Serial.println(alarm_min_);
                request->send(200, "text/plain", "OK");
            });

        server_.on("/getdays", HTTP_GET,
                   [this](AsyncWebServerRequest *request) {
                       Serial.println("Get time");
                       AsyncResponseStream *response =
                           request->beginResponseStream("application/json");
                       DynamicJsonDocument json(254);
                       json["mon"] = alarm_days_[0];
                       json["tue"] = alarm_days_[1];
                       json["wed"] = alarm_days_[2];
                       json["thu"] = alarm_days_[3];
                       json["fri"] = alarm_days_[4];
                       json["sat"] = alarm_days_[5];
                       json["sun"] = alarm_days_[6];
                       serializeJson(json, *response);
                       request->send(response);
                   });

        server_.on("/setday", HTTP_GET, [this](AsyncWebServerRequest *request) {
            int8_t day = 0;
            int8_t onoff = 0;

            if (request->hasParam("day") && request->hasParam("onoff")) {
                day = request->getParam("day")->value().toInt();
                onoff = request->getParam("onoff")->value().toInt();
            } else {
                Serial.println("Failed to set alarm days!");
                request->send(400, "text/plain", "Arguments incomplete!");
                return;
            }

            alarm_days_[day] = bool(onoff);

            Serial.print("Alarm day: ");
            Serial.print(day);
            Serial.print(" onoff: ");
            Serial.println(onoff);
            request->send(200, "text/plain", "OK");
        });

        server_.on("/getalarmstring", HTTP_GET,
                   [this](AsyncWebServerRequest *request) {
                       String next_alarm_string = alarm_string_;
                       request->send(200, "text/plain", next_alarm_string);
                   });

        server_.begin();
    }

    int getAlarmHour() { return alarm_hour_; }

    int getAlarmMin() { return alarm_min_; }

    std::array<bool, 7> getAlarmDays() { return alarm_days_; }

    void setAlarmString(String string) { alarm_string_ = string; }

   private:
    void serveImages() {
        File favicon = SPIFFS.open("/favicon.png");
        if (!favicon) Serial.println("Failed to open favion!");
        server_.on("/favicon.ico", HTTP_GET,
                   [](AsyncWebServerRequest *request) {
                       request->send(SPIFFS, "/favicon.png", "image/png");
                   });

        File apple_touch_icon = SPIFFS.open("/apple_touch_icon.png");
        if (!apple_touch_icon)
            Serial.println("Failed to open apple_touch_icon!");
        server_.on("/apple_touch_icon.png", HTTP_GET,
                   [](AsyncWebServerRequest *request) {
                       request->send(SPIFFS, "/favicon.png", "image/png");
                   });
    }

    AsyncWebServer server_;
    uint8_t alarm_hour_;
    uint8_t alarm_min_;
    String alarm_string_;
    std::array<bool, 7> alarm_days_;
};

}  // namespace tick
