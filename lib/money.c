#include "money.h"
#include <string.h>
#include <math.h>
#include <stdio.h>

static bool is_valid_currency(const char *currency) {
    if (!currency) return false;
    for (int i = 0; i < 3; i++) {
        if (currency[i] < 'A' || currency[i] > 'Z') return false;
    }
    return currency[3] == '\0';
}

MoneyStatus money_make(Money *m, int64_t subunits, const char *currency) {
    if (!m || !is_valid_currency(currency)) return MONEY_ERR_INVALID_ARG;
    m->subunits = subunits;
    memcpy(m->currency, currency, 4);
    return MONEY_OK;
}

MoneyStatus money_add(const Money *a, const Money *b, Money *result) {
    if (!a || !b || !result) return MONEY_ERR_INVALID_ARG;
    if (strcmp(a->currency, b->currency) != 0) return MONEY_ERR_CURRENCY_MISMATCH;

    // Check for overflow before adding
    if ((b->subunits > 0 && a->subunits > INT64_MAX - b->subunits) ||
        (b->subunits < 0 && a->subunits < INT64_MIN - b->subunits)) {
        return MONEY_ERR_OVERFLOW;
    }

    result->subunits = a->subunits + b->subunits;
    memcpy(result->currency, a->currency, 4);
    return MONEY_OK;
}

MoneyStatus money_sub(const Money *a, const Money *b, Money *result) {
    if (!a || !b || !result) return MONEY_ERR_INVALID_ARG;
    if (strcmp(a->currency, b->currency) != 0) return MONEY_ERR_CURRENCY_MISMATCH;

    // Check for overflow before subtracting
    if ((b->subunits < 0 && a->subunits > INT64_MAX + b->subunits) ||
        (b->subunits > 0 && a->subunits < INT64_MIN + b->subunits)) {
        return MONEY_ERR_OVERFLOW;
    }

    result->subunits = a->subunits - b->subunits;
    memcpy(result->currency, a->currency, 4);
    return MONEY_OK;
}

MoneyStatus money_scale(const Money *m, double factor, Money *result) {
    if (!m || !result) return MONEY_ERR_INVALID_ARG;
    if (isnan(factor) || isinf(factor)) return MONEY_ERR_INVALID_ARG;

    // Perform multiplication using double, round-half-away-from-zero
    double calculated = round((double)m->subunits * factor);

    if (calculated > (double)INT64_MAX || calculated < (double)INT64_MIN) {
        return MONEY_ERR_OVERFLOW;
    }

    result->subunits = (int64_t)calculated;
    memcpy(result->currency, m->currency, 4);
    return MONEY_OK;
}

MoneyStatus money_compare(const Money *a, const Money *b, int *result) {
    if (!a || !b || !result) return MONEY_ERR_INVALID_ARG;
    if (strcmp(a->currency, b->currency) != 0) return MONEY_ERR_CURRENCY_MISMATCH;

    if (a->subunits < b->subunits) *result = -1;
    else if (a->subunits > b->subunits) *result = 1;
    else *result = 0;

    return MONEY_OK;
}

MoneyStatus money_format(const Money *m, char *buf, size_t buf_sz) {
    if (!m || !buf || buf_sz == 0) return MONEY_ERR_INVALID_ARG;

    int64_t abs_subunits = m->subunits < 0 ? -m->subunits : m->subunits;
    int64_t dollars = abs_subunits / 100;
    int64_t cents = abs_subunits % 100;
    const char *sign = m->subunits < 0 ? "-" : "";

    int written = snprintf(buf, buf_sz, "%s %s%lld.%02lld", m->currency, sign, (long long)dollars, (long long)cents);

    if (written < 0 || (size_t)written >= buf_sz) {
        return MONEY_ERR_BUFFER_TOO_SMALL;
    }
    return MONEY_OK;
}
