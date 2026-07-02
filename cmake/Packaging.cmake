# CPack packaging: Generals and Zero Hour ship as separate components, each a
# self-contained tree (game binary + the shared Common tools + bundled libraries).
#
# This file is included from the top-level CMakeLists.txt *after* all targets are
# defined so it can reference both the game and the Common/Tools targets.

set(CPACK_PACKAGE_NAME "CnC-Generals-Zero-Hour")
set(CPACK_PACKAGE_VENDOR "CnC_Generals_Zero_Hour contributors")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Command & Conquer: Generals and Zero Hour")
# Version of the port, not of the retail games (Generals 1.08 / Zero Hour 1.04)
set(CPACK_PACKAGE_VERSION "0.1.0")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE.md")
set(CPACK_PACKAGE_CHECKSUM SHA256)
set(CPACK_STRIP_FILES ON)

if(WIN32)
    set(CPACK_GENERATOR "ZIP")
else()
    set(CPACK_GENERATOR "TGZ")
endif()

# One archive per component (e.g. ...-Generals.tar.gz and ...-ZeroHour.tar.gz).
set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)

set(CPACK_COMPONENTS_ALL Generals ZeroHour)

set(CPACK_COMPONENT_GENERALS_DISPLAY_NAME "Command & Conquer: Generals")
set(CPACK_COMPONENT_GENERALS_DESCRIPTION
    "The base game. Requires the game data of the original release.")
set(CPACK_COMPONENT_ZEROHOUR_DISPLAY_NAME "Command & Conquer: Generals - Zero Hour")
set(CPACK_COMPONENT_ZEROHOUR_DESCRIPTION
    "The Zero Hour expansion. Requires the game data of the original release.")

# -----------------------------------------------------------------------------
# Shared Common tools shipped with both games. Only the ones actually configured
# (SAGE_BUILD_TOOLS, platform availability) are packaged.
set(_sage_common_tools "")
foreach(_tool c_crcdiff c_compress c_imagepacker c_w3dview c_wdump)
    if(TARGET ${_tool})
        list(APPEND _sage_common_tools ${_tool})
    endif()
endforeach()

# Install a game executable plus the Common tools into one component, bundling
# their non-system shared-library dependencies into <root>/lib. Everything is
# installed at the archive root (DESTINATION ".") so the tarball has no redundant
# top-level directory -- the archive is already named after the component.
function(sage_install_component component game_target depset)
    set(_targets ${game_target} ${_sage_common_tools})

    # Relocatable: load the bundled libraries from lib/ next to the executables.
    if(APPLE)
        set_target_properties(${_targets} PROPERTIES INSTALL_RPATH "@loader_path/lib")
    elseif(UNIX)
        set_target_properties(${_targets} PROPERTIES INSTALL_RPATH "$ORIGIN/lib")
    endif()

    install(
        TARGETS ${_targets}
        RUNTIME_DEPENDENCY_SET ${depset}
        RUNTIME DESTINATION .
        COMPONENT ${component})

    # Bundle the shared libraries (SDL3, DXVK, FFmpeg, Qt, ...) into lib/.
    # System libraries are expected to come from the host.
    install(
        RUNTIME_DEPENDENCY_SET ${depset}
        PRE_EXCLUDE_REGEXES
            "^ld-linux"
            "^lib(c|m|dl|rt|pthread|gcc_s|stdc\\+\\+)\\.so"
        POST_EXCLUDE_REGEXES
            "^/(lib|lib64|usr/lib|usr/lib64|usr/local/lib)/"
            "^/System/"
        RUNTIME DESTINATION .
        LIBRARY DESTINATION lib
        COMPONENT ${component})
endfunction()

sage_install_component(Generals g_rts_sdl g_pkg_deps)
sage_install_component(ZeroHour z_rts_sdl z_pkg_deps)

include(CPack)
