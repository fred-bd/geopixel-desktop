/*!
  \file geopx-desktop/src/geopixeldesktop/layer/TiledLayerRendererFactory.cpp

  \brief  This is the concrete factory for renderers of a TiledLayer.
*/

//TerraLib
#include "TiledLayerRenderer.h"
#include "TiledLayerRendererFactory.h"


geopx::desktop::layer::TiledLayerRendererFactory geopx::desktop::layer::TiledLayerRendererFactory::sm_factory;

geopx::desktop::layer::TiledLayerRendererFactory::~TiledLayerRendererFactory() = default;

te::map::AbstractRenderer* geopx::desktop::layer::TiledLayerRendererFactory::build()
{
  return new geopx::desktop::layer::TiledLayerRenderer;
}

geopx::desktop::layer::TiledLayerRendererFactory::TiledLayerRendererFactory()
  : te::map::RendererFactory("TILED_LAYER_RENDERER")
{

}
