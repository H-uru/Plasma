#include "plFormat.h"
#include "plFileSystem.h"

#include <gtest/gtest.h>
#include <string>
#include <limits>

TEST(plFormat, Escapes)
{
    EXPECT_EQ(plString("{x"), plFormat("{{{}", "x"));
    EXPECT_EQ(plString("x{"), plFormat("{}{{", "x"));
    EXPECT_EQ(plString("{{{{"), plFormat("{{{}{{{{", "{"));
    EXPECT_EQ(plString("{xxx{{yyy{"), plFormat("{{{}{{{{{}{{", "xxx", "yyy"));
}

TEST(plFormat, Errors)
{
    // Ensure these get printed to the console
    ErrorEnableGui(false);

    EXPECT_DEATH(plFormat("{}", 1, 2),
        "Message: Too many actual parameters for format string");
    EXPECT_DEATH(plFormat("{} {}", 1),
        "Message: Not enough actual parameters for format string");
    EXPECT_DEATH(plFormat("{", 1),
        "Message: Unterminated format specifier");
    EXPECT_DEATH(plFormat("{.", 1),
        "Message: Unterminated format specifier");
    EXPECT_DEATH(plFormat("{_", 1),
        "Message: Unterminated format specifier");
    EXPECT_DEATH(plFormat(nullptr, 1),
        "Message: Passed a null format string");
}

TEST(plFormat, Strings)
{
    EXPECT_EQ(plString("TEST"), plFormat("{}", "TEST"));
    EXPECT_EQ(plString("xxTESTxx"), plFormat("xx{}xx", "TEST"));
    EXPECT_EQ(plString("xxTESTxx"), plFormat("xx{2}xx", "TEST"));
    EXPECT_EQ(plString("xxTESTxx"), plFormat("xx{>2}xx", "TEST"));
    EXPECT_EQ(plString("xxTESTxx"), plFormat("xx{<2}xx", "TEST"));
    EXPECT_EQ(plString("xxTESTxx"), plFormat("xx{_-2}xx", "TEST"));
    EXPECT_EQ(plString("xxTEST  xx"), plFormat("xx{6}xx", "TEST"));
    EXPECT_EQ(plString("xxTEST  xx"), plFormat("xx{<6}xx", "TEST"));
    EXPECT_EQ(plString("xx  TESTxx"), plFormat("xx{>6}xx", "TEST"));
    EXPECT_EQ(plString("xxTEST--xx"), plFormat("xx{_-6}xx", "TEST"));
    EXPECT_EQ(plString("xxTEST--xx"), plFormat("xx{<_-6}xx", "TEST"));
    EXPECT_EQ(plString("xx--TESTxx"), plFormat("xx{>_-6}xx", "TEST"));
    EXPECT_EQ(plString("xxONE  TWO    THREExx"), plFormat("xx{5}{<5}{>7}xx", "ONE", "TWO", "THREE"));

    // Ensure braces are parsed properly within padding chars
    EXPECT_EQ(plString("xxTEST}}xx"), plFormat("xx{_}6}xx", "TEST"));
    EXPECT_EQ(plString("xxTEST}}xx"), plFormat("xx{6_}}xx", "TEST"));
    EXPECT_EQ(plString("xxTEST{{xx"), plFormat("xx{_{6}xx", "TEST"));
    EXPECT_EQ(plString("xxTEST{{xx"), plFormat("xx{6_{}xx", "TEST"));
}

TEST(plFormat, StringClasses)
{
    // These should be handled just like normal const char* string params
    // (see above), so just need to test that the wrappers are working
    EXPECT_EQ(plString("xxTESTxx"), plFormat("xx{}xx", L"TEST"));
    EXPECT_EQ(plString("xxTESTxx"), plFormat("xx{}xx", plString("TEST")));
    EXPECT_EQ(plString("xxTESTxx"), plFormat("xx{}xx", std::string("TEST")));
    EXPECT_EQ(plString("xxTESTxx"), plFormat("xx{}xx", std::wstring(L"TEST")));
    EXPECT_EQ(plString("xx/dev/nullxx"), plFormat("xx{}xx", plFileName("/dev/null")));
    EXPECT_EQ(plString(R"(xxC:\Users\Nobodyxx)"),
              plFormat("xx{}xx", plFileName(R"(C:\Users\Nobody)")));
}

TEST(plFormat, Char)
{
    EXPECT_EQ(plString("xxAxx"), plFormat("xx{}xx", 'A'));
    EXPECT_EQ(plString("xxAxx"), plFormat("xx{c}xx", (signed char)'A'));
    EXPECT_EQ(plString("xxAxx"), plFormat("xx{c}xx", (unsigned char)'A'));

    // UTF-8 encoding of wide (16-bit) char
    EXPECT_EQ(plString("xx\xef\xbf\xbexx"), plFormat("xx{}xx", L'\ufffe'));
    EXPECT_EQ(plString("xx\xe7\xbf\xbexx"), plFormat("xx{c}xx", (short)0x7ffe));
    EXPECT_EQ(plString("xx\xef\xbf\xbexx"), plFormat("xx{c}xx", (unsigned short)0xfffe));

    // UTF-8 encoding of UCS4 (32-bit) char
    EXPECT_EQ(plString("xx\xf4\x8f\xbf\xbfxx"), plFormat("xx{c}xx", (int)0x10ffff));
    EXPECT_EQ(plString("xx\xf4\x8f\xbf\xbfxx"), plFormat("xx{c}xx", (unsigned int)0x10ffff));
    EXPECT_EQ(plString("xx\xf4\x8f\xbf\xbfxx"), plFormat("xx{c}xx", (long)0x10ffff));
    EXPECT_EQ(plString("xx\xf4\x8f\xbf\xbfxx"), plFormat("xx{c}xx", (unsigned long)0x10ffff));
    EXPECT_EQ(plString("xx\xf4\x8f\xbf\xbfxx"), plFormat("xx{c}xx", (int64_t)0x10ffff));
    EXPECT_EQ(plString("xx\xf4\x8f\xbf\xbfxx"), plFormat("xx{c}xx", (uint64_t)0x10ffff));
}

TEST(plFormat, Decimal)
{
    EXPECT_EQ(plString("xx1234xx"), plFormat("xx{}xx", 1234));
    EXPECT_EQ(plString("xx1234xx"), plFormat("xx{d}xx", 1234));
    EXPECT_EQ(plString("xx1234xx"), plFormat("xx{2}xx", 1234));
    EXPECT_EQ(plString("xx1234xx"), plFormat("xx{>2}xx", 1234));
    EXPECT_EQ(plString("xx1234xx"), plFormat("xx{<2}xx", 1234));
    EXPECT_EQ(plString("xx  1234xx"), plFormat("xx{6}xx", 1234));
    EXPECT_EQ(plString("xx  1234xx"), plFormat("xx{>6}xx", 1234));
    EXPECT_EQ(plString("xx1234  xx"), plFormat("xx{<6}xx", 1234));

    // Character types
    EXPECT_EQ(plString("xx0xx"), plFormat("xx{d}xx", '\0'));
    EXPECT_EQ(plString("xx65xx"), plFormat("xx{d}xx", 'A'));
    EXPECT_EQ(plString("xx0xx"), plFormat("xx{d}xx", L'\0'));
    EXPECT_EQ(plString("xx65xx"), plFormat("xx{d}xx", L'A'));
    EXPECT_EQ(plString("xx32767xx"), plFormat("xx{d}xx", L'\u7fff'));

    // Numeric char types
    EXPECT_EQ(plString("xx0xx"), plFormat("xx{}xx", (signed char)0));
    EXPECT_EQ(plString("xx127xx"), plFormat("xx{}xx", std::numeric_limits<signed char>::max()));
    EXPECT_EQ(plString("xx+127xx"), plFormat("xx{+}xx", std::numeric_limits<signed char>::max()));
    EXPECT_EQ(plString("xx-128xx"), plFormat("xx{}xx", std::numeric_limits<signed char>::min()));
    EXPECT_EQ(plString("xx-128xx"), plFormat("xx{+}xx", std::numeric_limits<signed char>::min()));
    EXPECT_EQ(plString("xx0xx"), plFormat("xx{}xx", (unsigned char)0));
    EXPECT_EQ(plString("xx255xx"), plFormat("xx{}xx", std::numeric_limits<unsigned char>::max()));
    EXPECT_EQ(plString("xx+255xx"), plFormat("xx{+}xx", std::numeric_limits<unsigned char>::max()));

    // 16-bit ints
    EXPECT_EQ(plString("xx0xx"), plFormat("xx{}xx", (short)0));
    EXPECT_EQ(plString("xx32767xx"), plFormat("xx{}xx", std::numeric_limits<short>::max()));
    EXPECT_EQ(plString("xx+32767xx"), plFormat("xx{+}xx", std::numeric_limits<short>::max()));
    EXPECT_EQ(plString("xx-32768xx"), plFormat("xx{}xx", std::numeric_limits<short>::min()));
    EXPECT_EQ(plString("xx-32768xx"), plFormat("xx{+}xx", std::numeric_limits<short>::min()));
    EXPECT_EQ(plString("xx0xx"), plFormat("xx{}xx", (unsigned short)0));
    EXPECT_EQ(plString("xx65535xx"), plFormat("xx{}xx", std::numeric_limits<unsigned short>::max()));
    EXPECT_EQ(plString("xx+65535xx"), plFormat("xx{+}xx", std::numeric_limits<unsigned short>::max()));

    // 32-bit ints
    EXPECT_EQ(plString("xx0xx"), plFormat("xx{}xx", (int)0));
    EXPECT_EQ(plString("xx2147483647xx"), plFormat("xx{}xx", std::numeric_limits<int>::max()));
    EXPECT_EQ(plString("xx+2147483647xx"), plFormat("xx{+}xx", std::numeric_limits<int>::max()));
    EXPECT_EQ(plString("xx-2147483648xx"), plFormat("xx{}xx", std::numeric_limits<int>::min()));
    EXPECT_EQ(plString("xx-2147483648xx"), plFormat("xx{+}xx", std::numeric_limits<int>::min()));
    EXPECT_EQ(plString("xx0xx"), plFormat("xx{}xx", (unsigned int)0));
    EXPECT_EQ(plString("xx4294967295xx"), plFormat("xx{}xx", std::numeric_limits<unsigned int>::max()));
    EXPECT_EQ(plString("xx+4294967295xx"), plFormat("xx{+}xx", std::numeric_limits<unsigned int>::max()));

    // 64-bit ints
    EXPECT_EQ(plString("xx0xx"), plFormat("xx{}xx", (int64_t)0));
    EXPECT_EQ(plString("xx9223372036854775807xx"), plFormat("xx{}xx", std::numeric_limits<int64_t>::max()));
    EXPECT_EQ(plString("xx+9223372036854775807xx"), plFormat("xx{+}xx", std::numeric_limits<int64_t>::max()));
    EXPECT_EQ(plString("xx-9223372036854775808xx"), plFormat("xx{}xx", std::numeric_limits<int64_t>::min()));
    EXPECT_EQ(plString("xx-9223372036854775808xx"), plFormat("xx{+}xx", std::numeric_limits<int64_t>::min()));
    EXPECT_EQ(plString("xx0xx"), plFormat("xx{}xx", (uint64_t)0));
    EXPECT_EQ(plString("xx18446744073709551615xx"), plFormat("xx{}xx", std::numeric_limits<uint64_t>::max()));
    EXPECT_EQ(plString("xx+18446744073709551615xx"), plFormat("xx{+}xx", std::numeric_limits<uint64_t>::max()));
}

TEST(plFormat, Hex)
{
    EXPECT_EQ(plString("xx4d2xx"), plFormat("xx{x}xx", 1234));
    EXPECT_EQ(plString("xx4d2xx"), plFormat("xx{x}xx", 1234));
    EXPECT_EQ(plString("xx4d2xx"), plFormat("xx{2x}xx", 1234));
    EXPECT_EQ(plString("xx4d2xx"), plFormat("xx{>2x}xx", 1234));
    EXPECT_EQ(plString("xx4d2xx"), plFormat("xx{<2x}xx", 1234));
    EXPECT_EQ(plString("xx   4d2xx"), plFormat("xx{6x}xx", 1234));
    EXPECT_EQ(plString("xx   4d2xx"), plFormat("xx{>6x}xx", 1234));
    EXPECT_EQ(plString("xx4d2   xx"), plFormat("xx{<6x}xx", 1234));

    // Character types
    EXPECT_EQ(plString("xx0xx"), plFormat("xx{x}xx", '\0'));
    EXPECT_EQ(plString("xx41xx"), plFormat("xx{x}xx", 'A'));
    EXPECT_EQ(plString("xx0xx"), plFormat("xx{x}xx", L'\0'));
    EXPECT_EQ(plString("xx41xx"), plFormat("xx{x}xx", L'A'));
    EXPECT_EQ(plString("xx7fffxx"), plFormat("xx{x}xx", L'\u7fff'));

    // Numeric char types
    EXPECT_EQ(plString("xx0xx"), plFormat("xx{x}xx", (signed char)0));
    EXPECT_EQ(plString("xx7fxx"), plFormat("xx{x}xx", std::numeric_limits<signed char>::max()));
    EXPECT_EQ(plString("xx80xx"), plFormat("xx{x}xx", std::numeric_limits<signed char>::min()));
    EXPECT_EQ(plString("xx0xx"), plFormat("xx{x}xx", (unsigned char)0));
    EXPECT_EQ(plString("xxffxx"), plFormat("xx{x}xx", std::numeric_limits<unsigned char>::max()));

    // 16-bit ints
    EXPECT_EQ(plString("xx0xx"), plFormat("xx{x}xx", (short)0));
    EXPECT_EQ(plString("xx7fffxx"), plFormat("xx{x}xx", std::numeric_limits<short>::max()));
    EXPECT_EQ(plString("xx8000xx"), plFormat("xx{x}xx", std::numeric_limits<short>::min()));
    EXPECT_EQ(plString("xx0xx"), plFormat("xx{x}xx", (unsigned short)0));
    EXPECT_EQ(plString("xxffffxx"), plFormat("xx{x}xx", std::numeric_limits<unsigned short>::max()));

    // 32-bit ints
    EXPECT_EQ(plString("xx0xx"), plFormat("xx{x}xx", (int)0));
    EXPECT_EQ(plString("xx7fffffffxx"), plFormat("xx{x}xx", std::numeric_limits<int>::max()));
    EXPECT_EQ(plString("xx80000000xx"), plFormat("xx{x}xx", std::numeric_limits<int>::min()));
    EXPECT_EQ(plString("xx0xx"), plFormat("xx{x}xx", (unsigned int)0));
    EXPECT_EQ(plString("xxffffffffxx"), plFormat("xx{x}xx", std::numeric_limits<unsigned int>::max()));

    // 64-bit ints
    EXPECT_EQ(plString("xx0xx"), plFormat("xx{x}xx", (int64_t)0));
    EXPECT_EQ(plString("xx7fffffffffffffffxx"), plFormat("xx{x}xx", std::numeric_limits<int64_t>::max()));
    EXPECT_EQ(plString("xx8000000000000000xx"), plFormat("xx{x}xx", std::numeric_limits<int64_t>::min()));
    EXPECT_EQ(plString("xx0xx"), plFormat("xx{x}xx", (uint64_t)0));
    EXPECT_EQ(plString("xxffffffffffffffffxx"), plFormat("xx{x}xx", std::numeric_limits<uint64_t>::max()));
}

TEST(plFormat, HexUpper)
{
    EXPECT_EQ(plString("xx4D2xx"), plFormat("xx{X}xx", 1234));
    EXPECT_EQ(plString("xx4D2xx"), plFormat("xx{X}xx", 1234));
    EXPECT_EQ(plString("xx4D2xx"), plFormat("xx{2X}xx", 1234));
    EXPECT_EQ(plString("xx4D2xx"), plFormat("xx{>2X}xx", 1234));
    EXPECT_EQ(plString("xx4D2xx"), plFormat("xx{<2X}xx", 1234));
    EXPECT_EQ(plString("xx   4D2xx"), plFormat("xx{6X}xx", 1234));
    EXPECT_EQ(plString("xx   4D2xx"), plFormat("xx{>6X}xx", 1234));
    EXPECT_EQ(plString("xx4D2   xx"), plFormat("xx{<6X}xx", 1234));

    // Character types
    EXPECT_EQ(plString("xx0xx"), plFormat("xx{X}xx", '\0'));
    EXPECT_EQ(plString("xx41xx"), plFormat("xx{X}xx", 'A'));
    EXPECT_EQ(plString("xx0xx"), plFormat("xx{X}xx", L'\0'));
    EXPECT_EQ(plString("xx41xx"), plFormat("xx{X}xx", L'A'));
    EXPECT_EQ(plString("xx7FFFxx"), plFormat("xx{X}xx", L'\u7fff'));

    // Numeric char types
    EXPECT_EQ(plString("xx0xx"), plFormat("xx{X}xx", (signed char)0));
    EXPECT_EQ(plString("xx7Fxx"), plFormat("xx{X}xx", std::numeric_limits<signed char>::max()));
    EXPECT_EQ(plString("xx80xx"), plFormat("xx{X}xx", std::numeric_limits<signed char>::min()));
    EXPECT_EQ(plString("xx0xx"), plFormat("xx{X}xx", (unsigned char)0));
    EXPECT_EQ(plString("xxFFxx"), plFormat("xx{X}xx", std::numeric_limits<unsigned char>::max()));

    // 16-bit ints
    EXPECT_EQ(plString("xx0xx"), plFormat("xx{X}xx", (short)0));
    EXPECT_EQ(plString("xx7FFFxx"), plFormat("xx{X}xx", std::numeric_limits<short>::max()));
    EXPECT_EQ(plString("xx8000xx"), plFormat("xx{X}xx", std::numeric_limits<short>::min()));
    EXPECT_EQ(plString("xx0xx"), plFormat("xx{X}xx", (unsigned short)0));
    EXPECT_EQ(plString("xxFFFFxx"), plFormat("xx{X}xx", std::numeric_limits<unsigned short>::max()));

    // 32-bit ints
    EXPECT_EQ(plString("xx0xx"), plFormat("xx{X}xx", (int)0));
    EXPECT_EQ(plString("xx7FFFFFFFxx"), plFormat("xx{X}xx", std::numeric_limits<int>::max()));
    EXPECT_EQ(plString("xx80000000xx"), plFormat("xx{X}xx", std::numeric_limits<int>::min()));
    EXPECT_EQ(plString("xx0xx"), plFormat("xx{X}xx", (unsigned int)0));
    EXPECT_EQ(plString("xxFFFFFFFFxx"), plFormat("xx{X}xx", std::numeric_limits<unsigned int>::max()));

    // 64-bit ints
    EXPECT_EQ(plString("xx0xx"), plFormat("xx{X}xx", (int64_t)0));
    EXPECT_EQ(plString("xx7FFFFFFFFFFFFFFFxx"), plFormat("xx{X}xx", std::numeric_limits<int64_t>::max()));
    EXPECT_EQ(plString("xx8000000000000000xx"), plFormat("xx{X}xx", std::numeric_limits<int64_t>::min()));
    EXPECT_EQ(plString("xx0xx"), plFormat("xx{X}xx", (uint64_t)0));
    EXPECT_EQ(plString("xxFFFFFFFFFFFFFFFFxx"), plFormat("xx{X}xx", std::numeric_limits<uint64_t>::max()));
}

TEST(plFormat, Octal)
{
    EXPECT_EQ(plString("xx2322xx"), plFormat("xx{o}xx", 1234));
    EXPECT_EQ(plString("xx2322xx"), plFormat("xx{o}xx", 1234));
    EXPECT_EQ(plString("xx2322xx"), plFormat("xx{2o}xx", 1234));
    EXPECT_EQ(plString("xx2322xx"), plFormat("xx{>2o}xx", 1234));
    EXPECT_EQ(plString("xx2322xx"), plFormat("xx{<2o}xx", 1234));
    EXPECT_EQ(plString("xx  2322xx"), plFormat("xx{6o}xx", 1234));
    EXPECT_EQ(plString("xx  2322xx"), plFormat("xx{>6o}xx", 1234));
    EXPECT_EQ(plString("xx2322  xx"), plFormat("xx{<6o}xx", 1234));

    // Character types
    EXPECT_EQ(plString("xx0xx"), plFormat("xx{o}xx", '\0'));
    EXPECT_EQ(plString("xx101xx"), plFormat("xx{o}xx", 'A'));
    EXPECT_EQ(plString("xx0xx"), plFormat("xx{o}xx", L'\0'));
    EXPECT_EQ(plString("xx101xx"), plFormat("xx{o}xx", L'A'));
    EXPECT_EQ(plString("xx77777xx"), plFormat("xx{o}xx", L'\u7fff'));

    // Numeric char types
    EXPECT_EQ(plString("xx0xx"), plFormat("xx{o}xx", (signed char)0));
    EXPECT_EQ(plString("xx177xx"), plFormat("xx{o}xx", std::numeric_limits<signed char>::max()));
    EXPECT_EQ(plString("xx200xx"), plFormat("xx{o}xx", std::numeric_limits<signed char>::min()));
    EXPECT_EQ(plString("xx0xx"), plFormat("xx{o}xx", (unsigned char)0));
    EXPECT_EQ(plString("xx377xx"), plFormat("xx{o}xx", std::numeric_limits<unsigned char>::max()));

    // 16-bit ints
    EXPECT_EQ(plString("xx0xx"), plFormat("xx{o}xx", (short)0));
    EXPECT_EQ(plString("xx77777xx"), plFormat("xx{o}xx", std::numeric_limits<short>::max()));
    EXPECT_EQ(plString("xx100000xx"), plFormat("xx{o}xx", std::numeric_limits<short>::min()));
    EXPECT_EQ(plString("xx0xx"), plFormat("xx{o}xx", (unsigned short)0));
    EXPECT_EQ(plString("xx177777xx"), plFormat("xx{o}xx", std::numeric_limits<unsigned short>::max()));

    // 32-bit ints
    EXPECT_EQ(plString("xx0xx"), plFormat("xx{o}xx", (int)0));
    EXPECT_EQ(plString("xx17777777777xx"), plFormat("xx{o}xx", std::numeric_limits<int>::max()));
    EXPECT_EQ(plString("xx20000000000xx"), plFormat("xx{o}xx", std::numeric_limits<int>::min()));
    EXPECT_EQ(plString("xx0xx"), plFormat("xx{o}xx", (unsigned int)0));
    EXPECT_EQ(plString("xx37777777777xx"), plFormat("xx{o}xx", std::numeric_limits<unsigned int>::max()));

    // 64-bit ints
    EXPECT_EQ(plString("xx0xx"), plFormat("xx{o}xx", (int64_t)0));
    EXPECT_EQ(plString("xx777777777777777777777xx"), plFormat("xx{o}xx", std::numeric_limits<int64_t>::max()));
    EXPECT_EQ(plString("xx1000000000000000000000xx"), plFormat("xx{o}xx", std::numeric_limits<int64_t>::min()));
    EXPECT_EQ(plString("xx0xx"), plFormat("xx{o}xx", (uint64_t)0));
    EXPECT_EQ(plString("xx1777777777777777777777xx"), plFormat("xx{o}xx", std::numeric_limits<uint64_t>::max()));
}

TEST(plFormat, Binary)
{
    EXPECT_EQ(plString("xx10011010010xx"), plFormat("xx{b}xx", 1234));
    EXPECT_EQ(plString("xx10011010010xx"), plFormat("xx{b}xx", 1234));
    EXPECT_EQ(plString("xx10011010010xx"), plFormat("xx{2b}xx", 1234));
    EXPECT_EQ(plString("xx10011010010xx"), plFormat("xx{>2b}xx", 1234));
    EXPECT_EQ(plString("xx10011010010xx"), plFormat("xx{<2b}xx", 1234));
    EXPECT_EQ(plString("xx     10011010010xx"), plFormat("xx{16b}xx", 1234));
    EXPECT_EQ(plString("xx     10011010010xx"), plFormat("xx{>16b}xx", 1234));
    EXPECT_EQ(plString("xx10011010010     xx"), plFormat("xx{<16b}xx", 1234));

    // Character types
    EXPECT_EQ(plString("xx0xx"), plFormat("xx{b}xx", '\0'));
    EXPECT_EQ(plString("xx1000001xx"), plFormat("xx{b}xx", 'A'));
    EXPECT_EQ(plString("xx0xx"), plFormat("xx{b}xx", L'\0'));
    EXPECT_EQ(plString("xx1000001xx"), plFormat("xx{b}xx", L'A'));
    EXPECT_EQ(plString("xx111111111111111xx"), plFormat("xx{b}xx", L'\u7fff'));

    // Numeric char types
    EXPECT_EQ(plString("xx0xx"), plFormat("xx{b}xx", (signed char)0));
    EXPECT_EQ(plString("xx1111111xx"), plFormat("xx{b}xx", std::numeric_limits<signed char>::max()));
    EXPECT_EQ(plString("xx10000000xx"), plFormat("xx{b}xx", std::numeric_limits<signed char>::min()));
    EXPECT_EQ(plString("xx0xx"), plFormat("xx{b}xx", (unsigned char)0));
    EXPECT_EQ(plString("xx11111111xx"), plFormat("xx{b}xx", std::numeric_limits<unsigned char>::max()));

    // 16-bit ints
    EXPECT_EQ(plString("xx0xx"), plFormat("xx{b}xx", (short)0));
    EXPECT_EQ(plString("xx111111111111111xx"), plFormat("xx{b}xx", std::numeric_limits<short>::max()));
    EXPECT_EQ(plString("xx1000000000000000xx"), plFormat("xx{b}xx", std::numeric_limits<short>::min()));
    EXPECT_EQ(plString("xx0xx"), plFormat("xx{b}xx", (unsigned short)0));
    EXPECT_EQ(plString("xx1111111111111111xx"), plFormat("xx{b}xx", std::numeric_limits<unsigned short>::max()));

    // 32-bit ints
    EXPECT_EQ(plString("xx0xx"), plFormat("xx{b}xx", (int)0));
    EXPECT_EQ(plString("xx1111111111111111111111111111111xx"),
              plFormat("xx{b}xx", std::numeric_limits<int>::max()));
    EXPECT_EQ(plString("xx10000000000000000000000000000000xx"),
              plFormat("xx{b}xx", std::numeric_limits<int>::min()));
    EXPECT_EQ(plString("xx0xx"), plFormat("xx{b}xx", (unsigned int)0));
    EXPECT_EQ(plString("xx11111111111111111111111111111111xx"),
              plFormat("xx{b}xx", std::numeric_limits<unsigned int>::max()));

    // 64-bit ints
    EXPECT_EQ(plString("xx0xx"), plFormat("xx{b}xx", (int64_t)0));
    EXPECT_EQ(plString("xx111111111111111111111111111111111111111111111111111111111111111xx"),
              plFormat("xx{b}xx", std::numeric_limits<int64_t>::max()));
    EXPECT_EQ(plString("xx1000000000000000000000000000000000000000000000000000000000000000xx"),
              plFormat("xx{b}xx", std::numeric_limits<int64_t>::min()));
    EXPECT_EQ(plString("xx0xx"), plFormat("xx{b}xx", (uint64_t)0));
    EXPECT_EQ(plString("xx1111111111111111111111111111111111111111111111111111111111111111xx"),
              plFormat("xx{b}xx", std::numeric_limits<uint64_t>::max()));
}

TEST(plFormat, FloatingPoint)
{
    // The actual formatting is handled by libc, so we just need to test
    // that the flags get passed along properly.

    EXPECT_EQ(plString("xx1.5xx"), plFormat("xx{}xx", 1.5));
    EXPECT_EQ(plString("xx+1.5xx"), plFormat("xx{+}xx", 1.5));
    EXPECT_EQ(plString("xx-1.5xx"), plFormat("xx{}xx", -1.5));
    EXPECT_EQ(plString("xx-1.5xx"), plFormat("xx{+}xx", -1.5));

    // Padding
    EXPECT_EQ(plString("xx  1.50xx"), plFormat("xx{6.2f}xx", 1.5));
    EXPECT_EQ(plString("xx -1.50xx"), plFormat("xx{6.2f}xx", -1.5));
    EXPECT_EQ(plString("xx1.50  xx"), plFormat("xx{<6.2f}xx", 1.5));
    EXPECT_EQ(plString("xx-1.50 xx"), plFormat("xx{<6.2f}xx", -1.5));

    // Fixed notation
    EXPECT_EQ(plString("xx3.14xx"), plFormat("xx{.2f}xx", 3.14159));
    EXPECT_EQ(plString("xx3.141590xx"), plFormat("xx{.6f}xx", 3.14159));
    EXPECT_EQ(plString("xx16384.00xx"), plFormat("xx{.2f}xx", 16384.0));
    EXPECT_EQ(plString("xx0.01xx"), plFormat("xx{.2f}xx", 1.0 / 128));

    // MSVC uses 3 digits for the exponent, whereas GCC uses two :/
#ifdef _MSC_VER
#   define EXTRA_DIGIT "0"
#else
#   define EXTRA_DIGIT ""
#endif

    // Scientific notation (MSVC uses 3 digits for the exponent)
    EXPECT_EQ(plString("xx3.14e+" EXTRA_DIGIT "00xx"), plFormat("xx{.2e}xx", 3.14159));
    EXPECT_EQ(plString("xx3.141590e+" EXTRA_DIGIT "00xx"), plFormat("xx{.6e}xx", 3.14159));
    EXPECT_EQ(plString("xx1.64e+" EXTRA_DIGIT "04xx"), plFormat("xx{.2e}xx", 16384.0));
    EXPECT_EQ(plString("xx7.81e-" EXTRA_DIGIT "03xx"), plFormat("xx{.2e}xx", 1.0 / 128));

    // Scientific notation (upper-case E)
    EXPECT_EQ(plString("xx3.14E+" EXTRA_DIGIT "00xx"), plFormat("xx{.2E}xx", 3.14159));
    EXPECT_EQ(plString("xx3.141590E+" EXTRA_DIGIT "00xx"), plFormat("xx{.6E}xx", 3.14159));
    EXPECT_EQ(plString("xx1.64E+" EXTRA_DIGIT "04xx"), plFormat("xx{.2E}xx", 16384.0));
    EXPECT_EQ(plString("xx7.81E-" EXTRA_DIGIT "03xx"), plFormat("xx{.2E}xx", 1.0 / 128));

    // Automatic (based on input)
    EXPECT_EQ(plString("xx3.1xx"), plFormat("xx{.2}xx", 3.14159));
    EXPECT_EQ(plString("xx3.14159xx"), plFormat("xx{.6}xx", 3.14159));
    EXPECT_EQ(plString("xx1.6e+" EXTRA_DIGIT "04xx"), plFormat("xx{.2}xx", 16384.0));
    EXPECT_EQ(plString("xx0.0078xx"), plFormat("xx{.2}xx", 1.0 / 128));
}

TEST(plFormat, Booleans)
{
    // This basically just uses the string formatter with constant strings
    EXPECT_EQ(plString("xxtrue xx"), plFormat("xx{5}xx", true));
    EXPECT_EQ(plString("xxfalsexx"), plFormat("xx{5}xx", false));
}
