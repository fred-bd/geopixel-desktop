/*!
  \file geopx-desktop/src/geopixeldesktop/layer/GeopxDataSetLayer.cpp

  \brief Implements Terralib layer GeopxDataSetLayer.
*/

#include "GeopxDataSetLayer.h"

// TerraLib
#include <terralib/maptools/Utils.h>

// STL
#include <memory>

const std::string geopx::desktop::layer::GeopxDataSetLayer::sm_type("GEOPXDATASETLAYER");

geopx::desktop::layer::GeopxDataSetLayer::GeopxDataSetLayer(AbstractLayer* parent)
  : DataSetLayer(parent)
{
}

geopx::desktop::layer::GeopxDataSetLayer::GeopxDataSetLayer(const std::string& id, AbstractLayer* parent)
  : DataSetLayer(id, parent)
{
}

geopx::desktop::layer::GeopxDataSetLayer::GeopxDataSetLayer(const std::string& id,
                                    const std::string& title,
                                    AbstractLayer* parent)
  : DataSetLayer(id, title, parent)
{
}

geopx::desktop::layer::GeopxDataSetLayer::~GeopxDataSetLayer()
{

}

te::map::AbstractLayer* geopx::desktop::layer::GeopxDataSetLayer::clone()
{
  geopx::desktop::layer::GeopxDataSetLayer* layer = new geopx::desktop::layer::GeopxDataSetLayer;
  
  te::map::CopyAbstractLayerInfo(this, layer);

  layer->setRendererType(getRendererType());
  layer->setUser(m_user);
  layer->setProfile(m_profile);
  layer->setThemeId(m_themeId);

  return layer; 
}

const std::string& geopx::desktop::layer::GeopxDataSetLayer::getType() const
{
  return sm_type;
}

const std::string geopx::desktop::layer::GeopxDataSetLayer::getUser() const
{
  return m_user;
}

void geopx::desktop::layer::GeopxDataSetLayer::setUser(const std::string& user)
{
  m_user = user;
}

const std::string geopx::desktop::layer::GeopxDataSetLayer::getProfile() const
{
  return m_profile;
}

void geopx::desktop::layer::GeopxDataSetLayer::setProfile(const std::string& profile)
{
  m_profile = profile;
}

const int geopx::desktop::layer::GeopxDataSetLayer::getThemeId() const
{
  return m_themeId;
}

void geopx::desktop::layer::GeopxDataSetLayer::setThemeId(const int& themeId)
{
  m_themeId = themeId;
}
