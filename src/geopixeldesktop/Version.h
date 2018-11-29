/*!
  \file geopx-desktop/src/geopixeldesktop/Version.h

  \brief Utility class for system versioning.
*/

#ifndef __GEOPXDESKTOP_DESKTOP_VERSION_H
#define __GEOPXDESKTOP_DESKTOP_VERSION_H

// TerraLib
#include "../Config.h"

// STL
#include <string>

namespace geopx
{
  namespace desktop
  {
    class Version
    {
      public:

        static int majorNumber();

        static int minorNumber();

        static int patchNumber();

        static std::string releaseStatus();

        static std::string buildDate();

        static std::string asString();

        static int asInt();

      private:

        Version()
        {

        }

        ~Version()
        {

        }
    };
  } // end namespace desktop
}   // end namespace geopx

#endif  // __GEOPXDESKTOP_DESKTOP_VERSION_H



