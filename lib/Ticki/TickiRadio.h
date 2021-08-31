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

namespace tick {

// mp3 board
#define VS1053_CS 32
#define VS1053_DCS 33
#define VS1053_DREQ 35

struct Station {
    String host;
    String path;
    int port;
    Station(String _host, String _path, int _port)
        : host(_host), path(_path), port(_port) {}
};

Station station_list[3] = {Station("149.255.59.162", "/1", 8062),
                           Station("radiostreaming.ert.gr", "/ert-kosmos", 80),
                           Station("realfm.live24.gr", "/realfm", 80)};

class Radio {
   public:
    Radio() : player_(VS1053_CS, VS1053_DCS, VS1053_DREQ) {}

    void initMP3Decoder() {
        player_.begin();
        player_.switchToMp3Mode();  // optional, some boards require this
        player_.setVolume(50);
    }

    void connectStation(int station_number) {
        if (radio_client_.connect(station_list[station_number].host.c_str(),
                                  station_list[station_number].port))
            Serial.println("Connected now");
        radio_client_.print(String("GET ") + station_list[station_number].path +
                            " HTTP/1.1\r\n" +
                            "Host: " + station_list[station_number].host +
                            "\r\n" + "Connection: close\r\n\r\n");
    }

    void setup() {
        // initMP3Decoder();
        connectStation(0);
    }

    void playRadioChunk() {
        if (radio_client_.available() > 0) {
            uint8_t bytesread = radio_client_.read(mp3buff_, 32);
            player_.playChunk(mp3buff_, bytesread);
        }
        Serial.print('*');
    }

   private:
    WiFiClient radio_client_;
    uint8_t mp3buff_[32];
    VS1053 player_;
};

}  // namespace tick