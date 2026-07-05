find_package(GTest REQUIRED)
find_package(SDL3 REQUIRED)

add_executable(c_gameenginetest
    Common/AsciiStringTest.cpp
    Common/CRCTest.cpp
    Common/DictTest.cpp
    Common/EncryptTest.cpp
    Common/INITest.cpp
    Common/NameKeyGeneratorTest.cpp
    Common/UnicodeStringTest.cpp
    MainStub.cpp
)

target_link_libraries(c_gameenginetest c_gameengine c_gameenginedevice GTest::gtest GTest::gtest_main SDL3::SDL3)
target_compile_definitions(c_gameenginetest PRIVATE 
    SAGE_MOD_SUPPORT_DIR="${SAGE_MOD_SUPPORT_DIR}"
    SAGE_TEST_DATA_DIR="${SAGE_TEST_DATA_DIR}"
)

set_target_properties(c_gameenginetest PROPERTIES OUTPUT_NAME "GameEngineTest")

include(GoogleTest)
gtest_discover_tests(c_gameenginetest)