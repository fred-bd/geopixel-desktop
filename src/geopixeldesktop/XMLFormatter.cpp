/*!
\file geopx-desktop/src/geopixeldesktop/XMLFormatter.cpp

\brief A class for xml serialization formatting strings.
*/

#include "XMLFormatter.h"

// Geopixel Desktop
#include "Project.h"

// Terralib
#include <terralib/dataaccess/datasource/DataSourceInfo.h>
#include <terralib/dataaccess/datasource/DataSourceInfoManager.h>
#include <terralib/maptools/DataSetLayer.h>
#include <terralib/qt/af/XMLFormatter.h>

// Qt
#include <QUrl>

void geopx::desktop::XMLFormatter::format(geopx::desktop::ProjectMetadata* p, const std::list<te::map::AbstractLayerPtr>& layers, const bool& encode)
{
  p->m_author = QString::fromUtf8(te::qt::af::XMLFormatter::format(p->m_author.toUtf8().data(), encode).c_str());
  p->m_title = QString::fromUtf8(te::qt::af::XMLFormatter::format(p->m_title.toUtf8().data(), encode).c_str());

  for(std::list<te::map::AbstractLayerPtr>::const_iterator it = layers.begin(); it != layers.end(); ++it)
    te::qt::af::XMLFormatter::format((*it).get(), encode);
}
