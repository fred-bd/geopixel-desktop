/*!
  \file geopx-desktop/src/geopixeltools/ForestMonitorToolBarAction.cpp

  \brief This file defines the Forest Monitor Tool Bar Action class
*/

#include "ForestMonitorToolBarAction.h"

// Terralib
#include <terralib/qt/af/connectors/MapDisplay.h>
#include <terralib/qt/af/events/MapEvents.h>
#include <terralib/qt/af/ApplicationController.h>
#include <terralib/qt/af/BaseApplication.h>
#include <terralib/qt/widgets/canvas/MapDisplay.h>

// Qt
#include <QtCore/QObject>

// STL
#include <memory>

geopx::tools::ForestMonitorToolBarAction::ForestMonitorToolBarAction(QMenu* menu) :geopx::tools::AbstractAction(menu)
{
  createAction(tr("Forest Monitor Tool Bar...").toStdString(), "");

  m_dlg = 0;
}

geopx::tools::ForestMonitorToolBarAction::~ForestMonitorToolBarAction()
{
  delete m_dlg;
}

void geopx::tools::ForestMonitorToolBarAction::onActionActivated(bool checked)
{
  //get input layers
  std::list<te::map::AbstractLayerPtr> layersList = getLayers();

  //get display extent
  te::qt::af::BaseApplication* ba = dynamic_cast<te::qt::af::BaseApplication*>(te::qt::af::AppCtrlSingleton::getInstance().getMainWindow());

  if (!ba || !ba->getMapDisplay())
  {
    return;
  }

  te::qt::af::evt::GetMapDisplay e;
  emit triggered(&e);

  //show interface
  if (m_dlg)
    delete m_dlg;

  m_dlg = new geopx::tools::ForestMonitorToolBarDialog(te::qt::af::AppCtrlSingleton::getInstance().getMainWindow(), Qt::Tool);

  m_dlg->setLayerList(layersList);

  m_dlg->setMapDisplay(e.m_display);

  m_dlg->show();
}
