/*!
  \file geopx-desktop/src/geopixeltools/NDVIAction.cpp

  \brief This file defines the NDVI Class Action class
*/

#include "NDVIAction.h"
#include "qt/NDVIDialog.h"

// Terralib
#include <terralib/qt/af/ApplicationController.h>
#include <terralib/qt/af/BaseApplication.h>

// Qt
#include <QtCore/QObject>

// STL
#include <memory>

geopx::tools::NDVIAction::NDVIAction(QMenu* menu):geopx::tools::AbstractAction(menu)
{
  createAction(tr("NDVI...").toStdString(), "");
}

geopx::tools::NDVIAction::~NDVIAction()
{
}

void geopx::tools::NDVIAction::onActionActivated(bool checked)
{
  //get input layers
  std::list<te::map::AbstractLayerPtr> layersList = getLayers();

  //get display extent
  te::qt::af::BaseApplication* ba = dynamic_cast<te::qt::af::BaseApplication*>(te::qt::af::AppCtrlSingleton::getInstance().getMainWindow());

  //show interface
  geopx::tools::NDVIDialog dlg(te::qt::af::AppCtrlSingleton::getInstance().getMainWindow());

  dlg.setLayerList(layersList);

  if(dlg.exec() == QDialog::Accepted)
  {
    //add new layer
    addNewLayer(dlg.getOutputLayer());
  }
}
