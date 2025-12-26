#include <array>
#include <bit>
#include <compare>
#include <concepts>
#include <cstdint>
#include <stdexcept>
#include <type_traits>

namespace ArbitraryPrecision {
template <size_t Bits>
  requires(std::has_single_bit(Bits) && (Bits > 64))
class Integer {
private:
  static constexpr size_t NUM_SEGMENTS = Bits / 64;
  std::array<uint64_t, NUM_SEGMENTS> segments{};

  // Helper: add with carry
  static constexpr bool add_with_carry(uint64_t &result, uint64_t a, uint64_t b,
                                       bool carry_in) {
    result = a + b + (carry_in ? 1 : 0);
    return result < a || (carry_in && result == a);
  }

  // Helper: subtract with borrow
  static constexpr bool sub_with_borrow(uint64_t &result, uint64_t a,
                                        uint64_t b, bool borrow_in) {
    result = a - b - (borrow_in ? 1 : 0);
    return b > a || (borrow_in && b == a);
  }

  // Helper: multiply 64-bit numbers to get 128-bit result
  static constexpr std::pair<uint64_t, uint64_t> mul128(uint64_t a,
                                                        uint64_t b) {
    uint64_t a_lo = a & 0xFFFFFFFF;
    uint64_t a_hi = a >> 32;
    uint64_t b_lo = b & 0xFFFFFFFF;
    uint64_t b_hi = b >> 32;

    uint64_t p0 = a_lo * b_lo;
    uint64_t p1 = a_lo * b_hi;
    uint64_t p2 = a_hi * b_lo;
    uint64_t p3 = a_hi * b_hi;

    uint64_t mid = p1 + (p0 >> 32);
    mid += p2;
    uint64_t carry = (mid < p1) ? 1 : 0;

    uint64_t lo = (mid << 32) | (p0 & 0xFFFFFFFF);
    uint64_t hi = p3 + (mid >> 32) + (carry << 32);

    return {lo, hi};
  }

public:
  constexpr Integer() = default;

  // Constructor from integral types
  template <std::integral T> constexpr Integer(T value) {
    if constexpr (std::is_signed_v<T>) {
      if (value < 0) {
        // Two's complement for negative values
        segments[0] = static_cast<uint64_t>(value);
        for (size_t i = 1; i < NUM_SEGMENTS; ++i) {
          segments[i] = ~0ULL;
        }
      } else {
        segments[0] = static_cast<uint64_t>(value);
      }
    } else {
      segments[0] = static_cast<uint64_t>(value);
    }
  }

  // Unary operators
  constexpr Integer operator+() const { return *this; }

  constexpr Integer operator-() const {
    Integer result;
    bool borrow = false;
    for (size_t i = 0; i < NUM_SEGMENTS; ++i) {
      borrow = sub_with_borrow(result.segments[i], 0, segments[i], borrow);
    }
    return result;
  }

  constexpr Integer operator~() const {
    Integer result;
    for (size_t i = 0; i < NUM_SEGMENTS; ++i) {
      result.segments[i] = ~segments[i];
    }
    return result;
  }

  // Addition
  constexpr Integer &operator+=(const Integer &other) {
    bool carry = false;
    for (size_t i = 0; i < NUM_SEGMENTS; ++i) {
      carry =
          add_with_carry(segments[i], segments[i], other.segments[i], carry);
    }
    return *this;
  }

  constexpr Integer operator+(const Integer &other) const {
    Integer result = *this;
    result += other;
    return result;
  }

  // Subtraction
  constexpr Integer &operator-=(const Integer &other) {
    bool borrow = false;
    for (size_t i = 0; i < NUM_SEGMENTS; ++i) {
      borrow =
          sub_with_borrow(segments[i], segments[i], other.segments[i], borrow);
    }
    return *this;
  }

  constexpr Integer operator-(const Integer &other) const {
    Integer result = *this;
    result -= other;
    return result;
  }

  // Multiplication
  constexpr Integer &operator*=(const Integer &other) {
    Integer result;
    for (size_t i = 0; i < NUM_SEGMENTS; ++i) {
      uint64_t carry = 0;
      for (size_t j = 0; j < NUM_SEGMENTS - i; ++j) {
        auto [lo, hi] = mul128(segments[i], other.segments[j]);

        bool c1 = add_with_carry(lo, lo, carry, false);
        bool c2 = add_with_carry(lo, lo, result.segments[i + j], false);

        result.segments[i + j] = lo;
        carry = hi + c1 + c2;
      }
    }
    *this = result;
    return *this;
  }

  constexpr Integer operator*(const Integer &other) const {
    Integer result = *this;
    result *= other;
    return result;
  }

  // Division (unsigned division algorithm)
  constexpr Integer &operator/=(const Integer &other) {
    *this = divide(*this, other).first;
    return *this;
  }

  constexpr Integer operator/(const Integer &other) const {
    return divide(*this, other).first;
  }

  // Modulo
  constexpr Integer &operator%=(const Integer &other) {
    *this = divide(*this, other).second;
    return *this;
  }

  constexpr Integer operator%(const Integer &other) const {
    return divide(*this, other).second;
  }

  // Bitwise AND
  constexpr Integer &operator&=(const Integer &other) {
    for (size_t i = 0; i < NUM_SEGMENTS; ++i) {
      segments[i] &= other.segments[i];
    }
    return *this;
  }

  constexpr Integer operator&(const Integer &other) const {
    Integer result = *this;
    result &= other;
    return result;
  }

  // Bitwise OR
  constexpr Integer &operator|=(const Integer &other) {
    for (size_t i = 0; i < NUM_SEGMENTS; ++i) {
      segments[i] |= other.segments[i];
    }
    return *this;
  }

  constexpr Integer operator|(const Integer &other) const {
    Integer result = *this;
    result |= other;
    return result;
  }

  // Bitwise XOR
  constexpr Integer &operator^=(const Integer &other) {
    for (size_t i = 0; i < NUM_SEGMENTS; ++i) {
      segments[i] ^= other.segments[i];
    }
    return *this;
  }

  constexpr Integer operator^(const Integer &other) const {
    Integer result = *this;
    result ^= other;
    return result;
  }

  // Left shift
  constexpr Integer &operator<<=(size_t shift) {
    if (shift >= Bits) {
      for (auto &seg : segments)
        seg = 0;
      return *this;
    }

    size_t seg_shift = shift / 64;
    size_t bit_shift = shift % 64;

    if (bit_shift == 0) {
      for (size_t i = NUM_SEGMENTS - 1; i >= seg_shift; --i) {
        segments[i] = segments[i - seg_shift];
        if (i == seg_shift)
          break;
      }
    } else {
      for (size_t i = NUM_SEGMENTS - 1; i > seg_shift; --i) {
        segments[i] = (segments[i - seg_shift] << bit_shift) |
                      (segments[i - seg_shift - 1] >> (64 - bit_shift));
      }
      segments[seg_shift] = segments[0] << bit_shift;
    }

    for (size_t i = 0; i < seg_shift; ++i) {
      segments[i] = 0;
    }

    return *this;
  }

  constexpr Integer operator<<(size_t shift) const {
    Integer result = *this;
    result <<= shift;
    return result;
  }

  // Right shift (logical)
  constexpr Integer &operator>>=(size_t shift) {
    if (shift >= Bits) {
      for (auto &seg : segments)
        seg = 0;
      return *this;
    }

    size_t seg_shift = shift / 64;
    size_t bit_shift = shift % 64;

    if (bit_shift == 0) {
      for (size_t i = 0; i < NUM_SEGMENTS - seg_shift; ++i) {
        segments[i] = segments[i + seg_shift];
      }
    } else {
      for (size_t i = 0; i < NUM_SEGMENTS - seg_shift - 1; ++i) {
        segments[i] = (segments[i + seg_shift] >> bit_shift) |
                      (segments[i + seg_shift + 1] << (64 - bit_shift));
      }
      segments[NUM_SEGMENTS - seg_shift - 1] =
          segments[NUM_SEGMENTS - 1] >> bit_shift;
    }

    for (size_t i = NUM_SEGMENTS - seg_shift; i < NUM_SEGMENTS; ++i) {
      segments[i] = 0;
    }

    return *this;
  }

  constexpr Integer operator>>(size_t shift) const {
    Integer result = *this;
    result >>= shift;
    return result;
  }

  // Increment/Decrement
  constexpr Integer &operator++() {
    for (size_t i = 0; i < NUM_SEGMENTS; ++i) {
      if (++segments[i] != 0)
        break;
    }
    return *this;
  }

  constexpr Integer operator++(int) {
    Integer temp = *this;
    ++(*this);
    return temp;
  }

  constexpr Integer &operator--() {
    for (size_t i = 0; i < NUM_SEGMENTS; ++i) {
      if (segments[i]-- != 0)
        break;
    }
    return *this;
  }

  constexpr Integer operator--(int) {
    Integer temp = *this;
    --(*this);
    return temp;
  }

  // Spaceship operator
  constexpr std::strong_ordering operator<=>(const Integer &other) const {
    for (size_t i = NUM_SEGMENTS; i > 0; --i) {
      if (auto cmp = segments[i - 1] <=> other.segments[i - 1]; cmp != 0) {
        return cmp;
      }
    }
    return std::strong_ordering::equal;
  }

  constexpr bool operator==(const Integer &other) const {
    return segments == other.segments;
  }

  // Conversion to bool
  constexpr explicit operator bool() const {
    for (auto seg : segments) {
      if (seg != 0)
        return true;
    }
    return false;
  }

private:
  // Helper for division
  static constexpr std::pair<Integer, Integer> divide(const Integer &dividend,
                                                      const Integer &divisor) {
    if (!divisor) {
      throw std::domain_error("Division by zero");
    }

    Integer quotient;
    Integer remainder;

    for (size_t i = Bits; i > 0; --i) {
      size_t bit_idx = i - 1;
      size_t seg_idx = bit_idx / 64;
      size_t bit_in_seg = bit_idx % 64;

      remainder <<= 1;
      if (dividend.segments[seg_idx] & (1ULL << bit_in_seg)) {
        remainder.segments[0] |= 1;
      }

      if (remainder >= divisor) {
        remainder -= divisor;
        quotient.segments[seg_idx] |= (1ULL << bit_in_seg);
      }
    }

    return {quotient, remainder};
  }
};

} // namespace ArbitraryPrecision

// std::numeric_limits specialization
namespace std {
template <size_t Bits>
  requires(std::has_single_bit(Bits) && (Bits > 64))
class numeric_limits<ArbitraryPrecision::Integer<Bits>> {
  using Integer = ArbitraryPrecision::Integer<Bits>;

public:
  static constexpr bool is_specialized = true;
  static constexpr bool is_signed = false;
  static constexpr bool is_integer = true;
  static constexpr bool is_exact = true;
  static constexpr bool has_infinity = false;
  static constexpr bool has_quiet_NaN = false;
  static constexpr bool has_signaling_NaN = false;
  static constexpr bool is_iec559 = false;
  static constexpr bool is_bounded = true;
  static constexpr bool is_modulo = true;
  static constexpr int digits = Bits;
  static constexpr int digits10 =
      static_cast<int>(Bits * 0.30102999566398119521); // log10(2) * Bits
  static constexpr int max_digits10 = 0;
  static constexpr int radix = 2;
  static constexpr int min_exponent = 0;
  static constexpr int min_exponent10 = 0;
  static constexpr int max_exponent = 0;
  static constexpr int max_exponent10 = 0;
  static constexpr bool traps = false;
  static constexpr bool tinyness_before = false;

  static constexpr Integer min() noexcept { return Integer(0); }

  static constexpr Integer lowest() noexcept { return Integer(0); }

  static constexpr Integer max() noexcept { return ~Integer(0); }
};
} // namespace std
