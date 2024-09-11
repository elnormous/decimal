#ifndef EDL_DECIMAL_HPP
#define EDL_DECIMAL_HPP

#include <cstdint>

namespace edl
{
    template<std::size_t size> struct traits;

    template<> struct traits<32U>
    {
        using unsigned_type = std::uint32_t;
        using signed_type = std::int32_t;
        static constexpr std::uint32_t sign_offset = 31U;
        static constexpr std::uint32_t exponent_offset = 23U;
        static constexpr std::int32_t exponent_bias = 127;
        static constexpr std::uint32_t exponent_mask = 0xFFU;
        static constexpr std::uint32_t significand_mask = 0x7FFFFFU;
    };

    template<> struct traits<64U>
    {
        using unsigned_type = std::uint64_t;
        using signed_type = std::int64_t;
        static constexpr std::uint64_t sign_offset = 63U;
        static constexpr std::uint64_t exponent_offset = 52U;
        static constexpr std::int64_t exponent_bias = 1023;
        static constexpr std::uint64_t exponent_mask = 0x7FFU;
        static constexpr std::uint64_t significand_mask = 0xFFFFFFFFFFFFFU;
    };

    template<std::size_t size>
    class decimal final
    {
        constexpr auto sign() const noexcept { return static_cast<bool>(d >> traits<size>::sign_offset); }
        constexpr auto exponent() const noexcept { return static_cast<typename traits<size>::signed_type>((d >> traits<size>::exponent_offset) & traits<size>::exponent_mask) - traits<size>::exponent_bias; }
        constexpr auto significand() const noexcept { return d & traits<size>::significand_mask; }

        typename traits<size>::unsigned_type d = static_cast<typename traits<size>::unsigned_type>(traits<size>::exponent_bias) << traits<size>::exponent_offset;

    public:
        constexpr decimal() noexcept = default;

        constexpr decimal(typename traits<size>::unsigned_type signif, typename traits<size>::signed_type exp, bool sig) noexcept:
            d{
                (sig ? 0x01U : 0x00U) << traits<size>::sign_offset |
                ((exp < 0 ?
                  (traits<size>::exponent_bias - 1U - ~static_cast<typename traits<size>::unsigned_type>(exp)) :
                  (static_cast<typename traits<size>::unsigned_type>(exp) + traits<size>::exponent_bias)) & traits<size>::exponent_mask) << traits<size>::exponent_offset | (signif & traits<size>::significand_mask)
            }
        {
        }

        constexpr decimal(typename traits<size>::signed_type value, typename traits<size>::signed_type exp) noexcept:
            decimal{value < 0 ? ~static_cast<typename traits<size>::unsigned_type>(value) + 0x01U : static_cast<typename traits<size>::unsigned_type>(value), exp, value < 0}
        {
        }

        constexpr decimal(typename traits<size>::signed_type value) noexcept:
            decimal{value < 0 ? ~static_cast<typename traits<size>::unsigned_type>(value) + 0x01U : static_cast<typename traits<size>::unsigned_type>(value), 0U, value < 0}
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
            result.d ^= (1U << traits<size>::sign_offset);
            return result;
        }

        [[nodiscard]] constexpr decimal operator+(decimal other) const noexcept
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
                return decimal{(self_sign ?
                                -static_cast<typename traits<size>::signed_type>(self_significand) :
                                static_cast<typename traits<size>::signed_type>(self_significand)) + (other_sign ? -static_cast<typename traits<size>::signed_type>(other_significand) : static_cast<typename traits<size>::signed_type>(other_significand)), self_exponent};

            return *this;
        }

        [[nodiscard]] constexpr decimal operator-(decimal other) const noexcept
        {
            const auto self_sign = sign();
            const auto self_exponent = exponent();
            const auto self_significand = significand();

            const auto other_sign = other.sign();
            const auto other_exponent = other.exponent();
            const auto other_significand = other.significand();

            if (self_significand == 0)
                return decimal{other_significand, self_exponent, !other_sign};

            if (other_significand == 0)
                return *this;

            if (self_exponent == other_exponent)
                return decimal{(self_sign ?
                                -static_cast<typename traits<size>::signed_type>(self_significand) :
                                static_cast<typename traits<size>::signed_type>(self_significand)) - (other_sign ? -static_cast<typename traits<size>::signed_type>(other_significand) : static_cast<typename traits<size>::signed_type>(other_significand)), self_exponent};

            return *this;
        }

        [[nodiscard]] constexpr auto data() const noexcept
        {
            return d;
        }
    };

    using decimal32 = decimal<32U>;
    using decimal64 = decimal<64U>;

    template<std::size_t size>
    inline std::string to_string(const decimal<size>& value)
    {
        const auto sign = value.data() >> 31U;
        const auto exponent = static_cast<typename traits<size>::signed_type>((value.data() >> traits<size>::exponent_offset) & traits<size>::exponent_mask) - traits<size>::exponent_bias;
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
