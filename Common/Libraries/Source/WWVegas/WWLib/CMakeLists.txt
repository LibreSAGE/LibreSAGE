# Target setup
add_library(c_wwlib)

target_sources(
  c_wwlib
  PRIVATE _convert.cpp
          _mono.cpp
          _mono.h
          # growmix.h
          always.h
          argv.h
          b64pipe.cpp
          b64pipe.h
          b64straw.cpp
          b64straw.h
          base64.cpp
          base64.h
          bfiofile.cpp
          bfiofile.h
          binheap.h
          bittype.h
          blit.cpp
          blit.h
          blitter.h
          blowfish.cpp
          blowfish.h
          blowpipe.cpp
          blowpipe.h
          blwstraw.cpp
          blwstraw.h
          bool.h
          borlandc.h
          bound.h
          bsearch.h
          bsurface.h
          buff.cpp
          buff.h
          bufffile.cpp
          bufffile.h
          callbackhook.h
          chunkio.cpp
          chunkio.h
          convert.cpp
          convert.h
          cpudetect.cpp
          cpudetect.h
          crc.cpp
          crc.h
          crcpipe.cpp
          crcpipe.h
          crcstraw.cpp
          crcstraw.h
          cstraw.cpp
          cstraw.h
          data.cpp
          data.h
          draw.cpp
          draw.h
          ffactory.cpp
          ffactory.h
          fixed.cpp
          fixed.h
          font.h
          gcd_lcm.cpp
          gcd_lcm.h
          hash.cpp
          hash.h
          hashcalc.h
          hashlist.h
          hashtemplate.h
          hsv.cpp
          hsv.h
          iff.h
          incdec.h
          index.h
          ini.cpp
          ini.h
          inisup.h
          int.cpp
          int.h
          iostruct.h
          jshell.cpp
          lcw.cpp
          lcw.h
          lcwpipe.cpp
          lcwpipe.h
          listnode.h
          load.cpp
          lzo.cpp
          lzo.h
          lzopipe.cpp
          lzopipe.h
          lzostraw.cpp
          lzostraw.h
          md5.cpp
          md5.h
          mempool.h
          misc.h
          mixfile.cpp
          mixfile.h
          mmsys.h
          mono.cpp
          mono.h
          monodrvr.h
          mpmath.cpp
          mpmath.h
          mpu.cpp
          mpu.h
          # msgloop.cpp msgloop.h
          multilist.cpp
          multilist.h
          mutex.cpp
          mutex.h
          noinit.h
          notify.h
          nstrdup.cpp
          nstrdup.h
          ntree.h
          palette.cpp
          palette.h
          pcx.cpp
          pcx.h
          pipe.cpp
          pipe.h
          pk.cpp
          pk.h
          pkpipe.cpp
          pkpipe.h
          pkstraw.cpp
          pkstraw.h
          point.h
          ramfile.cpp
          ramfile.h
          random.cpp
          random.h
          rawfile.cpp
          rawfile.h
          rc4.cpp
          rc4.h
          readline.cpp
          readline.h
          realcrc.cpp
          realcrc.h
          ref_ptr.h
          refcount.cpp
          refcount.h
          registry.cpp
          registry.h
          rgb.cpp
          rgb.h
          rle.cpp
          rle.h
          rlerle.h
          rndstraw.cpp
          rndstraw.h
          rndstrng.cpp
          rndstrng.h
          rng.h
          rsacrypt.h
          sampler.cpp
          sampler.h
          sha.cpp
          sha.h
          shapipe.cpp
          shapipe.h
          shapeset.h
          sharebuf.h
          signaler.h
          simplevec.h
          slist.h
          slnode.cpp
          slnode.h
          srandom.cpp
          srandom.h
          stimer.cpp
          stl.h
          straw.cpp
          straw.h
          strtok_r.cpp
          strtok_r.h
          surface.h
          surfrect.h
          systimer.cpp
          systimer.h
          tagblock.cpp
          tagblock.h
          targa.cpp
          targa.h
          textfile.cpp
          textfile.h
          thread.cpp
          thread.h
          timer.h
          trackwin.h
          trect.h
          trim.cpp
          trim.h
          uarray.h
          vector.cpp
          vector.h
          verchk.h
          visualc.h
          watcom.h
          widestring.cpp
          widestring.h
          win.cpp
          win.h
          wwfile.cpp
          wwfile.h
          wwfont.cpp
          wwstring.cpp
          wwstring.h
          xmouse.h
          xpipe.cpp
          xpipe.h
          xstraw.cpp
          xstraw.h
          xsurface.cpp
          xsurface.h)

if(WIN32)
  target_sources(
    c_wwlib
    PRIVATE ddraw.cpp
            dib.cpp
            dib.h
            dsurface.cpp
            dsurface.h
            keyboard.cpp
            keyboard.h
            launchweb.cpp
            launchweb.h
            rcfile.cpp
            rcfile.h
            wwcomutil.cpp
            wwcomutil.h)
endif()

set_target_properties(c_wwlib PROPERTIES OUTPUT_NAME "WWLib"
                                         POSITION_INDEPENDENT_CODE TRUE)

target_include_directories(c_wwlib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
find_package(DXVK CONFIG REQUIRED)
target_link_libraries(c_wwlib PUBLIC c_library_include c_wwdebug DXVK::d3d8)

# DirectDraw target_link_libraries(c_wwlib PRIVATE dxlib::ddraw)
# LZO Compression library
find_path(LZO_INCLUDE_DIR NAMES lzo/lzo1.h)
find_library(LZO_LIBRARY NAMES liblzo2.so)
target_include_directories(c_wwlib PRIVATE ${LZO_INCLUDE_DIR})
target_link_libraries(c_wwlib PRIVATE ${LZO_LIBRARY})

# SDL3 library
find_package(SDL3 CONFIG REQUIRED)
target_link_libraries(c_wwlib PRIVATE SDL3::SDL3)