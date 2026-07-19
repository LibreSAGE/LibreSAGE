#include <gtest/gtest.h>

#include "Common/File.h"
#include "SDL3Device/Common/SDL3BIGFileSystem.h"
#include "SDL3Device/Common/SDL3LocalFileSystem.h"
#include "TestSupport/RetailData.h"

#define SAGE_TEST_BIG_DATA_DIR SAGE_TEST_DATA_DIR "/big"

TEST(SDL3BIGFileSystem, ListFiles)
{
    SDL3LocalFileSystem local_fs;
    TheLocalFileSystem = &local_fs;
    SDL3BIGFileSystem fs;

    EXPECT_TRUE(fs.loadBigFilesFromDirectory(SAGE_TEST_BIG_DATA_DIR, "*.big", true));

    // Recurse through the mod directory and get a list of all files.
    FilenameList fileList;
    fs.getFileListInDirectory("data", "", "*.txt", fileList, true);
    EXPECT_EQ(fileList.size(), 6) << "Expected 6 files in the data directory, but found " << fileList.size();
}

TEST(SDL3BIGFileSystem, OpenFileExistent)
{
    SDL3LocalFileSystem local_fs;
    TheLocalFileSystem = &local_fs;
    SDL3BIGFileSystem fs;
    ASSERT_TRUE(fs.loadBigFilesFromDirectory(SAGE_TEST_BIG_DATA_DIR, "*.big", true));

    File* file = fs.openFile("data\\a.txt", File::READ);
    ASSERT_NE(file, nullptr) << "Failed to open a.txt file.";
    EXPECT_STREQ("a.txt", file->getName());

    file->close();
}

TEST(SDL3BIGFileSystem, OpenFileNonExistent)
{
    SDL3LocalFileSystem local_fs;
    TheLocalFileSystem = &local_fs;
    SDL3BIGFileSystem fs;
    ASSERT_TRUE(fs.loadBigFilesFromDirectory(SAGE_TEST_BIG_DATA_DIR, "*.big", true));

    File* file = fs.openFile(SAGE_MOD_SUPPORT_DIR "/NON_EXISTENT_FILE.md", File::READ);
    ASSERT_EQ(file, nullptr) << "Unexpectedly opened a non-existent file.";
}

TEST(SDL3BIGFileSystem, DoesFileExistExistent)
{
    SDL3LocalFileSystem local_fs;
    TheLocalFileSystem = &local_fs;
    SDL3BIGFileSystem fs;
    ASSERT_TRUE(fs.loadBigFilesFromDirectory(SAGE_TEST_BIG_DATA_DIR, "*.big", true));

    EXPECT_TRUE(fs.doesFileExist("data\\a.txt"));
}

TEST(SDL3BIGFileSystem, DoesFileExistNonExistent)
{
    SDL3LocalFileSystem local_fs;
    TheLocalFileSystem = &local_fs;
    SDL3BIGFileSystem fs;
    ASSERT_TRUE(fs.loadBigFilesFromDirectory(SAGE_TEST_BIG_DATA_DIR, "*.big", true));

    EXPECT_FALSE(fs.doesFileExist("data\\NON_EXISTENT_FILE.txt"));
}

TEST(SDL3BIGFileSystem, GetFileInfoExistent)
{
    SDL3LocalFileSystem local_fs;
    TheLocalFileSystem = &local_fs;
    SDL3BIGFileSystem fs;
    ASSERT_TRUE(fs.loadBigFilesFromDirectory(SAGE_TEST_BIG_DATA_DIR, "*.big", true));

    FileInfo fileInfo;
    EXPECT_TRUE(fs.getFileInfo("data\\a.txt", &fileInfo)) << "Failed to get file info for a.txt";
    Int64 size = (static_cast<Int64>(fileInfo.sizeHigh) << 32) | static_cast<Int64>(fileInfo.sizeLow);
    EXPECT_GT(size, 0) << "File size should be greater than 0 for a.txt";
}

TEST(SDL3BIGFileSystem, GetFileInfoNonExistent)
{
    SDL3LocalFileSystem local_fs;
    TheLocalFileSystem = &local_fs;
    SDL3BIGFileSystem fs;
    ASSERT_TRUE(fs.loadBigFilesFromDirectory(SAGE_TEST_BIG_DATA_DIR, "*.big", true));

    FileInfo fileInfo;
    EXPECT_FALSE(fs.getFileInfo("data\\NON_EXISTENT_FILE.txt", &fileInfo)) << "Unexpectedly got file info for a non-existent file.";
}

TEST(SDL3BIGFileSystem, GetArchiveNameForFileExistent)
{
    SDL3LocalFileSystem local_fs;
    TheLocalFileSystem = &local_fs;
    SDL3BIGFileSystem fs;
    ASSERT_TRUE(fs.loadBigFilesFromDirectory(SAGE_TEST_BIG_DATA_DIR, "*.big", true));

    AsciiString archiveName = fs.getArchiveFilenameForFile("data\\a.txt");
    EXPECT_STREQ(archiveName.str(), (SAGE_TEST_BIG_DATA_DIR "/a.big")) << "Unexpected archive name for a.txt";

    archiveName = fs.getArchiveFilenameForFile("data\\d.txt");
    EXPECT_STREQ(archiveName.str(), (SAGE_TEST_BIG_DATA_DIR "/b.big")) << "Unexpected archive name for d.txt";
}

// Smoke test over a real game install: the archives are far larger and far more
// numerous than the hand-made fixtures above, so this catches parsing problems
// the fixtures cannot reach. Skipped unless a retail data directory is configured.
TEST(SDL3BIGFileSystem, LoadRetailArchives)
{
    SAGE_REQUIRE_RETAIL_DATA(retailDir, SAGE_RETAIL_DATA_ENV_GENERALSMD);

    SDL3LocalFileSystem local_fs;
    TheLocalFileSystem = &local_fs;
    SDL3BIGFileSystem fs;

    ASSERT_TRUE(fs.loadBigFilesFromDirectory(retailDir, "*.big", true))
        << "Failed to load retail .big archives from " << retailDir;

    FilenameList fileList;
    fs.getFileListInDirectory("data", "", "*.ini", fileList, true);
    EXPECT_FALSE(fileList.empty()) << "Retail archives contain no .ini files under data/";
}
