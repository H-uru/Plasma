#include <plString.h>
#include <HeadSpin.h>

#include <gtest/gtest.h>
#include <wchar.h>


TEST(PlStringTest,ToUtf16)
{
    uint16_t text[] = {0x0061,0x0062,0x0063,0x0064}; //abcd as in utf16
    plStringBuffer<uint16_t> expected = plStringBuffer<uint16_t>(text,arrsize(text));
    plStringBuffer<uint16_t> output = plString("abcd").ToUtf16();
    
    EXPECT_EQ(expected.GetSize(), output.GetSize()); //not really a good test
}

TEST(PlStringTest,ToWchar)
{
    wchar_t text[] =L"abcd\u00E9";
    plStringBuffer<wchar_t> expected = plStringBuffer<wchar_t>(text,arrsize(text));
    plStringBuffer<wchar_t> output = plString("abcd\xC3\xA9").ToWchar();
    EXPECT_STREQ(expected.GetData(),output.GetData());
}

TEST(PlStringTest,ToIso8859_1)
{
    char text[] ="abcde";
    plStringBuffer<char> expected = plStringBuffer<char>(text,arrsize(text));
    plStringBuffer<char> output = plString("abcde").ToIso8859_1();
    EXPECT_STREQ(expected.GetData(),output.GetData());
}

TEST(PlStringTest,FindChar)
{
    plString input = plString("abCdcBAeab");
    int result=0;
    //available char, case sensitive
    result = input.Find('B',plString::kCaseSensitive);
    EXPECT_EQ(5,result);

    //available char, case insensitive
    result = input.Find('B',plString::kCaseInsensitive);
    EXPECT_EQ(1,result);

    //unavailable char, case sensitive
    result = input.Find('f',plString::kCaseSensitive);
    EXPECT_EQ(-1,result);

    //unavailable char, case insensitive
    result=0;
    result = input.Find('f',plString::kCaseInsensitive);
    EXPECT_EQ(-1,result);

    plString input1 = plString("abCdcBÁèab");
    //available accented char, case sensitive
    result = input1.Find('Á',plString::kCaseSensitive);
    EXPECT_EQ(7,result);

    //available accented char, case insensitive
    result = input1.Find('è',plString::kCaseInsensitive);
    EXPECT_EQ(9,result);
}

TEST(PlStringTest,FindLast)
{
    plString input = plString("abCdcBAeab");
    int result=0;
    //available char, case sensitive
    result = input.FindLast('B',plString::kCaseSensitive);
    EXPECT_EQ(5,result);

    //available char, case insensitive
    result = input.FindLast('B',plString::kCaseInsensitive);
    EXPECT_EQ(9,result);

    //unavailable char, case sensitive
    result = input.FindLast('f',plString::kCaseSensitive);
    EXPECT_EQ(-1,result);

    //unavailable char, case insensitive
    result=0;
    result = input.FindLast('f',plString::kCaseInsensitive);
    EXPECT_EQ(-1,result);

    plString input1 = plString("éeÉß");
    //available accented char, case sensitive
    result = input1.FindLast('e',plString::kCaseSensitive);
    EXPECT_EQ(2,result);

    //available accented char, case insensitive
    result = input1.FindLast('ß',plString::kCaseInsensitive);
    EXPECT_EQ(6,result);
}

TEST(PlStringTest,FindString)
{
    plString input = plString("abABÁè");
    int result=0;
    //available string, case sensitive
    result = input.Find("AB",plString::kCaseSensitive);
    EXPECT_EQ(2,result);

    //available string, case insensitive
    result = input.Find("ab",plString::kCaseInsensitive);
    EXPECT_EQ(0,result);

    //unavailable string, case sensitive
    result = input.Find("cd",plString::kCaseSensitive);
    EXPECT_EQ(-1,result);

    //unavailable string, case insensitive
    result=0;
    result = input.Find("cd",plString::kCaseInsensitive);
    EXPECT_EQ(-1,result);

    plString input1 = plString("àbéCdcBÀéab");
    //available accented string, case sensitive
    result = input1.Find("À",plString::kCaseSensitive);
    EXPECT_EQ(9,result);

    //the strnicmp method used does not support unicode
    //available accented string, case insensitive
   // result = input1.Find("À",plString::kCaseInsensitive);
   // EXPECT_EQ(1,result);
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

//overload operator+
TEST(PlStringTest,Addition)
{
    plString expected = "abcde";
    plString input1 = "ab";
    plString input2 = "cde";

    //plstring+plstring
    plString output = input1+input2;
    EXPECT_EQ(expected,output);

    //plstring+char*
    plString output1 = input1 + input2.c_str();
    EXPECT_EQ(expected,output1);

    //char*+plstring
    plString output2 = input1.c_str() + input2;
    EXPECT_EQ(expected,output2);
}
