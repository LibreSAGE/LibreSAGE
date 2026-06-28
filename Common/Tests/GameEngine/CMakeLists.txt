find_package(GTest REQUIRED)
find_package(SDL3 REQUIRED)

add_executable(c_gameenginetest
    Common/AsciiStringTest.cpp
    Common/NameKeyGeneratorTest.cpp
    Common/UnicodeStringTest.cpp
    MainStub.cpp
)

target_link_libraries(c_gameenginetest c_gameengine GTest::gtest GTest::gtest_main SDL3::SDL3)
set_target_properties(c_gameenginetest PROPERTIES OUTPUT_NAME "GameEngineTest")

include(GoogleTest)
gtest_discover_tests(c_gameenginetest)