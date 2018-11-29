/*!
  \file geopx-desktop/src/geopixeldesktop/ApplicationConnector.cpp

  \brief A connector for the TerraLib application items to the Geopixel App
*/

#include "ApplicationConnector.h"

geopx::desktop::ApplicationConnector::ApplicationConnector(te::qt::widgets::LayerItemView* layerExplorer, te::qt::widgets::StyleDockWidget* styleExplorer, QObject* parent)
  : QObject(parent),
  m_layerExplorer(layerExplorer),
  m_styleExplorer(styleExplorer)
{
  assert(m_layerExplorer);
  assert(m_styleExplorer);

  connect(m_layerExplorer, SIGNAL(visibilityChanged()), SLOT(onLayerVisibilityChanged()));

  connect(m_styleExplorer, SIGNAL(symbolChanged(te::map::AbstractLayer*)), SLOT(styleChanged(te::map::AbstractLayer*)));
}

geopx::desktop::ApplicationConnector::~ApplicationConnector() = default;


void geopx::desktop::ApplicationConnector::onLayerVisibilityChanged()
{
  emit drawLayers();
}

void  geopx::desktop::ApplicationConnector::styleChanged(te::map::AbstractLayer* l)
{
  emit drawLayers();
}
