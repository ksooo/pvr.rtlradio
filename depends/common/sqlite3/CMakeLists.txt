cmake_minimum_required(VERSION 3.5)

project(sqlite3 VERSION 3.13.0 LANGUAGES C)

file(WRITE cmake/sqlite3-config.cmake "include(\${CMAKE_CURRENT_LIST_DIR}/sqlite3.cmake)")

add_library(sqlite3
  src/sqlite3.h
  src/sqlite3ext.h
  src/sqlite3.c
)

if(CMAKE_SYSTEM_NAME STREQUAL WindowsStore)
  target_compile_definitions(sqlite3
    PRIVATE
    SQLITE_OS_WINRT
  )
endif()

target_include_directories(
  sqlite3 PRIVATE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  INTERFACE
  $<INSTALL_INTERFACE:include>
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/sqlite3-config-version.cmake
  VERSION ${sqlite_VERSION}
  COMPATIBILITY AnyNewerVersion
)

install(TARGETS sqlite3 EXPORT sqlite3
  RUNTIME DESTINATION bin
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
)

install(FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/src/sqlite3.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/sqlite3ext.h
  DESTINATION include
)

install(EXPORT sqlite3
  FILE
    sqlite3.cmake
  NAMESPACE
    sqlite3::
  DESTINATION
    lib/cmake/sqlite3
)
install(
  FILES
    cmake/sqlite3-config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/sqlite3-config-version.cmake
  DESTINATION
    lib/cmake/sqlite3
)

if(MSVC AND BUILD_SHARED_LIBS)
  install(FILES
    $<TARGET_PDB_FILE:sqlite3>
    DESTINATION lib
  )
endif()
