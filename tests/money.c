#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "money.h"

void test_creation() {
    Money m;
    assert(money_make(&m, 1050, "USD") == MONEY_OK);
    assert(m.subunits == 1050);
    assert(strcmp(m.currency, "USD") == 0);

    // Invalid currencies
    assert(money_make(&m, 100, "usd") == MONEY_ERR_INVALID_ARG); // must be uppercase
    assert(money_make(&m, 100, "USDT") == MONEY_ERR_INVALID_ARG); // 3 letters only
    assert(money_make(&m, 100, "US") == MONEY_ERR_INVALID_ARG);   // too short
}

void test_addition_and_subtraction() {
    Money a, b, res;
    money_make(&a, 1050, "USD"); // $10.50
    money_make(&b, 450, "USD");  //  $4.50

    // Basic Add
    assert(money_add(&a, &b, &res) == MONEY_OK);
    assert(res.subunits == 1500);

    // Basic Sub
    assert(money_sub(&a, &b, &res) == MONEY_OK);
    assert(res.subunits == 600);

    // Currency Mismatch Protection
    Money eur;
    money_make(&eur, 500, "EUR");
    assert(money_add(&a, &eur, &res) == MONEY_ERR_CURRENCY_MISMATCH);
    assert(money_sub(&a, &eur, &res) == MONEY_ERR_CURRENCY_MISMATCH);
}

void test_overflows() {
    Money max, min, one, res;
    money_make(&max, INT64_MAX, "USD");
    money_make(&min, INT64_MIN, "USD");
    money_make(&one, 1, "USD");

    // Add overflow
    assert(money_add(&max, &one, &res) == MONEY_ERR_OVERFLOW);
    // Sub overflow
    assert(money_sub(&min, &one, &res) == MONEY_ERR_OVERFLOW);
}

void test_scaling_and_rounding() {
    Money m, res;
    
    // Test scale up
    money_make(&m, 100, "USD"); // $1.00
    assert(money_scale(&m, 1.0825, &res) == MONEY_OK); // 8.25% Tax
    assert(res.subunits == 108); // rounds 108.25 -> 108

    // Test Round half-up (away from zero)
    money_make(&m, 105, "USD"); // $1.05
    assert(money_scale(&m, 1.10, &res) == MONEY_OK); // 105 * 1.10 = 115.5
    assert(res.subunits == 116); // rounds up to 116

    // Test Scale overflow
    money_make(&m, INT64_MAX / 2 + 100, "USD");
    assert(money_scale(&m, 2.01, &res) == MONEY_ERR_OVERFLOW);
}

void test_comparison() {
    Money a, b;
    int res;
    money_make(&a, 100, "USD");
    money_make(&b, 200, "USD");

    assert(money_compare(&a, &b, &res) == MONEY_OK);
    assert(res == -1);

    assert(money_compare(&b, &a, &res) == MONEY_OK);
    assert(res == 1);

    assert(money_compare(&a, &a, &res) == MONEY_OK);
    assert(res == 0);
}

void test_formatting() {
    Money m;
    char buf[32];

    money_make(&m, 123456, "USD"); // $1234.56
    assert(money_format(&m, buf, sizeof(buf)) == MONEY_OK);
    assert(strcmp(buf, "USD 1234.56") == 0);

    // Negative value formatting
    money_make(&m, -5, "EUR"); // -0.05 EUR
    assert(money_format(&m, buf, sizeof(buf)) == MONEY_OK);
    assert(strcmp(buf, "EUR -0.05") == 0);

    // Buffer too small protection
    assert(money_format(&m, buf, 5) == MONEY_ERR_BUFFER_TOO_SMALL);
}

int main() {
    test_creation();
    test_addition_and_subtraction();
    test_overflows();
    test_scaling_and_rounding();
    test_comparison();
    test_formatting();

    printf("ALL TESTS PASSED SUCCESSFULLY!\n");
    return 0;
}
