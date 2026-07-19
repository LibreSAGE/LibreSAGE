#include <gtest/gtest.h>

#include "Common/File.h"
#include "SDL3Device/Common/SDL3BIGFileSystem.h"
#include "SDL3Device/Common/SDL3LocalFileSystem.h"
#include "TestSupport/RetailData.h"

// Loads the retail archives of both games through the Common device code. The
// per-game test trees cover the same ground against their own engine
// libraries; this one exists to exercise the archive reader on its own, with
// no game library linked in at all.
class RetailBigArchives : public ::testing::TestWithParam<const char*>
{
};

TEST_P(RetailBigArchives, Load)
{
    SAGE_REQUIRE_RETAIL_DATA(retailDir, GetParam());

    SDL3LocalFileSystem local_fs;
    TheLocalFileSystem = &local_fs;
    SDL3BIGFileSystem fs;

    ASSERT_TRUE(fs.loadBigFilesFromDirectory(retailDir, "*.big", true))
        << "Failed to load retail .big archives from " << retailDir;

    FilenameList fileList;
    fs.getFileListInDirectory("data", "", "*.ini", fileList, true);
    EXPECT_FALSE(fileList.empty()) << "Retail archives contain no .ini files under data/";
}

const char* retail_data_dirs[] = {
    SAGE_RETAIL_DATA_ENV_GENERALS,
    SAGE_RETAIL_DATA_ENV_GENERALSMD,
};

INSTANTIATE_TEST_SUITE_P(Common, RetailBigArchives, ::testing::ValuesIn(retail_data_dirs));
