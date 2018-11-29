/*!
  \file geopx-desktop/src/geopixeldesktop/datasource/GeopixelConnector.h

  \brief  Geopixel connector implementation for the Qt data source widget.
*/

#ifndef __GEOPXDESKTOP_DESKTOP_DATASOURCE_GEOPIXELCONNECTOR_H
#define __GEOPXDESKTOP_DESKTOP_DATASOURCE_GEOPIXELCONNECTOR_H

//! TerraLib
#include <terralib/qt/widgets/datasource/connector/AbstractDataSourceConnector.h>

//! Qt
#include <QWidget>

namespace geopx
{
  namespace desktop
  {
    /*!
      \class GeopixelConnector

      \brief Geopixel connector implementation for the Qt data source widget.
    */
    class GeopixelConnector : public te::qt::widgets::AbstractDataSourceConnector
    {
      public:

        GeopixelConnector(QWidget* parent = 0, Qt::WindowFlags f = 0);

        ~GeopixelConnector();

        void connect(std::list<te::da::DataSourceInfoPtr>& datasources);

        void create(std::list<te::da::DataSourceInfoPtr>& datasources);

        void update(std::list<te::da::DataSourceInfoPtr>& datasources);

        void remove(std::list<te::da::DataSourceInfoPtr>& datasources);
    }; 

  } // end namespace desktop
} // end namespace geopx

#endif  // __GEOPXDESKTOP_DESKTOP_DATASOURCE_GEOPIXELCONNECTOR_H

