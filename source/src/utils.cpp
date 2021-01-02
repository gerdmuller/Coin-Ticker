#include "utils.h"

String cleanUp(const String& s)
{
    String ret(s.substring(0, std::string(s.c_str()).find_first_not_of("abcdefghijklmnopqrstuvwxyz-ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789")));
    ret.toLowerCase();
    return ret;
}

void formatNumber(double n, String& s, NumberFormat format, bool forceSign)
{
    char buf[21];
    char buf2[21];

    uint8_t decimalPlaces(2);
    if ((n < 0 ? n * -1. : n) < 0.000001) {
        decimalPlaces = 8;
    } else if ((n < 0 ? n * -1. : n) < 0.00001) {
        decimalPlaces = 8;
    } else if ((n < 0 ? n * -1. : n) < 0.0001) {
        decimalPlaces = 7;
    } else if ((n < 0 ? n * -1. : n) < 0.001) {
        decimalPlaces = 6;
    } else if ((n < 0 ? n * -1. : n) < 0.01) {
        decimalPlaces = 6;
    } else if ((n < 0 ? n * -1. : n) < 0.1) {
        decimalPlaces = 5;
    } else if ((n < 0 ? n * -1. : n) < 1) {
        decimalPlaces = 5;
    }

    if (forceSign) {
        snprintf(buf, sizeof(buf), "%+.*f", decimalPlaces, n);
    } else {
        snprintf(buf, sizeof(buf), "%.*f", decimalPlaces, n);
    }

    bool hasDecimalSep(false);
    if (format != NumberFormat::DECIMAL_DOT) {
        int rev(0);
        for (auto pos = strlen(buf); pos != 0; --pos, ++rev) {
            char c = buf[pos - 1];
            if (c == '.') {
                hasDecimalSep = true;
            }
            buf2[rev] = c;
        }
        buf2[rev] = 0; // buf2 holds number string in reverse order

        bool decimalSeparator(false);
        uint8_t jj(sizeof(buf) - 1);
        uint8_t group(0);

        buf[jj] = 0;
        --jj;

        for (uint8_t ii = 0; buf2[ii] != 0 && jj != 0; ++ii) {
            char c = buf2[ii];

            if (!hasDecimalSep || decimalSeparator) {
                ++group;
            }

            if (c == '.') {
                if (format == NumberFormat::THOUSAND_DOT_DECIMAL_COMMA
                    || format == NumberFormat::DECIMAL_COMMA) {
                    c = ',';
                }
                decimalSeparator = true;
            }

            buf[jj] = c;
            --jj;

            if (group > 0
                && group % 3 == 0
                && buf2[ii + 1] != 0
                && buf2[ii + 1] != '+') {
                if (format == NumberFormat::THOUSAND_DOT_DECIMAL_COMMA) {
                    buf[jj] = '.';
                    --jj;
                } else if (format == NumberFormat::THOUSAND_COMMA_DECIMAL_DOT) {
                    buf[jj] = ',';
                    --jj;
                }
            }
        }
        s = &buf[jj + 1];
    } else {
        s = buf;
    }

    if (format == NumberFormat::THOUSAND_DOT_DECIMAL_COMMA
        || format == NumberFormat::DECIMAL_COMMA) {
        s.replace(",00", ",‒"); // Figure Dash
    } else {
        s.replace(".00", ".‒"); // Figure Dash
    }
}

const char* getCurrencySymbol(const char* currency)
{
    if (strcmp(currency, "eur") == 0) {
        return "€";
    } else if (strcmp(currency, "usd") == 0) {
        return "$";
    } else {
        return currency;
    }
}
