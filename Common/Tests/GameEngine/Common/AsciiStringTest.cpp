#include <gtest/gtest.h>

#include "Common/AsciiString.h"

TEST(AsciiString, CheckEmpty)
{
    AsciiString str;

    EXPECT_TRUE(str.isEmpty());
    EXPECT_EQ(str.getLength(), 0);
    EXPECT_STREQ(str.str(), "");
    EXPECT_EQ(str, AsciiString::TheEmptyString);
}

TEST(AsciiString, CheckSetAndGet)
{
    AsciiString str;
    str.set("Hello, World!");

    EXPECT_FALSE(str.isEmpty());
    EXPECT_EQ(str.getLength(), 13);
    EXPECT_STREQ(str.str(), "Hello, World!");
    EXPECT_EQ(str, AsciiString("Hello, World!"));
    EXPECT_EQ(str, "Hello, World!");
}

TEST(AsciiString, CheckConcat)
{
    AsciiString str;
    str.set("Hello");
    str.concat(", ");
    str.concat("World!");
    
    EXPECT_FALSE(str.isEmpty());
    EXPECT_EQ(str.getLength(), 13);
    EXPECT_STREQ(str.str(), "Hello, World!");
}

TEST(AsciiString, CheckTrim)
{
    AsciiString str;
    str.set("   Hello, World!   ");
    str.trim();
    
    EXPECT_FALSE(str.isEmpty());
    EXPECT_EQ(str.getLength(), 13);
    EXPECT_STREQ(str.str(), "Hello, World!");
}

TEST(AsciiString, Utf8)
{
    AsciiString str;
    str.set(u8"Hello, 世界!"); // "Hello, World!" in Chinese characters
    
    EXPECT_FALSE(str.isEmpty());
    EXPECT_EQ(str.getLength(), 10); // Each Chinese character counts as one character
    EXPECT_STREQ(str.str(), u8"Hello, 世界!");
}

TEST(AsciiString, CheckToLower)
{
    AsciiString str;
    str.set("Hello, World!");
    str.toLower();
    
    EXPECT_EQ(str.getLength(), 13);
    EXPECT_STREQ(str.str(), "hello, world!");
}

TEST(AsciiString, CheckRemoveLastChar)
{
    AsciiString str;
    str.set("Hello, World!");
    str.removeLastChar();
    
    EXPECT_EQ(str.getLength(), 12);
    EXPECT_STREQ(str.str(), "Hello, World");
}

TEST(AsciiString, CheckFormat)
{
    AsciiString str;
    str.format("Hello, %s!", "World");
    
    EXPECT_EQ(str.getLength(), 13);
    EXPECT_STREQ(str.str(), "Hello, World!");

    str.clear();
    str.format(AsciiString("Hello, %s!"), "World");

    EXPECT_EQ(str.getLength(), 13);
    EXPECT_STREQ(str.str(), "Hello, World!");
}