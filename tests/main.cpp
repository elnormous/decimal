#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include "Decimal.hpp"

TEST_CASE("Default constructor")
{
    edl::decimal d;
}

TEST_CASE("Zero")
{
    edl::decimal d{0};
    CHECK(d == 0);
    CHECK(d != 1);
    CHECK(d != -1);
    CHECK(d != -2);

    CHECK(((d.data() >> 31U) & 0x01U) == 0U); // sign
    CHECK(((d.data() >> 23U) & 0xFFU)  == 127U); // exponent
    CHECK((d.data() & 0x1FFFFFU) == 0U); // fraction
}

TEST_CASE("One")
{
    edl::decimal d{1};
    CHECK(d != 0);
    CHECK(d == 1);
    CHECK(d != -1);
    CHECK(d != -2);

    CHECK(((d.data() >> 31U) & 0x01U) == 0U); // sign
    CHECK(((d.data() >> 23U) & 0xFFU) == 127U); // exponent
    CHECK((d.data() & 0x1FFFFFU) == 1U); // fraction
}

TEST_CASE("Minus one")
{
    edl::decimal d{-1};
    CHECK(d != 0);
    CHECK(d != 1);
    CHECK(d == -1);
    CHECK(d != -2);
    CHECK(d != 0x1FFFFF);
    CHECK(d != 0x1FFFFE);

    CHECK(((d.data() >> 31U) & 0x01U) == 1U); // sign
    CHECK(((d.data() >> 23U) & 0xFFU) == 127U); // exponent
    CHECK((d.data() & 0x1FFFFFU) == 1U); // fraction
}

TEST_CASE("Minus two")
{
    edl::decimal d{-2};
    CHECK(d != 0);
    CHECK(d != 1);
    CHECK(d != -1);
    CHECK(d == -2);
    CHECK(d != 0x1FFFFF);
    CHECK(d != 0x1FFFFE);

    CHECK(((d.data() >> 31U) & 0x01U) == 1U); // sign
    CHECK(((d.data() >> 23U) & 0xFFU) == 127U); // exponent
    CHECK((d.data() & 0x1FFFFFU) == 2U); // fraction
}

TEST_CASE("Exponent")
{
    edl::decimal d{-2, 1};
    CHECK(d != 0);
    CHECK(d != 1);
    CHECK(d != -1);
    CHECK(d != -2);
    CHECK(d != 0x1FFFFF);
    CHECK(d != 0x1FFFFE);
    CHECK(d != edl::decimal{-1, 1});
    CHECK(d == edl::decimal{-2, 1});
    CHECK(d != edl::decimal{-1, -1});
    CHECK(d != edl::decimal{-2, -1});

    CHECK(((d.data() >> 31U) & 0x01U) == 1U); // sign
    CHECK(((d.data() >> 23U) & 0xFFU) == 128U); // exponent
    CHECK((d.data() & 0x1FFFFFU) == 2U); // fraction
}

TEST_CASE("Negative xponent")
{
    edl::decimal d{-2, -1};
    CHECK(d != 0);
    CHECK(d != 1);
    CHECK(d != -1);
    CHECK(d != -2);
    CHECK(d != 0x1FFFFF);
    CHECK(d != 0x1FFFFE);
    CHECK(d != edl::decimal{-1, 1});
    CHECK(d != edl::decimal{-2, 1});
    CHECK(d != edl::decimal{-1, -1});
    CHECK(d == edl::decimal{-2, -1});

    CHECK(((d.data() >> 31U) & 0x01U) == 1U); // sign
    CHECK(((d.data() >> 23U) & 0xFFU) == 126U); // exponent
    CHECK((d.data() & 0x1FFFFFU) == 2U); // fraction
}

