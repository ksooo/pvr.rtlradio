cmake_minimum_required(VERSION 3.12)
project(pvr.rtlradio)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)

option(DISTRIBUTION_BUILD "Linux distribution build (without use of depends folder)" OFF)
if(CORE_SYSTEM_NAME STREQUAL windowsstore OR
   CORE_SYSTEM_NAME STREQUAL darwin_embedded OR
   CORE_SYSTEM_NAME STREQUAL android)
  set(USB_DEVICE_SUPPORT False)
else()
  option(USB_DEVICE_SUPPORT "Build add-on with USB device support, if set to \"OFF\" RTL-SDR can only be accessed via network" ON)
endif()

# Set values used about configure of settings.xml.in on add-on
if(NOT USB_DEVICE_SUPPORT)
  set(SETTINGS_XML_IN_CONNECTION_TYPE_NOTE "USB device not supported on this add-on, due to this the following setting is not usable and marked as not visible.")
  set(SETTINGS_XML_IN_CONNECTION_TYPE_DEFAULT "1")
  set(SETTINGS_XML_IN_CONNECTION_TYPE_VISIBLE "false")
  set(SETTINGS_XML_IN_NO_USB_BEGIN "<!--")
  set(SETTINGS_XML_IN_NO_USB_END "-->")
else()
  set(SETTINGS_XML_IN_CONNECTION_TYPE_NOTE "USB device supported on this add-on.")
  set(SETTINGS_XML_IN_CONNECTION_TYPE_DEFAULT "0")
  set(SETTINGS_XML_IN_CONNECTION_TYPE_VISIBLE "true")
  set(SETTINGS_XML_IN_NO_USB_BEGIN "")
  set(SETTINGS_XML_IN_NO_USB_END "")
endif()

find_package(Kodi REQUIRED)
find_package(FFTW REQUIRED)
find_package(FDK_AAC REQUIRED)
find_package(glm REQUIRED)
find_package(MPG123 REQUIRED)
find_package(RapidJSON 1.1.0 REQUIRED)
find_package(SQLite3 REQUIRED)
if(USB_DEVICE_SUPPORT)
  find_package(libusb-1.0 REQUIRED)
  find_package(rtlsdr REQUIRED)
endif()

if(NOT WIN32 AND (APP_RENDER_SYSTEM STREQUAL "gl" OR NOT APP_RENDER_SYSTEM))
  find_package(OpenGl REQUIRED)
  set(DEPLIBS ${OPENGL_LIBRARIES})
  set(includes ${OPENGL_INCLUDE_DIR})
  add_definitions(${OPENGL_DEFINITIONS})
else()
  find_package(OpenGLES REQUIRED)
  set(DEPLIBS ${OPENGLES_LIBRARIES})
  set(includes ${OPENGLES_INCLUDE_DIR})
  add_definitions(${OPENGLES_DEFINITIONS})
endif()

add_definitions(-DDABLIN_AAC_FDKAAC -DFFTW_NO_Complex -Drtlsdr_EXPORTS=1)

include_directories(${KODI_INCLUDE_DIR}/.. # Hack way with "/..", need bigger Kodi cmake rework to match right include ways
                    ${FFTW_INCLUDE_DIRS}
                    ${FDK_AAC_INCLUDE_DIRS}
                    ${GLM_INCLUDE_DIR}
                    ${MPG123_INCLUDE_DIR}
                    ${RAPIDJSON_INCLUDE_DIRS}
                    ${SQLITE3_INCLUDE_DIR})

list(APPEND DEPLIBS ${FDK_AAC_LIBRARIES}
                    ${FFTW_LONGDOUBLE_LIB}
                    ${MPG123_LIBRARIES}
                    ${SQLITE3_LIBRARY})

if(USB_DEVICE_SUPPORT)
  if(NOT DISTRIBUTION_BUILD)
    list(APPEND DEPLIBS ${LIBUSB_1_LIBRARIES})
  endif()

  include_directories(${LIBUSB_1_INCLUDE_DIRS}
                      ${rtlsdr_INCLUDE_DIRS})
  list(APPEND DEPLIBS ${RTLSDR_LIBRARIES})

  add_definitions(-DUSB_DEVICE_SUPPORT)
endif()

add_subdirectory(${PROJECT_SOURCE_DIR}/src)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/dsp_dab)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/dsp_dab/decoders)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/dsp_dab/decoders/audio)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/dsp_dab/decoders/audio/aac)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/dsp_dab/decoders/audio/mp2)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/dsp_dab/decoders/data/pad)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/dsp_dab/decoders/data/mot)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/dsp_fm)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/dsp_hd)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/gui)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/exception_control)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/utils)

list(APPEND DEPLIBS code_src
                    code_src_dsp_dab
                    code_src_dsp_dab_decoders
                    code_src_dsp_dab_decoders_audio
                    code_src_dsp_dab_decoders_audio_aac
                    code_src_dsp_dab_decoders_audio_mp2
                    code_src_dsp_dab_decoders_data_mot
                    code_src_dsp_dab_decoders_data_pad
                    code_src_dsp_fm
                    code_src_dsp_hd
                    code_src_gui
                    code_src_utils
                    code_src_exception_control
                    $<$<BOOL:${WIN32}>:ws2_32>
                    $<$<BOOL:${WIN32}>:Shlwapi>)

set(PVRRTLRADIO_SOURCES src/addon.cpp)
set(PVRRTLRADIO_HEADERS src/addon.h)

build_addon(${CMAKE_PROJECT_NAME} PVRRTLRADIO DEPLIBS)

target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE "VERSION_PRODUCTNAME_ANSI=\"${CMAKE_PROJECT_NAME}\"")
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE "VERSION_VERSION3_ANSI=\"${PVRRTLRADIO_VERSION}\"")
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/src)

include(CPack)
