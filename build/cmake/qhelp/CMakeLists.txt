if(EXISTS "${terralib_DIR}/CMakeCache.txt")
  set(_TESRC "${terralib_INCLUDE_DIRS}")
else()
  set(_TESRC "${terralib_DIR}/../../..")
endif()

file(WRITE
     ${CMAKE_BINARY_DIR}/config_qhelp.cmake
     "configure_file (${GEOPIXELDESKTOP_ABSOLUTE_ROOT_DIR}/doc/qhelp/help.qhcp.in ${CMAKE_BINARY_DIR}/share/geopixeldesktop/help/help.qhcp @ONLY)"
)

add_custom_command(
  OUTPUT del_dir
  COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/share/geopixeldesktop/help
  COMMENT "Removing QHelp directory..."
)

add_custom_command(
  OUTPUT copy_dir
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${GEOPIXELDESKTOP_ABSOLUTE_ROOT_DIR}/doc/qhelp/src ${CMAKE_BINARY_DIR}/share/geopixeldesktop/help
  DEPENDS del_dir
  COMMENT "Copying QHelp directory..."
)

add_custom_command(
  OUTPUT copy_terralib_help
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${_TESRC}/share/terraview/help/apf ${CMAKE_BINARY_DIR}/share/geopixeldesktop/help/apf
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${_TESRC}/share/terraview/help/plugins ${CMAKE_BINARY_DIR}/share/geopixeldesktop/help/plugins
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${_TESRC}/share/terraview/help/widgets ${CMAKE_BINARY_DIR}/share/geopixeldesktop/help/widgets
  DEPENDS copy_dir
  COMMENT "Copying QHelp directory from terralib..."
)

add_custom_command(
  OUTPUT ${CMAKE_BINARY_DIR}/share/geopixeldesktop/help/help.qhcp
  COMMAND  ${CMAKE_COMMAND} -P ${CMAKE_BINARY_DIR}/config_qhelp.cmake
  DEPENDS copy_terralib_help
  COMMENT "Configuring QHelp collection file..."
)

add_custom_command(
  OUTPUT copy_icon
  COMMAND  ${CMAKE_COMMAND} -E copy ${GEOPIXELDESKTOP_ABSOLUTE_ROOT_DIR}/share/geopixeldesktop/images/png/geopixeldesktop-icon.png ${CMAKE_BINARY_DIR}/share/geopixeldesktop/help
  DEPENDS copy_dir
  COMMENT "Copying icon file..."
)

add_custom_command(
  OUTPUT ${CMAKE_BINARY_DIR}/share/geopixeldesktop/help/help.qhc
  COMMAND ${QT_QCOLLECTIONGENERATOR_EXECUTABLE} ${CMAKE_BINARY_DIR}/share/geopixeldesktop/help/help.qhcp -o ${CMAKE_BINARY_DIR}/share/geopixeldesktop/help/help.qhc
  DEPENDS copy_icon ${CMAKE_BINARY_DIR}/share/geopixeldesktop/help/help.qhcp
  COMMENT "Building QHelp files ..."
)

add_custom_command(
  OUTPUT remove_files
  COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/share/geopixeldesktop/help/apf
  COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/share/geopixeldesktop/help/plugins
  COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/share/geopixeldesktop/help/widgets
  COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/share/geopixeldesktop/help/geopixeldesktop
  COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/share/geopixeldesktop/help/gen_icones
  COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/share/geopixeldesktop/help/GeopixelDesktop
  COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/share/geopixeldesktop/help/about.txt
  COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/share/geopixeldesktop/help/sobre.txt
  COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/share/geopixeldesktop/help/geopixeldesktop.qhp
  COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/share/geopixeldesktop/help/help.qhcp
  COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/share/geopixeldesktop/help/geopixeldesktop-icon.png
  DEPENDS ${CMAKE_BINARY_DIR}/share/geopixeldesktop/help/help.qhc
  COMMENT "Removing copied files"
)

add_custom_target(
  "qhelp"
  DEPENDS remove_files
  COMMENT "Generate QHelp project"
  VERBATIM
)

install(DIRECTORY ${CMAKE_BINARY_DIR}/share/geopixeldesktop/help/
        DESTINATION ${GEOPIXELDESKTOP_INSTALL_PREFIX}/share/geopixeldesktop/help COMPONENT runtime)
        
if(Qt5_FOUND AND WIN32)

  find_program(ASSIST_PRG
               NAME assistant
               PATHS ${Qt5_DIR}/../../../bin)

  if(ASSIST_PRG)
    install (FILES ${ASSIST_PRG}
             DESTINATION ${GEOPIXELDESKTOP_INSTALL_PREFIX}/bin
             PERMISSIONS OWNER_READ OWNER_EXECUTE
                         GROUP_READ GROUP_EXECUTE
                         WORLD_READ WORLD_EXECUTE
             COMPONENT runtime)
  endif()
  
endif()
