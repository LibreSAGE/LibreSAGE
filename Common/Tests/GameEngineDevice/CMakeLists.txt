find_package(GTest REQUIRED)
find_package(SDL3 REQUIRED)

add_executable(c_gameenginedevicetest SDL3Device/SDL3BigFileSystemTest.cpp)

target_link_libraries(c_gameenginedevicetest
    c_gameenginestub GTest::gtest GTest::gtest_main SDL3::SDL3 sage_test_support)

set_target_properties(c_gameenginedevicetest PROPERTIES OUTPUT_NAME "GameEngineDeviceTest")

include(GoogleTest)
gtest_discover_tests(c_gameenginedevicetest DISCOVERY_TIMEOUT 20)
