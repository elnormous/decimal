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
        static constexpr unsigned_type sign_offset = 31U;
        static constexpr unsigned_type exponent_offset = 23U;
        static constexpr signed_type exponent_bias = 127;
        static constexpr unsigned_type exponent_mask = 0xFFU;
        static constexpr unsigned_type significand_mask = 0x7FFFFFU;
    };

    template<> struct traits<64U>
    {
        using unsigned_type = std::uint64_t;
        using signed_type = std::int64_t;
        static constexpr unsigned_type sign_offset = 63U;
        static constexpr unsigned_type exponent_offset = 52U;
        static constexpr signed_type exponent_bias = 1023;
        static constexpr unsigned_type exponent_mask = 0x7FFU;
        static constexpr unsigned_type significand_mask = 0xFFFFFFFFFFFFFU;
    };

    template<std::size_t size>
    class decimal final
    {
        using unsigned_type = typename traits<size>::unsigned_type;
        using signed_type = typename traits<size>::signed_type;
        static constexpr auto sign_offset = traits<size>::sign_offset;
        static constexpr auto exponent_offset = traits<size>::exponent_offset;
        static constexpr auto exponent_bias = traits<size>::exponent_bias;
        static constexpr auto exponent_mask = traits<size>::exponent_mask;
        static constexpr auto significand_mask = traits<size>::significand_mask;

        constexpr auto sign() const noexcept { return static_cast<bool>(d >> sign_offset); }
        constexpr auto exponent() const noexcept { return static_cast<signed_type>((d >> exponent_offset) & exponent_mask) - exponent_bias; }
        constexpr auto significand() const noexcept { return d & significand_mask; }
        constexpr auto value() const noexcept { return (d & significand_mask) ? -static_cast<signed_type>(significand()) : static_cast<signed_type>(significand()); }

        unsigned_type d = static_cast<unsigned_type>(exponent_bias) << exponent_offset;

    public:
        constexpr decimal() noexcept = default;

        constexpr decimal(const signed_type value, const signed_type exp = 0) noexcept:
            d{
                static_cast<unsigned_type>(value < 0 ? 0x01U : 0x00U) << sign_offset |
                ((exp < 0 ?
                  (exponent_bias - 1U - ~static_cast<unsigned_type>(exp)) :
                  (static_cast<unsigned_type>(exp) + exponent_bias)) & exponent_mask) << exponent_offset |
                (static_cast<unsigned_type>(value >= 0 ? value : -value) & significand_mask)
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
            result.d ^= (1U << sign_offset);
            return result;
        }

        [[nodiscard]] constexpr decimal operator+(const decimal other) const noexcept
        {
            const auto self_sign = sign();
            const auto self_exponent = exponent();
            const auto self_significand = significand();

            const auto other_sign = other.sign();
            const auto other_exponent = other.exponent();
            const auto other_significand = other.significand();

            if (self_significand == 0U)
                return other;

            if (other_significand == 0U)
                return *this;

            if (self_exponent == other_exponent)
                return decimal{(self_sign ?
                                -static_cast<signed_type>(self_significand) :
                                static_cast<signed_type>(self_significand)) + (other_sign ? -static_cast<signed_type>(other_significand) : static_cast<signed_type>(other_significand)), self_exponent};

            return *this;
        }

        [[nodiscard]] constexpr decimal operator-(const decimal other) const noexcept
        {
            const auto self_sign = sign();
            const auto self_exponent = exponent();
            const auto self_significand = significand();

            const auto other_sign = other.sign();
            const auto other_exponent = other.exponent();
            const auto other_significand = other.significand();

            if (self_significand == 0U)
                return decimal{other_sign ? static_cast<signed_type>(other_significand) : -static_cast<signed_type>(other_significand), self_exponent};

            if (other_significand == 0U)
                return *this;

            if (self_exponent == other_exponent)
                return decimal{(self_sign ?
                                -static_cast<signed_type>(self_significand) :
                                static_cast<signed_type>(self_significand)) - (other_sign ? -static_cast<signed_type>(other_significand) : static_cast<signed_type>(other_significand)), self_exponent};

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
        const auto sign = static_cast<bool>(value.data() >> traits<size>::sign_offset);
        const auto exponent = static_cast<typename traits<size>::signed_type>((value.data() >> traits<size>::exponent_offset) & traits<size>::exponent_mask) - traits<size>::exponent_bias;
        const auto significand = value.data() & traits<size>::significand_mask;

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
                for (std::int32_t i = point; i < 0; ++i) result += '0';
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
                for (std::int32_t i = 0; i < exponent; ++i) result += '0';
                if (static_cast<std::uint32_t>(point) >= digits) result += ".0";
            }

            return result;
        }
    }

    template<class T> class numeric_limits;

    template<> class numeric_limits<decimal32>
    {
        static constexpr bool is_signed = true;
        static constexpr bool is_integer = false;
        static constexpr bool is_exact = false;
        static constexpr bool has_infinity = true;

        static constexpr decimal32 min() noexcept { return decimal32{1, 0}; }
    };

    template<> class numeric_limits<decimal64>
    {
        static constexpr bool is_signed = true;
        static constexpr bool is_integer = false;
        static constexpr bool is_exact = false;
        static constexpr bool has_infinity = true;

        static constexpr decimal64 min() noexcept { return decimal64{1, 0}; }
    };

    template<std::size_t size>
    constexpr bool isnormal(const decimal<size>& value)
    {
        const auto exponent = (value.data() >> traits<size>::exponent_offset) & traits<size>::exponent_mask;
        return exponent != traits<size>::exponent_mask;
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

        return decimal32{sign ? -static_cast<traits<32U>::signed_type>(significand) : static_cast<traits<32U>::signed_type>(significand), exponent};
    }
}

#endif
