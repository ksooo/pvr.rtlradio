find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
  pkg_check_modules(PC_FDK_AAC fdk-aac QUIET)
endif()

find_path(FDK_AAC_INCLUDE_DIRS FDK_audio.h
                              PATHS ${PC_FDK_AAC_INCLUDEDIR}
                              PATH_SUFFIXES fdk-aac)
find_library(FDK_AAC_LIBRARIES fdk-aac
                              PATHS ${PC_FDK_AAC_LIBDIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(fdk-aac REQUIRED_VARS FDK_AAC_LIBRARIES FDK_AAC_INCLUDE_DIRS)

mark_as_advanced(FDK_AAC_INCLUDE_DIRS FDK_AAC_LIBRARIES)
