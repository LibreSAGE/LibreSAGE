#include "base64.h"

#include <gtest/gtest.h>

TEST(WWLib, Base64Encode)
{
    const char *input = "Lorem ipsum";
    char output[256] = {0};
    Base64_Encode(input, strlen(input), output, sizeof(output));
    EXPECT_STREQ(output, "TG9yZW0gaXBzdW0=");
}

TEST(WWLib, Base64Decode)
{
    const char *input = "TG9yZW0gaXBzdW0=";
    char output[256] = {0};
    int decoded_length = Base64_Decode(input, strlen(input), output, sizeof(output));
    EXPECT_EQ(decoded_length, 11);
    EXPECT_STREQ(output, "Lorem ipsum");
}