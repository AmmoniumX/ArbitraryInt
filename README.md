# ArbitraryPrecision::Integer

A C++20 arbitrary precision integer class with compile-time size specification.

## Overview

`ArbitraryPrecision::Integer<Bits>` provides unsigned integer arithmetic with arbitrary bit width, implemented as a header-only library. All operations are constexpr-enabled for compile-time computation.

## Template Parameters

- `Bits`: Number of bits (must be a power of 2 and greater than 64)
  - Valid: 128, 256, 512, 1024, 2048, etc.
  - Invalid: 96, 100, 64, 60

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
- `std::numeric_limits` specialization

## Implementation Details

- Internally stores value as array of 64-bit segments (little-endian)
- Uses two's complement representation for negative values
- Division uses bit-by-bit algorithm
- All operations handle carry/borrow propagation across segments
- Zero-overhead abstraction with constexpr support

## Usage Example

```cpp
#include "ArbitraryInteger.hpp"

using Int256 = ArbitraryPrecision::Integer<256>;

constexpr Int256 a = 12345;
constexpr Int256 b = 67890;
constexpr Int256 c = a * b + 100;

// Compile-time computation
static_assert(Int256(10) + Int256(20) == Int256(30));
```

## Requirements

- C++20 compiler
- Standard library with `<concepts>`, `<compare>`, and `<bit>` support

## Exception Safety

Throws `std::domain_error` on division by zero. All other operations are noexcept.
