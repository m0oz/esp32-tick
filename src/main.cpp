#include <U8g2lib.h>
#include <WiFi.h>

#include "TickiClock.h"
#include "TickiDisplay.h"
#include "TickiRadio.h"
#include "TickiWeather.h"
#include "TickiWeb.h"
#include "private.h"

// const char *ssid = "SSID";
// const char *password = "ENTER HERE!";

time_t current_t = 0;
time_t prev_led_t = 0;
time_t prev_config_t = 0;
time_t prev_timeupd_t = 0;
time_t prev_weather_t = 0;

time_t led_interval = 2 * 1000;  // led blinking
time_t config_interval =
    3 * 1000;  // check if config has been changed via web interface
time_t timeupd_interval = 10 * 1000;  // check if alarm should go off
time_t weather_interval = 60 * 1000;  // update weather report

tick::Clock clock_;
tick::Display display_;
tick::Radio radio_;
tick::WebInterface web_;
tick::Weather weather_;
String weather_string_ = "";
String weather_symbol_ = "";

#define LED_BUILTIN 2

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.begin(9600);

    // setup wifi
    {
        Serial.println();
        Serial.print("Connecting to ");
        Serial.println(ssid);
        WiFi.begin(ssid, password);
        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            Serial.print(".");
        }
        Serial.println("");
        Serial.println("WiFi connected.");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
    }

    display_.setup(150);
    clock_.setup();
    radio_.setup();
    web_.setup();
    weather_.setup();

    weather_.updateWeatherReport(clock_.getUnixTime());
    weather_.printWeatherReport(clock_.getUnixTime());

    Serial.println("Finished setup!");
}

void loop() {
    current_t = millis();

    // check to see if the interval time is passed.
    if (current_t - prev_led_t >= led_interval) {
        digitalWrite(LED_BUILTIN, HIGH);
        digitalWrite(LED_BUILTIN, LOW);
        prev_led_t = current_t;
    }

    if (current_t - prev_config_t >= config_interval) {
        clock_.setAlarm(web_.getAlarmHour(), web_.getAlarmMin());
        clock_.setAlarmDays(web_.getAlarmDays());
        prev_config_t = current_t;
    }

    if (current_t - prev_timeupd_t >= timeupd_interval) {
        String time_string = clock_.printTime();
        String alarm_string = clock_.printAlarm();
        display_.setText(time_string, weather_symbol_, weather_string_,
                         alarm_string,
                         "♥");  // TODO: Radiosender instead of heart
        clock_.checkAlarm();
        web_.setAlarmString(alarm_string);
        prev_timeupd_t = current_t;
    }

    // radio_.playRadioChunk();

    // update weather report
    if (current_t - prev_weather_t >= weather_interval) {
        weather_.updateWeatherReport(clock_.getUnixTime());
        weather_string_ = weather_.printWeatherReport(clock_.getUnixTime());
        weather_symbol_ = weather_.printWeatherSymbol();
        prev_weather_t = current_t;
    }
}
