/*!
  \file geopx-desktop/src/geopixeltools/AbstractAction.cpp

  \brief This file defines the abstract class AbstractAction
*/

#include "AbstractAction.h"

// Terralib
#include <terralib/qt/af/events/LayerEvents.h>
#include <terralib/qt/af/ApplicationController.h>

// STL
#include <cassert>

geopx::tools::AbstractAction::AbstractAction(QMenu* menu) :
  QObject(),
  m_menu(menu), 
  m_action(0)
{
}

geopx::tools::AbstractAction::~AbstractAction()
{
  // do not delete m_action pointer because its son of ?????
}

void geopx::tools::AbstractAction::createAction(std::string name, std::string pixmap)
{
  assert(m_menu);

  m_action = new QAction(m_menu);

  m_action->setText(name.c_str());

  if(pixmap.empty() == false)
    m_action->setIcon(QIcon::fromTheme(pixmap.c_str()));

  connect(m_action, SIGNAL(triggered(bool)), this, SLOT(onActionActivated(bool)));

  m_menu->addAction(m_action);
}

void geopx::tools::AbstractAction::addNewLayer(te::map::AbstractLayerPtr layer)
{
  te::qt::af::evt::LayerAdded evt(layer.get());

  te::qt::af::AppCtrlSingleton::getInstance().trigger(&evt);
}

std::list<te::map::AbstractLayerPtr> geopx::tools::AbstractAction::getLayers()
{
  te::qt::af::evt::GetAvailableLayers e;

  te::qt::af::AppCtrlSingleton::getInstance().trigger(&e);

  std::list<te::map::AbstractLayerPtr> allLayers = e.m_layers;

  std::list<te::map::AbstractLayerPtr> layers;

  for (std::list<te::map::AbstractLayerPtr>::iterator it = allLayers.begin(); it != allLayers.end(); ++it)
  {
    if ((*it)->isValid())
      layers.push_back(*it);
  }

  return layers;
}
