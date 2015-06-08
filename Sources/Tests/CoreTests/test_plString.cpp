#include <plString.h>
#include <HeadSpin.h>

#include <gtest/gtest.h>
#include <wchar.h>

static const plUniChar test_data[] = {
    0x20,       0x7f,       /* Normal ASCII chars */
    0xff,       0x100,      /* 8-bit boundary chars */
    0x7fff,                 /* UTF-8 2-byte boundary */
    0xffff,     0x10000,    /* 16-bit boundary chars */
    0x10020,    0x40000,    /* Non-edge UTF-16 surrogate pairs */
    0x10ffff,               /* Highest Unicode character */
    0                       /* Null terminator */
};

/* UTF-8 version of above test data */
static const char utf8_test_data[] =
    "\x20"              "\x7f"
    "\xc3\xbf"          "\xc4\x80"
    "\xe7\xbf\xbf"
    "\xef\xbf\xbf"      "\xf0\x90\x80\x80"
    "\xf0\x90\x80\xa0"  "\xf1\x80\x80\x80"
    "\xf4\x8f\xbf\xbf";
static const size_t utf8_test_data_length = arrsize(utf8_test_data) - 1;

/* UTF-16 version of test data */
static const uint16_t utf16_test_data[] = {
    0x20, 0x7f,
    0xff, 0x100,
    0x7fff,
    0xffff,
    /* surrogate pairs for chars >0xffff */
    0xd800, 0xdc00,
    0xd800, 0xdc20,
    0xd8c0, 0xdc00,
    0xdbff, 0xdfff,
    0
};

/* Utility for comparing plUniChar buffers */
template <typename _Ch>
static int T_strcmp(const _Ch *left, const _Ch *right)
{
    for ( ;; ) {
        if (*left != *right)
            return *left - *right;
        if (*left == 0)
            return (*right == 0) ? 0 : -1;
        if (*right == 0)
            return 1;

        ++left;
        ++right;
    }
}

TEST(PlStringTest, TestHelpers)
{
    /* Ensure the utilities for testing the module function properly */
    EXPECT_EQ(0, T_strcmp("abc", "abc"));
    EXPECT_LT(0, T_strcmp("abc", "aba"));
    EXPECT_GT(0, T_strcmp("abc", "abe"));
    EXPECT_LT(0, T_strcmp("abc", "ab"));
    EXPECT_GT(0, T_strcmp("abc", "abcd"));
    EXPECT_EQ(0, T_strcmp("", ""));
    EXPECT_GT(0, T_strcmp("", "a"));
    EXPECT_LT(0, T_strcmp("a", ""));
}

TEST(PlStringTest, ConvertUtf8)
{
    // From UTF-8 to plString
    plString from_utf8 = plString::FromUtf8(utf8_test_data);
    EXPECT_STREQ(utf8_test_data, from_utf8.c_str());
    EXPECT_EQ(utf8_test_data_length, from_utf8.GetSize());
    plUnicodeBuffer unicode = from_utf8.GetUnicodeArray();
    EXPECT_EQ(0, T_strcmp(test_data, unicode.GetData()));

    // From plString to UTF-8
    plString to_utf8 = plString::FromUtf32(test_data);
    EXPECT_STREQ(utf8_test_data, to_utf8.c_str());

    // Empty strings
    plString empty = plString::FromUtf8("");
    EXPECT_EQ(0, empty.GetSize());
    EXPECT_EQ(0, T_strcmp(empty.c_str(), ""));

    const plUniChar empty_data[] = { 0 };
    empty = plString::FromUtf32(empty_data);
    EXPECT_EQ(0, empty.GetSize());
    EXPECT_EQ(0, T_strcmp(empty.c_str(), ""));
}

TEST(PlStringTest, ConvertUtf16)
{
    // From UTF-16 to plString
    plString from_utf16 = plString::FromUtf16(utf16_test_data);
    EXPECT_EQ(utf8_test_data_length, from_utf16.GetSize());
    plUnicodeBuffer unicode = from_utf16.GetUnicodeArray();
    EXPECT_EQ(0, T_strcmp(test_data, unicode.GetData()));

    // From plString to UTF-16
    plStringBuffer<uint16_t> to_utf16 = plString::FromUtf32(test_data).ToUtf16();
    EXPECT_EQ(0, T_strcmp(utf16_test_data, to_utf16.GetData()));

    // Empty string
    const uint16_t empty_data[] = { 0 };
    plString empty = plString::FromUtf16(empty_data);
    EXPECT_EQ(0, empty.GetSize());
    EXPECT_EQ(0, T_strcmp(empty.c_str(), ""));
}

TEST(PlStringTest, ConvertIso8859_1)
{
    // From ISO-8859-1 to plString
    const char latin1[] = "\x20\x7e\xa0\xff";
    const plUniChar unicode_cp0[] = { 0x20, 0x7e, 0xa0, 0xff, 0 };
    static const size_t latin1_utf8_length = 6;
    plString from_latin1 = plString::FromIso8859_1(latin1);
    EXPECT_EQ(latin1_utf8_length, from_latin1.GetSize());
    plUnicodeBuffer unicode = from_latin1.GetUnicodeArray();
    EXPECT_EQ(0, T_strcmp(unicode_cp0, unicode.GetData()));

    // From plString to ISO-8859-1
    plStringBuffer<char> to_latin1 = plString::FromUtf32(unicode_cp0).ToIso8859_1();
    EXPECT_STREQ(latin1, to_latin1.GetData());

    // Empty string
    plString empty = plString::FromIso8859_1("");
    EXPECT_EQ(0, empty.GetSize());
    EXPECT_EQ(0, T_strcmp(empty.c_str(), ""));
}

TEST(PlStringTest, ConvertWchar)
{
    // UTF-8 and UTF-16 are already tested, so just make sure we test
    // wchar_t and L"" conversions

    const wchar_t wtext[] = L"\x20\x7f\xff\u0100\uffff";
    const plUniChar unicode_text[] = { 0x20, 0x7f, 0xff, 0x100, 0xffff, 0 };
    static const size_t wtext_utf8_length = 9;
    plString from_wchar = plString::FromWchar(wtext);
    EXPECT_EQ(wtext_utf8_length, from_wchar.GetSize());
    plUnicodeBuffer unicode = from_wchar.GetUnicodeArray();
    EXPECT_EQ(0, T_strcmp(unicode_text, unicode.GetData()));

    // From plString to wchar_t
    plStringBuffer<wchar_t> to_wchar = plString::FromUtf32(unicode_text).ToWchar();
    EXPECT_STREQ(wtext, to_wchar.GetData());

    // Empty string
    plString empty = plString::FromWchar(L"");
    EXPECT_EQ(0, empty.GetSize());
    EXPECT_EQ(0, T_strcmp(empty.c_str(), ""));
}

TEST(PlStringTest, ConvertInvalid)
{
    // The following should encode replacement characters for invalid chars
    const plUniChar unicode_replacement[] = { 0xfffd, 0 };
    const char latin1_replacement[] = "?";

    // Character outside of Unicode specification range
    const plUniChar too_big_c[] = { 0xffffff, 0 };
    plUnicodeBuffer too_big = plString::FromUtf32(too_big_c).GetUnicodeArray();
    EXPECT_EQ(0, T_strcmp(unicode_replacement, too_big.GetData()));

    // Invalid surrogate pairs
    const uint16_t incomplete_surr_c[] = { 0xd800, 0 };
    plString incomplete_surr = plString::FromUtf16(incomplete_surr_c);
    EXPECT_EQ(0, T_strcmp(unicode_replacement,
                          incomplete_surr.GetUnicodeArray().GetData()));

    const uint16_t double_low_c[] = { 0xd800, 0xd801, 0 };
    plString double_low = plString::FromUtf16(double_low_c);
    EXPECT_EQ(0, T_strcmp(unicode_replacement, double_low.GetUnicodeArray().GetData()));

    const uint16_t bad_combo_c[] = { 0xdc00, 0x20, 0 };
    plString bad_combo = plString::FromUtf16(double_low_c);
    EXPECT_EQ(0, T_strcmp(unicode_replacement, bad_combo.GetUnicodeArray().GetData()));

    // ISO-8859-1 doesn't have \ufffd, so it uses '?' instead
    const plUniChar non_latin1_c[] = { 0x1ff, 0 };
    plStringBuffer<char> non_latin1 = plString::FromUtf32(non_latin1_c).ToIso8859_1();
    EXPECT_STREQ(latin1_replacement, non_latin1.GetData());
}

TEST(PlStringTest, Concatenation)
{
    // If this changes, this test may need to be updated to match
    ASSERT_EQ(16, SSO_CHARS);

    plString expected_short = "xxxxyyy";
    plString input1 = "xxxx";
    plString input2 = "yyy";

    plString expected_long = "xxxxxxxxxxyyyyyyyyy";
    plString input3 = "xxxxxxxxxx";
    plString input4 = "yyyyyyyyy";

    // plString + plString
    EXPECT_EQ(expected_short, input1 + input2);
    EXPECT_EQ(expected_long, input3 + input4);
    EXPECT_EQ(input1, input1 + plString());
    EXPECT_EQ(input1, plString() + input1);

    // plstring + const char*
    EXPECT_EQ(expected_short, input1 + input2.c_str());
    EXPECT_EQ(expected_short, input1.c_str() + input2);
    EXPECT_EQ(expected_long, input3 + input4.c_str());
    EXPECT_EQ(expected_long, input3.c_str() + input4);
    EXPECT_EQ(input1, input1 + "");
    EXPECT_EQ(input1, "" + input1);
}

TEST(PlStringTest, Compare)
{
    // Same length, case sensitive
    EXPECT_EQ(0, plString("abc").Compare("abc", plString::kCaseSensitive));
    EXPECT_GT(0, plString("abc").Compare("abd", plString::kCaseSensitive));
    EXPECT_LT(0, plString("abc").Compare("abb", plString::kCaseSensitive));
    EXPECT_GT(0, plString("abC").Compare("abc", plString::kCaseSensitive));
    EXPECT_GT(0, plString("Abc").Compare("abc", plString::kCaseSensitive));
    EXPECT_EQ(0, plString().Compare("", plString::kCaseSensitive));

    // Same length, case insensitive
    EXPECT_EQ(0, plString("abc").Compare("abc", plString::kCaseInsensitive));
    EXPECT_EQ(0, plString("abc").Compare("ABC", plString::kCaseInsensitive));
    EXPECT_GT(0, plString("abc").Compare("abD", plString::kCaseInsensitive));
    EXPECT_LT(0, plString("abc").Compare("abB", plString::kCaseInsensitive));
    EXPECT_EQ(0, plString().Compare("", plString::kCaseInsensitive));

    // Mismatched length, case sensitive
    EXPECT_LT(0, plString("abc").Compare("ab", plString::kCaseSensitive));
    EXPECT_GT(0, plString("abc").Compare("abcd", plString::kCaseSensitive));
    EXPECT_LT(0, plString("abc").Compare("", plString::kCaseSensitive));
    EXPECT_GT(0, plString("").Compare("abc", plString::kCaseSensitive));

    // Mismatched length, case insensitive
    EXPECT_LT(0, plString("abc").Compare("Ab", plString::kCaseInsensitive));
    EXPECT_GT(0, plString("abc").Compare("Abcd", plString::kCaseInsensitive));
    EXPECT_LT(0, plString("abc").Compare("", plString::kCaseInsensitive));
    EXPECT_GT(0, plString().Compare("abc", plString::kCaseInsensitive));
}

TEST(PlStringTest, CompareN)
{
    // Same length, case sensitive
    EXPECT_EQ(0, plString("abcXX").CompareN("abcYY", 3, plString::kCaseSensitive));
    EXPECT_GT(0, plString("abcXX").CompareN("abdYY", 3, plString::kCaseSensitive));
    EXPECT_LT(0, plString("abcXX").CompareN("abbYY", 3, plString::kCaseSensitive));
    EXPECT_GT(0, plString("abCXX").CompareN("abcYY", 3, plString::kCaseSensitive));
    EXPECT_GT(0, plString("AbcXX").CompareN("abcYY", 3, plString::kCaseSensitive));

    // Same length, case insensitive
    EXPECT_EQ(0, plString("abcXX").CompareN("abcYY", 3, plString::kCaseInsensitive));
    EXPECT_EQ(0, plString("abcXX").CompareN("ABCYY", 3, plString::kCaseInsensitive));
    EXPECT_GT(0, plString("abcXX").CompareN("abDYY", 3, plString::kCaseInsensitive));
    EXPECT_LT(0, plString("abcXX").CompareN("abBYY", 3, plString::kCaseInsensitive));

    // Mismatched length, case sensitive
    EXPECT_LT(0, plString("abc").CompareN("ab", 3, plString::kCaseSensitive));
    EXPECT_GT(0, plString("abc").CompareN("abcd", 4, plString::kCaseSensitive));
    EXPECT_LT(0, plString("abc").CompareN("", 3, plString::kCaseSensitive));
    EXPECT_GT(0, plString("").CompareN("abc", 3, plString::kCaseSensitive));

    // Mismatched length, case insensitive
    EXPECT_LT(0, plString("abc").CompareN("Ab", 3, plString::kCaseInsensitive));
    EXPECT_GT(0, plString("abc").CompareN("Abcd", 4, plString::kCaseInsensitive));
    EXPECT_LT(0, plString("abc").CompareN("", 3, plString::kCaseInsensitive));
    EXPECT_GT(0, plString().CompareN("abc", 3, plString::kCaseInsensitive));
}

TEST(PlStringTest, FindChar)
{
    // Available char, case sensitive
    EXPECT_EQ(0, plString("Aaaaaaaa").Find('A', plString::kCaseSensitive));
    EXPECT_EQ(0, plString("AaaaAaaa").Find('A', plString::kCaseSensitive));
    EXPECT_EQ(4, plString("aaaaAaaa").Find('A', plString::kCaseSensitive));
    EXPECT_EQ(7, plString("aaaaaaaA").Find('A', plString::kCaseSensitive));

    // Available char, case insensitive
    EXPECT_EQ(0, plString("Abbbbbbb").Find('A', plString::kCaseInsensitive));
    EXPECT_EQ(0, plString("AbbbAbbb").Find('A', plString::kCaseInsensitive));
    EXPECT_EQ(4, plString("bbbbAbbb").Find('A', plString::kCaseInsensitive));
    EXPECT_EQ(7, plString("bbbbbbbA").Find('A', plString::kCaseInsensitive));
    EXPECT_EQ(0, plString("abbbbbbb").Find('A', plString::kCaseInsensitive));
    EXPECT_EQ(0, plString("abbbabbb").Find('A', plString::kCaseInsensitive));
    EXPECT_EQ(4, plString("bbbbabbb").Find('A', plString::kCaseInsensitive));
    EXPECT_EQ(7, plString("bbbbbbba").Find('A', plString::kCaseInsensitive));
    EXPECT_EQ(0, plString("Abbbbbbb").Find('a', plString::kCaseInsensitive));
    EXPECT_EQ(0, plString("AbbbAbbb").Find('a', plString::kCaseInsensitive));
    EXPECT_EQ(4, plString("bbbbAbbb").Find('a', plString::kCaseInsensitive));
    EXPECT_EQ(7, plString("bbbbbbbA").Find('a', plString::kCaseInsensitive));

    // Unavailable char
    EXPECT_EQ(-1, plString("AaaaAaaa").Find('C', plString::kCaseSensitive));
    EXPECT_EQ(-1, plString("caaacaaa").Find('C', plString::kCaseSensitive));
    EXPECT_EQ(-1, plString("AaaaAaaa").Find('C', plString::kCaseInsensitive));

    // Empty string
    EXPECT_EQ(-1, plString().Find('A', plString::kCaseSensitive));
    EXPECT_EQ(-1, plString().Find('A', plString::kCaseInsensitive));
}

TEST(PlStringTest, FindLast)
{
    // Available char, case sensitive
    EXPECT_EQ(0, plString("Aaaaaaaa").FindLast('A', plString::kCaseSensitive));
    EXPECT_EQ(4, plString("AaaaAaaa").FindLast('A', plString::kCaseSensitive));
    EXPECT_EQ(4, plString("aaaaAaaa").FindLast('A', plString::kCaseSensitive));
    EXPECT_EQ(7, plString("aaaaaaaA").FindLast('A', plString::kCaseSensitive));

    // Available char, case insensitive
    EXPECT_EQ(0, plString("Abbbbbbb").FindLast('A', plString::kCaseInsensitive));
    EXPECT_EQ(4, plString("AbbbAbbb").FindLast('A', plString::kCaseInsensitive));
    EXPECT_EQ(4, plString("bbbbAbbb").FindLast('A', plString::kCaseInsensitive));
    EXPECT_EQ(7, plString("bbbbbbbA").FindLast('A', plString::kCaseInsensitive));
    EXPECT_EQ(0, plString("abbbbbbb").FindLast('A', plString::kCaseInsensitive));
    EXPECT_EQ(4, plString("abbbabbb").FindLast('A', plString::kCaseInsensitive));
    EXPECT_EQ(4, plString("bbbbabbb").FindLast('A', plString::kCaseInsensitive));
    EXPECT_EQ(7, plString("bbbbbbba").FindLast('A', plString::kCaseInsensitive));
    EXPECT_EQ(0, plString("Abbbbbbb").FindLast('a', plString::kCaseInsensitive));
    EXPECT_EQ(4, plString("AbbbAbbb").FindLast('a', plString::kCaseInsensitive));
    EXPECT_EQ(4, plString("bbbbAbbb").FindLast('a', plString::kCaseInsensitive));
    EXPECT_EQ(7, plString("bbbbbbbA").FindLast('a', plString::kCaseInsensitive));

    // Unavailable char
    EXPECT_EQ(-1, plString("AaaaAaaa").FindLast('C', plString::kCaseSensitive));
    EXPECT_EQ(-1, plString("caaacaaa").FindLast('C', plString::kCaseSensitive));
    EXPECT_EQ(-1, plString("AaaaAaaa").FindLast('C', plString::kCaseInsensitive));

    // Empty string
    EXPECT_EQ(-1, plString().FindLast('A', plString::kCaseSensitive));
    EXPECT_EQ(-1, plString().FindLast('A', plString::kCaseInsensitive));
}

TEST(PlStringTest, FindString)
{
    // Available string, case sensitive
    EXPECT_EQ(0, plString("ABCDabcd").Find("ABCD", plString::kCaseSensitive));
    EXPECT_EQ(4, plString("abcdABCDABCDabcd").Find("ABCD", plString::kCaseSensitive));
    EXPECT_EQ(4, plString("abcdABCDabcd").Find("ABCD", plString::kCaseSensitive));
    EXPECT_EQ(4, plString("abcdABCD").Find("ABCD", plString::kCaseSensitive));

    // Available string, case insensitive
    EXPECT_EQ(0, plString("ABCDxxxx").Find("ABCD", plString::kCaseInsensitive));
    EXPECT_EQ(4, plString("xxxxABCDABCDxxxx").Find("ABCD", plString::kCaseInsensitive));
    EXPECT_EQ(4, plString("xxxxABCDxxxx").Find("ABCD", plString::kCaseInsensitive));
    EXPECT_EQ(4, plString("xxxxABCD").Find("ABCD", plString::kCaseInsensitive));
    EXPECT_EQ(0, plString("abcdxxxx").Find("ABCD", plString::kCaseInsensitive));
    EXPECT_EQ(4, plString("xxxxabcdABCDxxxx").Find("ABCD", plString::kCaseInsensitive));
    EXPECT_EQ(4, plString("xxxxabcdxxxx").Find("ABCD", plString::kCaseInsensitive));
    EXPECT_EQ(4, plString("xxxxabcd").Find("ABCD", plString::kCaseInsensitive));
    EXPECT_EQ(0, plString("ABCDxxxx").Find("abcd", plString::kCaseInsensitive));
    EXPECT_EQ(4, plString("xxxxABCDabcdxxxx").Find("abcd", plString::kCaseInsensitive));
    EXPECT_EQ(4, plString("xxxxABCDxxxx").Find("abcd", plString::kCaseInsensitive));
    EXPECT_EQ(4, plString("xxxxABCD").Find("abcd", plString::kCaseInsensitive));

    // Unavailable string
    EXPECT_EQ(-1, plString("xxxx").Find("ABCD", plString::kCaseSensitive));
    EXPECT_EQ(-1, plString("xxxx").Find("ABCD", plString::kCaseInsensitive));

    // Empty string
    EXPECT_EQ(-1, plString().Find("AAAA", plString::kCaseSensitive));
    EXPECT_EQ(-1, plString().Find("AAAA", plString::kCaseInsensitive));

    // Unicode substring
    plString haystack;
    haystack = plString("xxxx") + plString::FromUtf32(test_data);
    EXPECT_EQ(4, haystack.Find(utf8_test_data, plString::kCaseSensitive));
    EXPECT_EQ(4, haystack.Find(utf8_test_data, plString::kCaseInsensitive));

    haystack = plString::FromUtf32(test_data) + plString("xxxx");
    EXPECT_EQ(0, haystack.Find(utf8_test_data, plString::kCaseSensitive));
    EXPECT_EQ(0, haystack.Find(utf8_test_data, plString::kCaseInsensitive));

    haystack = plString("xxxx") + plString::FromUtf32(test_data) + plString("xxxx");
    EXPECT_EQ(4, haystack.Find(utf8_test_data, plString::kCaseSensitive));
    EXPECT_EQ(4, haystack.Find(utf8_test_data, plString::kCaseInsensitive));
}

//TODO: test regex functions

TEST(PlStringTest,TrimLeft)
{
    plString input = plString("abcdefgh");
    plString output = input.TrimLeft("abc");
    plString expected = plString("defgh");
    EXPECT_EQ(expected,output);

    plString input1 = plString("abcdefgh");
    plString output1 = input1.TrimLeft("bc");
    EXPECT_EQ(input1,output1);
}

TEST(PlStringTest,TrimRight)
{
    plString input = plString("abcdefgh");
    plString output = input.TrimRight("fgh");
    plString expected = plString("abcde");
    EXPECT_EQ(expected,output);

    plString input1 = plString("abcdefgh");
    plString output1 = input1.TrimRight("fg");
    EXPECT_EQ(input1,output1);
}

TEST(PlStringTest,Trim)
{
    plString input = plString("abcdefba");
    plString output = input.Trim("ab");
    plString expected = plString("cdef");
    EXPECT_EQ(expected,output);

    plString input1 = plString("abcdefba");
    plString output1 = input1.Trim("f");
    EXPECT_EQ(input1,output1);
}

TEST(PlStringTest,Substr)
{
    plString input = plString("abcdefgh");

    //start > size returns null
    plString output = input.Substr(15,1);
    EXPECT_EQ(plString::Null,output);

    //start<0
    plString output1 =input.Substr(-3,3);
    plString expected1 = plString("fgh");
    EXPECT_EQ(expected1,output1);

    //start+size>size string
    plString output2 =input.Substr(4,6);
    plString expected2 = plString("efgh");
    EXPECT_EQ(expected2,output2);

    //start =0 size = length string
    plString output3 =input.Substr(0,input.GetSize());
    EXPECT_EQ(input,output3);

    //normal case
    plString output4 =input.Substr(1,3);
    plString expected4 = plString("bcd");
    EXPECT_EQ(expected4,output4);
}

TEST(PlStringTest,Replace)
{
    plString input = plString("abcdabcd");

    plString output = input.Replace("ab","cd");
    plString expected = plString("cdcdcdcd");
    EXPECT_EQ(expected,output);

    plString output1 = input.Replace("a","cd");
    plString expected1 = plString("cdbcdcdbcd");
    EXPECT_EQ(expected1,output1);

}

TEST(PlStringTest,ToUpper)
{
    plString input = plString("abCDe");
    plString output = input.ToUpper();
    plString expected = plString("ABCDE");
    EXPECT_EQ(expected,output);
}

TEST(PlStringTest,ToLower)
{
    plString input = plString("aBcDe");
    plString output = input.ToLower();
    plString expected = plString("abcde");
    EXPECT_EQ(expected,output);
}

TEST(PlStringTest,Tokenize)
{
    std::vector<plString> expected;
    expected.push_back(plString("a"));
    expected.push_back(plString("b"));
    expected.push_back(plString("c"));
    expected.push_back(plString("d"));
    expected.push_back(plString("è"));

    plString input = plString("a\t\tb\n;c-d;è");
    std::vector<plString> output = input.Tokenize("\t\n-;");
    EXPECT_EQ(expected,output);

}

TEST(PlStringTest,Split)
{
    std::vector<plString> expected;
    expected.push_back(plString("a"));
    expected.push_back(plString("b"));
    expected.push_back(plString("c"));
    expected.push_back(plString("d"));
    expected.push_back(plString("è"));

    plString input = plString("a-b-c-d-è");
    std::vector<plString> output = input.Split("-",4);
    EXPECT_EQ(expected,output);

}

TEST(PlStringTest,Fill)
{
    plString expected = plString("aaaaa");
    plString output = plString::Fill(5,'a');
    EXPECT_EQ(expected,output);
}

