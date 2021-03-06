#include "gecko.h"
#include "http_json.h"
#include "utils.h"
#include <ArduinoJson.h>

// https://arduinojson.org/v6/assistant/
#define DYNAMIC_JSON_PING_SIZE 64
#define DYNAMIC_JSON_PRICE_CHANGE_SIZE 384
#define DYNAMIC_JSON_CHART_SIZE 3072

#define PRICE_FETCH_INTERVAL (10 * 1000)
#define CHART_48_H_FETCH_INTERVAL (15 * 60 * 1000)
#define CHART_60_D_FETCH_INTERVAL (6 * 60 * 60 * 1000)
#define PING_INTERVAL (2 * 1000)

Gecko::Gecko(HttpJson& http, Settings& settings)
    : m_http(http)
    , m_settings(settings)
{
    m_chart_48h.reserve(48);
    m_chart_60d.reserve(60);
}

void Gecko::begin()
{
    ping();
}

void Gecko::loop()
{
    if (m_succeeded) {
        if (m_settings.valid()) {
            if (m_settings.lastChange() != m_last_seen_settings) {
                m_last_seen_settings = m_settings.lastChange();
                m_last_price_fetch = 0;
                m_last_chart_48h_fetch = 0;
                m_last_chart_60d_fetch = 0;
            }
        } else {
            m_last_price_fetch = 0;
            m_last_seen_settings = 0;
        }
    } else {
        ping();
        m_last_price_fetch = 0;
        m_last_seen_settings = 0;
    }
}

void Gecko::prefetch(uint32_t coinIndex, Settings::ChartPeriod chartPeriod)
{
    LOG_FUNC

    gecko_t p;
    gecko_t p2;
    gecko_t pct;
    bool refetched;

    m_last_price_fetch = 0;
    price(coinIndex, p, p2, pct);
    if (chartPeriod == Settings::ChartPeriod::PERIOD_24_H
        || chartPeriod == Settings::ChartPeriod::PERIOD_48_H) {
        m_last_chart_48h_fetch = 0;
        chart_48h(coinIndex, refetched);
    } else {
        m_last_chart_60d_fetch = 0;
        chart_60d(coinIndex, refetched);
    }
}

void Gecko::price(uint32_t coinIndex, gecko_t& price, gecko_t& price2, gecko_t& change_pct)
{
    LOG_FUNC

    if (doInterval(m_last_price_fetch, PRICE_FETCH_INTERVAL)) {
        if (fetchCoinPriceChange(coinIndex)) {
            m_last_price_fetch = millis_test();
        } else {
            m_last_price_fetch = 0;
        }
    }
    price = m_price;
    price2 = m_price2;
    change_pct = m_change_pct;
}

void Gecko::twoPrices(gecko_t& price_1, gecko_t& price2_1, gecko_t& change_pct_1,
    gecko_t& price_2, gecko_t& price2_2, gecko_t& change_pct_2)
{
    LOG_FUNC

    if (doInterval(m_last_price_fetch, PRICE_FETCH_INTERVAL)) {
        if (fetchTwoCoinsPriceChange()) {
            m_last_price_fetch = millis_test();
        } else {
            m_last_price_fetch = 0;
        }
    }
    price_1 = m_price;
    price2_1 = m_price2;
    change_pct_1 = m_change_pct;

    price_2 = m_price_2;
    price2_2 = m_price2_2;
    change_pct_2 = m_change_pct_2;
}

const std::vector<gecko_t>& Gecko::chart_48h(uint32_t coinIndex, bool& refetched)
{
    LOG_FUNC

    refetched = false;
    if (doInterval(m_last_chart_48h_fetch, CHART_48_H_FETCH_INTERVAL)) {
        if (fetchCoinChart(coinIndex, Settings::ChartPeriod::PERIOD_48_H)) {
            m_last_chart_48h_fetch = millis_test();
            refetched = true;
        } else {
            m_last_chart_48h_fetch = 0;
        }
    }
    return m_chart_48h;
}

const std::vector<gecko_t>& Gecko::chart_60d(uint32_t coinIndex, bool& refetched)
{
    LOG_FUNC

    refetched = false;
    if (doInterval(m_last_chart_60d_fetch, CHART_60_D_FETCH_INTERVAL)) {
        if (fetchCoinChart(coinIndex, Settings::ChartPeriod::PERIOD_60_D)) {
            m_last_chart_60d_fetch = millis_test();
            refetched = true;
        } else {
            m_last_chart_60d_fetch = 0;
        }
    }
    return m_chart_60d;
}

bool Gecko::fetchCoinPriceChange(uint32_t coinIndex)
{
    LOG_FUNC

    String coinId(m_settings.coin(coinIndex));
    String currency(m_settings.currency());
    String currency2(m_settings.currency2());

    coinId.toLowerCase();
    currency.toLowerCase();
    currency2.toLowerCase();

    String change24h(currency);
    change24h += F("_24h_change");

    String url(F("https://api.coingecko.com/api/v3/simple/price?ids="));
    url += coinId;
    url += F("&vs_currencies=");
    url += currency;
    url += ",";
    url += currency2;
    url += F("&include_24hr_change=true");

    DynamicJsonDocument doc(DYNAMIC_JSON_PRICE_CHANGE_SIZE);

    if (m_http.read(url.c_str(), doc)) {
        m_price = doc[coinId][currency] | std::numeric_limits<gecko_t>::infinity();
        m_price2 = doc[coinId][currency2] | std::numeric_limits<gecko_t>::infinity();
        m_change_pct = doc[coinId][change24h] | std::numeric_limits<gecko_t>::infinity();
        m_last_price_fetch = millis_test();
        return m_price != std::numeric_limits<gecko_t>::infinity()
            && m_change_pct != std::numeric_limits<gecko_t>::infinity();
    }
    return false;
}

bool Gecko::fetchTwoCoinsPriceChange()
{
    LOG_FUNC

    String coinId1(m_settings.coin(0));
    String coinId2(m_settings.coin(1));
    String currency(m_settings.currency());
    String currency2(m_settings.currency2());

    coinId1.toLowerCase();
    coinId2.toLowerCase();
    currency.toLowerCase();
    currency2.toLowerCase();

    String change24h(currency);
    change24h += F("_24h_change");

    String url(F("https://api.coingecko.com/api/v3/simple/price?ids="));
    url += coinId1;
    url += F(",");
    url += coinId2;
    url += F("&vs_currencies=");
    url += currency;
    url += F(",");
    url += currency2;
    url += F("&include_24hr_change=true");

    DynamicJsonDocument doc(DYNAMIC_JSON_PRICE_CHANGE_SIZE);

    if (m_http.read(url.c_str(), doc)) {
        m_price = doc[coinId1][currency] | std::numeric_limits<gecko_t>::infinity();
        m_price2 = doc[coinId1][currency2] | std::numeric_limits<gecko_t>::infinity();
        m_change_pct = doc[coinId1][change24h] | std::numeric_limits<gecko_t>::infinity();

        m_price_2 = doc[coinId2][currency] | std::numeric_limits<gecko_t>::infinity();
        m_price2_2 = doc[coinId2][currency2] | std::numeric_limits<gecko_t>::infinity();
        m_change_pct_2 = doc[coinId2][change24h] | std::numeric_limits<gecko_t>::infinity();

        m_last_price_fetch = millis_test();
        return m_price != std::numeric_limits<gecko_t>::infinity()
            && m_price_2 != std::numeric_limits<gecko_t>::infinity()
            && m_change_pct != std::numeric_limits<gecko_t>::infinity()
            && m_change_pct_2 != std::numeric_limits<gecko_t>::infinity();
    }
    return false;
}

bool Gecko::fetchCoinChart(uint32_t coinIndex, Settings::ChartPeriod type)
{
    LOG_FUNC

#if COIN_THING_SERIAL > 0
    Serial.printf("type: %u\n", type);
#endif

    String coinId(m_settings.coin(coinIndex));
    String currency(m_settings.currency());

    coinId.toLowerCase();
    currency.toLowerCase();

    String url(F("https://api.coingecko.com/api/v3/coins/"));
    url += coinId;
    url += F("/market_chart?vs_currency=");
    url += currency;
    std::vector<gecko_t>* targetChart;

    uint8_t expectValues;
    if (type == Settings::ChartPeriod::PERIOD_24_H
        || type == Settings::ChartPeriod::PERIOD_48_H) {
        url += F("&days=2");
        expectValues = 48;
        m_last_chart_48h_fetch = millis_test();
        targetChart = &m_chart_48h;
    } else if (type == Settings::ChartPeriod::PERIOD_30_D
        || type == Settings::ChartPeriod::PERIOD_60_D) {
        url += F("&days=60&interval=daily");
        expectValues = 60;
        m_last_chart_60d_fetch = millis_test();
        targetChart = &m_chart_60d;
    } else {
        return false;
    }
    targetChart->clear();
    targetChart->reserve(expectValues);

    DynamicJsonDocument filter(16);
    filter["prices"] = true;

    DynamicJsonDocument doc(DYNAMIC_JSON_CHART_SIZE);

    if (m_http.read(url.c_str(), doc, filter)) {
        JsonArray jPrices = doc["prices"];
        size_t jj(jPrices.size());
        for (const auto& p : jPrices) {
            if (jj <= expectValues) { // omit oldest, because there are some more (1, 2) entries in JSON result than expected
                auto f(p[1].as<gecko_t>());
                targetChart->emplace_back(f);
            }
            --jj;
        }
        return !targetChart->empty();
    }
    return false;
}

bool Gecko::ping()
{
    if (doInterval(m_last_ping, PING_INTERVAL)) {
        m_succeeded = false;
        DynamicJsonDocument doc(DYNAMIC_JSON_PING_SIZE);
        if (m_http.read("https://api.coingecko.com/api/v3/ping", doc)) {
            const char* gecko_says = doc["gecko_says"] | ""; // "(V3) To the Moon!"
            m_succeeded = strcmp(gecko_says, "(V3) To the Moon!") == 0;
        }
        m_last_ping = millis_test();
    }
    return m_succeeded;
}
