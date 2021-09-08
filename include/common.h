/**
 *
 * @file common.h
 * @author Moritz Zimmermann
 * @brief
 * @date 2021-08-31
 *
 * copyright Copyright 2021 Moritz Zimmermann. All rights reserved.
 */

#pragma once

#include <WString.h>

#include <array>

namespace tick {
const std::array<String, 7> WEEKDAYS = {"Mon", "Tue", "Wed", "Thu",
                                        "Fri", "Sat", "Sun"};

struct TickiConfig {
    uint8_t alarm_hour;
    uint8_t alarm_min;
    std::array<bool, 7> alarm_days;
    String radio_station;
    bool radio_on;
    uint8_t radio_volume;
};

const struct TickiConfig DEFAULT_CONFIG = {
    7, 0, {true, true, true, true, true, false}, "SRF2", true, 100};

}  // namespace tick
