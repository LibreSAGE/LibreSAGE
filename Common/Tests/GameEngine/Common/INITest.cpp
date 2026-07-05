#include <gtest/gtest.h>

#include "Common/INI.h"
#include "GameClient/Water.h"
#include "SDL3Device/Common/SDL3BIGFileSystem.h"
#include "SDL3Device/Common/SDL3LocalFileSystem.h"
#include "Common/FileSystem.h"

#define SAGE_TEST_INI_DATA_DIR SAGE_TEST_DATA_DIR "/ini"

TEST(INI, ParseWaterINI)
{
    // Filesystem Boilerplate
    SDL3LocalFileSystem local_fs;
    TheLocalFileSystem = &local_fs;
    FileSystem fs;
    TheFileSystem = &fs;

    // Register parsing functions
    INI::registerBlockParse("WaterSet", WaterSetting::parse);
    INI::registerBlockParse("WaterTransparency", WaterTransparencySetting::parse);

    INI ini;
    WaterSetting waterSetting;

    ini.load(SAGE_TEST_INI_DATA_DIR "/water.ini", INI_LOAD_OVERWRITE, NULL );

}