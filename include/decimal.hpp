#ifndef EDL_DECIMAL_HPP
#define EDL_DECIMAL_HPP

#include <cstdint>

namespace edl
{
    template<std::size_t size> class decimal;

    template<> class decimal<32U>
    {
        static constexpr std::uint32_t signOffset = 31U;
        static constexpr std::uint32_t exponentOffset = 23U;
        static constexpr std::uint32_t exponentBias = 127;
        static constexpr std::uint32_t exponentMask = 0xFFU;
        static constexpr std::uint32_t significandMask = 0x1FFFFFU;

    public:
        constexpr decimal() noexcept = default;
        constexpr decimal(int value, int exponent = 0) noexcept:
            d{
                (value < 0 ? 0x01U : 0x00U) << signOffset |
                ((exponent < 0 ? (exponentBias - 1 - ~static_cast<std::uint32_t>(exponent)) : (static_cast<std::uint32_t>(exponent) + exponentBias)) & exponentMask) << exponentOffset |
                ((value < 0 ? ~static_cast<std::uint32_t>(value) + 0x01U : static_cast<std::uint32_t>(value)) & significandMask)
            }
        {
        }

        constexpr decimal(const decimal&) = default;
        constexpr decimal(decimal&&) = default;

        [[nodiscard]] constexpr bool operator==(const decimal& other) const noexcept
        {
            return d == other.d;
        }

        [[nodiscard]] constexpr bool operator!=(const decimal& other) const noexcept
        {
            return d != other.d;
        }

        [[nodiscard]] constexpr decimal operator-() const noexcept
        {
            decimal result = *this;
            result.d ^= (1U << signOffset);
            return result;
        }

        [[nodiscard]] constexpr std::uint32_t data() const noexcept
        {
            return d;
        }

    private:
        std::uint32_t d = 127U << exponentOffset;
    };

    using decimal32 = decimal<32U>;

    inline std::string to_string(const decimal<32>& value)
    {
        const auto sign = value.data() >> 31U;
        const auto exponent = static_cast<int>((value.data() >> 23U) & 0xFFU) - 127;
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

            const auto point = exponent + static_cast<int>(digits);
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

            if (significand * 10U + static_cast<std::uint32_t>(str[i] - '0') < 0x1FFFFFU)
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

                significand = significand * 10U + static_cast<std::uint32_t>(str[i] - '0');
                --exponent;
            }
        }

        if (pos) *pos = i;

        return decimal32{sign ? -static_cast<int>(significand) : static_cast<int>(significand), exponent};
    }
}

#endif
