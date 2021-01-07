#pragma once
#include "utils.h"
#include <Arduino.h>

class Gecko;

class Settings {
public:
    enum class Status : uint8_t {
        OK = 0,
        COIN_INVALID,
        CURRENCY_INVALID,
    };

    Settings(Gecko& http);

    bool begin();
    bool read();
    void write() const;
    const String& coin() const { return m_coin; }
    const String& currency() const { return m_currency; }
    NumberFormat numberFormat() const { return m_number_format; }
    const String& name() const { return m_name; }
    const String& symbol() const { return m_symbol; }
    bool valid() const { return m_valid; }

    void displayed(bool d) { m_displayed = d; }
    bool displayed() const { return m_displayed; }

    Status set(const char* coin, const char* currency, uint8_t number_format);

    Gecko& getGecko() { return m_gecko; }

private:
    String m_coin;
    String m_currency;
    NumberFormat m_number_format { NumberFormat::DECIMAL_DOT };
    String m_name;
    String m_symbol;
    bool m_valid { false };
    bool m_displayed { false };

    Gecko& m_gecko;
};