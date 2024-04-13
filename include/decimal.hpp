#ifndef EDL_DECIMAL_HPP
#define EDL_DECIMAL_HPP

#include <cstdint>

namespace edl
{
    class decimal32 final
    {
        static constexpr std::uint32_t sign_offset = 31U;
        static constexpr std::uint32_t exponent_offset = 23U;
        static constexpr std::int32_t exponent_bias = 127;
        static constexpr std::uint32_t exponent_mask = 0xFFU;
        static constexpr std::uint32_t significand_mask = 0x1FFFFFU;

    public:
        constexpr decimal32() noexcept = default;

        constexpr decimal32(std::uint32_t signif, std::int32_t exp, bool sig) noexcept:
            d{
                (sig ? 0x01U : 0x00U) << sign_offset |
                ((exp < 0 ? (exponent_bias - 1 - ~static_cast<std::uint32_t>(exp)) : (exp + exponent_bias)) & exponent_mask) << exponent_offset |
                (signif & significand_mask)
        }
        {
        }

        constexpr decimal32(std::int32_t value, std::int32_t exp) noexcept:
            decimal32{value < 0 ? ~static_cast<std::uint32_t>(value) + 0x01U : static_cast<std::uint32_t>(value), exp, value < 0}
        {
        }

        constexpr decimal32(std::int32_t value) noexcept:
            decimal32{value < 0 ? ~static_cast<std::uint32_t>(value) + 0x01U : static_cast<std::uint32_t>(value), 0U, value < 0}
        {
        }

        constexpr decimal32(const decimal32&) = default;
        constexpr decimal32(decimal32&&) = default;

        [[nodiscard]] constexpr bool operator==(const decimal32& other) const noexcept
        {
            return d == other.d;
        }

        [[nodiscard]] constexpr bool operator!=(const decimal32& other) const noexcept
        {
            return d != other.d;
        }

        [[nodiscard]] constexpr decimal32 operator-() const noexcept
        {
            decimal32 result = *this;
            result.d ^= (1U << sign_offset);
            return result;
        }

        [[nodiscard]] constexpr decimal32 operator+(decimal32 other) const noexcept
        {
            const auto self_sign = sign();
            const auto self_exponent = exponent();
            const auto self_significand = significand();

            const auto other_sign = other.sign();
            const auto other_exponent = other.exponent();
            const auto other_significand = other.significand();

            if (self_significand == 0)
                return other;

            if (other_significand == 0)
                return *this;

            if (self_exponent == other_exponent)
                return decimal32{(self_sign ? -static_cast<std::int32_t>(self_significand) : static_cast<std::int32_t>(self_significand)) + (other_sign ? -static_cast<std::int32_t>(other_significand) : static_cast<std::int32_t>(other_significand)), self_exponent};

            return *this;
        }

        [[nodiscard]] constexpr decimal32 operator-(decimal32 other) const noexcept
        {
            const auto self_sign = sign();
            const auto self_exponent = exponent();
            const auto self_significand = significand();

            const auto other_sign = other.sign();
            const auto other_exponent = other.exponent();
            const auto other_significand = other.significand();

            if (self_significand == 0)
                return decimal32{other_significand, self_exponent, !other_sign};

            if (other_significand == 0)
                return *this;

            if (self_exponent == other_exponent)
                return decimal32{(self_sign ? -static_cast<std::int32_t>(self_significand) : static_cast<std::int32_t>(self_significand)) - (other_sign ? -static_cast<std::int32_t>(other_significand) : static_cast<std::int32_t>(other_significand)), self_exponent};

            return *this;
        }

        [[nodiscard]] constexpr std::uint32_t data() const noexcept
        {
            return d;
        }

    private:
        constexpr auto sign() const noexcept { return static_cast<bool>(d >> sign_offset); }
        constexpr auto exponent() const noexcept { return static_cast<std::int32_t>((d >> exponent_offset) & exponent_mask) - exponent_bias; }
        constexpr auto significand() const noexcept { return d & significand_mask; }

        std::uint32_t d = 127U << exponent_offset;
    };

    inline std::string to_string(const decimal32& value)
    {
        const auto sign = value.data() >> 31U;
        const auto exponent = static_cast<std::int32_t>((value.data() >> 23U) & 0xFFU) - 127;
        const auto significand = value.data() & 0x1FFFFFU;

        std::string result;
        if (sign) result += '-';
        if (significand == 0U)
        {
            result += "0.0";
            return result;
        }
        else
        {
            std::uint32_t digits = 0U;
            std::uint32_t divisor = 1U;
            for (std::uint32_t i = significand; i != 0U; i /= 10U)
            {
                ++digits;
                divisor *= 10U;
            }

            const auto point = exponent + static_cast<std::int32_t>(digits);
            if (exponent < 0 && point <= 0)
            {
                result += "0.";
                for (int i = point; i < 0; ++i) result += '0';
            }

            for (std::uint32_t i = 0U, f = significand; i < digits; ++i)
            {
                divisor /= 10U;

                if (point > 0 && static_cast<std::uint32_t>(point) == i) result += '.';
                result += static_cast<char>('0' + f / divisor);
                f %= divisor;
            }

            if (point > 0)
            {
                for (int i = 0; i < exponent; ++i) result += '0';
                if (static_cast<std::uint32_t>(point) >= digits) result += ".0";
            }

            return result;
        }
    }

    inline decimal32 stod32(const std::string& str, std::size_t* pos = nullptr)
    {
        std::size_t i = 0U;
        bool sign = false;
        if (str[i] == '-')
        {
            sign = true;
            ++i;
        }

        std::uint32_t significand = 0U;
        int exponent = 0;

        for (; i < str.size(); ++i)
        {
            if (str[i] < '0' || str[i] > '9') break;

            if (significand * 10U + static_cast<std::uint32_t>(str[i] - '0') <= 0x1FFFFFU)
                significand = significand * 10U + static_cast<std::uint32_t>(str[i] - '0');
            else
                ++exponent;
        }

        if (str[i] == '.')
        {
            ++i;

            for (; i < str.size(); ++i)
            {
                if (str[i] < '0' || str[i] > '9') break;

                if (exponent <= 0)
                {
                    significand = significand * 10U + static_cast<std::uint32_t>(str[i] - '0');
                    --exponent;
                }
            }
        }

        if (pos) *pos = i;

        return decimal32{significand, exponent, sign};
    }
}

#endif
