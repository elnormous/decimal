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
        static constexpr std::uint32_t fractionMask = 0x1FFFFFU;

    public:
        constexpr decimal() noexcept = default;
        constexpr decimal(int value, int exponent = 0) noexcept:
            d{
                (value < 0 ? 0x01U : 0x00U) << signOffset |
                ((exponent < 0 ? (exponentBias - 1 - ~static_cast<std::uint32_t>(exponent)) : (static_cast<std::uint32_t>(exponent) + exponentBias)) & exponentMask) << exponentOffset |
                ((value < 0 ? ~static_cast<std::uint32_t>(value) + 0x01U : static_cast<std::uint32_t>(value)) & fractionMask)
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
        const auto d = value.data();
        std::string result;
        if (d >> 31U) result += '-';

        auto fraction = d & 0x1FFFFFU;
        do
        {
            result += static_cast<char>('0' + fraction % 10U);
            fraction /= 10U;
        } while (fraction != 0U);

        result += ".0";

        return result;
    }
}

#endif
