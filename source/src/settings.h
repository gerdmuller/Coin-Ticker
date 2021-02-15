#pragma once
#include "utils.h"
#include <Arduino.h>

#define USER_CONFIG "/settings.json"
#define DYNAMIC_JSON_CONFIG_SIZE 256

class Gecko;

class Settings {
public:
    enum class Status : uint8_t {
        OK = 0,
        COIN_INVALID,
        CURRENCY_INVALID,
    };
    enum class ChartStyle : uint8_t {
        SIMPLE = 0,
        HIGH_LOW,
        HIGH_LOW_FIRST_LAST
    };
    enum ChartPeriod : uint8_t {
        PERIOD_NONE = 0,
        PERIOD_24_H = 1,
        PERIOD_48_H = 2,
        PERIOD_30_D = 4,
        PERIOD_60_D = 8,
        ALL_PERIODS = PERIOD_24_H + PERIOD_48_H + PERIOD_30_D + PERIOD_60_D
    };

    Settings();

    bool begin(const Gecko& gecko);
    bool read(const Gecko& gecko);
    void write();
    void deleteFile();
    const char* coin() const { return m_coin.c_str(); }
    const char* currency() const { return m_currency.c_str(); }
    NumberFormat numberFormat() const { return m_number_format; }
    uint8_t chartPeriod() const { return m_chart_period; }
    ChartStyle chartStyle() const { return m_chart_style; }
    bool heartbeat() const { return m_heartbeat; }
    const char* name() const { return m_name.c_str(); }
    const char* symbol() const { return m_symbol.c_str(); }
    uint8_t brightness() const;
    bool valid() const { return m_valid; }

    Status set(const Gecko& gecko, const char* coin, const char* currency, uint8_t number_format, uint8_t chartPeriod, uint8_t chartStyle, bool heartbeat);
    bool setBrightness(uint8_t b);

    uint32_t lastChange() const { return m_lastChange; }

private:
    String m_coin;
    String m_currency;
    NumberFormat m_number_format { NumberFormat::DECIMAL_DOT };
    uint8_t m_chart_period { ChartPeriod::PERIOD_24_H };
    ChartStyle m_chart_style { ChartStyle::SIMPLE };
    bool m_heartbeat { true };
    String m_name;
    String m_symbol;
    uint8_t m_brightness { std::numeric_limits<uint8_t>::max() };
    bool m_valid { false };

    uint32_t m_lastChange { 0 };
};
