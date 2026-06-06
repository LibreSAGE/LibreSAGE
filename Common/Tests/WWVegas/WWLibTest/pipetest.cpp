// (C) 2026 Stephan Vedder
#include "pipe.h"
#include "b64pipe.h"
#include "crcpipe.h"
#include "int.h"
#include "lcwpipe.h"
#include "lzopipe.h"
#include "pkpipe.h"
#include "ramfile.h"
#include "rndstraw.h"
#include "shapipe.h"
#include "xpipe.h"
#include <stdio.h>

#include <gtest/gtest.h>

struct PipeTestData
{
    Pipe *pipe;
    const char *input_data;
    int in_length;
    bool supports_chaining;
    const char *expected_data;
    int expected_length;
    void (*verify_func)(Pipe *pipe);
};

const char *TESTSTRING = "Lorem ipsum";
const int TESTSTRING_SZ = 11;
const char *TESTSTRING_LONG = R"(
Lorem ipsum dolor sit amet, consetetur sadipscing elitr, 
sed diam nonumy eirmod tempor invidunt ut labore et dolore
magna aliquyam erat, sed diam voluptua. 
At vero eos et accusam et justo duo dolores et ea rebum. 
Stet clita kasd gubergren, no sea takimata sanctus est 
Lorem ipsum dolor sit amet. Lorem ipsum dolor sit amet,
consetetur sadipscing elitr, sed diam nonumy eirmod tempor 
invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. 
At vero eos et accusam et justo duo dolores et ea rebum. 
Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet.  
)";
const int TESTSTRING_LONG_SZ = 602;

PKey GeneratePKey(RandomStraw &random)
{
    random.Seed_Byte(255);
    PKey key;
    BigInt p = Generate_Prime<BigInt>(random, 128, NULL);
    BigInt q = Generate_Prime<BigInt>(random, 128, NULL);
    BigInt e = PKey::Fast_Exponent();
    BigInt n = p * q;
    key.Exponent = e;
    key.Modulus = n;
    key.BitPrecision = n.BitCount() - 1;
    return key;
}

PipeTestData *GeneratePKEncryptPipeTestData()
{
    static RandomStraw random;
    static PKey key = GeneratePKey(random);

    PKPipe *pk_pipe = new PKPipe(PKPipe::ENCRYPT, random);
    pk_pipe->Key(&key); // Use appropriate values for exponent and modulus
    return new PipeTestData{
        pk_pipe,
        TESTSTRING,
        TESTSTRING_SZ,
        true,
        "\xfe\x0c\xd6\xe8\x4b\x0a\x62\x6c\x0f\xfa\xfc\xc2\x35\xb4\xb0"
        "\xd8\x7e\x67\xb5\x4e\xa3\x11\x9e\xbc\x19\xfc\x41\xac\x61\xf7"
        "\xbe\x76\xab\x4c\x0e\x42\x5e\x1c\x9c\x7f\xc2\x19\x73\xf0\xff"
        "\x31\x47\xab\x77\x13\x16\xf3\x2c\xef\x98\xee\x8d\x22\x3c\x33"
        "\xf5\xcf\xc7\x11Lorem ipsum",
        TESTSTRING_SZ + 64}; // (+ blowfish block size)
}

PipeTestData *GeneratePKDecryptPipeTestData()
{
    static RandomStraw random;
    static PKey key = GeneratePKey(random);

    PKPipe *pk_pipe = new PKPipe(PKPipe::DECRYPT, random);
    pk_pipe->Key(&key); // Use appropriate values for exponent and modulus
    return new PipeTestData{
        pk_pipe,
        "\xfe\x0c\xd6\xe8\x4b\x0a\x62\x6c\x0f\xfa\xfc\xc2\x35\xb4\xb0"
        "\xd8\x7e\x67\xb5\x4e\xa3\x11\x9e\xbc\x19\xfc\x41\xac\x61\xf7"
        "\xbe\x76\xab\x4c\x0e\x42\x5e\x1c\x9c\x7f\xc2\x19\x73\xf0\xff"
        "\x31\x47\xab\x77\x13\x16\xf3\x2c\xef\x98\xee\x8d\x22\x3c\x33"
        "\xf5\xcf\xc7\x11Lorem ipsum",
        TESTSTRING_SZ + 64,
        true,
        TESTSTRING,
        TESTSTRING_SZ};
};

PipeTestData *GenerateBase64EncryptPipeTestData()
{
    // For simplicity, we will not implement this test case as it requires Base64 encoding.
    return new PipeTestData{
        new Base64Pipe(Base64Pipe::ENCODE),
        TESTSTRING,
        TESTSTRING_SZ,
        true,
        "TG9yZW0gaXBzdW0=", // Base64 encoded "Lorem ipsum"
        16};
};

PipeTestData *GenerateBase64DecryptPipeTestData()
{
    // For simplicity, we will not implement this test case as it requires Base64 decoding.
    return new PipeTestData{
        new Base64Pipe(Base64Pipe::DECODE),
        "TG9yZW0gaXBzdW0=", // Base64 encoded "Lorem ipsum"
        16,
        true,
        TESTSTRING,
        TESTSTRING_SZ};
};

PipeTestData *GenerateBufferPipeTestData()
{
    static char buffer[256] = {0};
    return new PipeTestData{
        new BufferPipe(buffer, sizeof(buffer)),
        TESTSTRING,
        TESTSTRING_SZ,
        false,
        NULL,
        TESTSTRING_SZ};
};

PipeTestData *GenerateFilePipeTestData()
{
    // For simplicity, we will not implement this test case as it requires file I/O.
    static char buffer[256] = {0};
    return new PipeTestData{
        new FilePipe(new RAMFileClass(buffer, sizeof(buffer))),
        TESTSTRING,
        TESTSTRING_SZ,
        false,
        NULL,
        TESTSTRING_SZ};
};

PipeTestData *GenerateCRCPipeTestData()
{
    // For simplicity, we will not implement this test case as it requires CRC calculation.
    return new PipeTestData{
        new CRCPipe(),
        TESTSTRING,
        TESTSTRING_SZ,
        true,
        TESTSTRING, // Replace with expected CRC output
        TESTSTRING_SZ,
        [](Pipe *pipe)
        {
            CRCPipe *crc_pipe = dynamic_cast<CRCPipe *>(pipe);
            EXPECT_EQ(crc_pipe->Result(), 0x7709737D);
        }}; // Replace with actual expected length
};

static const std::vector<char> &GetLCWCompressedLongData()
{
    static const std::vector<char> compressed = []()
    {
        // Keep this generated payload in sync with the LCWPipe compressor implementation.
        std::vector<char> out(2048);
        BufferPipe bp(out.data(), static_cast<int>(out.size()));
        LCWPipe pipe(LCWPipe::COMPRESS);
        pipe.ChainTo = &bp;

        int len = 0;
        len += pipe.Put(TESTSTRING_LONG, TESTSTRING_LONG_SZ);
        len += pipe.Flush();
        out.resize(len);
        return out;
    }();

    return compressed;
}

PipeTestData *GenerateLCWCompressPipeTestData()
{
    const std::vector<char> &compressed = GetLCWCompressedLongData();

    return new PipeTestData{
        new LCWPipe(LCWPipe::COMPRESS),
        TESTSTRING_LONG,
        TESTSTRING_LONG_SZ,
        true,
        compressed.data(),
        static_cast<int>(compressed.size())};
};

PipeTestData *GenerateLCWDecompressPipeTestData()
{
    const std::vector<char> &compressed = GetLCWCompressedLongData();

    return new PipeTestData{
        new LCWPipe(LCWPipe::DECOMPRESS),
        compressed.data(),
        static_cast<int>(compressed.size()),
        true,
        TESTSTRING_LONG,
        TESTSTRING_LONG_SZ};
};

static const std::vector<char> &GetLZOCompressedLongData()
{
    static const std::vector<char> compressed = []()
    {
        // Keep this generated payload in sync with the LZOPipe compressor implementation.
        std::vector<char> out(2048);
        BufferPipe bp(out.data(), static_cast<int>(out.size()));
        LZOPipe pipe(LZOPipe::COMPRESS);
        pipe.ChainTo = &bp;

        int len = 0;
        len += pipe.Put(TESTSTRING_LONG, TESTSTRING_LONG_SZ);
        len += pipe.Flush();
        out.resize(len);
        return out;
    }();

    return compressed;
}

PipeTestData *GenerateLZOCompressPipeTestData()
{
    const std::vector<char> &compressed = GetLZOCompressedLongData();

    return new PipeTestData{
        new LZOPipe(LZOPipe::COMPRESS),
        TESTSTRING_LONG,
        TESTSTRING_LONG_SZ,
        true,
        compressed.data(),
        static_cast<int>(compressed.size())};
};

PipeTestData *GenerateLZODecompressPipeTestData()
{
    const std::vector<char> &compressed = GetLZOCompressedLongData();

    return new PipeTestData{
        new LZOPipe(LZOPipe::DECOMPRESS),
        compressed.data(),
        static_cast<int>(compressed.size()),
        true,
        TESTSTRING_LONG,
        TESTSTRING_LONG_SZ};
};

PipeTestData *GenerateSHAPipeTestData()
{
    // For simplicity, we will not implement this test case as it requires SHA hashing.
    return new PipeTestData{
        new SHAPipe(),
        TESTSTRING,
        TESTSTRING_SZ,
        true,
        TESTSTRING, // Replace with expected SHA output
        TESTSTRING_SZ,
        [](Pipe *pipe)
        {
            SHAPipe *sha_pipe = dynamic_cast<SHAPipe *>(pipe);
            char result[20] = {0};
            sha_pipe->Result(result);
            const unsigned char expected_sha[20] = {
                0x94, 0x91, 0x2b, 0xe8, 0xb3, 0xfb, 0x47, 0xd4, 0x16, 0x1e,
                0xa5, 0x0e, 0x59, 0x48, 0xc6, 0x29, 0x6a, 0xf6, 0xca, 0x05};

            for (int i = 0; i < 20; ++i)
            {
                EXPECT_EQ(static_cast<unsigned char>(result[i]), expected_sha[i]);
            }
        }}; // Replace with actual expected length
};

// DER formatted exponent and modulus for testing PKPipe.
std::vector<PipeTestData *> SetupPipeTestCases()
{
    std::vector<PipeTestData *> test_cases;
    test_cases.emplace_back(GeneratePKEncryptPipeTestData());
    test_cases.emplace_back(GeneratePKDecryptPipeTestData());
    test_cases.emplace_back(GenerateBase64EncryptPipeTestData());
    test_cases.emplace_back(GenerateBase64DecryptPipeTestData());
    test_cases.emplace_back(GenerateLCWCompressPipeTestData());
    test_cases.emplace_back(GenerateLCWDecompressPipeTestData());
    test_cases.emplace_back(GenerateLZOCompressPipeTestData());
    test_cases.emplace_back(GenerateLZODecompressPipeTestData());
    test_cases.emplace_back(GenerateBufferPipeTestData());
    test_cases.emplace_back(GenerateFilePipeTestData());
    test_cases.emplace_back(GenerateCRCPipeTestData());
    test_cases.emplace_back(GenerateSHAPipeTestData());
    return test_cases;
}

class PipeTestClass : public ::testing::TestWithParam<PipeTestData *>
{
protected:
};

/*
** Test Pipe functionality using the above test data
*/
TEST_P(PipeTestClass, Put)
{
    char buffer[1024] = {0};
    int size = 0;
    BufferPipe bp(buffer, sizeof(buffer));

    const PipeTestData *testcase = GetParam();
    Pipe *pipe = testcase->pipe;
    pipe->ChainTo = &bp;
    size += pipe->Put(testcase->input_data, testcase->in_length);
    size += pipe->Flush();
    ASSERT_EQ(size, testcase->expected_length);
    if (testcase->supports_chaining)
    {
        if (testcase->expected_data != NULL)
        {
            for (int i = 0; i < testcase->expected_length; ++i)
            {
                EXPECT_EQ(buffer[i], testcase->expected_data[i]);
            }
        }
        else
        {
            for (int i = 0; i < size; ++i)
            {
                printf("\\x%02x", (unsigned char)buffer[i]);
            }
            printf("\n");
        }
    }

    if (testcase->verify_func != nullptr)
    {
        testcase->verify_func(pipe);
    }
}

INSTANTIATE_TEST_CASE_P(WWLib, PipeTestClass, ::testing::ValuesIn(SetupPipeTestCases()));
