/*!
\file geopx-desktop/src/geopixeltools/tileGenerator/TileGeneratorAction.cpp

\brief This file defines the Tile Generator Action class
*/

#include "TileGeneratorAction.h"

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

geopx::tools::TileGeneratorAction::TileGeneratorAction(QMenu* menu):geopx::tools::AbstractAction(menu)
{
  createAction(tr("Tile Generator...").toStdString(), "");

  m_dlg = 0;
}

geopx::tools::TileGeneratorAction::~TileGeneratorAction()
{
  delete m_dlg;
}

void geopx::tools::TileGeneratorAction::onActionActivated(bool checked)
{
  //get input layers
  std::list<te::map::AbstractLayerPtr> layersList = getLayers();

  //get display extent
  te::qt::af::BaseApplication* ba = dynamic_cast<te::qt::af::BaseApplication*>(te::qt::af::AppCtrlSingleton::getInstance().getMainWindow());

  te::gm::Envelope env;
  int srid = TE_UNKNOWN_SRS;
  
  if(ba && ba->getMapDisplay())
  {
    env = ba->getMapDisplay()->getExtent();
    srid = ba->getMapDisplay()->getSRID();
  }

  te::qt::af::evt::GetMapDisplay e;

  te::qt::af::AppCtrlSingleton::getInstance().trigger(&e);

  //show interface
  if (m_dlg)
    delete m_dlg;

  m_dlg = new geopx::tools::TileGeneratorDialog(te::qt::af::AppCtrlSingleton::getInstance().getMainWindow());

  m_dlg->setExtentInfo(env, srid);

  m_dlg->setLayerList(layersList);

  m_dlg->setMapDisplay(e.m_display);

  m_dlg->show();
}
