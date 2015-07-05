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
    EXPECT_EQ(plString("xxтəßtxx"), plFormat("xx{}xx", L"тəßt"));
    EXPECT_EQ(plString("xxтəßtxx"), plFormat("xx{}xx", plString("тəßt")));
    EXPECT_EQ(plString("xxTESTxx"), plFormat("xx{}xx", std::string("TEST")));
    EXPECT_EQ(plString("xxтəßtxx"), plFormat("xx{}xx", std::wstring(L"тəßt")));
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
