/*!
  \file geopx-desktop/src/geopixeldesktop/datasource/GeopixelType.cpp

  \brief  GeopixelType data source type.
*/

#include "GeopixelType.h"
#include "GeopixelConnector.h"
#include "Utils.h"

// TerraLib
#include <terralib/core/translator/Translator.h>
#include <terralib/qt/widgets/layer/selector/DataSetLayerSelector.h>

// STL
#include <cassert>

geopx::desktop::GeopixelType::GeopixelType() = default;

geopx::desktop::GeopixelType::~GeopixelType() = default;

bool geopx::desktop::GeopixelType::hasDatabaseSupport() const
{
  return false;
}

bool geopx::desktop::GeopixelType::hasFileSupport() const
{
  return true;
}

bool geopx::desktop::GeopixelType::hasRasterSupport() const
{
  return false;
}

bool geopx::desktop::GeopixelType::hasVectorialSupport() const
{
  return true;
}

std::string geopx::desktop::GeopixelType::getName() const
{
  return "GEOPIXEL";
}

std::string geopx::desktop::GeopixelType::getTitle() const
{
  return TE_TR("Geopixel");
}

std::string geopx::desktop::GeopixelType::getDescription() const
{
  return TE_TR("Access geospatial data in a Geopixel database");
}

QWidget* geopx::desktop::GeopixelType::getWidget(int widgetType, QWidget* parent, Qt::WindowFlags f) const
{
  switch(widgetType)
  {
    case te::qt::widgets::DataSourceType::WIDGET_DATASOURCE_CONNECTOR:
      return new GeopixelConnector(parent, f);

    case DataSourceType::WIDGET_LAYER_SELECTOR:
      return new te::qt::widgets::DataSetLayerSelector(parent, f);

    default:
      return nullptr;
  }
}

QIcon geopx::desktop::GeopixelType::getIcon(int iconType) const
{
  std::string iconGeopixel = geopx::desktop::FindInPath("share/geopixeldesktop/images/svg/geopixel-logo-2.svg").c_str();
  QIcon icon(iconGeopixel.c_str());

  switch(iconType)
  {
    case te::qt::widgets::DataSourceType::ICON_DATASOURCE_SMALL:
      return icon;

    case te::qt::widgets::DataSourceType::ICON_DATASOURCE_CONNECTOR:
      return icon;

    default:
      return QIcon::fromTheme("unknown-icon");
  }
}


