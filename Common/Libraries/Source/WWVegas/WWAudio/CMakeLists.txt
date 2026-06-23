# Target setup
add_library(c_wwaudio)

target_sources(
  c_wwaudio
  PRIVATE aabtreesoundcullclass.h
          audiblesound.cpp
          audiblesound.h
          audioevents.h
          audiosaveload.cpp
          audiosaveload.h
          filteredsound.cpp
          filteredsound.h
          listener.cpp
          listener.h
          listenerhandle.cpp
          listenerhandle.h
          logicallistener.cpp
          logicallistener.h
          logicalsound.cpp
          logicalsound.h
          priorityvector.h
          sound3d.cpp
          sound3d.h
          sound2dhandle.cpp
          sound2dhandle.h
          sound3dhandle.cpp
          sound3dhandle.h
          soundbuffer.cpp
          soundbuffer.h
          soundchunkids.h
          soundcullobj.h
          soundhandle.cpp
          soundhandle.h
          soundpseudo3d.cpp
          soundpseudo3d.h
          soundscene.cpp
          soundscene.h
          soundsceneobj.cpp
          soundsceneobj.h
          soundstreamhandle.cpp
          soundstreamhandle.h
          threads.cpp
          threads.h
          utils.cpp
          utils.h
          wwaudio.cpp
          wwaudio.h)

set_target_properties(c_wwaudio PROPERTIES OUTPUT_NAME "WWAudio")

target_include_directories(c_wwaudio PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(c_wwaudio PUBLIC c_library_include c_wwlib c_wwmath
                                       c_wwsaveload c_ww3d2)

if(SAGE_USE_MILES)
  target_compile_definitions(c_wwaudio PUBLIC SAGE_USE_MILES)
endif()
