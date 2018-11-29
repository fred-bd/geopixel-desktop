/*!
\file geopx-desktop/src/geopixeldesktop/XMLFormatter.h

\brief A class for xml serialization formatting strings.
*/

#ifndef __GEOPXDESKTOP_DESKTOP_XMLFORMATTER_H
#define __GEOPXDESKTOP_DESKTOP_XMLFORMATTER_H

#include "../Config.h"
#include "Project.h"

// Terralib
#include <terralib/maptools/AbstractLayer.h>

// STL
#include <list>
#include <string>

namespace te 
{
  //Forward declarations
  namespace da
  {
    class DataSourceInfo;
  }
}

/*!
  \class XMLFormatter

  \brief A class that formats strings.

  Use this class for changing strings, contained in some TerraLib objects, to XML percentage encode format and/or to human readable format. 
  Use XML formatting before serializing the object to a XML file, for example, and human readable format after read the object from a XML.
*/
namespace geopx
{
  namespace desktop
  {
    class XMLFormatter
    {
    public:

      /*!
        \brief Formats the project informations.

        \param p The project.

        \param encode \a Pass true to change for XML percentage format and \a false for human readable format.
      */
      static void format(geopx::desktop::ProjectMetadata* p, const std::list<te::map::AbstractLayerPtr>& layers, const bool& encode);

    };
  } // end namespace desktop
}   // end namespace geopx

#endif // __GEOPXDESKTOP_DESKTOP_XMLFORMATTER_H
