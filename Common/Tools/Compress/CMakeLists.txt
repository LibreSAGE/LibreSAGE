add_executable(c_compress)

target_sources(c_compress PRIVATE Compress.cpp ../CRCDiff/debug.cpp ../CRCDiff/debug.h)

target_include_directories(
	c_compress PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../Libraries/Source
										${CMAKE_CURRENT_SOURCE_DIR}/../../Libraries/Include)

set_target_properties(c_compress PROPERTIES OUTPUT_NAME "Compress")

target_link_libraries(c_compress PRIVATE c_compression c_compat)
