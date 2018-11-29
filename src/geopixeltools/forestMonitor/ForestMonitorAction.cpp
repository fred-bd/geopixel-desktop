/*!
  \file geopx-desktop/src/geopixeltools/ForestMonitorAction.cpp

  \brief This file defines the Forest Monitor Action class
*/

#include "ForestMonitorAction.h"
#include "qt/ForestMonitorDialog.h"

// Terralib
#include <terralib/qt/af/ApplicationController.h>
#include <terralib/qt/af/BaseApplication.h>


// Qt
#include <QtCore/QObject>

// STL
#include <memory>

geopx::tools::ForestMonitorAction::ForestMonitorAction(QMenu* menu):geopx::tools::AbstractAction(menu)
{
  createAction(tr("Forest Monitor...").toStdString(), "");
}

geopx::tools::ForestMonitorAction::~ForestMonitorAction()
{
}

void geopx::tools::ForestMonitorAction::onActionActivated(bool checked)
{
  //get input layers
  std::list<te::map::AbstractLayerPtr> layersList = getLayers();

  //get display extent
  te::qt::af::BaseApplication* ba = dynamic_cast<te::qt::af::BaseApplication*>(te::qt::af::AppCtrlSingleton::getInstance().getMainWindow());

  //show interface
  geopx::tools::ForestMonitorDialog dlg(te::qt::af::AppCtrlSingleton::getInstance().getMainWindow());

  dlg.setLayerList(layersList);

  if(dlg.exec() == QDialog::Accepted)
  {
    //add new layer
    addNewLayer(dlg.getOutputLayer());
  }
}
