#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <ArbitraryInteger.hpp>
#include <doctest/doctest.h>
#include <limits>

// Type aliases for common sizes
using Int128 = ArbitraryPrecision::Fixed<128>;
using Int256 = ArbitraryPrecision::Fixed<256>;
using Int512 = ArbitraryPrecision::Fixed<512>;

TEST_SUITE("Construction and Initialization") {
  TEST_CASE("Default constructor initializes to zero") {
    Int128 a;
    CHECK(a == Int128(0));
    CHECK_FALSE(static_cast<bool>(a));
  }

  TEST_CASE("Constructor from unsigned integers") {
    Int128 a(42);
    Int128 b(0);
    Int128 c(std::numeric_limits<uint64_t>::max());

    CHECK(a == Int128(42));
    CHECK(b == Int128(0));
    CHECK(c == Int128(std::numeric_limits<uint64_t>::max()));
  }

  TEST_CASE("Constructor from signed integers") {
    Int128 a(42);
    Int128 b(-42);
    Int128 c(0);

    CHECK(a == Int128(42));
    CHECK(b == Int128(-42));
    CHECK(c == Int128(0));
  }

  TEST_CASE("Constructor with maximum uint64_t value") {
    Int256 a(UINT64_MAX);
    CHECK(a == Int256(UINT64_MAX));
  }

  TEST_CASE("Different bit sizes") {
    Int128 a(100);
    Int256 b(100);
    Int512 c(100);

    CHECK(a == Int128(100));
    CHECK(b == Int256(100));
    CHECK(c == Int512(100));
  }
}

TEST_SUITE("Unary Operators") {
  TEST_CASE("Unary plus") {
    Int128 a(42);
    Int128 b = +a;
    CHECK(b == Int128(42));
    CHECK(b == a);
  }

  TEST_CASE("Unary minus") {
    Int128 a(42);
    Int128 b = -a;
    Int128 c = -b;

    CHECK(b == Int128(-42));
    CHECK(c == Int128(42));
    CHECK(c == a);
  }

  TEST_CASE("Unary minus on zero") {
    Int128 zero(0);
    Int128 result = -zero;
    CHECK(result == Int128(0));
  }

  TEST_CASE("Bitwise NOT") {
    Int128 a(0);
    Int128 b = ~a;
    Int128 c = ~b;

    CHECK(c == a);
  }

  TEST_CASE("Bitwise NOT preserves value through double application") {
    Int128 a(12345);
    Int128 b = ~~a;
    CHECK(b == a);
  }
}

TEST_SUITE("Addition") {
  TEST_CASE("Basic addition") {
    Int128 a(10);
    Int128 b(20);
    Int128 c = a + b;

    CHECK(c == Int128(30));
  }

  TEST_CASE("Addition with zero") {
    Int128 a(42);
    Int128 zero(0);

    CHECK(a + zero == a);
    CHECK(zero + a == a);
  }

  TEST_CASE("Addition is commutative") {
    Int128 a(123);
    Int128 b(456);

    CHECK(a + b == b + a);
    // Note: Other commutative operations (*, &, |, ^) inherit this property
  }

  TEST_CASE("Addition with carry") {
    Int128 a(UINT64_MAX);
    Int128 b(1);
    Int128 c = a + b;

    CHECK(c != Int128(0));
    CHECK(c > a);
  }

  TEST_CASE("Compound addition") {
    Int128 a(10);
    a += Int128(5);
    CHECK(a == Int128(15));

    a += Int128(25);
    CHECK(a == Int128(40));
  }

  TEST_CASE("Addition overflow behavior") {
    Int128 max = ~Int128(0);
    Int128 result = max + Int128(1);
    CHECK(result == Int128(0));
  }

  TEST_CASE("Multiple additions") {
    Int128 sum(0);
    for (int i = 1; i <= 100; ++i) {
      sum += Int128(i);
    }
    CHECK(sum == Int128(5050)); // Sum of 1 to 100
  }
}

TEST_SUITE("Subtraction") {
  TEST_CASE("Basic subtraction") {
    Int128 a(30);
    Int128 b(10);
    Int128 c = a - b;

    CHECK(c == Int128(20));
  }

  TEST_CASE("Subtraction with zero") {
    Int128 a(42);
    Int128 zero(0);

    CHECK(a - zero == a);
  }

  TEST_CASE("Subtraction resulting in zero") {
    Int128 a(42);
    Int128 b(42);

    CHECK(a - b == Int128(0));
  }

  TEST_CASE("Subtraction with borrow") {
    Int128 a(1);
    Int128 b(UINT64_MAX);
    Int128 c = a - b;

    CHECK(c != Int128(0));
  }

  TEST_CASE("Compound subtraction") {
    Int128 a(100);
    a -= Int128(25);
    CHECK(a == Int128(75));

    a -= Int128(50);
    CHECK(a == Int128(25));
  }

  TEST_CASE("Subtraction underflow behavior") {
    Int128 zero(0);
    Int128 one(1);
    Int128 result = zero - one;

    CHECK(result == Int128(-1));
  }

  TEST_CASE("Add then subtract returns original") {
    Int128 original(12345);
    Int128 value(6789);
    Int128 result = original + value - value;

    CHECK(result == original);
  }
}

TEST_SUITE("Multiplication") {
  TEST_CASE("Basic multiplication") {
    Int128 a(6);
    Int128 b(7);
    Int128 c = a * b;

    CHECK(c == Int128(42));
  }

  TEST_CASE("Multiplication by zero") {
    Int128 a(42);
    Int128 zero(0);

    CHECK(a * zero == Int128(0));
    CHECK(zero * a == Int128(0));
  }

  TEST_CASE("Multiplication by one") {
    Int128 a(42);
    Int128 one(1);

    CHECK(a * one == a);
    CHECK(one * a == a);
  }

  TEST_CASE("Large multiplication") {
    Int128 a(1000000);
    Int128 b(1000000);
    Int128 c = a * b;

    CHECK(c == Int128(1000000000000ULL));
  }

  TEST_CASE("Compound multiplication") {
    Int128 a(5);
    a *= Int128(3);
    CHECK(a == Int128(15));

    a *= Int128(2);
    CHECK(a == Int128(30));
  }

  TEST_CASE("Multiplication with powers of two") {
    Int128 a(7);
    CHECK(a * Int128(2) == Int128(14));
    CHECK(a * Int128(4) == Int128(28));
    CHECK(a * Int128(8) == Int128(56));
  }

  TEST_CASE("Multiplication overflow wraps around") {
    Int128 large = Int128(1) << 120;
    Int128 result = large * Int128(256);

    CHECK(result == Int128(0));
  }
}

TEST_SUITE("Division") {
  TEST_CASE("Basic division") {
    Int128 a(42);
    Int128 b(6);
    Int128 c = a / b;

    CHECK(c == Int128(7));
  }

  TEST_CASE("Division by one") {
    Int128 a(42);
    Int128 one(1);

    CHECK(a / one == a);
  }

  TEST_CASE("Division of equal numbers") {
    Int128 a(42);
    Int128 b(42);

    CHECK(a / b == Int128(1));
  }

  TEST_CASE("Division with remainder") {
    Int128 a(43);
    Int128 b(6);
    Int128 c = a / b;

    CHECK(c == Int128(7));
  }

  TEST_CASE("Division by larger number") {
    Int128 a(5);
    Int128 b(10);

    CHECK(a / b == Int128(0));
  }

  TEST_CASE("Division by zero throws") {
    Int128 a(42);
    Int128 zero(0);

    CHECK_THROWS_AS(a / zero, std::domain_error);
  }

  TEST_CASE("Compound division") {
    Int128 a(100);
    a /= Int128(5);
    CHECK(a == Int128(20));

    a /= Int128(4);
    CHECK(a == Int128(5));
  }

  TEST_CASE("Large division") {
    Int128 a(1000000000000ULL);
    Int128 b(1000000);

    CHECK(a / b == Int128(1000000));
  }

  TEST_CASE("Powers of two division") {
    Int128 a(1024);
    CHECK(a / Int128(2) == Int128(512));
    CHECK(a / Int128(4) == Int128(256));
    CHECK(a / Int128(8) == Int128(128));
  }
}

TEST_SUITE("Modulo") {
  TEST_CASE("Basic modulo") {
    Int128 a(43);
    Int128 b(6);
    Int128 c = a % b;

    CHECK(c == Int128(1));
  }

  TEST_CASE("Modulo with no remainder") {
    Int128 a(42);
    Int128 b(6);

    CHECK(a % b == Int128(0));
  }

  TEST_CASE("Modulo by one") {
    Int128 a(42);
    Int128 one(1);

    CHECK(a % one == Int128(0));
  }

  TEST_CASE("Modulo by larger number") {
    Int128 a(5);
    Int128 b(10);

    CHECK(a % b == Int128(5));
  }

  TEST_CASE("Modulo by zero throws") {
    Int128 a(42);
    Int128 zero(0);

    CHECK_THROWS_AS(a % zero, std::domain_error);
  }

  TEST_CASE("Compound modulo") {
    Int128 a(100);
    a %= Int128(7);
    CHECK(a == Int128(2));
  }

  TEST_CASE("Division and modulo relationship") {
    Int128 dividend(123);
    Int128 divisor(7);
    Int128 quotient = dividend / divisor;
    Int128 remainder = dividend % divisor;

    CHECK(quotient * divisor + remainder == dividend);
  }

  TEST_CASE("Modulo with powers of two") {
    Int128 a(1234);
    CHECK(a % Int128(2) == Int128(0));
    CHECK(a % Int128(4) == Int128(2));
    CHECK(a % Int128(8) == Int128(2));
  }
}

TEST_SUITE("Bitwise AND") {
  TEST_CASE("Basic AND") {
    Int128 a(0b1100);
    Int128 b(0b1010);
    Int128 c = a & b;

    CHECK(c == Int128(0b1000));
  }

  TEST_CASE("AND with zero") {
    Int128 a(42);
    Int128 zero(0);

    CHECK((a & zero) == Int128(0));
  }

  TEST_CASE("AND with all ones") {
    Int128 a(42);
    Int128 ones = ~Int128(0);

    CHECK((a & ones) == a);
  }

  TEST_CASE("Compound AND") {
    Int128 a(0b1111);
    a &= Int128(0b1100);
    CHECK(a == Int128(0b1100));

    a &= Int128(0b1010);
    CHECK(a == Int128(0b1000));
  }
}

TEST_SUITE("Bitwise OR") {
  TEST_CASE("Basic OR") {
    Int128 a(0b1100);
    Int128 b(0b1010);
    Int128 c = a | b;

    CHECK(c == Int128(0b1110));
  }

  TEST_CASE("OR with zero") {
    Int128 a(42);
    Int128 zero(0);

    CHECK((a | zero) == a);
  }

  TEST_CASE("OR with all ones") {
    Int128 a(42);
    Int128 ones = ~Int128(0);

    CHECK((a | ones) == ones);
  }

  TEST_CASE("Compound OR") {
    Int128 a(0b1000);
    a |= Int128(0b0100);
    CHECK(a == Int128(0b1100));

    a |= Int128(0b0010);
    CHECK(a == Int128(0b1110));
  }
}

TEST_SUITE("Bitwise XOR") {
  TEST_CASE("Basic XOR") {
    Int128 a(0b1100);
    Int128 b(0b1010);
    Int128 c = a ^ b;

    CHECK(c == Int128(0b0110));
  }

  TEST_CASE("XOR with zero") {
    Int128 a(42);
    Int128 zero(0);

    CHECK((a ^ zero) == a);
  }

  TEST_CASE("XOR with self") {
    Int128 a(42);
    CHECK((a ^ a) == Int128(0));
  }

  TEST_CASE("Double XOR returns original") {
    Int128 a(42);
    Int128 b(123);
    Int128 c = a ^ b ^ b;

    CHECK(c == a);
  }

  TEST_CASE("Compound XOR") {
    Int128 a(0b1111);
    a ^= Int128(0b1100);
    CHECK(a == Int128(0b0011));

    a ^= Int128(0b0101);
    CHECK(a == Int128(0b0110));
  }
}

TEST_SUITE("Left Shift") {
  TEST_CASE("Basic left shift") {
    Int128 a(1);
    Int128 b = a << 3;

    CHECK(b == Int128(8));
  }

  TEST_CASE("Left shift is multiplication by power of two") {
    Int128 a(5);
    CHECK((a << 1) == (a * Int128(2)));
    CHECK((a << 2) == (a * Int128(4)));
    CHECK((a << 3) == (a * Int128(8)));
  }

  TEST_CASE("Left shift across segment boundary") {
    Int128 a(1);
    Int128 b = a << 64;

    CHECK(b != Int128(0));
    CHECK(b != Int128(1));
  }

  TEST_CASE("Left shift beyond bit width") {
    Int128 a(42);
    Int128 b = a << 128;

    CHECK(b == Int128(0));
  }

  TEST_CASE("Left shift far beyond bit width") {
    Int128 a(42);
    Int128 b = a << 256;

    CHECK(b == Int128(0));
  }

  TEST_CASE("Compound left shift") {
    Int128 a(1);
    a <<= 2;
    CHECK(a == Int128(4));

    a <<= 3;
    CHECK(a == Int128(32));
  }

  TEST_CASE("Left shift preserves bits") {
    Int128 a(0b10101);
    Int128 b = a << 3;
    Int128 c = b >> 3;

    CHECK(c == a);
  }
}

TEST_SUITE("Right Shift") {
  TEST_CASE("Basic right shift") {
    Int128 a(8);
    Int128 b = a >> 3;

    CHECK(b == Int128(1));
  }

  TEST_CASE("Right shift is division by power of two") {
    Int128 a(40);
    CHECK((a >> 1) == (a / Int128(2)));
    CHECK((a >> 2) == (a / Int128(4)));
    CHECK((a >> 3) == (a / Int128(8)));
  }

  TEST_CASE("Right shift to zero") {
    Int128 a(42);
    Int128 b = a >> 64;

    CHECK(b == Int128(0));
  }

  TEST_CASE("Right shift beyond bit width") {
    Int128 a(42);
    Int128 b = a >> 128;

    CHECK(b == Int128(0));
  }

  TEST_CASE("Compound right shift") {
    Int128 a(32);
    a >>= 2;
    CHECK(a == Int128(8));

    a >>= 3;
    CHECK(a == Int128(1));
  }

  TEST_CASE("Left then right shift") {
    Int128 a(42);
    Int128 b = (a << 5) >> 5;

    CHECK(b == a);
  }
}

TEST_SUITE("Increment and Decrement") {
  TEST_CASE("Pre-increment") {
    Int128 a(42);
    Int128 b = ++a;

    CHECK(a == Int128(43));
    CHECK(b == Int128(43));
  }

  TEST_CASE("Post-increment") {
    Int128 a(42);
    Int128 b = a++;

    CHECK(a == Int128(43));
    CHECK(b == Int128(42));
  }

  TEST_CASE("Pre-decrement") {
    Int128 a(42);
    Int128 b = --a;

    CHECK(a == Int128(41));
    CHECK(b == Int128(41));
  }

  TEST_CASE("Post-decrement") {
    Int128 a(42);
    Int128 b = a--;

    CHECK(a == Int128(41));
    CHECK(b == Int128(42));
  }

  TEST_CASE("Increment from zero") {
    Int128 a(0);
    ++a;
    CHECK(a == Int128(1));
  }

  TEST_CASE("Decrement to zero") {
    Int128 a(1);
    --a;
    CHECK(a == Int128(0));
  }

  TEST_CASE("Multiple increments") {
    Int128 a(0);
    for (int i = 0; i < 100; ++i) {
      ++a;
    }
    CHECK(a == Int128(100));
  }

  TEST_CASE("Increment with carry") {
    Int128 a(UINT64_MAX);
    ++a;
    CHECK(a != Int128(0));
    CHECK(a > Int128(UINT64_MAX));
  }

  TEST_CASE("Decrement with borrow") {
    Int128 a(0);
    --a;
    CHECK(a == Int128(-1));
  }
}

TEST_SUITE("Comparison Operators") {
  TEST_CASE("Equality") {
    Int128 a(42);
    Int128 b(42);
    Int128 c(43);

    CHECK(a == b);
    CHECK_FALSE(a == c);
  }

  TEST_CASE("Inequality with spaceship") {
    Int128 a(42);
    Int128 b(43);

    CHECK(a != b);
    CHECK_FALSE(a != a);
  }

  TEST_CASE("Less than") {
    Int128 a(42);
    Int128 b(43);

    CHECK(a < b);
    CHECK_FALSE(b < a);
    CHECK_FALSE(a < a);
  }

  TEST_CASE("Greater than") {
    Int128 a(42);
    Int128 b(43);

    CHECK(b > a);
    CHECK_FALSE(a > b);
    CHECK_FALSE(a > a);
  }

  TEST_CASE("Less than or equal") {
    Int128 a(42);
    Int128 b(43);

    CHECK(a <= b);
    CHECK(a <= a);
    CHECK_FALSE(b <= a);
  }

  TEST_CASE("Greater than or equal") {
    Int128 a(42);
    Int128 b(43);

    CHECK(b >= a);
    CHECK(a >= a);
    CHECK_FALSE(a >= b);
  }

  TEST_CASE("Zero comparisons") {
    Int128 zero(0);
    Int128 one(1);

    CHECK(zero == Int128(0));
    CHECK(zero < one);
    CHECK_FALSE(zero > one);
  }

  TEST_CASE("Large value comparisons") {
    Int128 a(UINT64_MAX);
    Int128 b(UINT64_MAX);
    Int128 c = b + Int128(1);

    CHECK(a == b);
    CHECK(a < c);
    CHECK(c > a);
  }

  TEST_CASE("Spaceship operator transitivity") {
    Int128 a(10);
    Int128 b(20);
    Int128 c(30);

    CHECK(a < b);
    CHECK(b < c);
    CHECK(a < c);
  }
}

TEST_SUITE("Boolean Conversion") {
  TEST_CASE("Zero is false") {
    Int128 zero(0);
    CHECK_FALSE(static_cast<bool>(zero));
  }

  TEST_CASE("Non-zero is true") {
    Int128 one(1);
    Int128 large(UINT64_MAX);

    CHECK(static_cast<bool>(one));
    CHECK(static_cast<bool>(large));
  }

  TEST_CASE("Negative values are true") {
    Int128 neg(-1);
    CHECK(static_cast<bool>(neg));
  }
}

TEST_SUITE("Complex Operations") {
  TEST_CASE("Arithmetic expression") {
    Int128 a(10);
    Int128 b(20);
    Int128 c(5);
    Int128 result = (a + b) * c - Int128(50);

    CHECK(result == Int128(100));
  }

  TEST_CASE("Mixed operations") {
    Int128 a(100);
    Int128 result = (a / Int128(5) + Int128(10)) * Int128(2);

    CHECK(result == Int128(60));
  }

  TEST_CASE("Bitwise combination") {
    Int128 a(0b1100);
    Int128 b(0b1010);
    Int128 result = (a & b) | (a ^ b);

    CHECK(result == (a | b));
  }

  TEST_CASE("Shift and add") {
    Int128 a(5);
    Int128 result = (a << 2) + a; // 5 * 4 + 5 = 25

    CHECK(result == Int128(25));
  }

  TEST_CASE("Power calculation") {
    Int128 base(2);
    Int128 power(1);
    for (int i = 0; i < 10; ++i) {
      power *= base;
    }

    CHECK(power == Int128(1024));
  }

  TEST_CASE("Factorial") {
    Int128 factorial(1);
    for (int i = 2; i <= 20; ++i) {
      factorial *= Int128(i);
    }

    CHECK(factorial == Int128(2432902008176640000ULL));
  }

  TEST_CASE("GCD using Euclidean algorithm") {
    Int128 a(48);
    Int128 b(18);

    while (static_cast<bool>(b)) {
      Int128 temp = b;
      b = a % b;
      a = temp;
    }

    CHECK(a == Int128(6));
  }

  TEST_CASE("Fibonacci sequence") {
    Int128 a(0), b(1);
    for (int i = 0; i < 10; ++i) {
      Int128 temp = a + b;
      a = b;
      b = temp;
    }

    CHECK(b == Int128(89));
  }
}

TEST_SUITE("Edge Cases") {
  TEST_CASE("Maximum value") {
    Int128 max = ~Int128(0);
    CHECK(static_cast<bool>(max));
    CHECK(max > Int128(0));
  }

  TEST_CASE("Zero minus one") {
    Int128 zero(0);
    Int128 result = zero - Int128(1);
    CHECK(result == Int128(-1));
  }

  TEST_CASE("Division edge cases") {
    Int128 max = ~Int128(0);
    CHECK(max / max == Int128(1));
    CHECK(max / Int128(1) == max);
    CHECK(Int128(0) / max == Int128(0));
  }

  TEST_CASE("Modulo edge cases") {
    Int128 max = ~Int128(0);
    CHECK(max % max == Int128(0));
    CHECK(max % Int128(1) == Int128(0));
  }

  TEST_CASE("Shift by exact segment size") {
    Int128 a(1);
    Int128 b = a << 64;
    Int128 c = b >> 64;

    CHECK(c == a);
  }

  TEST_CASE("All bits set operations") {
    Int128 all = ~Int128(0);
    CHECK((all & all) == all);
    CHECK((all | all) == all);
    CHECK((all ^ all) == Int128(0));
  }
}

TEST_SUITE("Different Bit Sizes") {
  TEST_CASE("128-bit specific") {
    Int128 a(UINT64_MAX);
    Int128 b = a + Int128(1);

    CHECK(b > a);
  }

  TEST_CASE("256-bit specific") {
    Int256 a(UINT64_MAX);
    Int256 b = (a << 64) | a;

    CHECK(b > a);
  }

  TEST_CASE("512-bit specific") {
    Int512 a(1);
    Int512 b = a << 256;

    CHECK(b != a);
    CHECK(b != Int512(0));
  }

  TEST_CASE("Cross-size consistency") {
    Int128 a128(12345);
    Int256 a256(12345);
    Int512 a512(12345);

    CHECK(a128 == Int128(12345));
    CHECK(a256 == Int256(12345));
    CHECK(a512 == Int512(12345));
  }
}

TEST_SUITE("Inverse Elements") {
  TEST_CASE("Additive inverse") {
    Int128 a(42);
    Int128 neg_a = -a;

    CHECK(a + neg_a == Int128(0));
  }

  TEST_CASE("NOT NOT returns original") {
    Int128 a(42);
    CHECK(~~a == a);
  }
}

TEST_SUITE("Overflow and Underflow") {
  TEST_CASE("Addition overflow wraps") {
    Int128 max = ~Int128(0);
    Int128 one(1);
    Int128 result = max + one;

    CHECK(result == Int128(0));
  }

  TEST_CASE("Multiplication overflow") {
    Int128 large = Int128(1) << 100;
    Int128 result = large * Int128(256);

    // Result should wrap around
    CHECK(result != large);
  }

  TEST_CASE("Left shift overflow") {
    Int128 max = ~Int128(0);
    Int128 result = max << 1;

    CHECK(result != max);
  }
}

TEST_SUITE("Bit Manipulation Patterns") {
  TEST_CASE("Set bit") {
    Int128 a(0);
    a |= (Int128(1) << 5);

    CHECK((a & (Int128(1) << 5)) != Int128(0));
  }

  TEST_CASE("Clear bit") {
    Int128 a = ~Int128(0);
    a &= ~(Int128(1) << 5);

    CHECK((a & (Int128(1) << 5)) == Int128(0));
  }

  TEST_CASE("Toggle bit") {
    Int128 a(0);
    a ^= (Int128(1) << 5);
    CHECK((a & (Int128(1) << 5)) != Int128(0));

    a ^= (Int128(1) << 5);
    CHECK((a & (Int128(1) << 5)) == Int128(0));
  }

  TEST_CASE("Test bit") {
    Int128 a = Int128(1) << 10;

    CHECK((a & (Int128(1) << 10)) != Int128(0));
    CHECK((a & (Int128(1) << 9)) == Int128(0));
  }

  TEST_CASE("Isolate lowest set bit") {
    Int128 a(0b101100);
    Int128 lowest = a & (-a);

    CHECK(lowest == Int128(0b000100));
  }

  TEST_CASE("Clear lowest set bit") {
    Int128 a(0b101100);
    Int128 cleared = a & (a - Int128(1));

    CHECK(cleared == Int128(0b101000));
  }
}

TEST_SUITE("Multi-Segment Operations") {
  TEST_CASE("Value spanning segments - addition") {
    Int128 a = (Int128(1) << 63) - Int128(1);
    Int128 b(2);
    Int128 c = a + b;

    CHECK(c > a);
    CHECK(c > b);
  }

  TEST_CASE("Value spanning segments - subtraction") {
    Int128 a = Int128(1) << 64;
    Int128 b(1);
    Int128 c = a - b;

    CHECK(c < a);
  }

  TEST_CASE("Value spanning segments - multiplication") {
    Int128 a = Int128(1) << 32;
    Int128 b = Int128(1) << 32;
    Int128 c = a * b;

    CHECK(c == (Int128(1) << 64));
  }

  TEST_CASE("Shift across multiple segments") {
    Int256 a(1);
    Int256 b = a << 200;
    Int256 c = b >> 200;

    CHECK(c == a);
  }

  TEST_CASE("Large value across all segments") {
    Int256 a = ~Int256(0);
    Int256 b = a >> 1;

    CHECK(b < a);
    CHECK(b != Int256(0));
  }
}

TEST_SUITE("Division and Modulo Edge Cases") {
  TEST_CASE("Division by self") {
    Int128 a(123456789);
    CHECK(a / a == Int128(1));
  }

  TEST_CASE("Division resulting in zero") {
    Int128 a(100);
    Int128 b(200);
    CHECK(a / b == Int128(0));
  }

  TEST_CASE("Modulo with dividend less than divisor") {
    Int128 a(50);
    Int128 b(100);
    CHECK(a % b == a);
  }

  TEST_CASE("Modulo equals dividend minus divisor") {
    Int128 a(150);
    Int128 b(100);
    CHECK(a % b == Int128(50));
  }

  TEST_CASE("Large division") {
    Int256 dividend = (Int256(1) << 200);
    Int256 divisor = (Int256(1) << 100);
    Int256 quotient = dividend / divisor;

    CHECK(quotient == (Int256(1) << 100));
  }

  TEST_CASE("Division and modulo with powers of 2") {
    Int128 a(1000);
    for (int shift = 0; shift < 10; ++shift) {
      Int128 divisor = Int128(1) << shift;
      Int128 quotient = a / divisor;
      Int128 remainder = a % divisor;

      CHECK(quotient * divisor + remainder == a);
    }
  }
}

TEST_SUITE("Signed Behavior") {
  TEST_CASE("Negative number representation") {
    Int128 pos(42);
    Int128 neg = -pos;

    CHECK(neg != pos);
    CHECK(neg + pos == Int128(0));
  }

  TEST_CASE("Negative plus negative") {
    Int128 a(-10);
    Int128 b(-20);
    Int128 c = a + b;

    CHECK(c == Int128(-30));
  }

  TEST_CASE("Negative minus negative") {
    Int128 a(-10);
    Int128 b(-20);
    Int128 c = a - b;

    CHECK(c == Int128(10));
  }

  TEST_CASE("Negative multiplication") {
    Int128 a(-6);
    Int128 b(7);
    Int128 c = a * b;

    CHECK(c == Int128(-42));
  }

  TEST_CASE("Double negation") {
    Int128 a(42);
    Int128 b = -(-a);

    CHECK(b == a);
  }
}

TEST_SUITE("Chained Operations") {
  TEST_CASE("Multiple additions") {
    Int128 result = Int128(1) + Int128(2) + Int128(3) + Int128(4) + Int128(5);
    CHECK(result == Int128(15));
  }

  TEST_CASE("Multiple multiplications") {
    Int128 result = Int128(2) * Int128(3) * Int128(5);
    CHECK(result == Int128(30));
  }

  TEST_CASE("Mixed arithmetic operations") {
    Int128 result = Int128(10) + Int128(20) * Int128(3) - Int128(5);
    // 10 + 60 - 5 = 65
    CHECK(result == Int128(65));
  }

  TEST_CASE("Multiple bitwise operations") {
    Int128 result = Int128(0b1111) & Int128(0b1100) | Int128(0b0010);
    // (0b1100) | 0b0010 = 0b1110
    CHECK(result == Int128(0b1110));
  }

  TEST_CASE("Multiple shifts") {
    Int128 a(1);
    Int128 result = (a << 10) >> 5;
    CHECK(result == Int128(32));
  }
}

TEST_SUITE("Boundary Value Testing") {
  TEST_CASE("UINT64_MAX operations") {
    Int128 a(UINT64_MAX);
    Int128 b(1);

    CHECK(a + b > a);
    CHECK(a - b < a);
    CHECK(a * Int128(2) > a);
  }

  TEST_CASE("Operations at segment boundaries") {
    Int128 boundary = Int128(1) << 64;

    CHECK(boundary > Int128(UINT64_MAX));
    CHECK(boundary - Int128(1) == Int128(UINT64_MAX));
  }

  TEST_CASE("Maximum representable value") {
    Int128 max = ~Int128(0);

    CHECK(max > Int128(0));
    CHECK(max >= Int128(UINT64_MAX));
  }

  TEST_CASE("Alternating bit patterns") {
    Int128 a(0xAAAAAAAAAAAAAAAAULL);
    Int128 b(0x5555555555555555ULL);

    CHECK((a | b) == Int128(UINT64_MAX));
    CHECK((a & b) == Int128(0));
    CHECK((a ^ b) == Int128(UINT64_MAX));
  }
}

TEST_SUITE("Stress Tests") {
  TEST_CASE("Many sequential operations") {
    Int128 sum(0);
    for (int i = 0; i < 1000; ++i) {
      sum += Int128(i);
    }
    // Sum of 0 to 999 = 999 * 1000 / 2 = 499500
    CHECK(sum == Int128(499500));
  }

  TEST_CASE("Alternating add and subtract") {
    Int128 value(1000);
    for (int i = 0; i < 100; ++i) {
      value += Int128(10);
      value -= Int128(5);
    }
    // Net +5 per iteration, 100 iterations = +500
    CHECK(value == Int128(1500));
  }

  TEST_CASE("Power of 2 sequence") {
    Int128 power(1);
    for (int i = 0; i < 64; ++i) {
      power *= Int128(2);
    }
    CHECK(power == (Int128(1) << 64));
  }

  TEST_CASE("Shift pattern") {
    Int128 value(1);
    for (int i = 0; i < 10; ++i) {
      value <<= 1;
    }
    CHECK(value == Int128(1024));
  }

  TEST_CASE("Complex nested operations") {
    Int128 a(10), b(20), c(30);
    Int128 result = ((a + b) * c - (a * b)) / Int128(5);
    // ((10 + 20) * 30 - (10 * 20)) / 5
    // (30 * 30 - 200) / 5
    // (900 - 200) / 5
    // 700 / 5 = 140
    CHECK(result == Int128(140));
  }
}

TEST_SUITE("Consistency Checks") {
  TEST_CASE("Multiplication and division are inverse operations") {
    Int128 a(12345);
    Int128 b(67);

    CHECK((a * b) / b == a);
  }

  TEST_CASE("Shift left and shift right are inverse operations") {
    Int128 a(12345);

    for (int shift = 0; shift < 50; ++shift) {
      auto shifted = (a << shift) >> shift;
      CHECK(shifted == a);
    }
  }

  TEST_CASE("Bitwise operations consistency") {
    Int128 a(0b11001100);
    Int128 b(0b10101010);

    // (A AND B) OR (A AND NOT B) OR (NOT A AND B) == A XOR B OR (A AND B) == A
    // OR B
    Int128 lhs = (a & b) | (a & ~b) | (~a & b);
    Int128 rhs = a | b;
    CHECK(lhs == rhs);
  }

  TEST_CASE("Comparison consistency") {
    Int128 a(10);
    Int128 b(20);

    CHECK(!(a < b) == (a >= b));
    CHECK(!(a > b) == (a <= b));
    CHECK((a == b) == !(a != b));
  }
}

TEST_SUITE("Real-World Scenarios") {
  TEST_CASE("Calculate large factorial") {
    Int256 factorial(1);
    for (int i = 2; i <= 30; ++i) {
      factorial *= Int256(i);
    }
    // 30! = 265252859812191058636308480000000
    CHECK(factorial > Int256(0));
    CHECK(static_cast<bool>(factorial));
  }

  TEST_CASE("Binary counter simulation") {
    Int128 counter(0);
    for (int i = 0; i < 100; ++i) {
      ++counter;
    }
    CHECK(counter == Int128(100));
  }

  TEST_CASE("Bit mask operations") {
    Int128 permissions(0);

    // Set permissions
    permissions |= (Int128(1) << 0); // Read
    permissions |= (Int128(1) << 1); // Write
    permissions |= (Int128(1) << 2); // Execute

    // Check permissions
    CHECK((permissions & (Int128(1) << 0)) != Int128(0));
    CHECK((permissions & (Int128(1) << 1)) != Int128(0));
    CHECK((permissions & (Int128(1) << 2)) != Int128(0));

    // Clear write permission
    permissions &= ~(Int128(1) << 1);
    CHECK((permissions & (Int128(1) << 1)) == Int128(0));
  }

  TEST_CASE("Hash computation simulation") {
    Int128 hash(0);
    Int128 multiplier(31);

    // Simple polynomial rolling hash
    for (int i = 0; i < 10; ++i) {
      hash = hash * multiplier + Int128(i);
    }

    CHECK(hash > Int128(0));
  }

  TEST_CASE("Checksum calculation") {
    Int128 checksum(0);
    for (int i = 1; i <= 100; ++i) {
      checksum ^= Int128(i);
    }

    // XOR of 1 to 100
    CHECK(static_cast<bool>(checksum));
  }
}

TEST_SUITE("Regression Tests") {
  TEST_CASE("XOR with all ones inverts") {
    Int128 a(0b10101010);
    Int128 ones = ~Int128(0);
    CHECK((a ^ ones) == ~a);
  }

  TEST_CASE("Increment then decrement returns original") {
    Int128 a(42);
    ++a;
    --a;
    CHECK(a == Int128(42));
  }
}

TEST_SUITE("std::numeric_limits specialization") {
  TEST_CASE("is_specialized") {
    CHECK(std::numeric_limits<Int128>::is_specialized);
    CHECK(std::numeric_limits<Int256>::is_specialized);
    CHECK(std::numeric_limits<Int512>::is_specialized);
  }

  TEST_CASE("Type properties") {
    CHECK_FALSE(std::numeric_limits<Int128>::is_signed);
    CHECK(std::numeric_limits<Int128>::is_integer);
    CHECK(std::numeric_limits<Int128>::is_exact);
    CHECK(std::numeric_limits<Int128>::is_bounded);
    CHECK(std::numeric_limits<Int128>::is_modulo);
  }

  TEST_CASE("No floating point properties") {
    CHECK_FALSE(std::numeric_limits<Int128>::has_infinity);
    CHECK_FALSE(std::numeric_limits<Int128>::has_quiet_NaN);
    CHECK_FALSE(std::numeric_limits<Int128>::has_signaling_NaN);
    CHECK_FALSE(std::numeric_limits<Int128>::is_iec559);
  }

  TEST_CASE("Rounding and trapping") {
    CHECK_FALSE(std::numeric_limits<Int128>::traps);
    CHECK_FALSE(std::numeric_limits<Int128>::tinyness_before);
  }

  TEST_CASE("Radix is binary") {
    CHECK(std::numeric_limits<Int128>::radix == 2);
    CHECK(std::numeric_limits<Int256>::radix == 2);
    CHECK(std::numeric_limits<Int512>::radix == 2);
  }

  TEST_CASE("Digits for different bit sizes") {
    CHECK(std::numeric_limits<Int128>::digits == 128);
    CHECK(std::numeric_limits<Int256>::digits == 256);
    CHECK(std::numeric_limits<Int512>::digits == 512);
  }

  TEST_CASE("Digits10 calculation") {
    // digits10 should be approximately Bits * log10(2)
    CHECK(std::numeric_limits<Int128>::digits10 == 38);
    CHECK(std::numeric_limits<Int256>::digits10 == 77);
    CHECK(std::numeric_limits<Int512>::digits10 == 154);
  }

  TEST_CASE("min() returns zero") {
    CHECK(std::numeric_limits<Int128>::min() == Int128(0));
    CHECK(std::numeric_limits<Int256>::min() == Int256(0));
    CHECK(std::numeric_limits<Int512>::min() == Int512(0));
  }

  TEST_CASE("lowest() returns zero") {
    CHECK(std::numeric_limits<Int128>::lowest() == Int128(0));
    CHECK(std::numeric_limits<Int256>::lowest() == Int256(0));
    CHECK(std::numeric_limits<Int512>::lowest() == Int512(0));
  }

  TEST_CASE("max() returns all ones") {
    Int128 max128 = std::numeric_limits<Int128>::max();
    Int256 max256 = std::numeric_limits<Int256>::max();
    Int512 max512 = std::numeric_limits<Int512>::max();

    CHECK(max128 == ~Int128(0));
    CHECK(max256 == ~Int256(0));
    CHECK(max512 == ~Int512(0));
  }

  TEST_CASE("max() is greater than any small value") {
    Int128 max = std::numeric_limits<Int128>::max();

    CHECK(max > Int128(0));
    CHECK(max > Int128(1));
    CHECK(max > Int128(UINT64_MAX));
  }

  TEST_CASE("min() and max() relationship") {
    Int128 min = std::numeric_limits<Int128>::min();
    Int128 max = std::numeric_limits<Int128>::max();

    CHECK(min < max);
    CHECK(min == Int128(0));
  }

  TEST_CASE("Exponent properties are zero") {
    CHECK(std::numeric_limits<Int128>::min_exponent == 0);
    CHECK(std::numeric_limits<Int128>::min_exponent10 == 0);
    CHECK(std::numeric_limits<Int128>::max_exponent == 0);
    CHECK(std::numeric_limits<Int128>::max_exponent10 == 0);
    CHECK(std::numeric_limits<Int128>::max_digits10 == 0);
  }

  TEST_CASE("max() + 1 wraps to min()") {
    Int128 max = std::numeric_limits<Int128>::max();
    Int128 min = std::numeric_limits<Int128>::min();

    CHECK(max + Int128(1) == min);
  }

  TEST_CASE("Consistency with builtin integer limits") {
    // Our Integer should behave similarly to uint64_t
    CHECK(std::numeric_limits<Int128>::is_integer ==
          std::numeric_limits<uint64_t>::is_integer);
    CHECK(std::numeric_limits<Int128>::is_exact ==
          std::numeric_limits<uint64_t>::is_exact);
    CHECK(std::numeric_limits<Int128>::is_modulo ==
          std::numeric_limits<uint64_t>::is_modulo);
    CHECK(std::numeric_limits<Int128>::radix ==
          std::numeric_limits<uint64_t>::radix);
  }

  TEST_CASE("Using limits in generic code") {
    auto test_type = [](auto value) {
      using T = decltype(value);
      T max_val = std::numeric_limits<T>::max();
      T min_val = std::numeric_limits<T>::min();

      CHECK(max_val > min_val);
      CHECK(std::numeric_limits<T>::is_specialized);
    };

    test_type(Int128(0));
    test_type(Int256(0));
    test_type(Int512(0));
  }

  TEST_CASE("Range contains UINT64_MAX") {
    Int128 max = std::numeric_limits<Int128>::max();
    Int128 uint64_max(UINT64_MAX);

    CHECK(max > uint64_max);
    CHECK(uint64_max >= std::numeric_limits<Int128>::min());
    CHECK(uint64_max <= std::numeric_limits<Int128>::max());
  }

  TEST_CASE("Constexpr evaluation") {
    // These should all be constexpr
    constexpr Int128 max = std::numeric_limits<Int128>::max();
    constexpr Int128 min = std::numeric_limits<Int128>::min();
    constexpr int digits = std::numeric_limits<Int128>::digits;
    constexpr bool is_signed = std::numeric_limits<Int128>::is_signed;

    CHECK(digits == 128);
    CHECK_FALSE(is_signed);
    CHECK(static_cast<bool>(max));
    CHECK_FALSE(static_cast<bool>(min));
  }
}

TEST_SUITE("String Conversion") {
  TEST_CASE("to_string with zero") {
    Int128 zero(0);
    CHECK(ArbitraryPrecision::to_string(zero) == "0");
  }

  TEST_CASE("to_string with small numbers") {
    CHECK(ArbitraryPrecision::to_string(Int128(1)) == "1");
    CHECK(ArbitraryPrecision::to_string(Int128(42)) == "42");
    CHECK(ArbitraryPrecision::to_string(Int128(123)) == "123");
    CHECK(ArbitraryPrecision::to_string(Int128(999)) == "999");
  }

  TEST_CASE("to_string with larger numbers") {
    CHECK(ArbitraryPrecision::to_string(Int128(12345)) == "12345");
    CHECK(ArbitraryPrecision::to_string(Int128(1000000)) == "1000000");
    CHECK(ArbitraryPrecision::to_string(Int128(UINT64_MAX)) ==
          "18446744073709551615");
  }

  TEST_CASE("to_string with powers of 10") {
    CHECK(ArbitraryPrecision::to_string(Int128(10)) == "10");
    CHECK(ArbitraryPrecision::to_string(Int128(100)) == "100");
    CHECK(ArbitraryPrecision::to_string(Int128(1000)) == "1000");
    CHECK(ArbitraryPrecision::to_string(Int128(10000)) == "10000");
  }

  TEST_CASE("to_string with arithmetic results") {
    Int128 a(123);
    Int128 b(456);
    Int128 sum = a + b;
    CHECK(ArbitraryPrecision::to_string(sum) == "579");

    Int128 product = a * b;
    CHECK(ArbitraryPrecision::to_string(product) == "56088");
  }

  TEST_CASE("to_string with large values across segments") {
    Int128 large = (Int128(1) << 64) + Int128(42);
    std::string result = ArbitraryPrecision::to_string(large);
    CHECK(result == "18446744073709551658");
  }

  TEST_CASE("from_string with valid input") {
    CHECK(ArbitraryPrecision::from_string<ArbitraryPrecision::Kind::Fixed, 128>(
              "0")
              .value() == Int128(0));
    CHECK(ArbitraryPrecision::from_string<ArbitraryPrecision::Kind::Fixed, 128>(
              "1")
              .value() == Int128(1));
    CHECK(ArbitraryPrecision::from_string<ArbitraryPrecision::Kind::Fixed, 128>(
              "42")
              .value() == Int128(42));
    CHECK(ArbitraryPrecision::from_string<ArbitraryPrecision::Kind::Fixed, 128>(
              "123")
              .value() == Int128(123));
    CHECK(ArbitraryPrecision::from_string<ArbitraryPrecision::Kind::Fixed, 128>(
              "12345")
              .value() == Int128(12345));
  }

  TEST_CASE("from_string with large numbers") {
    CHECK(ArbitraryPrecision::from_string<ArbitraryPrecision::Kind::Fixed, 128>(
              "1000000")
              .value() == Int128(1000000));
    CHECK(ArbitraryPrecision::from_string<ArbitraryPrecision::Kind::Fixed, 128>(
              "18446744073709551615")
              .value() == Int128(UINT64_MAX));
  }

  TEST_CASE("from_string with leading zeros") {
    CHECK(ArbitraryPrecision::from_string<ArbitraryPrecision::Kind::Fixed, 128>(
              "00042")
              .value() == Int128(42));
    CHECK(ArbitraryPrecision::from_string<ArbitraryPrecision::Kind::Fixed, 128>(
              "0000")
              .value() == Int128(0));
  }

  TEST_CASE("to_string and from_string roundtrip") {
    Int128 original(12345);
    std::string str = ArbitraryPrecision::to_string(original);
    Int128 parsed =
        ArbitraryPrecision::from_string<ArbitraryPrecision::Kind::Fixed, 128>(
            str)
            .value();
    CHECK(parsed == original);
  }

  TEST_CASE("to_string and from_string roundtrip with large values") {
    Int128 original(UINT64_MAX);
    std::string str = ArbitraryPrecision::to_string(original);
    Int128 parsed =
        ArbitraryPrecision::from_string<ArbitraryPrecision::Kind::Fixed, 128>(
            str)
            .value();
    CHECK(parsed == original);
  }

  TEST_CASE("to_string and from_string roundtrip with zero") {
    Int128 original(0);
    std::string str = ArbitraryPrecision::to_string(original);
    Int128 parsed =
        ArbitraryPrecision::from_string<ArbitraryPrecision::Kind::Fixed, 128>(
            str)
            .value();
    CHECK(parsed == original);
    CHECK(str == "0");
  }

  TEST_CASE("to_string and from_string with different bit sizes") {
    Int256 val256(123456789);
    std::string str256 = ArbitraryPrecision::to_string(val256);
    Int256 parsed256 =
        ArbitraryPrecision::from_string<ArbitraryPrecision::Kind::Fixed, 256>(
            str256)
            .value();
    CHECK(parsed256 == val256);

    Int512 val512(987654321);
    std::string str512 = ArbitraryPrecision::to_string(val512);
    Int512 parsed512 =
        ArbitraryPrecision::from_string<ArbitraryPrecision::Kind::Fixed, 512>(
            str512)
            .value();
    CHECK(parsed512 == val512);
  }

  TEST_CASE("to_string with computed values") {
    // Test factorial converted to string
    Int128 factorial(1);
    for (int i = 2; i <= 10; ++i) {
      factorial *= Int128(i);
    }
    CHECK(ArbitraryPrecision::to_string(factorial) == "3628800");

    // Test power of 2
    Int128 power2 = Int128(1) << 20;
    CHECK(ArbitraryPrecision::to_string(power2) == "1048576");
  }

  TEST_CASE("to_string with very large multi-segment value") {
    // Create a value that spans multiple segments
    Int256 large = (Int256(1) << 128) - Int256(1);
    std::string result = ArbitraryPrecision::to_string(large);
    // 2^128 - 1 = 340282366920938463463374607431768211455
    CHECK(result == "340282366920938463463374607431768211455");
  }

  TEST_CASE("from_string with very large multi-segment value") {
    std::string large_str = "340282366920938463463374607431768211455";
    Int256 parsed =
        ArbitraryPrecision::from_string<ArbitraryPrecision::Kind::Fixed, 256>(
            large_str)
            .value();
    Int256 expected = (Int256(1) << 128) - Int256(1);
    CHECK(parsed == expected);
  }
}

// Type alias for Dynamic
using Dynamic = ArbitraryPrecision::Dynamic;

TEST_SUITE("Dynamic Integer - Basic Operations") {
  TEST_CASE("Default constructor initializes to zero") {
    Dynamic a;
    CHECK(a == Dynamic(0));
    CHECK_FALSE(static_cast<bool>(a));
    CHECK(a.length() == 1);
  }

  TEST_CASE("Constructor from unsigned integers") {
    Dynamic a(42);
    Dynamic b(0);
    Dynamic c(std::numeric_limits<uint64_t>::max());

    CHECK(a == Dynamic(42));
    CHECK(b == Dynamic(0));
    CHECK(c == Dynamic(std::numeric_limits<uint64_t>::max()));
  }

  TEST_CASE("Constructor from signed integers") {
    Dynamic a(42);
    Dynamic b(-42);
    Dynamic c(0);

    CHECK(a == Dynamic(42));
    CHECK(b == Dynamic(-42));
    CHECK(c == Dynamic(0));
  }

  TEST_CASE("Constructor with maximum uint64_t value") {
    Dynamic a(UINT64_MAX);
    CHECK(a == Dynamic(UINT64_MAX));
    CHECK(a.length() == 1);
  }

  TEST_CASE("Basic addition") {
    Dynamic a(10);
    Dynamic b(20);
    Dynamic c = a + b;

    CHECK(c == Dynamic(30));
  }

  TEST_CASE("Basic subtraction") {
    Dynamic a(30);
    Dynamic b(10);
    Dynamic c = a - b;

    CHECK(c == Dynamic(20));
  }

  TEST_CASE("Basic multiplication") {
    Dynamic a(6);
    Dynamic b(7);
    Dynamic c = a * b;

    CHECK(c == Dynamic(42));
  }

  TEST_CASE("Basic division") {
    Dynamic a(42);
    Dynamic b(6);
    Dynamic c = a / b;

    CHECK(c == Dynamic(7));
  }

  TEST_CASE("Basic modulo") {
    Dynamic a(43);
    Dynamic b(6);
    Dynamic c = a % b;

    CHECK(c == Dynamic(1));
  }

  TEST_CASE("Unary operators") {
    Dynamic a(42);
    CHECK(+a == a);
    CHECK(-a == Dynamic(-42));
    CHECK(~~a == a);
  }

  TEST_CASE("Increment and decrement") {
    Dynamic a(42);
    ++a;
    CHECK(a == Dynamic(43));
    --a;
    CHECK(a == Dynamic(42));
  }

  TEST_CASE("Comparison operators") {
    Dynamic a(10);
    Dynamic b(20);
    Dynamic c(10);

    CHECK(a < b);
    CHECK(b > a);
    CHECK(a == c);
    CHECK(a <= c);
    CHECK(a >= c);
    CHECK(a != b);
  }
}

TEST_SUITE("Dynamic Integer - Growth Behavior") {
  TEST_CASE("Addition causes growth") {
    Dynamic a(UINT64_MAX);
    Dynamic b(1);
    Dynamic c = a + b;

    CHECK(c.length() == 2);
    CHECK(c > a);
    CHECK(c != Dynamic(0));
  }

  TEST_CASE("Addition with carry propagation") {
    Dynamic a(UINT64_MAX);
    Dynamic b(UINT64_MAX);
    Dynamic c = a + b;

    CHECK(c.length() >= 2);
    CHECK(c > a);
    CHECK(c > b);
  }

  TEST_CASE("Multiplication causes growth") {
    Dynamic a(UINT64_MAX);
    Dynamic b(2);
    Dynamic c = a * b;

    CHECK(c.length() == 2);
    CHECK(c > a);
  }

  TEST_CASE("Large multiplication") {
    Dynamic a(1ULL << 32);
    Dynamic b(1ULL << 32);
    Dynamic c = a * b;

    CHECK(c.length() == 2);
    CHECK(c == (Dynamic(1) << 64));
  }

  TEST_CASE("Left shift causes growth") {
    Dynamic a(1);
    Dynamic b = a << 65;

    CHECK(b.length() == 2);
    CHECK(b != Dynamic(0));
  }

  TEST_CASE("Left shift with bit overflow") {
    Dynamic a(UINT64_MAX);
    Dynamic b = a << 1;

    CHECK(b.length() == 2);
    CHECK(b > a);
  }

  TEST_CASE("Increment with carry propagation") {
    Dynamic a(UINT64_MAX);
    ++a;

    CHECK(a.length() == 2);
    CHECK(a > Dynamic(UINT64_MAX));
  }

  TEST_CASE("Multiple increments cause growth") {
    Dynamic a(UINT64_MAX - 5);
    for (int i = 0; i < 10; ++i) {
      ++a;
    }

    CHECK(a.length() == 2);
  }

  TEST_CASE("Growth then shrink through subtraction") {
    Dynamic a(UINT64_MAX);
    a += Dynamic(1); // Grow to 2 segments
    CHECK(a.length() == 2);

    a -= Dynamic(1); // Should trim back to 1 segment
    CHECK(a.length() == 1);
    CHECK(a == Dynamic(UINT64_MAX));
  }

  TEST_CASE("Division shrinks result") {
    Dynamic a(UINT64_MAX);
    a += Dynamic(1); // 2 segments
    CHECK(a.length() == 2);

    Dynamic b = a / Dynamic(2);
    CHECK(b.length() == 1); // Result should be trimmed
  }

  TEST_CASE("Right shift reduces size") {
    Dynamic a(1);
    a <<= 100; // Grow significantly
    CHECK(a.length() > 1);

    a >>= 100; // Shift back
    CHECK(a == Dynamic(1));
    CHECK(a.length() == 1);
  }
}

TEST_SUITE("Dynamic Integer - Large Value Operations") {
  TEST_CASE("Very large factorial") {
    Dynamic factorial(1);
    for (int i = 2; i <= 30; ++i) {
      factorial *= Dynamic(i);
    }

    CHECK(factorial > Dynamic(0));
    CHECK(factorial.length() > 1);
  }

  TEST_CASE("Fibonacci sequence with large values") {
    Dynamic a(0), b(1);
    for (int i = 0; i < 100; ++i) {
      Dynamic temp = a + b;
      a = b;
      b = temp;
    }

    CHECK(b > Dynamic(0));
    CHECK(b.length() > 1);
  }

  TEST_CASE("Power of 2 sequences") {
    Dynamic power(1);
    for (int i = 0; i < 100; ++i) {
      power *= Dynamic(2);
    }

    CHECK(power == (Dynamic(1) << 100));
    CHECK(power.length() == 2);
  }

  TEST_CASE("Large addition chain") {
    Dynamic sum(0);
    for (int i = 0; i < 1000; ++i) {
      sum += Dynamic(UINT64_MAX);
    }

    CHECK(sum > Dynamic(UINT64_MAX));
    CHECK(sum.length() >= 2);
  }

  TEST_CASE("Very large bit shift") {
    Dynamic a(1);
    Dynamic b = a << 200;

    CHECK(b.length() >= 4);
    CHECK(b != Dynamic(0));

    Dynamic c = b >> 200;
    CHECK(c == Dynamic(1));
  }

  TEST_CASE("Multi-segment addition") {
    Dynamic a = (Dynamic(1) << 128);
    Dynamic b = (Dynamic(1) << 64);
    Dynamic c = a + b;

    CHECK(c > a);
    CHECK(c > b);
  }

  TEST_CASE("Multi-segment subtraction") {
    Dynamic a = (Dynamic(1) << 128);
    Dynamic b = (Dynamic(1) << 64);
    Dynamic c = a - b;

    CHECK(c < a);
    CHECK(c > b);
  }

  TEST_CASE("Multi-segment multiplication") {
    Dynamic a = (Dynamic(1) << 64) + Dynamic(42);
    Dynamic b(100);
    Dynamic c = a * b;

    CHECK(c > a);
    CHECK(c.length() >= 2);
  }

  TEST_CASE("Division of large by small") {
    Dynamic large = (Dynamic(1) << 100);
    Dynamic small(1000);
    Dynamic quotient = large / small;

    CHECK(quotient > Dynamic(0));
    CHECK(quotient < large);
  }
}

TEST_SUITE("Dynamic Integer - Bitwise Operations") {
  TEST_CASE("Bitwise AND trims result") {
    Dynamic a = (Dynamic(1) << 100);
    Dynamic b(UINT64_MAX);
    Dynamic c = a & b;

    CHECK(c == Dynamic(0));
    CHECK(c.length() == 1);
  }

  TEST_CASE("Bitwise OR expands result") {
    Dynamic a(0xFF);
    Dynamic b = (Dynamic(1) << 100);
    Dynamic c = a | b;

    CHECK(c.length() >= 2);
    CHECK(c > a);
    CHECK(c > b);
  }

  TEST_CASE("Bitwise XOR") {
    Dynamic a(0b1100);
    Dynamic b(0b1010);
    Dynamic c = a ^ b;

    CHECK(c == Dynamic(0b0110));
  }

  TEST_CASE("XOR with different sizes") {
    Dynamic a = (Dynamic(1) << 100);
    Dynamic b(UINT64_MAX);
    Dynamic c = a ^ b;

    CHECK(c.length() >= 2);
  }

  TEST_CASE("Bitwise NOT") {
    Dynamic a(0);
    Dynamic b = ~a;

    // NOT of zero should be large
    CHECK(b != Dynamic(0));
  }

  TEST_CASE("Bit manipulation patterns on large values") {
    Dynamic value = (Dynamic(1) << 100);

    // Set a bit
    value |= (Dynamic(1) << 50);
    CHECK((value & (Dynamic(1) << 50)) != Dynamic(0));

    // Clear a bit
    value &= ~(Dynamic(1) << 50);
    CHECK((value & (Dynamic(1) << 50)) == Dynamic(0));
  }
}

TEST_SUITE("Dynamic Integer - String Conversion") {
  TEST_CASE("to_string with small values") {
    CHECK(ArbitraryPrecision::to_string(Dynamic(0)) == "0");
    CHECK(ArbitraryPrecision::to_string(Dynamic(42)) == "42");
    CHECK(ArbitraryPrecision::to_string(Dynamic(12345)) == "12345");
  }

  TEST_CASE("to_string with large values") {
    Dynamic large(UINT64_MAX);
    CHECK(ArbitraryPrecision::to_string(large) == "18446744073709551615");
  }

  TEST_CASE("to_string with multi-segment value") {
    Dynamic large = (Dynamic(1) << 64) + Dynamic(42);
    CHECK(ArbitraryPrecision::to_string(large) == "18446744073709551658");
  }

  TEST_CASE("to_string with very large value") {
    Dynamic large = (Dynamic(1) << 100);
    std::string result = ArbitraryPrecision::to_string(large);
    CHECK(result == "1267650600228229401496703205376");
  }

  TEST_CASE("from_string with valid input") {
    CHECK(ArbitraryPrecision::from_string<ArbitraryPrecision::Kind::Dynamic, 0>(
              "0")
              .value() == Dynamic(0));
    CHECK(ArbitraryPrecision::from_string<ArbitraryPrecision::Kind::Dynamic, 0>(
              "42")
              .value() == Dynamic(42));
    CHECK(ArbitraryPrecision::from_string<ArbitraryPrecision::Kind::Dynamic, 0>(
              "12345")
              .value() == Dynamic(12345));
  }

  TEST_CASE("from_string with large value") {
    CHECK(ArbitraryPrecision::from_string<ArbitraryPrecision::Kind::Dynamic, 0>(
              "18446744073709551615")
              .value() == Dynamic(UINT64_MAX));
  }

  TEST_CASE("to_string and from_string roundtrip") {
    Dynamic original(123456789);
    std::string str = ArbitraryPrecision::to_string(original);
    Dynamic parsed =
        ArbitraryPrecision::from_string<ArbitraryPrecision::Kind::Dynamic, 0>(
            str)
            .value();
    CHECK(parsed == original);
  }

  TEST_CASE("to_string and from_string roundtrip with large value") {
    Dynamic original = (Dynamic(1) << 100);
    std::string str = ArbitraryPrecision::to_string(original);
    Dynamic parsed =
        ArbitraryPrecision::from_string<ArbitraryPrecision::Kind::Dynamic, 0>(
            str)
            .value();
    CHECK(parsed == original);
  }

  TEST_CASE("to_string with computed factorial") {
    Dynamic factorial(1);
    for (int i = 2; i <= 20; ++i) {
      factorial *= Dynamic(i);
    }
    CHECK(ArbitraryPrecision::to_string(factorial) == "2432902008176640000");
  }
}

TEST_SUITE("Dynamic Integer - Edge Cases") {
  TEST_CASE("Zero minus one") {
    Dynamic zero(0);
    Dynamic result = zero - Dynamic(1);
    CHECK(result == Dynamic(-1));
  }

  TEST_CASE("Division by zero throws") {
    Dynamic a(42);
    Dynamic zero(0);
    CHECK_THROWS_AS(a / zero, std::domain_error);
  }

  TEST_CASE("Modulo by zero throws") {
    Dynamic a(42);
    Dynamic zero(0);
    CHECK_THROWS_AS(a % zero, std::domain_error);
  }

  TEST_CASE("Multiple operations maintain consistency") {
    Dynamic a(100);
    Dynamic b(7);

    Dynamic quotient = a / b;
    Dynamic remainder = a % b;

    CHECK(quotient * b + remainder == a);
  }

  TEST_CASE("Shift by zero") {
    Dynamic a(42);
    CHECK((a << 0) == a);
    CHECK((a >> 0) == a);
  }

  TEST_CASE("Large shift then reverse") {
    Dynamic a(12345);
    for (int shift = 1; shift < 100; shift += 10) {
      Dynamic shifted = (a << shift) >> shift;
      CHECK(shifted == a);
    }
  }

  TEST_CASE("Boolean conversion") {
    CHECK_FALSE(static_cast<bool>(Dynamic(0)));
    CHECK(static_cast<bool>(Dynamic(1)));
    CHECK(static_cast<bool>(Dynamic(-1)));
    CHECK(static_cast<bool>(Dynamic(1) << 100));
  }

  TEST_CASE("tail() returns lowest 64 bits") {
    Dynamic a(12345);
    CHECK(a.tail() == 12345);

    Dynamic b = (Dynamic(1) << 100) + Dynamic(42);
    CHECK(b.tail() == 42);
  }
}

TEST_SUITE("Dynamic Integer - Stress Tests") {
  TEST_CASE("Many sequential additions") {
    Dynamic sum(0);
    for (int i = 0; i < 10000; ++i) {
      sum += Dynamic(i);
    }
    CHECK(sum == Dynamic(49995000));
  }

  TEST_CASE("Repeated doubling") {
    Dynamic value(1);
    for (int i = 0; i < 200; ++i) {
      value *= Dynamic(2);
    }
    CHECK(value == (Dynamic(1) << 200));
    CHECK(value.length() >= 4);
  }

  TEST_CASE("GCD with large numbers") {
    // Use simpler test case that we can verify
    Dynamic a(48);
    Dynamic b(18);

    // Euclidean algorithm
    while (static_cast<bool>(b)) {
      Dynamic temp = b;
      b = a % b;
      a = temp;
    }

    CHECK(a == Dynamic(6));
  }

  TEST_CASE("Alternating operations") {
    Dynamic value(1000);
    for (int i = 0; i < 1000; ++i) {
      value += Dynamic(100);
      value -= Dynamic(50);
      value *= Dynamic(2);
      value /= Dynamic(2);
    }
    CHECK(value > Dynamic(1000));
  }

  TEST_CASE("Complex arithmetic expression") {
    Dynamic a(12345);
    Dynamic b(67890);
    Dynamic c(111);

    Dynamic result = ((a + b) * c - (a * b / c)) % Dynamic(1000000);
    CHECK(result > Dynamic(0));
  }
}

TEST_SUITE("Dynamic Integer - Comparison with Fixed") {
  TEST_CASE("Dynamic and Fixed produce same results for small values") {
    Dynamic dyn_a(12345);
    Dynamic dyn_b(67890);

    Int128 fix_a(12345);
    Int128 fix_b(67890);

    // Addition
    CHECK(ArbitraryPrecision::to_string(dyn_a + dyn_b) ==
          ArbitraryPrecision::to_string(fix_a + fix_b));

    // Multiplication
    CHECK(ArbitraryPrecision::to_string(dyn_a * dyn_b) ==
          ArbitraryPrecision::to_string(fix_a * fix_b));

    // Division
    CHECK(ArbitraryPrecision::to_string(dyn_b / dyn_a) ==
          ArbitraryPrecision::to_string(fix_b / fix_a));
  }

  TEST_CASE("Dynamic can exceed Fixed size") {
    Dynamic dyn(1);
    dyn <<= 200; // Shift beyond 128 bits

    CHECK(dyn.length() >= 4);
    CHECK(dyn != Dynamic(0));

    // A 128-bit Fixed would wrap, but Dynamic grows
    CHECK(ArbitraryPrecision::to_string(dyn).length() > 38);
  }
}
