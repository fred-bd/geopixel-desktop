cmake_minimum_required(VERSION 3.11)

project(geopx-desktop)

#
# main variables for the build script
#

# variables that control the system versioning
set(GEOPIXELDESKTOP_VERSION_MAJOR 1)
set(GEOPIXELDESKTOP_VERSION_MINOR 0)
set(GEOPIXELDESKTOP_VERSION_PATCH 0)
set(GEOPIXELDESKTOP_VERSION_STATUS "" CACHE STRING "Define name of installer")
set(GEOPIXELDESKTOP_BUILD_NUMBER "" CACHE STRING "Number of build in Jenkins")
set(GEOPIXELDESKTOP_VERSION_STRING "${GEOPIXELDESKTOP_VERSION_MAJOR}.${GEOPIXELDESKTOP_VERSION_MINOR}.${GEOPIXELDESKTOP_VERSION_PATCH}")
set(GEOPIXELDESKTOP_PKG_VERSION ${GEOPIXELDESKTOP_VERSION_MAJOR}.${GEOPIXELDESKTOP_VERSION_MINOR}.${GEOPIXELDESKTOP_VERSION_PATCH})
set(GEOPIXELDESKTOP_VERSION "0x010000")
# TerraLib
set(GEOPIXELDESKTOP_TERRALIB_VERSION_MAJOR 5)
set(GEOPIXELDESKTOP_TERRALIB_VERSION_MINOR 4)
set(GEOPIXELDESKTOP_TERRALIB_VERSION_PATCH 0)

if(NOT DEFINED GEOPIXELDESKTOP_TERRALIB_VERSION_STRING OR
  GEOPIXELDESKTOP_TERRALIB_VERSION_STRING STREQUAL "")
  set(GEOPIXELDESKTOP_TERRALIB_VERSION_STRING "${GEOPIXELDESKTOP_TERRALIB_VERSION_MAJOR}.${GEOPIXELDESKTOP_TERRALIB_VERSION_MINOR}.${GEOPIXELDESKTOP_TERRALIB_VERSION_PATCH}")
endif()

if(GEOPIXELDESKTOP_VERSION_STATUS STREQUAL "build")
  set(GEOPIXELDESKTOP_VERSION_STATUS "${GEOPIXELDESKTOP_VERSION_STATUS}-${GEOPIXELDESKTOP_BUILD_NUMBER}")
endif()

if(GEOPIXELDESKTOP_VERSION_STATUS STREQUAL "")
  set(GEOPIXELDESKTOP_VERSION_STRING "${GEOPIXELDESKTOP_VERSION_MAJOR}.${GEOPIXELDESKTOP_VERSION_MINOR}.${GEOPIXELDESKTOP_VERSION_PATCH}")
else()
  set(GEOPIXELDESKTOP_VERSION_STRING "${GEOPIXELDESKTOP_VERSION_MAJOR}.${GEOPIXELDESKTOP_VERSION_MINOR}.${GEOPIXELDESKTOP_VERSION_PATCH}-${GEOPIXELDESKTOP_VERSION_STATUS}")
endif()


# variables that controls what is built
if(NOT DEFINED GEOPIXELDESKTOP_BUILD_AS_BUNDLE)
  set(GEOPIXELDESKTOP_BUILD_AS_BUNDLE 0 CACHE BOOL "If on, tells that the build will generate a bundle")
endif()

# Geopixel Portal Data Source Information
#set(GEOPIXEL_PORTAL_USER "" CACHE STRING "")
#set(GEOPIXEL_PORTAL_PASSWD "" CACHE STRING "")
#set(GEOPIXEL_PORTAL_URL "" CACHE STRING "")
#set(GEOPIXEL_PORTAL_PORT "" CACHE STRING "")
#set(GEOPIXEL_PORTAL_DATABASE "" CACHE STRING "")
#set(GEOPIXEL_PORTAL_ALIAS "" CACHE STRING "")

set(GEOPIXELDESKTOP_INTERNAL_BUILD 0 CACHE BOOL "If on, enable internal operations.")

get_filename_component(GEOPIXELDESKTOP_ABSOLUTE_ROOT_DIR "../../" ABSOLUTE)


#
# global definitions and includes
#
add_definitions(-D_SCL_SECURE_NO_WARNINGS -DBOOST_ALL_NO_LIB -DBOOST_FILESYSTEM_VERSION=3 -DBOOST_UBLAS_TYPE_CHECK=0 -DTERRALIB_LOGGER_INFO_ENABLED -DTERRALIB_LOGGER_WARN_ENABLED -DTERRALIB_LOGGER_ERROR_ENABLED -DTERRALIB_LOGGER_FATAL_ENABLED)

if(WIN32)
  add_definitions(-DBOOST_ALL_DYN_LINK)
endif()

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX OR (CMAKE_CXX_COMPILER MATCHES ".*clang") OR (CMAKE_CXX_COMPILER_ID STREQUAL "Clang"))
#  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif ()

# Note: automatically link executables with qtmain.lib on Windows when it has Qt dependency.
if("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}" VERSION_GREATER  2.8.9)
  cmake_policy(SET CMP0020 NEW)
endif()

if(${CMAKE_MAJOR_VERSION} GREATER 2)
  cmake_policy(SET CMP0007 NEW)
  cmake_policy(SET CMP0026 OLD)
  cmake_policy(SET CMP0042 NEW)
  cmake_policy(SET CMP0043 NEW)
  cmake_policy(SET CMP0045 OLD)
endif()

if(${CMAKE_VERSION} VERSION_GREATER "3.1.2")
  cmake_policy(SET CMP0053 OLD)
endif()

#
# Build will generate files into a single folder
#
if((CMAKE_GENERATOR STREQUAL Xcode) OR MSVC)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
else()
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif()

#
# global configurations
#
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/modules")

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if(MSVC)
  set(CMAKE_DEBUG_POSTFIX "d")
  add_definitions(/MP)
endif()

set(GEOPIXELDESKTOP_INSTALL_PREFIX .)

#
# first check of libraries
#

# Finding Required TerraLib version
find_package(terralib ${GEOPIXELDESKTOP_TERRALIB_VERSION_MAJOR}.${GEOPIXELDESKTOP_TERRALIB_VERSION_MINOR}.${GEOPIXELDESKTOP_TERRALIB_VERSION_PATCH} REQUIRED COMPONENTS qt_apf)

find_package(Boost 1.53.0 REQUIRED unit_test_framework)

find_package(Qt5 5.1 REQUIRED COMPONENTS Core Gui Widgets PrintSupport)

find_package(CURL REQUIRED)

find_package(GDAL QUIET)

find_package(terralib_layout QUIET)

find_package(Qt5LinguistTools QUIET)

find_package(Doxygen QUIET)

if(NOT QT_QCOLLECTIONGENERATOR_EXECUTABLE)
  find_program(QT_QCOLLECTIONGENERATOR_EXECUTABLE
               NAME qcollectiongenerator
               PATHS /usr
                     /usr/local
                     ${Qt5_DIR}/../../..
               PATH_SUFFIXES bin)
endif()

add_subdirectory(geopixeltools)

add_subdirectory(geopixeldesktop)

if(QT_QCOLLECTIONGENERATOR_EXECUTABLE)
  OPTION ( GEOPIXELDESKTOP_QHELP_ENABLED  "Enable Qt-Help build?" OFF )
endif()

if(GEOPIXELDESKTOP_QHELP_ENABLED)
  add_subdirectory(qhelp)
endif()

configure_file(${GEOPIXELDESKTOP_ABSOLUTE_ROOT_DIR}/COPYING ${CMAKE_BINARY_DIR}/COPYING.txt COPYONLY)

set(CPACK_SOURCE_INSTALLED_DIRECTORIES "${GEOPIXELDESKTOP_ABSOLUTE_ROOT_DIR};/")

configure_file(${GEOPIXELDESKTOP_ABSOLUTE_ROOT_DIR}/src/Version.h.in
               ${CMAKE_BINARY_DIR}/GeopixelDesktop_Version.h @ONLY)
			   
configure_file(${GEOPIXELDESKTOP_ABSOLUTE_ROOT_DIR}/src/Config.h.in
               ${CMAKE_BINARY_DIR}/GeopixelDesktop_Config.h @ONLY)
			   
#configure_file(${GEOPIXELDESKTOP_ABSOLUTE_ROOT_DIR}/src/Geopixel.h.in
#               ${CMAKE_BINARY_DIR}/Geopixel.h @ONLY)


#
# Share folder
#
install(
      DIRECTORY ${terralib_INCLUDE_DIRS}/../../share/
      DESTINATION ${GEOPIXELDESKTOP_INSTALL_PREFIX}/share
      COMPONENT runtime
    )		   

if(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/config.json)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/config.json DESTINATION bin COMPONENT runtime) 
endif()


#
# Layout Plugin
#			
MACRO(TeInstallLayoutAuxFiles)
  if(EXISTS ${terralib_layout_DIR}/CMakeCache.txt) #Build tree
    install(
      DIRECTORY ${terralib_layout_INCLUDE_DIRS}/../share/
      DESTINATION ${GEOPIXELDESKTOP_INSTALL_PREFIX}/share
      COMPONENT runtime
    )
  else()                                           #Install tree
    install(
      DIRECTORY ${terralib_layout_DIR}/../../../share/
      DESTINATION ${GEOPIXELDESKTOP_INSTALL_PREFIX}/share
      COMPONENT runtime
    )
  endif()
ENDMACRO(TeInstallLayoutAuxFiles)

MACRO(TeLayoutInstallPlugins plugins location)

  set(_files)
  set(_filesd)

  foreach(plugin ${plugins})
    get_target_property(_loc ${plugin} LOCATION_RELEASE)

    if(EXISTS ${_loc})
        list(APPEND _files ${_loc})
    endif()
  endforeach()

  install(FILES ${_files}
           DESTINATION ${location}
           CONFIGURATIONS Release
           COMPONENT runtime
           )

  foreach(plugin ${plugins})
    get_target_property(_loc ${plugin} LOCATION_DEBUG)

    if(EXISTS ${_loc})
        list(APPEND _filesd ${_loc})
    endif()
  endforeach()

  install(FILES ${_filesd}
           DESTINATION ${location}
           CONFIGURATIONS Debug
           COMPONENT runtime
           )

  TeInstallLayoutAuxFiles()
ENDMACRO(TeLayoutInstallPlugins)
			
if(terralib_layout_FOUND AND GEOPIXELDESKTOP_BUILD_AS_BUNDLE)
    TeLayoutInstallPlugins ("terralib_mod_layout;terralib_mod_qt_plugins_layout" ${GEOPIXELDESKTOP_INSTALL_PREFIX}/bin)
endif()
			   
#
# CPack
#
include(InstallRequiredSystemLibraries)

configure_file(${CMAKE_SOURCE_DIR}/geopixeldesktop-cpack-options.cmake.in
               ${CMAKE_BINARY_DIR}/geopixeldesktopCPackOptions.cmake @ONLY)

set(CPACK_PROJECT_CONFIG_FILE ${CMAKE_BINARY_DIR}/geopixeldesktopCPackOptions.cmake)

include(CPack)

# defining installation types, components and groups
cpack_add_install_type(full DISPLAY_NAME "Complete")
cpack_add_install_type(minimalistic DISPLAY_NAME "Minimalistic")

cpack_add_component_group(general_group
                          DISPLAY_NAME "Application"
                          DESCRIPTION "Geopixel Desktop executables"
                          EXPANDED)

cpack_add_component_group(devel_group
                          DISPLAY_NAME "Documentation"
                          DESCRIPTION "Development documentation files"
                          EXPANDED)

cpack_add_component(runtime
                    DISPLAY_NAME "Geopixel Desktop"
                    DESCRIPTION "Install necessary shared libraries (or DLLs) and executables"
					REQUIRED
                    GROUP general_group
                    INSTALL_TYPES full minimalistic)

cpack_add_component(devel
                    DISPLAY_NAME "Documentation"
                    DESCRIPTION "Install additional documentation files"
                    GROUP devel_group
                    INSTALL_TYPES full)
