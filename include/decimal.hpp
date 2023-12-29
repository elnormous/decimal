#ifndef EDL_DECIMAL_HPP
#define EDL_DECIMAL_HPP

#include <cstdint>

namespace edl
{
    class decimal
    {
        static constexpr std::uint32_t signOffset = 31U;
        static constexpr std::uint32_t exponentOffset = 23U;
        static constexpr std::uint32_t exponentMask = 0xFFU;
        static constexpr std::uint32_t fractionMask = 0x1FFFFFU;

    public:
        constexpr decimal() noexcept = default;
        constexpr decimal(int value, int exponent = 0) noexcept:
            d{
                (value < 0 ? 0x01U : 0x00U) << signOffset |
                (exponent < 0 ? 126U - ~static_cast<std::uint32_t>(exponent) & exponentMask : static_cast<std::uint32_t>(exponent + 127) & exponentMask) << exponentOffset |
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
        std::uint32_t d = 0;
    };
}

#endif
