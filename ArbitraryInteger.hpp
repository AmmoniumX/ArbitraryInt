#include <algorithm>
#include <array>
#include <bit>
#include <climits>
#include <compare>
#include <concepts>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

static_assert(CHAR_BIT == 8);

namespace ArbitraryPrecision {

namespace detail {
template <typename T, template <auto...> class C>
struct instantiation_of_nontype_impl : std::false_type {};

template <template <auto...> class C, auto... Args>
struct instantiation_of_nontype_impl<C<Args...>, C> : std::true_type {};

template <typename T, template <auto...> class C>
concept instantiation_of_nontype = instantiation_of_nontype_impl<T, C>::value;
} // namespace detail

template <size_t Bits>
  requires(std::has_single_bit(Bits) && (Bits > 64))
class FixedInteger;

class DynamicInteger;

template <typename T>
concept Integer = detail::instantiation_of_nontype<T, FixedInteger> ||
                  std::is_same_v<T, DynamicInteger>;

// Fixed precision
template <size_t Bits_>
  requires(std::has_single_bit(Bits_) && (Bits_ > 64))
class FixedInteger {
public:
  static constexpr size_t Bits = Bits_;

  using Chunk = std::uint64_t;
  using Segments = std::array<Chunk, (Bits / 64)>;

  static constexpr bool is_dynamic = false;

private:
  Segments segments{};
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
  constexpr FixedInteger() = default;

  constexpr size_t length() const { return this->segments.size(); }
  constexpr size_t bits() const {
    return length() * (sizeof(Chunk) * CHAR_BIT);
  }

  // Constructor from integral types
  constexpr FixedInteger(std::integral auto value) {
    static_assert(sizeof(decltype(value)) <= sizeof(Chunk),
                  "Integral value cannot be larger than Chunk value");
    if constexpr (std::is_signed_v<decltype(value)>) {
      if (value < 0) {
        // Two's complement for negative values
        segments[0] = static_cast<Chunk>(value);
        for (size_t i = 1; i < length(); ++i) {
          segments[i] = ~0ULL;
        }
      } else {
        segments[0] = static_cast<Chunk>(value);
      }
    } else {
      segments[0] = static_cast<Chunk>(value);
    }
  }

  // Unary operators
  constexpr FixedInteger operator+() const { return *this; }

  constexpr FixedInteger operator-() const {
    FixedInteger result;
    bool borrow = false;
    for (size_t i = 0; i < length(); ++i) {
      borrow = sub_with_borrow(result.segments[i], 0, segments[i], borrow);
    }
    return result;
  }

  constexpr FixedInteger operator~() const {
    FixedInteger result;
    for (size_t i = 0; i < length(); ++i) {
      result.segments[i] = ~segments[i];
    }
    return result;
  }

  // Addition
  constexpr FixedInteger &operator+=(const FixedInteger &other) {
    bool carry = false;
    for (size_t i = 0; i < length(); ++i) {
      carry =
          add_with_carry(segments[i], segments[i], other.segments[i], carry);
    }
    return *this;
  }

  constexpr FixedInteger operator+(const FixedInteger &other) const {
    FixedInteger result = *this;
    result += other;
    return result;
  }

  // Subtraction
  constexpr FixedInteger &operator-=(const FixedInteger &other) {
    bool borrow = false;
    for (size_t i = 0; i < length(); ++i) {
      borrow =
          sub_with_borrow(segments[i], segments[i], other.segments[i], borrow);
    }
    return *this;
  }

  constexpr FixedInteger operator-(const FixedInteger &other) const {
    FixedInteger result = *this;
    result -= other;
    return result;
  }

  // Multiplication
  constexpr FixedInteger &operator*=(const FixedInteger &other) {
    FixedInteger result;
    for (size_t i = 0; i < length(); ++i) {
      Chunk carry = 0;
      for (size_t j = 0; j < length() - i; ++j) {
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

  constexpr FixedInteger operator*(const FixedInteger &other) const {
    FixedInteger result = *this;
    result *= other;
    return result;
  }

  // Division (unsigned division algorithm)
  constexpr FixedInteger &operator/=(const FixedInteger &other) {
    *this = divide(*this, other).first;
    return *this;
  }

  constexpr FixedInteger operator/(const FixedInteger &other) const {
    return divide(*this, other).first;
  }

  // Modulo
  constexpr FixedInteger &operator%=(const FixedInteger &other) {
    *this = divide(*this, other).second;
    return *this;
  }

  constexpr FixedInteger operator%(const FixedInteger &other) const {
    return divide(*this, other).second;
  }

  // Bitwise AND
  constexpr FixedInteger &operator&=(const FixedInteger &other) {
    for (size_t i = 0; i < length(); ++i) {
      segments[i] &= other.segments[i];
    }
    return *this;
  }

  constexpr FixedInteger operator&(const FixedInteger &other) const {
    FixedInteger result = *this;
    result &= other;
    return result;
  }

  // Bitwise OR
  constexpr FixedInteger &operator|=(const FixedInteger &other) {
    for (size_t i = 0; i < length(); ++i) {
      segments[i] |= other.segments[i];
    }
    return *this;
  }

  constexpr FixedInteger operator|(const FixedInteger &other) const {
    FixedInteger result = *this;
    result |= other;
    return result;
  }

  // Bitwise XOR
  constexpr FixedInteger &operator^=(const FixedInteger &other) {
    for (size_t i = 0; i < length(); ++i) {
      segments[i] ^= other.segments[i];
    }
    return *this;
  }

  constexpr FixedInteger operator^(const FixedInteger &other) const {
    FixedInteger result = *this;
    result ^= other;
    return result;
  }

  // Left shift
  constexpr FixedInteger &operator<<=(size_t shift) {
    if (shift >= Bits) {
      for (auto &seg : segments)
        seg = 0;
      return *this;
    }

    size_t seg_shift = shift / 64;
    size_t bit_shift = shift % 64;

    if (bit_shift == 0) {
      for (size_t i = length() - 1; i >= seg_shift; --i) {
        segments[i] = segments[i - seg_shift];
        if (i == seg_shift)
          break;
      }
    } else {
      for (size_t i = length() - 1; i > seg_shift; --i) {
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

  constexpr FixedInteger operator<<(size_t shift) const {
    FixedInteger result = *this;
    result <<= shift;
    return result;
  }

  // Right shift (logical)
  constexpr FixedInteger &operator>>=(size_t shift) {
    if (shift >= Bits) {
      for (auto &seg : segments)
        seg = 0;
      return *this;
    }

    size_t seg_shift = shift / 64;
    size_t bit_shift = shift % 64;

    if (bit_shift == 0) {
      for (size_t i = 0; i < length() - seg_shift; ++i) {
        segments[i] = segments[i + seg_shift];
      }
    } else {
      for (size_t i = 0; i < length() - seg_shift - 1; ++i) {
        segments[i] = (segments[i + seg_shift] >> bit_shift) |
                      (segments[i + seg_shift + 1] << (64 - bit_shift));
      }
      segments[length() - seg_shift - 1] = segments[length() - 1] >> bit_shift;
    }

    for (size_t i = length() - seg_shift; i < length(); ++i) {
      segments[i] = 0;
    }

    return *this;
  }

  constexpr FixedInteger operator>>(size_t shift) const {
    FixedInteger result = *this;
    result >>= shift;
    return result;
  }

  // Increment/Decrement
  constexpr FixedInteger &operator++() {
    for (size_t i = 0; i < length(); ++i) {
      if (++segments[i] != 0)
        break;
    }
    return *this;
  }

  constexpr FixedInteger operator++(int) {
    FixedInteger temp = *this;
    ++(*this);
    return temp;
  }

  constexpr FixedInteger &operator--() {
    for (size_t i = 0; i < length(); ++i) {
      if (segments[i]-- != 0)
        break;
    }
    return *this;
  }

  constexpr FixedInteger operator--(int) {
    FixedInteger temp = *this;
    --(*this);
    return temp;
  }

  // Spaceship operator
  constexpr std::strong_ordering operator<=>(const FixedInteger &other) const {
    for (size_t i = length(); i > 0; --i) {
      if (auto cmp = segments[i - 1] <=> other.segments[i - 1]; cmp != 0) {
        return cmp;
      }
    }
    return std::strong_ordering::equal;
  }

  constexpr bool operator==(const FixedInteger &other) const {
    return segments == other.segments;
  }

  // Conversion to bool
  constexpr explicit operator bool() const {
    for (const auto &seg : segments) {
      if (seg != 0)
        return true;
    }
    return false;
  }

  // Returns lowest 64 bits
  constexpr uint64_t tail() const { return segments[0]; }

  constexpr std::span<Chunk, (Bits / 64)> as_span() {
    return std::span{segments.begin(), segments.size()};
  }

private:
  // Helper for division
  static constexpr std::pair<FixedInteger, FixedInteger>
  divide(const FixedInteger &dividend, const FixedInteger &divisor) {
    if (!divisor) {
      throw std::domain_error("Division by zero");
    }

    FixedInteger quotient;
    FixedInteger remainder;

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

// Dynamic precision
class DynamicInteger {
public:
  using Chunk = std::uint64_t;
  using Segments = std::vector<Chunk>;

  static constexpr bool is_dynamic = true;

private:
  Segments segments;
  // Helper: add with carry
  static bool add_with_carry(uint64_t &result, uint64_t a, uint64_t b,
                             bool carry_in) {
    result = a + b + (carry_in ? 1 : 0);
    return result < a || (carry_in && result == a);
  }

  // Helper: subtract with borrow
  static bool sub_with_borrow(uint64_t &result, uint64_t a, uint64_t b,
                              bool borrow_in) {
    result = a - b - (borrow_in ? 1 : 0);
    return b > a || (borrow_in && b == a);
  }

  // Helper: multiply 64-bit numbers to get 128-bit result
  static std::pair<uint64_t, uint64_t> mul128(uint64_t a, uint64_t b) {
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

  // Helper: trim leading zeros (keep at least 1 segment)
  void trim() {
    while (segments.size() > 1 && segments.back() == 0) {
      segments.pop_back();
    }
  }

public:
  DynamicInteger() : segments(1, 0) {}

  size_t length() const { return segments.size(); }
  size_t bits() const { return length() * (sizeof(Chunk) * CHAR_BIT); }

  // Constructor from integral types
  DynamicInteger(std::integral auto value) : segments(1, 0) {
    static_assert(sizeof(decltype(value)) <= sizeof(Chunk),
                  "Integral value cannot be larger than Chunk value");
    // Simply cast the value to Chunk - for signed negative values,
    // this will produce the correct two's complement representation
    // in the lowest 64 bits without unnecessary sign extension
    segments[0] = static_cast<Chunk>(value);
  }

  // Unary operators
  DynamicInteger operator+() const { return *this; }

  DynamicInteger operator-() const {
    DynamicInteger result;
    result.segments.resize(length());
    bool borrow = false;
    for (size_t i = 0; i < length(); ++i) {
      borrow = sub_with_borrow(result.segments[i], 0, segments[i], borrow);
    }
    // Don't extend on borrow - this is unsigned arithmetic with wrapping
    result.trim();
    return result;
  }

  DynamicInteger operator~() const {
    DynamicInteger result;
    result.segments.resize(length());
    for (size_t i = 0; i < length(); ++i) {
      result.segments[i] = ~segments[i];
    }
    return result;
  }

  // Addition
  DynamicInteger &operator+=(const DynamicInteger &other) {
    size_t max_len = std::max(length(), other.length());
    segments.resize(max_len, 0);

    bool carry = false;
    for (size_t i = 0; i < max_len; ++i) {
      uint64_t other_val = (i < other.length()) ? other.segments[i] : 0;
      carry = add_with_carry(segments[i], segments[i], other_val, carry);
    }

    // If there's a final carry, grow
    if (carry) {
      segments.push_back(1);
    }

    trim();
    return *this;
  }

  DynamicInteger operator+(const DynamicInteger &other) const {
    DynamicInteger result = *this;
    result += other;
    return result;
  }

  // Subtraction
  DynamicInteger &operator-=(const DynamicInteger &other) {
    size_t max_len = std::max(length(), other.length());
    segments.resize(max_len, 0);

    bool borrow = false;
    for (size_t i = 0; i < max_len; ++i) {
      uint64_t other_val = (i < other.length()) ? other.segments[i] : 0;
      borrow = sub_with_borrow(segments[i], segments[i], other_val, borrow);
    }

    trim();
    return *this;
  }

  DynamicInteger operator-(const DynamicInteger &other) const {
    DynamicInteger result = *this;
    result -= other;
    return result;
  }

  // Multiplication
  DynamicInteger &operator*=(const DynamicInteger &other) {
    DynamicInteger result;
    result.segments.resize(length() + other.length(), 0);

    for (size_t i = 0; i < length(); ++i) {
      Chunk carry = 0;
      for (size_t j = 0; j < other.length(); ++j) {
        if (i + j >= result.length())
          break;

        auto [lo, hi] = mul128(segments[i], other.segments[j]);

        bool c1 = add_with_carry(lo, lo, carry, false);
        bool c2 = add_with_carry(lo, lo, result.segments[i + j], false);

        result.segments[i + j] = lo;
        carry = hi + c1 + c2;
      }
      if (i + other.length() < result.length()) {
        result.segments[i + other.length()] = carry;
      }
    }

    *this = result;
    trim();
    return *this;
  }

  DynamicInteger operator*(const DynamicInteger &other) const {
    DynamicInteger result = *this;
    result *= other;
    return result;
  }

  // Division (unsigned division algorithm)
  DynamicInteger &operator/=(const DynamicInteger &other) {
    *this = divide(*this, other).first;
    return *this;
  }

  DynamicInteger operator/(const DynamicInteger &other) const {
    return divide(*this, other).first;
  }

  // Modulo
  DynamicInteger &operator%=(const DynamicInteger &other) {
    *this = divide(*this, other).second;
    return *this;
  }

  DynamicInteger operator%(const DynamicInteger &other) const {
    return divide(*this, other).second;
  }

  // Bitwise AND
  DynamicInteger &operator&=(const DynamicInteger &other) {
    size_t min_len = std::min(length(), other.length());
    segments.resize(min_len);
    for (size_t i = 0; i < min_len; ++i) {
      segments[i] &= other.segments[i];
    }
    trim();
    return *this;
  }

  DynamicInteger operator&(const DynamicInteger &other) const {
    DynamicInteger result = *this;
    result &= other;
    return result;
  }

  // Bitwise OR
  DynamicInteger &operator|=(const DynamicInteger &other) {
    size_t max_len = std::max(length(), other.length());
    segments.resize(max_len, 0);
    for (size_t i = 0; i < other.length(); ++i) {
      segments[i] |= other.segments[i];
    }
    trim();
    return *this;
  }

  DynamicInteger operator|(const DynamicInteger &other) const {
    DynamicInteger result = *this;
    result |= other;
    return result;
  }

  // Bitwise XOR
  DynamicInteger &operator^=(const DynamicInteger &other) {
    size_t max_len = std::max(length(), other.length());
    segments.resize(max_len, 0);
    for (size_t i = 0; i < other.length(); ++i) {
      segments[i] ^= other.segments[i];
    }
    trim();
    return *this;
  }

  DynamicInteger operator^(const DynamicInteger &other) const {
    DynamicInteger result = *this;
    result ^= other;
    return result;
  }

  // Left shift
  DynamicInteger &operator<<=(size_t shift) {
    if (shift == 0)
      return *this;

    size_t seg_shift = shift / 64;
    size_t bit_shift = shift % 64;

    size_t old_len = length();
    size_t new_len = old_len + seg_shift;

    // Check if we need an extra segment for bit overflow
    if (bit_shift > 0 && old_len > 0 &&
        segments[old_len - 1] >> (64 - bit_shift)) {
      new_len++;
    }

    segments.resize(new_len, 0);

    // Shift segments
    if (bit_shift == 0) {
      for (size_t i = new_len - 1; i >= seg_shift; --i) {
        segments[i] = segments[i - seg_shift];
        if (i == seg_shift)
          break;
      }
    } else {
      for (size_t i = new_len - 1; i > seg_shift; --i) {
        size_t src_idx = i - seg_shift;
        segments[i] = (segments[src_idx] << bit_shift);
        if (src_idx > 0) {
          segments[i] |= (segments[src_idx - 1] >> (64 - bit_shift));
        }
      }
      segments[seg_shift] = segments[0] << bit_shift;
    }

    // Zero out lower segments
    for (size_t i = 0; i < seg_shift; ++i) {
      segments[i] = 0;
    }

    trim();
    return *this;
  }

  DynamicInteger operator<<(size_t shift) const {
    DynamicInteger result = *this;
    result <<= shift;
    return result;
  }

  // Right shift (logical)
  DynamicInteger &operator>>=(size_t shift) {
    if (shift == 0)
      return *this;

    size_t seg_shift = shift / 64;
    size_t bit_shift = shift % 64;

    if (seg_shift >= length()) {
      segments.resize(1);
      segments[0] = 0;
      return *this;
    }

    size_t new_len = length() - seg_shift;

    if (bit_shift == 0) {
      for (size_t i = 0; i < new_len; ++i) {
        segments[i] = segments[i + seg_shift];
      }
    } else {
      for (size_t i = 0; i < new_len - 1; ++i) {
        segments[i] = (segments[i + seg_shift] >> bit_shift) |
                      (segments[i + seg_shift + 1] << (64 - bit_shift));
      }
      segments[new_len - 1] = segments[length() - 1] >> bit_shift;
    }

    segments.resize(new_len);
    trim();
    return *this;
  }

  DynamicInteger operator>>(size_t shift) const {
    DynamicInteger result = *this;
    result >>= shift;
    return result;
  }

  // Increment/Decrement
  DynamicInteger &operator++() {
    for (size_t i = 0; i < length(); ++i) {
      if (++segments[i] != 0)
        return *this;
    }
    // Carry out, need to grow
    segments.push_back(1);
    return *this;
  }

  DynamicInteger operator++(int) {
    DynamicInteger temp = *this;
    ++(*this);
    return temp;
  }

  DynamicInteger &operator--() {
    for (size_t i = 0; i < length(); ++i) {
      if (segments[i]-- != 0) {
        trim();
        return *this;
      }
    }
    trim();
    return *this;
  }

  DynamicInteger operator--(int) {
    DynamicInteger temp = *this;
    --(*this);
    return temp;
  }

  // Spaceship operator
  std::strong_ordering operator<=>(const DynamicInteger &other) const {
    if (length() != other.length()) {
      return length() <=> other.length();
    }
    for (size_t i = length(); i > 0; --i) {
      if (auto cmp = segments[i - 1] <=> other.segments[i - 1]; cmp != 0) {
        return cmp;
      }
    }
    return std::strong_ordering::equal;
  }

  bool operator==(const DynamicInteger &other) const {
    return segments == other.segments;
  }

  // Conversion to bool
  explicit operator bool() const {
    for (const auto &seg : segments) {
      if (seg != 0)
        return true;
    }
    return false;
  }

  // Returns lowest 64 bits
  uint64_t tail() const { return segments[0]; }

  constexpr std::span<Chunk, std::dynamic_extent> as_span() {
    return std::span{segments.begin(), segments.size()};
  }

private:
  // Helper for division
  static std::pair<DynamicInteger, DynamicInteger>
  divide(const DynamicInteger &dividend, const DynamicInteger &divisor) {
    if (!divisor) {
      throw std::domain_error("Division by zero");
    }

    DynamicInteger quotient;
    DynamicInteger remainder;

    size_t total_bits = dividend.bits();

    for (size_t i = total_bits; i > 0; --i) {
      size_t bit_idx = i - 1;
      size_t seg_idx = bit_idx / 64;
      size_t bit_in_seg = bit_idx % 64;

      remainder <<= 1;
      if (seg_idx < dividend.length() &&
          (dividend.segments[seg_idx] & (1ULL << bit_in_seg))) {
        remainder.segments[0] |= 1;
      }

      if (remainder >= divisor) {
        remainder -= divisor;
        // Ensure quotient has enough segments
        if (seg_idx >= quotient.length()) {
          quotient.segments.resize(seg_idx + 1, 0);
        }
        quotient.segments[seg_idx] |= (1ULL << bit_in_seg);
      }
    }

    quotient.trim();
    remainder.trim();

    return {quotient, remainder};
  }
};

// Convert Integer to decimal string
std::string to_string(const Integer auto &value) {

  if (!value) {
    return "0";
  }

  std::string result;
  Integer auto temp = value;
  const decltype(temp) ten(10);

  while (temp) {
    Integer auto digit = temp % ten;
    result += static_cast<char>('0' + digit.tail());
    temp /= ten;
  }

  std::reverse(result.begin(), result.end());
  return result;
}

// Convert string to Integer
template <Integer T> std::optional<T> from_string(std::string_view from) {
  if (from.empty()) {
    return std::nullopt;
  }

  T result(0);
  const T ten(10);

  for (char c : from) {
    if (c < '0' || c > '9') {
      return std::nullopt;
    }

    result *= ten;
    result += T(static_cast<int>(c - '0'));
  }

  return result;
}

} // namespace ArbitraryPrecision

// std::numeric_limits specialization
namespace std {
template <size_t Bits>
class numeric_limits<ArbitraryPrecision::FixedInteger<Bits>> {
  using Integer = ArbitraryPrecision::FixedInteger<Bits>;

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
