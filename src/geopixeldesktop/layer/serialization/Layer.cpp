/*!
\file geopx-desktop/src/geopixeldesktop/layer/serialization/Layer.h

\brief  Auxiliary classes and functions to read/write Tiled layers using XML document.
*/

#include "Layer.h"
#include "../GeopxDataSetLayer.h"
#include "../TiledLayer.h"

//Terralib
#include <terralib/core/encoding/CharEncoding.h>
#include <terralib/geometry/serialization/xml/Serializer.h>
#include <terralib/maptools/Chart.h>
#include <terralib/maptools/RasterContrast.h>
#include <terralib/maptools/serialization/xml/Layer.h>
#include <terralib/maptools/serialization/xml/Utils.h>
#include <terralib/se/Style.h>
#include <terralib/se/serialization/xml/Style.h>
#include <terralib/xml/AbstractWriter.h>
#include <terralib/xml/Reader.h>

te::map::AbstractLayer* geopx::desktop::layer::serialize::LayerReader(te::xml::Reader &reader)
{
  std::string id = reader.getAttr("id");

  /* Title Element */
  reader.next();
  std::string title = te::map::serialize::ReadLayerTitle(reader);

  /* Visible Element */
  reader.next();
  std::string visible = te::map::serialize::ReadLayerVisibility(reader);

  /* Enconding Element */
  reader.next();
  std::string encodingStr = te::map::serialize::ReadLayerEncoding(reader);

  te::core::EncodingType encoding = te::core::CharEncoding::getEncodingType(encodingStr);

  /* DataSetName Element */
  reader.next();
  std::string dataset = te::map::serialize::ReadDataSetName(reader);

  /* DataSourceId Element */
  reader.next();
  std::string datasourceId = te::map::serialize::ReadDataSourceId(reader);

  /* SRID Element */
  reader.next();
  assert(reader.getNodeType() == te::xml::START_ELEMENT);
  assert(reader.getElementLocalName() == "SRID");
  reader.next();
  assert(reader.getNodeType() == te::xml::VALUE);
  int srid = reader.getElementValueAsInt32();
  reader.next();
  assert(reader.getNodeType() == te::xml::END_ELEMENT);

  /* Extent Element */
  reader.next();
  assert(reader.getNodeType() == te::xml::START_ELEMENT);
  assert(reader.getElementLocalName() == "Extent");
  std::unique_ptr<te::gm::Envelope> mbr(te::serialize::xml::ReadExtent(reader));

  /* RendererId Element */
  assert(reader.getNodeType() == te::xml::START_ELEMENT);
  assert(reader.getElementLocalName() == "RendererId");
  reader.next();
  assert(reader.getNodeType() == te::xml::VALUE);
  std::string rendererId = reader.getElementValue();
  reader.next();
  assert(reader.getNodeType() == te::xml::END_ELEMENT);

  /* TiledLayer URL */
  reader.next();
  assert(reader.getNodeType() == te::xml::START_ELEMENT);
  assert(reader.getElementLocalName() == "TiledLayerURL");
  reader.next();
  assert(reader.getNodeType() == te::xml::VALUE);
  std::string url = reader.getElementValue();
  reader.next();
  assert(reader.getNodeType() == te::xml::END_ELEMENT);

  /* TiledLayer Size */
  reader.next();
  assert(reader.getNodeType() == te::xml::START_ELEMENT);
  assert(reader.getElementLocalName() == "TiledLayerSize");
  reader.next();
  assert(reader.getNodeType() == te::xml::VALUE);
  int tileSize = reader.getElementValueAsInt32();
  reader.next();
  assert(reader.getNodeType() == te::xml::END_ELEMENT);


  reader.next();

  assert(reader.getElementLocalName() == "TiledLayer");

  std::unique_ptr<geopx::desktop::layer::TiledLayer> tiledLayer (new geopx::desktop::layer::TiledLayer(id, title));
  tiledLayer->setDataSetName(dataset);
  tiledLayer->setDataSourceId(datasourceId);
  tiledLayer->setEncoding(encoding);
  tiledLayer->setSRID(srid);
  tiledLayer->setExtent(*mbr.get());
  tiledLayer->setRendererType(rendererId);
  tiledLayer->setVisibility(te::map::serialize::GetVisibility(visible));

  tiledLayer->setTiledLayerURL(url);
  tiledLayer->setTileSize(tileSize);

  reader.next();

  return tiledLayer.release();
}

te::map::AbstractLayer* geopx::desktop::layer::serialize::GeopxDataSetLayerReader(te::xml::Reader& reader)
{
  std::string id = reader.getAttr("id");

  /* Title Element */
  reader.next();
  std::string title = te::map::serialize::ReadLayerTitle(reader);
  reader.next();

  /* Visible Element */
  std::string visible = te::map::serialize::ReadLayerVisibility(reader);
  reader.next();

  /* Encoding Element */
  std::string encoding = te::map::serialize::ReadLayerEncoding(reader);
  if (!encoding.empty())
    reader.next();
  else
    encoding = te::core::CharEncoding::getEncodingName(te::core::EncodingType::UTF8);

  /* Grouping */
  te::map::Grouping* grouping = te::map::serialize::ReadLayerGrouping(reader);

  /* Chart */
  std::unique_ptr<te::map::Chart> chart(te::map::serialize::ReadLayerChart(reader));

  /* RasterContrast */
  std::unique_ptr<te::map::RasterContrast> rc(te::map::serialize::ReadLayerRasterContrast(reader));

  /* DataSetName Element */
  std::string dataset = te::map::serialize::ReadDataSetName(reader);
  reader.next();

  /* DataSourceId Element */
  std::string datasourceId = te::map::serialize::ReadDataSourceId(reader);
  reader.next();

  /* SRID Element */
  int srid = te::map::serialize::ReadSRIDValue(reader);

  /* Extent Element */
  reader.next();
  assert(reader.getNodeType() == te::xml::START_ELEMENT);
  assert(reader.getElementLocalName() == "Extent");
  std::unique_ptr<te::gm::Envelope> mbr(te::serialize::xml::ReadExtent(reader));

  /* RendererId Element */
  assert(reader.getNodeType() == te::xml::START_ELEMENT);
  assert(reader.getElementLocalName() == "RendererId");
  reader.next();
  assert(reader.getNodeType() == te::xml::VALUE);
  std::string rendererId = reader.getElementValue();
  reader.next();
  assert(reader.getNodeType() == te::xml::END_ELEMENT);

  /* Composition Mode Element */
  reader.next();
  int compositionMode = te::map::SourceOver;
  if (reader.getNodeType() == te::xml::START_ELEMENT && reader.getElementLocalName() == "CompositionMode")
  {
    reader.next();
    assert(reader.getNodeType() == te::xml::VALUE);
    compositionMode = reader.getElementValueAsInt32();
    reader.next();
    assert(reader.getNodeType() == te::xml::END_ELEMENT);
    reader.next();
  }

  /* has a Style Element ? */
  std::unique_ptr<te::se::Style> style;

  if ((reader.getNodeType() == te::xml::START_ELEMENT) &&
    (reader.getElementLocalName() == "Style"))
  {
    reader.next();
    assert(reader.getNodeType() == te::xml::START_ELEMENT);

    style.reset(te::se::serialize::Style::getInstance().read(reader));

    assert(reader.getNodeType() == te::xml::END_ELEMENT);
    assert(reader.getElementLocalName() == "Style");

    reader.next();
  }

  /* has a Selection Style Element ? */
  std::unique_ptr<te::se::Style> selectionStyle;

  if ((reader.getNodeType() == te::xml::START_ELEMENT) &&
    (reader.getElementLocalName() == "SelectionStyle"))
  {
    reader.next();
    assert(reader.getNodeType() == te::xml::START_ELEMENT);

    selectionStyle.reset(te::se::serialize::Style::getInstance().read(reader));

    assert(reader.getNodeType() == te::xml::END_ELEMENT);
    assert(reader.getElementLocalName() == "SelectionStyle");

    reader.next();
  }

  /* GeopxDatasetLayer USER */
  assert(reader.getNodeType() == te::xml::START_ELEMENT);
  assert(reader.getElementLocalName() == "GeopxUser");
  reader.next();
  assert(reader.getNodeType() == te::xml::VALUE);
  std::string geopxUser = reader.getElementValue();
  reader.next();
  assert(reader.getNodeType() == te::xml::END_ELEMENT);

  /* GeopxDatasetLayer PROFILE */
  reader.next();
  assert(reader.getNodeType() == te::xml::START_ELEMENT);
  assert(reader.getElementLocalName() == "GeopxProfile");
  reader.next();
  assert(reader.getNodeType() == te::xml::VALUE);
  std::string geopxProfile = reader.getElementValue();
  reader.next();
  assert(reader.getNodeType() == te::xml::END_ELEMENT);

  /* GeopixelDataSetLayer THEME ID */
  reader.next();
  assert(reader.getNodeType() == te::xml::START_ELEMENT);
  assert(reader.getElementLocalName() == "GeopxThemeId");
  reader.next();
  assert(reader.getNodeType() == te::xml::VALUE);
  int themeId = reader.getElementValueAsInt32();
  reader.next();
  assert(reader.getNodeType() == te::xml::END_ELEMENT);

  reader.next();
  assert(reader.getNodeType() == te::xml::END_ELEMENT);
  assert(reader.getElementLocalName() == "GeopxDataSetLayer");

  reader.next();

  std::unique_ptr<geopx::desktop::layer::GeopxDataSetLayer> layer(new geopx::desktop::layer::GeopxDataSetLayer(id, title, nullptr));
  layer->setExtent(*mbr.get());
  layer->setVisibility(te::map::serialize::GetVisibility(visible));
  layer->setDataSetName(dataset);
  layer->setDataSourceId(datasourceId);
  layer->setRendererType(rendererId);
  layer->setCompositionMode(static_cast<te::map::CompositionMode>(compositionMode));
  layer->setStyle(style.release());
  layer->setSelectionStyle(selectionStyle.release());
  layer->setSRID(srid);
  layer->setEncoding(te::core::CharEncoding::getEncodingType(encoding));

  if (grouping)
    layer->setGrouping(grouping);

  if (chart.get())
    layer->setChart(chart.release());

  if (rc.get())
    layer->setRasterContrast(rc.release());

  layer->setUser(geopxUser);
  layer->setProfile(geopxProfile);
  layer->setThemeId(themeId);

  return layer.release();
}

void geopx::desktop::layer::serialize::LayerWriter(const te::map::AbstractLayer *alayer, te::xml::AbstractWriter &writer)
{
  const geopx::desktop::layer::TiledLayer* layer = dynamic_cast<const geopx::desktop::layer::TiledLayer*>(alayer);

  if(layer == nullptr)
    return;

  writer.writeStartElement("te_map:TiledLayer");

  te::map::serialize::WriteAbstractLayer(layer, writer);

  writer.writeElement("te_map:DataSetName", layer->getDataSetName());
  writer.writeElement("te_map:DataSourceId", layer->getDataSourceId());
  writer.writeElement("te_map:SRID", layer->getSRID());
  te::serialize::xml::SaveExtent(layer->getExtent(), writer);
  writer.writeElement("te_map:RendererId", layer->getRendererType());

  writer.writeElement("te_map:TiledLayerURL", layer->getTiledLayerURL());
  writer.writeElement("te_map:TiledLayerSize", layer->getTileSize());

  writer.writeEndElement("te_map:TiledLayer");
}

void geopx::desktop::layer::serialize::GeopxDataSetLayerWriter(const te::map::AbstractLayer* alayer, te::xml::AbstractWriter& writer)
{
  const geopx::desktop::layer::GeopxDataSetLayer* layer = dynamic_cast<const geopx::desktop::layer::GeopxDataSetLayer*>(alayer);

  if (layer == nullptr)
    return;

  writer.writeStartElement("te_map:GeopxDataSetLayer");

  te::map::serialize::WriteAbstractLayer(layer, writer);

  writer.writeElement("te_map:DataSetName", layer->getDataSetName());
  writer.writeElement("te_map:DataSourceId", layer->getDataSourceId());
  te::map::serialize::WriteSRIDValue(layer->getSRID(), writer);
  te::serialize::xml::SaveExtent(layer->getExtent(), writer);
  writer.writeElement("te_map:RendererId", layer->getRendererType());
  writer.writeElement("te_map:CompositionMode", static_cast<int>(layer->getCompositionMode()));

  if (layer->getStyle())
  {
    writer.writeStartElement("te_map:Style");

    te::se::serialize::Style::getInstance().write(layer->getStyle(), writer);

    writer.writeEndElement("te_map:Style");
  }

  if (layer->getSelectionStyle())
  {
    writer.writeStartElement("te_map:SelectionStyle");

    te::se::serialize::Style::getInstance().write(layer->getSelectionStyle(), writer);

    writer.writeEndElement("te_map:SelectionStyle");
  }

  writer.writeElement("te_map:GeopxUser", layer->getUser());
  writer.writeElement("te_map:GeopxProfile", layer->getProfile());
  writer.writeElement("te_map:GeopxThemeId", layer->getThemeId());

  writer.writeEndElement("te_map:GeopxDataSetLayer");
}
