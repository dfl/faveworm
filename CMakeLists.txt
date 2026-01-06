cmake_minimum_required(VERSION 3.17)
project(Faveworm VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (APPLE)
  enable_language(OBJC)
  enable_language(OBJCXX)
endif ()

include(FetchContent)

# Fetch visage GUI library
FetchContent_Declare(visage
  GIT_REPOSITORY https://github.com/VitalAudio/visage
  GIT_TAG main
)

set(VISAGE_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(VISAGE_BUILD_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(visage)

# Find PortAudio for audio playback
if (NOT EMSCRIPTEN)
  # Try finding via Config (vcpkg)
  find_package(portaudio CONFIG QUIET)
  if (portaudio_FOUND)
    set(PORTAUDIO_LIBRARIES portaudio)
  else()
    # Try legacy global find (custom module)
    find_package(PortAudio QUIET)
    if (PortAudio_FOUND)
      set(PORTAUDIO_LIBRARIES PortAudio::PortAudio)
    else()
      # Fallback to PkgConfig (Linux/macOS)
      find_package(PkgConfig REQUIRED)
      pkg_check_modules(PORTAUDIO REQUIRED portaudio-2.0)
    endif()
  endif()
endif ()

# Embed shaders
file(GLOB_RECURSE SHADERS resources/shaders/[vf]s_*.sc)
add_compile_definitions(SHADERS_FOLDER="${CMAKE_CURRENT_SOURCE_DIR}/resources/shaders")
visage_embed_shaders(EmbeddedShaders "faveworm_shaders.h" "resources::shaders" "${SHADERS}")

# Embed fonts
file(GLOB_RECURSE FONTS resources/fonts/*.ttf)
add_embedded_resources(EmbeddedFonts "faveworm_fonts.h" "resources::fonts" "${FONTS}")

# Source files
file(GLOB_RECURSE SOURCE_FILES src/*.cpp)
file(GLOB_RECURSE HEADER_FILES src/*.h)

# Icons
set(APP_ICON_MACOS "${CMAKE_CURRENT_SOURCE_DIR}/resources/icons/faveworm.icns")
set(APP_ICON_WIN "${CMAKE_CURRENT_SOURCE_DIR}/resources/icons/faveworm.ico")

if (APPLE)
  set_source_files_properties(${APP_ICON_MACOS} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
endif()

add_executable(Faveworm ${SOURCE_FILES} ${HEADER_FILES} ${APP_ICON_MACOS} ${APP_ICON_WIN})

target_include_directories(Faveworm PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

if (WIN32)
  set_target_properties(Faveworm PROPERTIES WIN32_EXECUTABLE YES)
elseif (APPLE)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in ${CMAKE_CURRENT_BINARY_DIR}/Info.plist)
  set_target_properties(Faveworm PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_ICON_FILE "faveworm.icns"
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_BINARY_DIR}/Info.plist
  )
elseif (EMSCRIPTEN)
  target_link_options(Faveworm
    PRIVATE
    -sGL_ENABLE_GET_PROC_ADDRESS
    -sALLOW_MEMORY_GROWTH
    -sUSE_SDL=2
    -sSTACK_SIZE=1MB
    --bind
    "-sEXPORTED_FUNCTIONS=['_main', '_pasteCallback', '_setVolume', '_setParameter', '_resizeWindow']"
    "-sEXPORTED_RUNTIME_METHODS=['ccall', 'cwrap', 'UTF8ToString']"
  )
  set_target_properties(Faveworm PROPERTIES
    SUFFIX ".html"
    OUTPUT_NAME "index"
  )
endif ()

target_link_libraries(Faveworm PRIVATE
  visage
  EmbeddedShaders
  EmbeddedFonts
)

# Link audio libraries
if (EMSCRIPTEN)
  # SDL2 audio handled by link options above
else()
  target_link_libraries(Faveworm PRIVATE ${PORTAUDIO_LIBRARIES})
  target_include_directories(Faveworm PRIVATE ${PORTAUDIO_INCLUDE_DIRS})
  if (APPLE)
    target_link_libraries(Faveworm PRIVATE
      "-framework AudioToolbox"
      "-framework CoreAudio"
      "-framework CoreFoundation"
    )
  endif()
endif()
