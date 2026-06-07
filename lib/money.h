#ifndef MONEY_H
#define MONEY_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct {
    int64_t subunits;   // e.g., 100 subunits = $1.00
    char currency[4];   // ISO 4217 Currency Code (e.g., "USD")
} Money;

typedef enum {
    MONEY_OK = 0,
    MONEY_ERR_CURRENCY_MISMATCH,
    MONEY_ERR_OVERFLOW,
    MONEY_ERR_INVALID_ARG,
    MONEY_ERR_BUFFER_TOO_SMALL
} MoneyStatus;

// Initialize a Money struct safely
MoneyStatus money_make(Money *m, int64_t subunits, const char *currency);

// Arithmetic operations with overflow checks
MoneyStatus money_add(const Money *a, const Money *b, Money *result);
MoneyStatus money_sub(const Money *a, const Money *b, Money *result);

// Scale money by a factor (e.g., interest, tax, splits) with round-half-away-from-zero
MoneyStatus money_scale(const Money *m, double factor, Money *result);

// Compare two money values. Sets *result to -1 (a < b), 0 (a == b), or 1 (a > b)
MoneyStatus money_compare(const Money *a, const Money *b, int *result);

// Format money to string (e.g., "USD 123.45" or "USD -0.05")
MoneyStatus money_format(const Money *m, char *buf, size_t buf_sz);

#endif // MONEY_H
