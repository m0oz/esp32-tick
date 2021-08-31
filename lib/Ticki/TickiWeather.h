/**
 *
 * @file TickiWeather.h
 * @author Moritz & Alba Zimmermann
 * @brief
 * @date 2021-08-28
 *
 * copyright Copyright 2021 Moritz Zimmermann. All rights reserved.
 */

#pragma once

#define ARDUINOJSON_USE_LONG_LONG 1
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>

#include <utility>

#include "MeteoswissCert.h"

namespace tick {

const char* meteo_server = "www.meteoswiss.admin.ch";

class Weather {
   public:
    Weather() : rain_report_(1200), temp_report_(1200) {}
    String printWeatherSymbol() {
        String weather_symbol;
        switch (weather_status_) {
            case rainy:
                weather_symbol = "\u0037";
                break;

            case cloudy:
                weather_symbol = "\u0035";
                break;

            case sunny:
                weather_symbol =
                    "\u002E";  // TODO: sonnenstunden addieren und zählen
        }
        return weather_symbol;
    }

    String printWeatherReport(time_t utc_now, bool verbose = false) {
        Serial.print("Max/Min: ");
        Serial.print(max_temp_.second);
        Serial.print("(");
        Serial.print(max_temp_.first);
        Serial.print("h) / ");
        Serial.print(min_temp_.second);
        Serial.print("(");
        Serial.print(min_temp_.first);
        Serial.print("h), ");

        // only print detailed report when verbose
        if (verbose) {
            bool now = false;
            for (int h = 0; h < 24; h++) {
                float utc_report = temp_report_[h][0].as<float>() * 1e-3;
                float temp = temp_report_[h][1].as<float>();
                float rain = rain_report_[h][1].as<float>();
                if ((utc_report > utc_now) && !now) {
                    Serial.println("NOW");
                    now = true;
                }
                Serial.print("Time h: ");
                Serial.print(h);
                Serial.print(", temp deg: ");
                Serial.print(temp);
                Serial.print(", rain mm/h: ");
                Serial.print(rain);
                Serial.println();
            }
        }

        // compose string for display
        char buf[32];
        sprintf(buf, "%d°/%d°", int(max_temp_.second), int(min_temp_.second));
        String weather_string = String(buf);
        return weather_string;
    }

    void updateMinMaxTemp(time_t utc_now) {
        min_temp_.second = 50.;
        max_temp_.second = -50.;
        weather_status_ = cloudy;  // default symbol
        for (int h = 0; h < 24; h++) {
            float utc_report = temp_report_[h][0].as<float>() * 1e-3;
            float temp = temp_report_[h][1].as<float>();
            float rain = rain_report_[h][1].as<float>();
            if (utc_report > utc_now) {
                // TODO, do we include night times for min/max
                if (temp < min_temp_.second) {
                    min_temp_.first = h;
                    min_temp_.second = temp;
                }
                if (temp > max_temp_.second) {
                    max_temp_.first = h;
                    max_temp_.second = temp;
                }
                if (rain >= 0.3) {
                    weather_status_ = rainy;
                }
            }
        }
    }

    void updateWeatherReport(time_t utc_now) {
        String id = getWeatherReportId();
        Serial.print("Update weather report ");
        Serial.println(id.c_str());

        if (!client_.connect(meteo_server, 443)) {
            Serial.println("Connection to meteoswiss failed!");
            return;
        }
        if (!client_.connected()) {
            Serial.println("Not connected to meteoswiss!");
            return;
        }

        // Send HTTP request
        String request_path =
            "/product/output/forecast-chart/version__" + id + "/en/800100.json";
        client_.print(String("GET ") + request_path + " HTTP/2\r\n" +
                      "Host: " + meteo_server + "\r\n" +
                      "Referer: " + meteo_server + "\r\n" +
                      "User-Agent: ESPBoard\r\n" + "Connection: close\r\n\r\n");

        Serial.println("Waiting for response...");
        uint8_t timeout = 20;
        uint8_t trys = 0;
        while (!client_.available()) {
            delay(50);
            trys++;
            if (trys == timeout) {
                return;
            }
        }

        // check response code
        char status[32] = {0};
        client_.readBytesUntil('\r', status, sizeof(status));
        if (strcmp(status, "HTTP/1.1 200 OK") != 0) {
            Serial.print(F("Unexpected response: "));
            Serial.println(status);
            return;
        }

        // read rainfall report
        char rain_data[512] = {0};
        if (client_.find(R"==(rainfall":)==")) {
            int num_read = 0;
            for (; num_read < (sizeof(rain_data) - 1); num_read++) {
                rain_data[num_read] = client_.read();
                // check if delimiter ']]' was reached
                if (num_read > 2) {
                    if (strcmp(rain_data + num_read - 1, "]]") == 0) {
                        Serial.println("Received rainfall report.");
                        break;
                    }
                }
            }
            rain_data[num_read + 1] = '\0';
        } else {
            Serial.println("Could not find rainfall");
            return;
        }

        // read temperature report
        char temperature_data[512] = {0};
        if (client_.find(R"==(temperature":)==")) {
            int num_read = 0;
            for (; num_read < (sizeof(temperature_data) - 1); num_read++) {
                temperature_data[num_read] = client_.read();
                // check if delimiter ']]' was reached
                if (num_read > 2) {
                    if (strcmp(temperature_data + num_read - 1, "]]") == 0) {
                        Serial.println("Received temperature report.");
                        break;
                    }
                }
            }
            temperature_data[num_read + 1] = '\0';
        } else {
            Serial.println("Could not find temperature");
            return;
        }
        client_.stop();

        // write report to json object
        rain_report_.clear();
        temp_report_.clear();
        deserializeJson(rain_report_, rain_data);
        deserializeJson(temp_report_, temperature_data);

        updateMinMaxTemp(utc_now);

        return;
    }

    void setup() { client_.setCACert(ca_cert); }

   private:
    String getWeatherReportId() {
        Serial.println("Get weather report ID");
        if (!client_.connect(meteo_server, 443)) {
            Serial.println("Connection to swissmeteo failed!");
            return String("");
        }
        if (!client_.connected()) {
            Serial.println("Not connected to swissmeteo!");
            return String("");
        }

        const char* path = "/home.html";
        client_.print(String("GET ") + path + " HTTP/2\r\n" +
                      "Host: " + meteo_server + "\r\n" +
                      "User-Agent: ESPBoard\r\n" + "Connection: close\r\n\r\n");

        Serial.println("Waiting for response...");
        while (!client_.available()) {
            delay(50);
        }

        char report_id[14] = {0};
        if (client_.find("/product/output/forecast-chart/version__")) {
            size_t num_read =
                client_.readBytesUntil('/', report_id, sizeof(report_id) - 1);
            report_id[num_read] = '\0';
        } else {
            Serial.println("Could not find local forecast div!");
            return String("");
        }
        client_.stop();
        return String(report_id);
    }

    WiFiClientSecure client_;
    std::pair<int, float> min_temp_;
    std::pair<int, float> max_temp_;
    enum WeatherStatus { rainy = 0, cloudy = 1, sunny = 2 } weather_status_;
    DynamicJsonDocument rain_report_;
    DynamicJsonDocument temp_report_;
};

}  // namespace tick