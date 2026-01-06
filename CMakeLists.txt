cmake_minimum_required(VERSION 3.17)
project(Faveworm VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (APPLE)
  set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum macOS version")
  enable_language(OBJC)
  enable_language(OBJCXX)
endif()

include(FetchContent)

# =========================
# Fetch visage GUI library
# =========================
FetchContent_Declare(visage
  GIT_REPOSITORY https://github.com/VitalAudio/visage
  GIT_TAG main
)

set(VISAGE_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(VISAGE_BUILD_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(visage)

# =========================
# PortAudio
# =========================
if (NOT EMSCRIPTEN)
  if (APPLE)
    # On macOS, manual search for absolute paths is the most reliable
    find_library(PORTAUDIO_LIB portaudio REQUIRED)
    find_path(PORTAUDIO_INCLUDE_DIR portaudio.h REQUIRED)

    set(PORTAUDIO_TARGET ${PORTAUDIO_LIB})
    set(PORTAUDIO_INCLUDES ${PORTAUDIO_INCLUDE_DIR})
  else()
    find_package(portaudio CONFIG QUIET)
    if (portaudio_FOUND)
      set(PORTAUDIO_TARGET portaudio)
    else()
      find_package(PortAudio QUIET)
      if (PortAudio_FOUND)
        set(PORTAUDIO_TARGET PortAudio::PortAudio)
      else()
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(PORTAUDIO REQUIRED portaudio-2.0)
        set(PORTAUDIO_TARGET ${PORTAUDIO_LDFLAGS})
        set(PORTAUDIO_INCLUDES ${PORTAUDIO_INCLUDE_DIRS})
      endif()
    endif()
  endif()
endif()


# =========================
# Embed shaders
# =========================
file(GLOB_RECURSE SHADERS resources/shaders/[vf]s_*.sc)
add_compile_definitions(SHADERS_FOLDER="${CMAKE_CURRENT_SOURCE_DIR}/resources/shaders")
visage_embed_shaders(
  EmbeddedShaders
  "faveworm_shaders.h"
  "resources::shaders"
  "${SHADERS}"
)

# =========================
# Embed fonts
# =========================
file(GLOB_RECURSE FONTS resources/fonts/*.ttf)
add_embedded_resources(
  EmbeddedFonts
  "faveworm_fonts.h"
  "resources::fonts"
  "${FONTS}"
)

# =========================
# Sources
# =========================
file(GLOB_RECURSE SOURCE_FILES src/*.cpp)
file(GLOB_RECURSE HEADER_FILES src/*.h)

# =========================
# Embed icons (for UI/WASM use)
# =========================
file(GLOB_RECURSE ICONS resources/icons/*)
add_embedded_resources(
  EmbeddedIcons
  "faveworm_icons.h"
  "resources::icons"
  "${ICONS}"
)

# =========================
# App Bundle Icons
# =========================
set(APP_ICON_FILENAME "faveworm.icns")
set(APP_ICON_MACOS "${CMAKE_CURRENT_SOURCE_DIR}/resources/icons/${APP_ICON_FILENAME}")
set(APP_ICON_WIN   "${CMAKE_CURRENT_SOURCE_DIR}/resources/icons/faveworm.ico")

set(APP_RESOURCE_FILES "")
if (APPLE)
  set_source_files_properties(
    ${APP_ICON_MACOS}
    PROPERTIES MACOSX_PACKAGE_LOCATION Resources
  )
  list(APPEND APP_RESOURCE_FILES ${APP_ICON_MACOS})
elseif (WIN32)
  set(ICON_RC "${CMAKE_CURRENT_BINARY_DIR}/faveworm.rc")
  file(WRITE "${ICON_RC}" "IDI_ICON1 ICON DISCARDABLE \"${APP_ICON_WIN}\"\n")
  list(APPEND APP_RESOURCE_FILES ${ICON_RC})
endif()

add_executable(
  Faveworm
  ${SOURCE_FILES}
  ${HEADER_FILES}
  ${APP_RESOURCE_FILES}
)

target_include_directories(
  Faveworm
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# =========================
# Platform configuration
# =========================
if (WIN32)
  # Console app (no WIN32_EXECUTABLE)
elseif (APPLE)
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in
    ${CMAKE_CURRENT_BINARY_DIR}/Info.plist
  )

  set_target_properties(Faveworm PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_ICON_FILE "${APP_ICON_FILENAME}"
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_BINARY_DIR}/Info.plist
  )
elseif (EMSCRIPTEN)
  target_link_options(Faveworm PRIVATE
    -sGL_ENABLE_GET_PROC_ADDRESS
    -sALLOW_MEMORY_GROWTH
    -sUSE_SDL=2
    -sSTACK_SIZE=1MB
    --bind
    "-sEXPORTED_FUNCTIONS=['_main','_pasteCallback','_setVolume','_setParameter','_resizeWindow']"
    "-sEXPORTED_RUNTIME_METHODS=['ccall','cwrap','UTF8ToString']"
  )

  set_target_properties(Faveworm PROPERTIES
    SUFFIX ".html"
    OUTPUT_NAME "index"
  )
endif()

# =========================
# Link core targets
# =========================
target_link_libraries(Faveworm PRIVATE
  visage
  EmbeddedShaders
  EmbeddedFonts
  EmbeddedIcons
)

# =========================
# Audio linking
# =========================
if (NOT EMSCRIPTEN)
  target_link_libraries(Faveworm PRIVATE ${PORTAUDIO_TARGET})
  target_include_directories(Faveworm PRIVATE ${PORTAUDIO_INCLUDES})

  if (APPLE)
    target_link_libraries(Faveworm PRIVATE
      "-framework AudioToolbox"
      "-framework CoreAudio"
      "-framework CoreFoundation"
    )
  endif()
endif()
