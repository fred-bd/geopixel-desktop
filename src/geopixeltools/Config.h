/*!
  \file geopx-desktop/src/geopixeltools/Config.h

  \brief Configuration flags for the geopixeltools module.
*/

#ifndef __GEOPXDESKTOP_TOOLS_CONFIG_H
#define __GEOPXDESKTOP_TOOLS_CONFIG_H


#define GEOPX_TOOLS_MODULE_NAME "geopx.tools"

/** @name DLL/LIB Module
 *  Flags for building Geopixel Tools as a DLL or as a Static Library
 */
//@{

/*!
  \def GEOPXTOOLSEXPORT

  \brief You can use this macro in order to export/import classes and functions from this module.

  \note If you want to compile Geopixel Tools as DLL in Windows, remember to insert GEOPXTOOLSDLL into the project's list of defines.

  \note If you want to compile Geopixel Tools as an Static Library under Windows, remember to insert the GEOPXTOOLSSTATIC flag into the project list of defines.
 */
#ifdef WIN32

  #ifdef _MSC_VER 
    #pragma warning( disable : 4251 )
    #pragma warning( disable : 4275 )
  #endif

  #ifdef GEOPXTOOLSSTATIC
    #define GEOPXTOOLSEXPORT                          // Don't need to export/import... it is a static library
  #elif GEOPXTOOLSDLL
    #define GEOPXTOOLSEXPORT  __declspec(dllexport)   // export DLL information
  #else
    #define GEOPXTOOLSEXPORT  __declspec(dllimport)   // import DLL information
  #endif 
#else
  #define GEOPXTOOLSEXPORT
#endif

//@}  

#endif  // __GEOPXDESKTOP_TOOLS_CONFIG_H

