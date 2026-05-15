add_executable(c_crcdiff)

target_sources(
	c_crcdiff
	PRIVATE CRCDiff.cpp
            debug.cpp
            expander.cpp
            KVPair.cpp
            debug.h
            expander.h
            KVPair.h
            misc.h)

set_target_properties(c_crcdiff PROPERTIES OUTPUT_NAME "CRCDiff")

target_link_libraries(c_crcdiff PRIVATE c_compat)
