/*!
  \file geopx-desktop/src/geopixeltools/ForestMonitorClassAction.cpp

  \brief This file defines the Forest Monitor Class Action class
*/

#include "ForestMonitorClassAction.h"
#include "qt/ForestMonitorClassDialog.h"

// Terralib
#include <terralib/qt/af/connectors/MapDisplay.h>
#include <terralib/qt/af/ApplicationController.h>
#include <terralib/qt/af/BaseApplication.h>
#include <terralib/qt/widgets/canvas/MapDisplay.h>

// Qt
#include <QtCore/QObject>

// STL
#include <memory>

geopx::tools::ForestMonitorClassAction::ForestMonitorClassAction(QMenu* menu):geopx::tools::AbstractAction(menu)
{
  createAction(tr("Forest Monitor Class...").toStdString(), "");
}

geopx::tools::ForestMonitorClassAction::~ForestMonitorClassAction()
{
}

void geopx::tools::ForestMonitorClassAction::onActionActivated(bool checked)
{
  //get input layers
  std::list<te::map::AbstractLayerPtr> layersList = getLayers();

  //get display extent
  te::qt::af::BaseApplication* ba = dynamic_cast<te::qt::af::BaseApplication*>(te::qt::af::AppCtrlSingleton::getInstance().getMainWindow());

  te::gm::Envelope env;
  int srid = TE_UNKNOWN_SRS;

  if (ba && ba->getMapDisplay())
  {
    env = ba->getMapDisplay()->getExtent();
    srid = ba->getMapDisplay()->getSRID();
  }

  //show interface
  geopx::tools::ForestMonitorClassDialog dlg(te::qt::af::AppCtrlSingleton::getInstance().getMainWindow());

  dlg.setLayerList(layersList);

  dlg.setExtentInfo(env, srid);

  if(dlg.exec() == QDialog::Accepted)
  {
    //add new layer
    addNewLayer(dlg.getOutputLayer());
  }
}
