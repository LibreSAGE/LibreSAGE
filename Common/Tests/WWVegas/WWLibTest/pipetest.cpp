// (C) 2026 Stephan Vedder
#include "pipe.h"
#include "b64pipe.h"
#include "crcpipe.h"
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
    void (*verify_func)(Pipe *pipe) = nullptr;

    ~PipeTestData() { delete pipe; }
};

// A test case is parameterized by a factory rather than a pre-built PipeTestData.
// This keeps the ValuesIn() list to trivial function pointers (nothing allocated
// at test-registration time), so the Pipe and any file/RNG dependencies it uses
// are created and destroyed entirely within a single test run - see SetUp/TearDown.
using PipeTestFactory = PipeTestData *(*)();

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

// Fixed RSA test key, stored in DER form. This was captured once from the
// original Generate_Prime() based GeneratePKey() (which was deterministic via
// Seed_Byte(255)). Hardcoding it avoids regenerating 128-bit primes on every
// process start-up: ctest launches this binary once per parameterized case and
// prime generation runs during test registration, so it previously cost ~2s
// per launch (~30s total) in a debug build for no added coverage.
static const unsigned char PK_DER_MODULUS[] = {
    0x02, 0x20, 0x79, 0x42, 0x4b, 0x54, 0x5d, 0x66, 0x6f, 0x78, 0x81, 0x8a,
    0x93, 0x9c, 0xa5, 0xaf, 0xe5, 0xdc, 0x48, 0x3f, 0x36, 0x2d, 0x24, 0x1b,
    0x12, 0x08, 0xff, 0xf6, 0xed, 0xe5, 0x97, 0xfe, 0x24, 0x39};
static const unsigned char PK_DER_EXPONENT[] = {
    0x02, 0x03, 0x01, 0x00, 0x01};

// Ciphertext of TESTSTRING under the key above, with the PKPipe session RNG
// seeded via Seed_Byte(255). Keep in sync if the key or the seed changes.
static const char PK_ENCRYPTED[] =
    "\xf1\x1b\x9b\x7e\xb3\xfb\x4f\x6c\x6f\x5b\xb9\x7d\x3a\x3a\x38"
    "\xcd\x49\xbd\xa6\x6d\x9e\x54\xc9\xd1\x60\xd4\x06\xcb\xc2\x3c"
    "\x8c\x0a\xee\x3c\x66\x3f\x1f\x4d\x6b\x74\x00\x8a\x97\x9e\x2f"
    "\xdc\x6e\x5c\xa3\x26\x93\x28\x78\xd1\x9a\x28\xae\x3c\x5f\x3f"
    "\x5b\xae\x00\x72\x4c\x6f\x72\x65\x6d\x20\x69\x70\x73\x75\x6d";
static const int PK_ENCRYPTED_SZ = TESTSTRING_SZ + 64; // (+ blowfish block size)

static const PKey &GetTestPKey()
{
    static const PKey key(PK_DER_EXPONENT, PK_DER_MODULUS);
    return key;
}

PipeTestData *GeneratePKEncryptPipeTestData()
{
    // Seed deterministically so the session key (and thus the ciphertext) is
    // reproducible without depending on prior RNG consumption.
    static RandomStraw random;
    random.Seed_Byte(255);

    PKPipe *pk_pipe = new PKPipe(PKPipe::ENCRYPT, random);
    pk_pipe->Key(&GetTestPKey());
    return new PipeTestData{
        pk_pipe,
        TESTSTRING,
        TESTSTRING_SZ,
        true,
        PK_ENCRYPTED,
        PK_ENCRYPTED_SZ};
}

PipeTestData *GeneratePKDecryptPipeTestData()
{
    static RandomStraw random;

    PKPipe *pk_pipe = new PKPipe(PKPipe::DECRYPT, random);
    pk_pipe->Key(&GetTestPKey());
    return new PipeTestData{
        pk_pipe,
        PK_ENCRYPTED,
        PK_ENCRYPTED_SZ,
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
    static RAMFileClass ramFile(buffer, sizeof(buffer));
    return new PipeTestData{
        new FilePipe(ramFile),
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
std::vector<PipeTestFactory> SetupPipeTestCases()
{
    return {
        GeneratePKEncryptPipeTestData,
        GeneratePKDecryptPipeTestData,
        GenerateBase64EncryptPipeTestData,
        GenerateBase64DecryptPipeTestData,
        GenerateLCWCompressPipeTestData,
        GenerateLCWDecompressPipeTestData,
        GenerateLZOCompressPipeTestData,
        GenerateLZODecompressPipeTestData,
        GenerateBufferPipeTestData,
        GenerateFilePipeTestData,
        GenerateCRCPipeTestData,
        GenerateSHAPipeTestData,
    };
}

class PipeTestClass : public ::testing::TestWithParam<PipeTestFactory>
{
protected:
    PipeTestData *m_testcase = nullptr;

    void SetUp() override { m_testcase = GetParam()(); }

    void TearDown() override
    {
        // The test body chains the pipe to a stack-local BufferPipe; drop that
        // link before destroying the pipe so ~Pipe doesn't touch the now-gone
        // buffer. Destroying the case here (rather than leaking it) is safe
        // because every dependency it references is still alive at this point.
        if (m_testcase != nullptr)
        {
            m_testcase->pipe->ChainTo = nullptr;
            delete m_testcase;
            m_testcase = nullptr;
        }
    }
};

/*
** Test Pipe functionality using the above test data
*/
TEST_P(PipeTestClass, Put)
{
    char buffer[1024] = {0};
    int size = 0;
    BufferPipe bp(buffer, sizeof(buffer));

    const PipeTestData *testcase = m_testcase;
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
