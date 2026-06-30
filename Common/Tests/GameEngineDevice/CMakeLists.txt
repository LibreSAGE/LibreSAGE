find_package(GTest REQUIRED)
find_package(SDL3 REQUIRED)

add_executable(c_gameenginedevicetest
    SDL3Device/SDL3LocalFileSystemTest.cpp
    SDL3Device/SDL3BigFileSystemTest.cpp
    MainStub.cpp
)

target_link_libraries(c_gameenginedevicetest c_gameenginedevice GTest::gtest GTest::gtest_main SDL3::SDL3)
target_compile_definitions(c_gameenginedevicetest PRIVATE 
    SAGE_MOD_SUPPORT_DIR="${SAGE_MOD_SUPPORT_DIR}"
    SAGE_TEST_DATA_DIR="${SAGE_TEST_DATA_DIR}"
)

set_target_properties(c_gameenginedevicetest PROPERTIES OUTPUT_NAME "GameEngineDeviceTest")

include(GoogleTest)
gtest_discover_tests(c_gameenginedevicetest)