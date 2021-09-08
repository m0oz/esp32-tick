/**
 *
 * @file TickiDisplay.h
 * @author Moritz Zimmermann
 * @brief
 * @date 2021-08-21
 *
 * copyright Copyright 2021 Moritz Zimmermann. All rights reserved.
 */
#pragma once

#include <U8g2lib.h>

namespace tick {

class Display {
   public:
    Display()
        : u8g2_(  // note "F_HW" version gave best results
              /* No Rotation*/ U8G2_R0,
              /* reset= */ U8X8_PIN_NONE,
              /* clock=*/25,
              /* data=*/26) {}

    void setup(int brightness = 200) {
        u8g2_.begin();
        u8g2_.setContrast(brightness);  //  Brightness setting from 0 to 255
        u8g2_.setBusClock(1000000);     // seems to work reliably, and very fast
        u8g2_.clearBuffer();
        u8g2_.drawLine(0, 0, 127, 0);
        u8g2_.drawLine(0, 0, 0, 127);
        u8g2_.drawLine(127, 0, 127, 127);
        u8g2_.drawLine(0, 127, 127, 127);
        u8g2_.setFont(u8g2_font_fub17_tr);
        u8g2_.drawStr(40, 50, "Ticki");
        u8g2_.setFont(u8g2_font_open_iconic_human_4x_t);
        u8g2_.drawUTF8(49, 90, "\u0042");  // heart
        u8g2_.sendBuffer();
    }

    void drawFrame() {
        u8g2_.drawLine(0, 0, 127, 0);
        u8g2_.drawLine(0, 0, 0, 127);
        u8g2_.drawLine(127, 0, 127, 127);
        u8g2_.drawLine(0, 127, 127, 127);
    }

    void setText(String time, String weather_symbol, String line1, String line2,
                 String line3) {
        u8g2_.clearBuffer();
        // drawFrame();
        u8g2_.setFont(u8g2_font_fub17_tr);
        u8g2_.drawStr(32, 30, time.c_str());
        u8g2_.setFont(u8g2_font_open_iconic_weather_2x_t);
        u8g2_.drawUTF8(4, 58, weather_symbol.c_str());
        u8g2_.setFont(u8g2_font_open_iconic_embedded_2x_t);
        u8g2_.drawUTF8(4, 83, "\u0041");
        u8g2_.setFont(u8g2_font_fub11_tr);
        u8g2_.drawUTF8(29, 55, line1.c_str());
        u8g2_.drawUTF8(29, 80, line2.c_str());
        u8g2_.drawUTF8(4, 105, line3.c_str());
        u8g2_.sendBuffer();
    }

   private:
    U8G2_SSD1327_MIDAS_128X128_F_HW_I2C u8g2_;
};

}  // namespace tick