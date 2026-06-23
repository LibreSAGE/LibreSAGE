add_subdirectory(WWDebug)
add_subdirectory(WWLib)
add_subdirectory(WWAudio)
add_subdirectory(WWDownload)
add_subdirectory(WWSaveLoad)
add_subdirectory(WWMath)
add_subdirectory(WW3D2)
add_subdirectory(WWStub)

# Create an interface library for WWVegas that links all the submodules together
add_library(c_wwvegas INTERFACE)
target_link_libraries(c_wwvegas INTERFACE c_wwdebug c_wwlib c_wwaudio c_wwdownload c_wwsaveload c_wwmath c_ww3d2)