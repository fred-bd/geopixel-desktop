/*!
\file geopx-desktop/src/geopixeldesktop/layer/TiledLayer.cpp

\brief A layer with reference to a Tiled Layer.
*/

#include "TiledLayer.h"
#include "TiledLayerRenderer.h"

//Terralib
#include <terralib/core/Exception.h>
#include <terralib/core/translator/Translator.h>
#include <terralib/maptools/AbstractLayerRenderer.h>
#include <terralib/maptools/RendererFactory.h>
#include <terralib/maptools/Utils.h>

//Boost
#include <boost/format.hpp>

//STL
#include <vector>

void FindAndReplace(std::string & data, std::string toSearch, std::string replaceStr)
{
  // Get the first occurrence
  size_t pos = data.find(toSearch);

  // Repeat till end is reached
  while (pos != std::string::npos)
  {
    // Replace this occurrence of Sub String
    data.replace(pos, toSearch.size(), replaceStr);

    // Get the next occurrence from the current position
    pos = data.find(toSearch, pos + toSearch.size());
  }
}

const std::string geopx::desktop::layer::TiledLayer::sm_type("TILEDLAYER");

geopx::desktop::layer::TiledLayer::TiledLayer(te::map::AbstractLayer *parent)
  : te::map::AbstractLayer(parent),
    m_rendererType("TILED_LAYER_RENDERER"),
    m_tiledLayerURL(""),
    m_tileSize(256)
{

}

geopx::desktop::layer::TiledLayer::TiledLayer(const std::string &id, te::map::AbstractLayer *parent)
  : te::map::AbstractLayer(id, parent),
    m_rendererType("TILED_LAYER_RENDERER"),
    m_tiledLayerURL(""),
    m_tileSize(256)
{

}

geopx::desktop::layer::TiledLayer::TiledLayer(const std::string &id, const std::string &title, te::map::AbstractLayer *parent)
  : te::map::AbstractLayer(id, title, parent),
    m_rendererType("TILED_LAYER_RENDERER"),
    m_tiledLayerURL(""),
    m_tileSize(256)
{

}

geopx::desktop::layer::TiledLayer::~TiledLayer() = default;

te::map::AbstractLayer* geopx::desktop::layer::TiledLayer::clone()
{
  geopx::desktop::layer::TiledLayer* layer = new geopx::desktop::layer::TiledLayer;

  te::map::CopyAbstractLayerInfo(this, layer);

  layer->setRendererType(m_rendererType);

  layer->setTiledLayerURL(m_tiledLayerURL);

  layer->setTileSize(m_tileSize);

  return layer;
}

void geopx::desktop::layer::TiledLayer::setTiledLayerURL(const std::string& url)
{
  m_tiledLayerURL = url;
}

std::string geopx::desktop::layer::TiledLayer::getTiledLayerURL(const int& x, const int& y, const int& z) const
{
  std::string url = m_tiledLayerURL;

  FindAndReplace(url, "${x}", std::to_string(x));
  FindAndReplace(url, "${y}", std::to_string(y));
  FindAndReplace(url, "${z}", std::to_string(z));

  return url;
}

std::string geopx::desktop::layer::TiledLayer::getTiledLayerURL() const
{
  return m_tiledLayerURL;
}

void geopx::desktop::layer::TiledLayer::setTileSize(const int& size)
{
  m_tileSize = size;
}

int geopx::desktop::layer::TiledLayer::getTileSize() const
{
  return m_tileSize;
}

std::unique_ptr<te::map::LayerSchema> geopx::desktop::layer::TiledLayer::getSchema() const
{
  te::da::DataSetType* dt = new  te::da::DataSetType("TiledDataSetType");

  dt->setTitle("Tyled Type");

  std::unique_ptr<te::da::DataSetType> type(dt);

  return type;
}

std::unique_ptr<te::da::DataSet> geopx::desktop::layer::TiledLayer::getData( te::common::TraverseType /*travType*/,
                                                                    const te::common::AccessPolicy /*accessPolicy*/) const
{
  return nullptr;
}

std::unique_ptr<te::da::DataSet> geopx::desktop::layer::TiledLayer::getData(const std::string& /*propertyName*/,
                                                                   const te::gm::Envelope* /*e*/,
                                                                   te::gm::SpatialRelation /*r*/,
                                                                   te::common::TraverseType travType,
                                                                   const te::common::AccessPolicy accessPolicy) const
{
  return getData(travType, accessPolicy);
}

std::unique_ptr<te::da::DataSet> geopx::desktop::layer::TiledLayer::getData(const std::string& /*propertyName*/,
                                                                   const te::gm::Geometry* /*g*/,
                                                                   te::gm::SpatialRelation /*r*/,
                                                                   te::common::TraverseType travType,
                                                                   const te::common::AccessPolicy accessPolicy) const
{
  return getData(travType, accessPolicy);
}

std::unique_ptr<te::da::DataSet> geopx::desktop::layer::TiledLayer::getData(te::da::Expression* /*restriction*/,
                                                                   te::common::TraverseType travType,
                                                                   const te::common::AccessPolicy accessPolicy) const
{
  return getData(travType, accessPolicy);
}

std::unique_ptr<te::da::DataSet> geopx::desktop::layer::TiledLayer::getData(const te::da::ObjectIdSet* /*oids*/,
                                                                   te::common::TraverseType travType,
                                                                   const te::common::AccessPolicy accessPolicy) const
{
  return getData(travType, accessPolicy);
}

bool geopx::desktop::layer::TiledLayer::isValid() const
{
  return true;
}

void geopx::desktop::layer::TiledLayer::draw(te::map::Canvas *canvas,
                                      const te::gm::Envelope &bbox, int srid,
                                      const double &scale, bool *cancel)
{
  if(m_rendererType.empty())
    throw te::core::Exception() << te::ErrorDescription((boost::format(TE_TR("Could not draw the Tiled layer %1%. The renderer type is empty!")) % getTitle()).str());

  // Try get the defined renderer
  std::unique_ptr<te::map::AbstractRenderer> renderer(te::map::RendererFactory::make(m_rendererType));

  if(renderer.get() == nullptr)
    throw te::core::Exception() << te::ErrorDescription((boost::format(TE_TR("Could not draw the WMS layer %1%. The renderer %2% could not be created!"))  % getTitle() % m_rendererType).str());

  geopx::desktop::layer::TiledLayerRenderer* layerRenderer = dynamic_cast<geopx::desktop::layer::TiledLayerRenderer*>(renderer.get());

  if(layerRenderer)
    layerRenderer->draw(this, canvas, bbox, srid, scale, cancel);
}

const std::string &geopx::desktop::layer::TiledLayer::getType() const
{
  return sm_type;
}

const std::string &geopx::desktop::layer::TiledLayer::getDataSourceId() const
{
  return m_datasourceId;
}

void geopx::desktop::layer::TiledLayer::setDataSourceId(const std::string &datasourceId)
{
  m_datasourceId = datasourceId;
}

const std::string &geopx::desktop::layer::TiledLayer::getRendererType() const
{
  return m_rendererType;
}

void geopx::desktop::layer::TiledLayer::setRendererType(const std::string &rendererType)
{
  m_rendererType = rendererType;
}

