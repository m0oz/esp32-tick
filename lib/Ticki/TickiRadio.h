/**
 *
 * @file radio.h
 * @author Moritz Zimmermann
 * @brief
 * @date 2021-08-21
 *
 * copyright Copyright 2021 Moritz Zimmermann. All rights reserved.
 */

#pragma once

#include <VS1053.h>
#include <WiFi.h>

#include "common.h"

namespace tick {

// mp3 board
#define VS1053_CS 5
#define VS1053_DCS 16
#define VS1053_DREQ 4
#define BUFFSIZE 64  // 32, 64 or 128

struct Station {
    String name;
    String host;
    String path;
    int port;
    Station(String _name, String _host, String _path, int _port)
        : name(_name), host(_host), path(_path), port(_port) {}
};
std::array<Station, 4> station_list = {
    Station("SRF2", "stream.srg-ssr.ch", "/m/drs2/mp3_128", 80),
    Station("Fritz", "addrad.io", "/4455cj3", 80),
    Station("egoFM", "streams.egofm.de", "/egoFM-hq", 80),
    Station("UNKNOWN", "", "", 0)};

class Radio {
   public:
    Radio()
        : config_{DEFAULT_CONFIG},
          connected_{false},
          connected_station_{"None"},
          player_(VS1053_CS, VS1053_DCS, VS1053_DREQ),
          current_volume_{config_.radio_volume} {}

    void setup() { initMP3Decoder(); }

    /**
     * @brief Play a single chunk of MP3 data from radio stream
     */
    void playRadioChunk() {
        connectStation(config_.radio_station);
        if (config_.radio_volume != current_volume_) {
            player_.setVolume(config_.radio_volume);
            current_volume_ = config_.radio_volume;
        }

        if (config_.radio_on && connected_) {
            if (radio_client_.available() > 0) {
                uint8_t bytesread = radio_client_.read(mp3buff_, BUFFSIZE);
                if (bytesread) {
                    player_.playChunk(mp3buff_, bytesread);
                }
            }
        }
    }

    void start() { config_.radio_on = true; }

    void stop() { config_.radio_on = false; }

    void setConfig(const TickiConfig& config) { config_ = config; }

   private:
    /**
     * @brief Connect to station if the current station does not match the
     * specified
     * @param station_name - Must be a configured name in station_list
     */
    void connectStation(String station_name) {
        if (station_name == connected_station_) return;

        Serial.printf("Connect to station %s\n", station_name.c_str());

        // find station in station_list
        size_t station_idx = 0;
        for (; station_idx < station_list.size(); ++station_idx) {
            if (station_name == station_list[station_idx].name) break;
        }
        if (station_idx == station_list.size()) {
            Serial.printf("Station %s not configured!\n", station_name.c_str());
            return;
        }

        // try to connect
        if (radio_client_.connect(station_list[station_idx].host.c_str(),
                                  station_list[station_idx].port)) {
            connected_ = true;
            connected_station_ = station_name;
            radio_client_.print(
                String("GET ") + station_list[station_idx].path +
                " HTTP/1.1\r\n" + "Host: " + station_list[station_idx].host +
                "\r\n" + "Connection: close\r\n\r\n");
            Serial.println("Connected to " + connected_station_);
        } else {
            Serial.println("Failed to connected to radio station" +
                           station_name);
        }
    }

    /**
     * @brief Setup communication with MP3 decoder board
     */
    void initMP3Decoder() {
        SPI.begin();
        player_.begin();

        if (player_.isChipConnected()) {
            Serial.println("Connected to MP3 decoder");
            player_.loadDefaultVs1053Patches();
        } else {
            Serial.println("Failed to connect to MP3 decoder!");
        }

        player_.switchToMp3Mode();
        player_.setVolume(config_.radio_volume);
    }

    struct TickiConfig config_;
    WiFiClient radio_client_;
    bool connected_;
    String connected_station_;
    uint8_t mp3buff_[BUFFSIZE];
    VS1053 player_;
    uint8_t current_volume_;
};

}  // namespace tick