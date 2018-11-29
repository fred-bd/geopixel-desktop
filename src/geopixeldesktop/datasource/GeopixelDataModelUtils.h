/*!
  \file geopx-desktop/src/geopixeldesktop/datasourec/GeopixelDataModelUtils.h

  \brief Utility routines for the Geopixel Database Model access.
*/

#ifndef __GEOPXDESKTOP_DESKTOP_DATASOURCE_GEOPIXELDATAMODELUTILS_H
#define __GEOPXDESKTOP_DESKTOP_DATASOURCE_GEOPIXELDATAMODELUTILS_H

// Geopixel Desktop
#include "../Config.h"

// TerraLib
#include <terralib/dataaccess/datasource/DataSource.h>

// STL
#include <string>

// Forward declarations
class QString;

namespace geopx
{
  namespace desktop
  {
    namespace datasource
    {

      std::string GetQueryLayersXYZ(const std::string& profile);

      std::string GetQueryLayersVec(const std::string& profile);

      int GetThemeId(te::da::DataSource* ds, const std::string& profile, const int& cmdId);

      bool IsLayerEditionEnabled(te::da::DataSource* ds, const int& themeId);

    }   // end namespace datasource 
  }   // end namespace desktop
}   // end namespace geopx

#endif  // __GEOPXDESKTOP_DESKTOP_DATASOURCE_GEOPIXELDATAMODELUTILS_H

