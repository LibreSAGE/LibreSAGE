find_package(GTest REQUIRED)

add_executable(c_wwlibtest base64test.cpp mempooltest.cpp pipetest.cpp)
target_link_libraries(c_wwlibtest c_wwlib c_wwstub GTest::gtest GTest::gtest_main)
set_target_properties(c_wwlibtest PROPERTIES OUTPUT_NAME "WWLibTest")

include(GoogleTest)
gtest_discover_tests(c_wwlibtest DISCOVERY_TIMEOUT 20)