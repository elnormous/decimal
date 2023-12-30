#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include "Decimal.hpp"

TEST_CASE("Default constructor")
{
    edl::decimal32 d;
    CHECK(d == 0);
    CHECK(d != 1);
    CHECK(d != -1);
    CHECK(d != -2);

    CHECK(((d.data() >> 31U) & 0x01U) == 0U); // sign
    CHECK(((d.data() >> 23U) & 0xFFU) == 127U); // exponent
    CHECK((d.data() & 0x1FFFFFU) == 0U); // fraction
}

TEST_CASE("Zero")
{
    edl::decimal32 d{0};
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
    edl::decimal32 d{1};
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
    edl::decimal32 d{-1};
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
    edl::decimal32 d{-2};
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
    edl::decimal32 d{-2, 1};
    CHECK(d != 0);
    CHECK(d != 1);
    CHECK(d != -1);
    CHECK(d != -2);
    CHECK(d != 0x1FFFFF);
    CHECK(d != 0x1FFFFE);
    CHECK(d != edl::decimal32{-1, 1});
    CHECK(d == edl::decimal32{-2, 1});
    CHECK(d != edl::decimal32{-1, -1});
    CHECK(d != edl::decimal32{-2, -1});

    CHECK(((d.data() >> 31U) & 0x01U) == 1U); // sign
    CHECK(((d.data() >> 23U) & 0xFFU) == 128U); // exponent
    CHECK((d.data() & 0x1FFFFFU) == 2U); // fraction
}

TEST_CASE("Negative exponent")
{
    edl::decimal32 d{-2, -1};
    CHECK(d != 0);
    CHECK(d != 1);
    CHECK(d != -1);
    CHECK(d != -2);
    CHECK(d != 0x1FFFFF);
    CHECK(d != 0x1FFFFE);
    CHECK(d != edl::decimal32{-1, 1});
    CHECK(d != edl::decimal32{-2, 1});
    CHECK(d != edl::decimal32{-1, -1});
    CHECK(d == edl::decimal32{-2, -1});

    CHECK(((d.data() >> 31U) & 0x01U) == 1U); // sign
    CHECK(((d.data() >> 23U) & 0xFFU) == 126U); // exponent
    CHECK((d.data() & 0x1FFFFFU) == 2U); // fraction
}

TEST_CASE("To string")
{
    CHECK(to_string(edl::decimal32{0}) == "0.0");
    CHECK(to_string(edl::decimal32{1}) == "1.0");
    CHECK(to_string(edl::decimal32{-1}) == "-1.0");
    CHECK(to_string(edl::decimal32{1, 1}) == "10.0");
    CHECK(to_string(edl::decimal32{1, -1}) == "0.1");
    CHECK(to_string(edl::decimal32{1, -2}) == "0.01");
    CHECK(to_string(edl::decimal32{10, 3}) == "10000.0");
    CHECK(to_string(edl::decimal32{50, 2}) == "5000.0");
    CHECK(to_string(edl::decimal32{15, 4}) == "150000.0");
    CHECK(to_string(edl::decimal32{-15, -4}) == "-0.0015");
    CHECK(to_string(edl::decimal32{-15, -1}) == "-1.5");
    CHECK(to_string(edl::decimal32{150, -1}) == "15.0");
}

TEST_CASE("From string")
{
    CHECK(edl::stod32("0.0") == edl::decimal32{0, -1});
    CHECK(edl::stod32("-1.0") == edl::decimal32{-10, -1});
    CHECK(edl::stod32("-1.5") == edl::decimal32{-15, -1});
}