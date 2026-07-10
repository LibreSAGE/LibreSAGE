#include <gtest/gtest.h>

#include "Common/FileSystem.h"
#include "Common/GlobalData.h"
#include "MiniAudioDevice/MiniAudioManager.h"
#include "SDL3Device/Common/SDL3BIGFileSystem.h"
#include "SDL3Device/Common/SDL3LocalFileSystem.h"
#include "VideoDevice/FFmpeg/FFmpegVideoPlayer.h"

#define SAGE_TEST_BIK_DATA_DIR SAGE_TEST_DATA_DIR "/bik"

TEST(FFmpegVideoPlayer, DISABLED_Load)
{
    // Filesystem Boilerplate
    SDL3LocalFileSystem local_fs;
    TheLocalFileSystem = &local_fs;
    FileSystem fs;
    TheFileSystem = &fs;
    MiniAudioManager am;
    TheAudio = &am;
    GlobalData gd;
    TheWritableGlobalData = &gd;
    FFmpegVideoPlayer player;
    
    Video logo = {
        .m_filename = SAGE_TEST_BIK_DATA_DIR "/EA_Logo640.bik",
        .m_internalName = "EA_Logo640",
        .m_commentForWB = "Electronic Arts Logo"
    };
    player.addVideo(&logo);

    VideoStreamInterface* stream = player.load("EA_Logo640");
    ASSERT_NE(stream, nullptr);
}