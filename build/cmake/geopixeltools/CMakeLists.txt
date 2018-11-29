include_directories(SYSTEM  ${Boost_INCLUDE_DIR})

if(WIN32)
    add_definitions(-D_SCL_SECURE_NO_WARNINGS  -DGEOPXTOOLSDLL)
endif()

include_directories(
    SYSTEM  ${GEOPIXELDESKTOP_ABSOLUTE_ROOT_DIR}/src
    SYSTEM  ${GEOPIXELDESKTOP_ABSOLUTE_ROOT_DIR}/src/geopixeltools
    SYSTEM  ${terralib_INCLUDE_DIRS}
    SYSTEM  ${terralib_DIR}
    SYSTEM  ${Boost_INCLUDE_DIR}
    SYSTEM  ${CURL_INCLUDE_DIRS}
    SYSTEM  ${CMAKE_BINARY_DIR}
)

set(CMAKE_AUTOMOC  ON)
set(CMAKE_INCLUDE_CURRENT_DIR  ON)

#
#  Threating  warning  as  errors.
#
if  ("${CMAKE_CXX_COMPILER_ID}"  STREQUAL  "GNU")
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS}  -Werror")
elseif(CMAKE_GENERATOR  MATCHES  "Visual  Studio")
    SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS}  -WX")
    SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS}  -WX")
endif()

#  geopixel-tools
file(GLOB  GEOPX_TOOLS_SRC_FILES  ${GEOPIXELDESKTOP_ABSOLUTE_ROOT_DIR}/src/geopixeltools/*.cpp)
file(GLOB  GEOPX_TOOLS_HDR_FILES  ${GEOPIXELDESKTOP_ABSOLUTE_ROOT_DIR}/src/geopixeltools/*.h)
source_group("Source  Files"    FILES  ${GEOPX_TOOLS_SRC_FILES})
source_group("Header  Files"    FILES  ${GEOPX_TOOLS_HDR_FILES})

#  geopixel-tools-photoindex
file(GLOB  GEOPX_TOOLS_SRC_PHOTOINDEX_FILES  ${GEOPIXELDESKTOP_ABSOLUTE_ROOT_DIR}/src/geopixeltools/photoIndex/*.cpp)
file(GLOB  GEOPX_TOOLS_HDR_PHOTOINDEX_FILES  ${GEOPIXELDESKTOP_ABSOLUTE_ROOT_DIR}/src/geopixeltools/photoIndex/*.h)
source_group("Source  Files\\photoIndex"    FILES  ${GEOPX_TOOLS_SRC_PHOTOINDEX_FILES})
source_group("Header  Files\\photoIndex"    FILES  ${GEOPX_TOOLS_HDR_PHOTOINDEX_FILES})

file(GLOB  GEOPX_TOOLS_SRC_PHOTOINDEX_CORE_FILES  ${GEOPIXELDESKTOP_ABSOLUTE_ROOT_DIR}/src/geopixeltools/photoIndex/core/*.cpp)
file(GLOB  GEOPX_TOOLS_HDR_PHOTOINDEX_CORE_FILES  ${GEOPIXELDESKTOP_ABSOLUTE_ROOT_DIR}/src/geopixeltools/photoIndex/core/*.h)
source_group("Source  Files\\photoIndex\\core"    FILES  ${GEOPX_TOOLS_SRC_PHOTOINDEX_CORE_FILES})
source_group("Header  Files\\photoIndex\\core"    FILES  ${GEOPX_TOOLS_HDR_PHOTOINDEX_CORE_FILES})

file(GLOB  GEOPX_TOOLS_SRC_PHOTOINDEX_QT_FILES  ${GEOPIXELDESKTOP_ABSOLUTE_ROOT_DIR}/src/geopixeltools/photoIndex/qt/*.cpp)
file(GLOB  GEOPX_TOOLS_HDR_PHOTOINDEX_QT_FILES  ${GEOPIXELDESKTOP_ABSOLUTE_ROOT_DIR}/src/geopixeltools/photoIndex/qt/*.h)
file(GLOB  GEOPX_TOOLS_PHOTOINDEX_UI_FILES  ${GEOPIXELDESKTOP_ABSOLUTE_ROOT_DIR}/src/geopixeltools/photoIndex/qt/ui/*.ui)
source_group("Source  Files\\photoIndex\\qt"    FILES  ${GEOPX_TOOLS_SRC_PHOTOINDEX_QT_FILES})
source_group("Header  Files\\photoIndex\\qt"    FILES  ${GEOPX_TOOLS_HDR_PHOTOINDEX_QT_FILES})

#  geopixel-tools-tilegenerator
file(GLOB  GEOPX_TOOLS_SRC_TILEGENERATOR_FILES  ${GEOPIXELDESKTOP_ABSOLUTE_ROOT_DIR}/src/geopixeltools/tileGenerator/*.cpp)
file(GLOB  GEOPX_TOOLS_HDR_TILEGENERATOR_FILES  ${GEOPIXELDESKTOP_ABSOLUTE_ROOT_DIR}/src/geopixeltools/tileGenerator/*.h)
source_group("Source  Files\\tileGenerator"    FILES  ${GEOPX_TOOLS_SRC_TILEGENERATOR_FILES})
source_group("Header  Files\\tileGenerator"    FILES  ${GEOPX_TOOLS_HDR_TILEGENERATOR_FILES})

file(GLOB  GEOPX_TOOLS_SRC_TILEGENERATOR_CORE_FILES  ${GEOPIXELDESKTOP_ABSOLUTE_ROOT_DIR}/src/geopixeltools/tileGenerator/core/*.cpp)
file(GLOB  GEOPX_TOOLS_HDR_TILEGENERATOR_CORE_FILES  ${GEOPIXELDESKTOP_ABSOLUTE_ROOT_DIR}/src/geopixeltools/tileGenerator/core/*.h)
source_group("Source  Files\\tileGenerator\\core"    FILES  ${GEOPX_TOOLS_SRC_TILEGENERATOR_CORE_FILES})
source_group("Header  Files\\tileGenerator\\core"    FILES  ${GEOPX_TOOLS_HDR_TILEGENERATOR_CORE_FILES})

file(GLOB  GEOPX_TOOLS_SRC_TILEGENERATOR_QT_FILES  ${GEOPIXELDESKTOP_ABSOLUTE_ROOT_DIR}/src/geopixeltools/tileGenerator/qt/*.cpp)
file(GLOB  GEOPX_TOOLS_HDR_TILEGENERATOR_QT_FILES  ${GEOPIXELDESKTOP_ABSOLUTE_ROOT_DIR}/src/geopixeltools/tileGenerator/qt/*.h)
file(GLOB  GEOPX_TOOLS_TILEGENERATOR_UI_FILES  ${GEOPIXELDESKTOP_ABSOLUTE_ROOT_DIR}/src/geopixeltools/tileGenerator/qt/ui/*.ui)
source_group("Source  Files\\tileGenerator\\qt"    FILES  ${GEOPX_TOOLS_SRC_TILEGENERATOR_QT_FILES})
source_group("Header  Files\\tileGenerator\\qt"    FILES  ${GEOPX_TOOLS_HDR_TILEGENERATOR_QT_FILES})

#  geopixel-tools-forestMonitor
file(GLOB  GEOPX_TOOLS_SRC_FORESTMONITOR_FILES  ${GEOPIXELDESKTOP_ABSOLUTE_ROOT_DIR}/src/geopixeltools/forestMonitor/*.cpp)
file(GLOB  GEOPX_TOOLS_HDR_FORESTMONITOR_FILES  ${GEOPIXELDESKTOP_ABSOLUTE_ROOT_DIR}/src/geopixeltools/forestMonitor/*.h)
source_group("Source  Files\\forestMonitor"    FILES  ${GEOPX_TOOLS_SRC_FORESTMONITOR_FILES})
source_group("Header  Files\\forestMonitor"    FILES  ${GEOPX_TOOLS_HDR_FORESTMONITOR_FILES})

file(GLOB  GEOPX_TOOLS_SRC_FORESTMONITOR_CORE_FILES  ${GEOPIXELDESKTOP_ABSOLUTE_ROOT_DIR}/src/geopixeltools/forestMonitor/core/*.cpp)
file(GLOB  GEOPX_TOOLS_HDR_FORESTMONITOR_CORE_FILES  ${GEOPIXELDESKTOP_ABSOLUTE_ROOT_DIR}/src/geopixeltools/forestMonitor/core/*.h)
source_group("Source  Files\\forestMonitor\\core"    FILES  ${GEOPX_TOOLS_SRC_FORESTMONITOR_CORE_FILES})
source_group("Header  Files\\forestMonitor\\core"    FILES  ${GEOPX_TOOLS_HDR_FORESTMONITOR_CORE_FILES})

file(GLOB  GEOPX_TOOLS_SRC_FORESTMONITOR_QT_FILES  ${GEOPIXELDESKTOP_ABSOLUTE_ROOT_DIR}/src/geopixeltools/forestMonitor/qt/*.cpp)
file(GLOB  GEOPX_TOOLS_HDR_FORESTMONITOR_QT_FILES  ${GEOPIXELDESKTOP_ABSOLUTE_ROOT_DIR}/src/geopixeltools/forestMonitor/qt/*.h)
file(GLOB  GEOPX_TOOLS_FORESTMONITOR_UI_FILES  ${GEOPIXELDESKTOP_ABSOLUTE_ROOT_DIR}/src/geopixeltools/forestMonitor/qt/ui/*.ui)
source_group("Source  Files\\forestMonitor\\qt"    FILES  ${GEOPX_TOOLS_SRC_FORESTMONITOR_QT_FILES})
source_group("Header  Files\\forestMonitor\\qt"    FILES  ${GEOPX_TOOLS_HDR_FORESTMONITOR_QT_FILES})

file(GLOB  GEOPX_TOOLS_SRC_FORESTMONITOR_QT_TOOLS_FILES  ${GEOPIXELDESKTOP_ABSOLUTE_ROOT_DIR}/src/geopixeltools/forestMonitor/qt/tools/*.cpp)
file(GLOB  GEOPX_TOOLS_HDR_FORESTMONITOR_QT_TOOLS_FILES  ${GEOPIXELDESKTOP_ABSOLUTE_ROOT_DIR}/src/geopixeltools/forestMonitor/qt/tools/*.h)
source_group("Source  Files\\forestMonitor\\qt\\tools"    FILES  ${GEOPX_TOOLS_SRC_FORESTMONITOR_QT_TOOLS_FILES})
source_group("Header  Files\\forestMonitor\\qt\\tools"    FILES  ${GEOPX_TOOLS_HDR_FORESTMONITOR_QT_TOOLS_FILES})


#  uic'ing
QT5_WRAP_UI(GEOPX_TOOLS_GEN_HDR_FILES  ${GEOPX_TOOLS_PHOTOINDEX_UI_FILES}
									   ${GEOPX_TOOLS_TILEGENERATOR_UI_FILES}
									   ${GEOPX_TOOLS_FORESTMONITOR_UI_FILES})

set(GEOPX_TOOLS_FILES  ${GEOPX_TOOLS_SRC_FILES} ${GEOPX_TOOLS_HDR_FILES}  
                       ${GEOPX_TOOLS_SRC_PHOTOINDEX_FILES} ${GEOPX_TOOLS_HDR_PHOTOINDEX_FILES}
					   ${GEOPX_TOOLS_SRC_PHOTOINDEX_CORE_FILES} ${GEOPX_TOOLS_HDR_PHOTOINDEX_CORE_FILES}
					   ${GEOPX_TOOLS_SRC_PHOTOINDEX_QT_FILES} ${GEOPX_TOOLS_HDR_PHOTOINDEX_QT_FILES}
					   ${GEOPX_TOOLS_SRC_TILEGENERATOR_FILES} ${GEOPX_TOOLS_HDR_TILEGENERATOR_FILES}
					   ${GEOPX_TOOLS_SRC_TILEGENERATOR_CORE_FILES} ${GEOPX_TOOLS_HDR_TILEGENERATOR_CORE_FILES}
					   ${GEOPX_TOOLS_SRC_TILEGENERATOR_QT_FILES} ${GEOPX_TOOLS_HDR_TILEGENERATOR_QT_FILES}
					   ${GEOPX_TOOLS_SRC_FORESTMONITOR_FILES} ${GEOPX_TOOLS_HDR_FORESTMONITOR_FILES}
					   ${GEOPX_TOOLS_SRC_FORESTMONITOR_CORE_FILES} ${GEOPX_TOOLS_HDR_FORESTMONITOR_CORE_FILES}
					   ${GEOPX_TOOLS_SRC_FORESTMONITOR_QT_FILES} ${GEOPX_TOOLS_HDR_FORESTMONITOR_QT_FILES}
					   ${GEOPX_TOOLS_SRC_FORESTMONITOR_QT_TOOLS_FILES} ${GEOPX_TOOLS_HDR_FORESTMONITOR_QT_TOOLS_FILES}
					   ${GEOPX_TOOLS_GEN_HDR_FILES})
					    
add_library(geopixeltools  SHARED  ${GEOPX_TOOLS_FILES})

target_link_libraries(geopixeltools
    ${TERRALIB_LIBRARIES}
    ${Boost_LOCALE_LIBRARY}
    ${Boost_REGEX_LIBRARY}
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_LOG_LIBRARY}
    ${Boost_LOG_SETUP_LIBRARY}
    ${Boost_THREAD_LIBRARY}
    )
										      
set_target_properties(geopixeltools
        PROPERTIES  VERSION  ${GEOPIXELDESKTOP_VERSION_MAJOR}.${GEOPIXELDESKTOP_VERSION_MINOR}
        SOVERSION  ${GEOPIXELDESKTOP_VERSION_MAJOR}.${GEOPIXELDESKTOP_VERSION_MINOR})
																  
if(Qt5LinguistTools_FOUND) 
  set(_ts_file "${GEOPIXELDESKTOP_ABSOLUTE_ROOT_DIR}/share/translations/geopixeltools_pt_br.ts")
  set(_qm_file "${CMAKE_BINARY_DIR}/translations/geopixeltools_pt_br.qm")

  add_custom_command(
    TARGET geopixeltools
    PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/translations"
    COMMAND ${Qt5_LUPDATE_EXECUTABLE} 
    ARGS -I "${GEOPIXELDESKTOP_ABSOLUTE_ROOT_DIR}/src/geopixeltools" -recursive "${GEOPIXELDESKTOP_ABSOLUTE_ROOT_DIR}/src/geopixeltools" -ts ${_ts_file}
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
								  
qt5_use_modules(geopixeltools  Widgets)
								  
install(TARGETS  geopixeltools
                EXPORT  geopixel-targets
                RUNTIME  DESTINATION  ${GEOPIXELDESKTOP_INSTALL_PREFIX}/bin  COMPONENT  runtime  
                LIBRARY  DESTINATION  ${GEOPIXELDESKTOP_INSTALL_PREFIX}/lib  COMPONENT  runtime  
                ARCHIVE  DESTINATION  ${GEOPIXELDESKTOP_INSTALL_PREFIX}/lib  COMPONENT  runtime
                )
