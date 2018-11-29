# geopx-desktop/src/geopixeldesktop
file(GLOB GEOPIXELDESKTOP_HDR_FILES ${GEOPIXELDESKTOP_ABSOLUTE_ROOT_DIR}/src/geopixeldesktop/*.h)
file(GLOB GEOPIXELDESKTOP_SRC_FILES ${GEOPIXELDESKTOP_ABSOLUTE_ROOT_DIR}/src/geopixeldesktop/*.cpp)
file(GLOB GEOPIXELDESKTOP_UI_FILES  ${GEOPIXELDESKTOP_ABSOLUTE_ROOT_DIR}/src/geopixeldesktop/ui/*.ui)
source_group("Header Files\\desktop"  FILES ${GEOPIXELDESKTOP_HDR_FILES})
source_group("Source Files\\desktop"  FILES ${GEOPIXELDESKTOP_SRC_FILES})
source_group("UI Files\\desktop"  FILES ${GEOPIXELDESKTOP_UI_FILES})

file(GLOB GEOPIXELDESKTOP_DATASOURCE_HDR_FILES ${GEOPIXELDESKTOP_ABSOLUTE_ROOT_DIR}/src/geopixeldesktop/datasource/*.h)
file(GLOB GEOPIXELDESKTOP_DATASOURCE_SRC_FILES ${GEOPIXELDESKTOP_ABSOLUTE_ROOT_DIR}/src/geopixeldesktop/datasource/*.cpp)
file(GLOB GEOPIXELDESKTOP_DATASOURCE_UI_FILES  ${GEOPIXELDESKTOP_ABSOLUTE_ROOT_DIR}/src/geopixeldesktop/datasource/ui/*.ui)
source_group("Header Files\\desktop\\datasource"  FILES ${GEOPIXELDESKTOP_DATASOURCE_HDR_FILES})
source_group("Source Files\\desktop\\datasource"  FILES ${GEOPIXELDESKTOP_DATASOURCE_SRC_FILES})
source_group("UI Files\\desktop\\datasource"  FILES ${GEOPIXELDESKTOP_DATASOURCE_UI_FILES})

file(GLOB GEOPIXELDESKTOP_LAYER_HDR_FILES ${GEOPIXELDESKTOP_ABSOLUTE_ROOT_DIR}/src/geopixeldesktop/layer/*.h)
file(GLOB GEOPIXELDESKTOP_LAYER_SRC_FILES ${GEOPIXELDESKTOP_ABSOLUTE_ROOT_DIR}/src/geopixeldesktop/layer/*.cpp)
source_group("Header Files\\desktop\\layer"  FILES ${GEOPIXELDESKTOP_LAYER_HDR_FILES})
source_group("Source Files\\desktop\\layer"  FILES ${GEOPIXELDESKTOP_LAYER_SRC_FILES})
file(GLOB GEOPIXELDESKTOP_LAYER_SERIALIZE_HDR_FILES ${GEOPIXELDESKTOP_ABSOLUTE_ROOT_DIR}/src/geopixeldesktop/layer/serialization/*.h)
file(GLOB GEOPIXELDESKTOP_LAYER_SERIALIZE_SRC_FILES ${GEOPIXELDESKTOP_ABSOLUTE_ROOT_DIR}/src/geopixeldesktop/layer/serialization/*.cpp)
source_group("Header Files\\desktop\\layer\\serialize"  FILES ${GEOPIXELDESKTOP_LAYER_SERIALIZE_HDR_FILES})
source_group("Source Files\\desktop\\layer\\serialize"  FILES ${GEOPIXELDESKTOP_LAYER_SERIALIZE_SRC_FILES})

include_directories(
  SYSTEM ${GEOPIXELDESKTOP_ABSOLUTE_ROOT_DIR}/src
  SYSTEM ${GEOPIXELDESKTOP_ABSOLUTE_ROOT_DIR}/src/geopixeldesktop
  SYSTEM ${GEOPIXELDESKTOP_ABSOLUTE_ROOT_DIR}/src/geopixeldesktop/datasource
  SYSTEM ${terralib_INCLUDE_DIRS}
  SYSTEM ${terralib_DIR}
  SYSTEM ${Boost_INCLUDE_DIR}
  SYSTEM ${CURL_INCLUDE_DIRS}
  SYSTEM ${CMAKE_BINARY_DIR}
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

#
#  Threating  warning  as  errors.
#
if  ("${CMAKE_CXX_COMPILER_ID}"  STREQUAL  "GNU")
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS}  -Werror")
elseif(CMAKE_GENERATOR  MATCHES  "Visual  Studio")
    SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS}  -WX")
    SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS}  -WX")
endif()

# uic'ing
QT5_WRAP_UI(GEOPIXELDESKTOP_GEN_HDR_FILES ${GEOPIXELDESKTOP_UI_FILES} ${GEOPIXELDESKTOP_DATASOURCE_UI_FILES})


set(GEOPIXELDESKTOP_FILES ${GEOPIXELDESKTOP_HDR_FILES} ${GEOPIXELDESKTOP_SRC_FILES} ${GEOPIXELDESKTOP_GEN_HDR_FILES}
                          ${GEOPIXELDESKTOP_DATASOURCE_HDR_FILES} ${GEOPIXELDESKTOP_DATASOURCE_SRC_FILES}
						  ${GEOPIXELDESKTOP_LAYER_HDR_FILES} ${GEOPIXELDESKTOP_LAYER_SRC_FILES}
						  ${GEOPIXELDESKTOP_LAYER_SERIALIZE_HDR_FILES} ${GEOPIXELDESKTOP_LAYER_SERIALIZE_SRC_FILES})

add_executable(geopixeldesktop ${GEOPIXELDESKTOP_APP_TYPE} ${GEOPIXELDESKTOP_FILES})

qt5_use_modules(geopixeldesktop Core Gui Widgets PrintSupport)

target_link_libraries(geopixeldesktop geopixeltools
                                      ${TERRALIB_LIBRARIES}
                                      terralib_mod_ws_core
                                      terralib_mod_symbology
                                      ${Boost_LIBRARIES}
                                      )
                                  
add_definitions(-DBOOST_ALL_NO_LIB -DBOOST_ALL_DYN_LINK)

if(WIN32 AND MSVC)
  set_target_properties(geopixeldesktop PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
  set_target_properties(geopixeldesktop PROPERTIES RELWITHDEBINFO "/SUBSYSTEM:CONSOLE")
  set_target_properties(geopixeldesktop PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
  set_target_properties(geopixeldesktop PROPERTIES MINSIZEREL "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
  
  string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif()

if(Qt5LinguistTools_FOUND) 
  set(_ts_file "${GEOPIXELDESKTOP_ABSOLUTE_ROOT_DIR}/share/translations/geopixeldesktop_pt_br.ts")
  set(_qm_file "${CMAKE_BINARY_DIR}/translations/geopixeldesktop_pt_br.qm")

  add_custom_command(
    TARGET geopixeldesktop
    PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/translations"
    COMMAND ${Qt5_LUPDATE_EXECUTABLE} 
    ARGS -I "${GEOPIXELDESKTOP_ABSOLUTE_ROOT_DIR}/src/geopixeldesktop" -recursive "${GEOPIXELDESKTOP_ABSOLUTE_ROOT_DIR}/src/geopixeldesktop" -ts ${_ts_file}
    COMMAND ${Qt5_LRELEASE_EXECUTABLE} 
    ARGS "${_ts_file}" -qm "${_qm_file}"
    COMMENT "Generating translations"
  )

   install(
     DIRECTORY ${CMAKE_BINARY_DIR}/translations
     DESTINATION ${GEOPIXELDESKTOP_INSTALL_PREFIX}/share/terralib
     COMPONENT runtime
   )
endif() 

if(GEOPIXELDESKTOP_BUILD_AS_BUNDLE)
  TeInstallQt5Plugins()
  
  if(WIN32)
    set(_subfolder bin)
  endif()
  
  TeInstallTerraLibPlugins("${TERRALIB_PLUGINS}" ${GEOPIXELDESKTOP_INSTALL_PREFIX}/${_subfolder})
endif()

install(TARGETS geopixeldesktop
        RUNTIME DESTINATION ${GEOPIXELDESKTOP_INSTALL_PREFIX}/bin COMPONENT runtime 
        LIBRARY DESTINATION ${GEOPIXELDESKTOP_INSTALL_PREFIX}/lib COMPONENT runtime 
        ARCHIVE DESTINATION ${GEOPIXELDESKTOP_INSTALL_PREFIX}/lib COMPONENT runtime
        BUNDLE DESTINATION . COMPONENT runtime
        )

if(EXISTS "${terralib_DIR}/CMakeCache.txt")
  set(_TESRC "${terralib_INCLUDE_DIRS}/..")
  if(WIN32)
    set(_TEBIN "${terralib_DIR}/Release")
  endif()
else()
  set(_TESRC "${terralib_DIR}/../../..")
  if(WIN32)
    set(_TEBIN "${_TESRC}/bin")
  endif()
endif()

install(
  DIRECTORY ${CMAKE_SOURCE_DIR}/../../share
  DESTINATION ${GEOPIXELDESKTOP_INSTALL_PREFIX}
  COMPONENT runtime
  PATTERN "*.qrc" EXCLUDE
  PATTERN "translations" EXCLUDE
)

if(WIN32)
    
  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/qt.conf
	"[Paths]\nBinaries = .\nPlugins = ../qtplugins\nTranslations = ../share/terralib/translations")
         
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/qt.conf
          DESTINATION bin COMPONENT runtime)          
endif()

#
# install
#
if(${GDAL_FOUND})
  install(DIRECTORY ${GDAL_DATA_DIR}
          DESTINATION ${GEOPIXELDESKTOP_INSTALL_PREFIX}/share
          COMPONENT runtime)        
endif()

if(WIN32)
  if(GEOPIXELDESKTOP_BUILD_AS_BUNDLE)
    configure_file(${GEOPIXELDESKTOP_ABSOLUTE_ROOT_DIR}/build/cmake/fixBundle.cmake.in ${CMAKE_BINARY_DIR}/fixBundle.cmake @ONLY)
    install (SCRIPT ${CMAKE_BINARY_DIR}/fixBundle.cmake COMPONENT runtime)
  endif()
endif()
