/*!
  \file geopx-desktop/src/geopixeldesktop/datasource/GeopixelType.h

  \brief  GeopixelType data source type.
*/

#ifndef __GEOPXDESKTOP_DESKTOP_DATASOURCE_GEOPIXELTYPE_H
#define __GEOPXDESKTOP_DESKTOP_DATASOURCE_GEOPIXELTYPE_H

// TerraLib
#include <terralib/qt/widgets/datasource/core/DataSourceType.h>


namespace geopx
{
  namespace desktop
  {
    class GeopixelType : public te::qt::widgets::DataSourceType
    {
      public:

        GeopixelType();

        ~GeopixelType();

        bool hasDatabaseSupport() const;

        bool hasFileSupport() const;

        bool hasRasterSupport() const;

        bool hasVectorialSupport() const;

        std::string getName() const;

        std::string getTitle() const;

        std::string getDescription() const;

        QWidget* getWidget(int widgetType, QWidget* parent = 0, Qt::WindowFlags f = 0) const;

        QIcon getIcon(int iconType) const;
    };

  } // end namespace desktop
} // end namespace geopx

#endif  // __GEOPXDESKTOP_DESKTOP_DATASOURCE_GEOPIXELTYPE_H
