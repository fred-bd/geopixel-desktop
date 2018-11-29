/*!
  \file geopx-desktop/src/geopixeldesktop/Project.h

  \brief This class models the concept of a project for the GeopixelDesktop.
*/

#ifndef __GEOPXDESKTOP_DESKTOP_PROJECT_H
#define __GEOPXDESKTOP_DESKTOP_PROJECT_H

#include "../Config.h"

// Terralib
#include <terralib/maptools/AbstractLayer.h>

// Qt
#include <QString>

// STL
#include <list>

namespace geopx
{
  namespace desktop
  {
    struct ProjectMetadata
    {
      QString m_title;                                      //!< The title of the project.
      QString m_author;                                     //!< The author of the project.
      bool m_changed;                                       //!< Flag indicating that the project needs to be saved.
      QString m_fileName;                                   //!< The project file.
    };

    void SaveProject(const ProjectMetadata& proj, const std::list<te::map::AbstractLayerPtr>& layers);

    void LoadProject(const QString& projFile, ProjectMetadata& proj, std::list<te::map::AbstractLayerPtr>& layers);

  } // end namespace desktop
}   // end namespace geopx

#endif  // __GEOPXDESKTOP_DESKTOP_PROJECT_H
