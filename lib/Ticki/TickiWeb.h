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
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>

#include "common.h"
#include "index.html"

namespace tick {

class WebInterface {
   public:
    WebInterface() : server_(80), config_{DEFAULT_CONFIG}, alarm_string_{""} {}

    void setup() {
        serveFavicon();

        server_.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
            request->send(200, "text/html", INDEX_HTML);
        });

        server_.on(
            "/checkconnection", HTTP_GET, [](AsyncWebServerRequest *request) {
                request->send(200, "text/html", "ESP32 radio connected!");
            });

        serveAlarm();
        serveRadio();

        server_.begin();
    }

    TickiConfig getConfig() { return config_; }

    void setAlarmString(String string) { alarm_string_ = string; }

   private:
    void serveFavicon() {
        if (!SPIFFS.begin()) {
            Serial.println("An Error has occurred while mounting SPIFFS");
            return;
        }

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

    void serveAlarm() {
        server_.on("/gettime", HTTP_GET,
                   [this](AsyncWebServerRequest *request) {
                       Serial.println("Get time");
                       AsyncResponseStream *response =
                           request->beginResponseStream("application/json");
                       DynamicJsonDocument json(512);
                       json["hour"] = config_.alarm_hour;
                       json["min"] = config_.alarm_min;
                       serializeJson(json, *response);
                       request->send(response);
                   });

        server_.on(
            "/settime", HTTP_GET, [this](AsyncWebServerRequest *request) {
                String hour;
                String min;
                if (request->hasParam("hour") && request->hasParam("min")) {
                    config_.alarm_hour =
                        request->getParam("hour")->value().toInt();
                    config_.alarm_min =
                        request->getParam("min")->value().toInt();
                } else {
                    Serial.println("Failed to set alarm time!");
                    request->send(400, "text/plain", "Arguments incomplete!");
                    return;
                }

                Serial.print("hour: ");
                Serial.print(config_.alarm_hour);
                Serial.print(" min: ");
                Serial.println(config_.alarm_min);
                request->send(200, "text/plain", "OK");
            });

        server_.on("/getdays", HTTP_GET,
                   [this](AsyncWebServerRequest *request) {
                       Serial.println("Get time");
                       AsyncResponseStream *response =
                           request->beginResponseStream("application/json");
                       DynamicJsonDocument json(254);
                       json["mon"] = config_.alarm_days[0];
                       json["tue"] = config_.alarm_days[1];
                       json["wed"] = config_.alarm_days[2];
                       json["thu"] = config_.alarm_days[3];
                       json["fri"] = config_.alarm_days[4];
                       json["sat"] = config_.alarm_days[5];
                       json["sun"] = config_.alarm_days[6];
                       serializeJson(json, *response);
                       request->send(response);
                   });

        server_.on("/setday", HTTP_GET, [this](AsyncWebServerRequest *request) {
            if (request->hasParam("day") && request->hasParam("onoff")) {
                uint8_t day = request->getParam("day")->value().toInt();
                config_.alarm_days[day] =
                    bool(request->getParam("onoff")->value().toInt());
            } else {
                Serial.println("Failed to set alarm days!");
                request->send(400, "text/plain", "Arguments incomplete!");
                return;
            }
            request->send(200, "text/plain", "OK");
        });

        server_.on("/getalarmstring", HTTP_GET,
                   [this](AsyncWebServerRequest *request) {
                       request->send(200, "text/plain",
                                     "\u23F0 " + alarm_string_);
                   });
    }

    void serveRadio() {
        server_.on("/getradiostation", HTTP_GET,
                   [this](AsyncWebServerRequest *request) {
                       request->send(200, "text/plain", config_.radio_station);
                   });

        server_.on(
            "/setradio", HTTP_GET, [this](AsyncWebServerRequest *request) {
                if (request->hasParam("station")) {
                    config_.radio_station =
                        request->getParam("station")->value();
                } else {
                    Serial.println("Failed to set radio station!");
                    request->send(400, "text/plain", "Arguments incomplete!");
                    return;
                }

                request->send(200, "text/plain", "OK");
            });

        server_.on("/startradio", HTTP_GET,
                   [this](AsyncWebServerRequest *request) {
                       config_.radio_on = true;
                       request->send(200, "text/plain", config_.radio_station);
                   });

        server_.on("/stopradio", HTTP_GET,
                   [this](AsyncWebServerRequest *request) {
                       config_.radio_on = false;
                       request->send(200, "text/plain", config_.radio_station);
                   });

        server_.on(
            "/setvolume", HTTP_GET, [this](AsyncWebServerRequest *request) {
                config_.radio_volume = false;
                if (request->hasParam("value")) {
                    config_.radio_volume =
                        request->getParam("value")->value().toInt();
                } else {
                    Serial.println("Failed to set volume!");
                    request->send(400, "text/plain", "Arguments incomplete!");
                    return;
                }
                request->send(200, "text/plain", "OK");
            });

        server_.on(
            "/getvolume", HTTP_GET, [this](AsyncWebServerRequest *request) {
                request->send(200, "text/plain", String(config_.radio_volume));
            });
    }

    AsyncWebServer server_;

    struct TickiConfig config_;
    String alarm_string_;
};

}  // namespace tick
