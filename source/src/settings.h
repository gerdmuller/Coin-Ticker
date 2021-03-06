#pragma once
#include "utils.h"
#include <Arduino.h>
#include <ArduinoJson.h>

#define SETTINGS_FILE "/settings.json"
#define BRIGHTNESS_FILE "/brightness.json"

class Settings {
public:
    enum class ChartStyle : uint8_t {
        SIMPLE = 0,
        HIGH_LOW,
        HIGH_LOW_FIRST_LAST
    };
    enum class Swap : uint8_t {
        INTERVAL_1 = 0, // 5s (chart), 10s (coin)
        INTERVAL_2, // 30s
        INTERVAL_3, // 1min
        INTERVAL_4 // 5min
    };

    enum ChartPeriod : uint8_t {
        PERIOD_NONE = 0,
        PERIOD_24_H = 1,
        PERIOD_48_H = 2,
        PERIOD_30_D = 4,
        PERIOD_60_D = 8,
        ALL_PERIODS = PERIOD_24_H + PERIOD_48_H + PERIOD_30_D + PERIOD_60_D
    };

    enum Mode : uint8_t {
        ONE_COIN = 1,
        TWO_COINS,
        MULTIPLE_COINS
    };

    struct Coin {
        String id;
        String symbol;
        String name;
    };

    struct Currency {
        String currency;
        String symbol;
    };

    Settings();

    void begin();
    void set(const char* json);
    void read();
    void write() const;
    void deleteFile() const;

    const String& coin(uint32_t index) const;
    const String& name(uint32_t index) const;
    const String& symbol(uint32_t index) const;

    const String& currency() const;
    const String& currencySymbol() const;
    const String& currency2() const;
    const String& currency2Symbol() const;

    Mode mode() const { return m_mode; }
    uint32_t numberCoins() const;
    NumberFormat numberFormat() const { return m_number_format; }
    uint8_t chartPeriod() const { return m_chart_period; }
    Swap swapInterval() const { return m_swap_interval; }
    ChartStyle chartStyle() const { return m_chart_style; }
    bool heartbeat() const { return m_heartbeat; }
    bool valid() const;

    void readBrightness();
    void setBrightness(uint8_t b);
    uint8_t brightness() const;

    uint32_t lastChange() const { return m_last_change; }

private:
    void set(DynamicJsonDocument& doc, bool toFile);
    void trace() const;

    uint32_t validCoinIndex(uint32_t index) const;

    Mode m_mode { Mode::ONE_COIN };

    std::vector<Coin> m_coins;
    std::array<Currency, 2> m_currencies;

    NumberFormat m_number_format { NumberFormat::DECIMAL_DOT };
    uint8_t m_chart_period { ChartPeriod::PERIOD_24_H };
    Swap m_swap_interval { Swap::INTERVAL_1 };
    ChartStyle m_chart_style { ChartStyle::SIMPLE };
    bool m_heartbeat { true };

    uint8_t m_brightness { std::numeric_limits<uint8_t>::max() };

    uint32_t m_last_change { 0 };
};
