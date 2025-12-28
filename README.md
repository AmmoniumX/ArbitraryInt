# ArbitraryPrecision::Integer

A C++20 arbitrary precision integer class with both fixed and dynamic size support.

## Overview

`ArbitraryPrecision::Integer<kind, Bits>` provides unsigned integer arithmetic with arbitrary bit width, implemented as a header-only library. The library supports two modes:

- **Fixed-size integers**: Compile-time fixed bit width with constexpr support
- **Dynamic-size integers**: Runtime-sized integers that can grow as needed

## Template Parameters

- `kind`: Either `Kind::Fixed` or `Kind::Dynamic` (defaults to `Kind::Dynamic`)
- `Bits`: Number of bits for fixed-size integers (must be a power of 2 and greater than 64)
  - Valid for Fixed: 128, 256, 512, 1024, 2048, etc.
  - Invalid: 96, 100, 64, 60
  - Not used for Dynamic integers

## Type Aliases

- `ArbitraryPrecision::Fixed<Bits>` - Fixed-size integer (e.g., `Fixed<256>`)
- `ArbitraryPrecision::Dynamic` - Dynamic-size integer

## Features

**Arithmetic Operations:**
- Addition, subtraction, multiplication (`+`, `-`, `*`)
- Division and modulo (`/`, `%`)
- Unary plus and negation (`+`, `-`)
- Increment and decrement (`++`, `--`)

**Bitwise Operations:**
- AND, OR, XOR (`&`, `|`, `^`)
- NOT (`~`)
- Left and right shift (`<<`, `>>`)

**Comparison:**
- All comparison operators via C++20 spaceship operator (`<=>`)
- Equality operator (`==`)

**Other:**
- Conversion from any integral type
- Explicit conversion to bool
- `std::numeric_limits` specialization (for Fixed only)
- String conversion: `to_string()` and `from_string()`
- Query methods: `length()` (number of 64-bit segments), `bits()` (total bits), `tail()` (lowest 64 bits)

## Implementation Details

**Fixed-size integers:**
- Internally stores value as `std::array<uint64_t, Bits/64>` (little-endian)
- Compile-time size, all operations are constexpr-enabled
- Zero-overhead abstraction with no dynamic allocation

**Dynamic-size integers:**
- Internally stores value as `std::vector<uint64_t>` (little-endian)
- Automatically grows/shrinks as needed
- Trims leading zeros to minimize memory usage

**Common to both:**
- Uses two's complement representation for negative values
- Division uses bit-by-bit algorithm
- All operations handle carry/borrow propagation across segments

## Usage Examples

### Fixed-size integers

```cpp
#include "ArbitraryInteger.hpp"

using namespace ArbitraryPrecision;

// Using type alias
using Int256 = Fixed<256>;

constexpr Int256 a = 12345;
constexpr Int256 b = 67890;
constexpr Int256 c = a * b + 100;

// Compile-time computation
static_assert(Int256(10) + Int256(20) == Int256(30));

// String conversion
Int256 value = 999999;
std::string str = to_string(value);
auto parsed = from_string<Kind::Fixed, 256>(str);
```

### Dynamic-size integers

```cpp
#include "ArbitraryInteger.hpp"

using namespace ArbitraryPrecision;

// Grows automatically as needed
Dynamic a = 12345;
Dynamic b = 67890;
Dynamic huge = a * b * a * b;  // No overflow!

// Check size
std::cout << "Segments: " << huge.length() << "\n";
std::cout << "Bits: " << huge.bits() << "\n";

// String conversion
std::string str = to_string(huge);
auto parsed = from_string<Kind::Dynamic, 0>(str);
```

## Requirements

- C++20 compiler
- Standard library with `<concepts>`, `<compare>`, and `<bit>` support

## Exception Safety

Throws `std::domain_error` on division by zero. All other operations are noexcept.
