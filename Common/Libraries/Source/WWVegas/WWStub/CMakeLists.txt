# Target setup
add_library(c_wwstub)

target_sources(
  c_wwstub
  PRIVATE wwallocstub.cpp
          wwdebugstub.cpp)

target_link_libraries(c_wwstub PUBLIC c_wwlib)