/*!
  \file geopx-desktop/src/geopixeltools/photoindex/PhotoIndexAction.cpp

  \brief This file defines the Photo Index Action class
*/


#include "PhotoIndexAction.h"
#include "qt/PhotoIndexDialog.h"

// Terralib
#include <terralib/qt/af/ApplicationController.h>

// Qt
#include <QtCore/QObject>

// STL
#include <memory>

geopx::tools::PhotoIndexAction::PhotoIndexAction(QMenu* menu):geopx::tools::AbstractAction(menu)
{
  createAction(tr("Photo Index...").toStdString(), "");
}

geopx::tools::PhotoIndexAction::~PhotoIndexAction()
{
}

void geopx::tools::PhotoIndexAction::onActionActivated(bool checked)
{
  //show interface
  geopx::tools::PhotoIndexDialog dlg(te::qt::af::AppCtrlSingleton::getInstance().getMainWindow());

  if(dlg.exec() == QDialog::Accepted)
  {
    //add new layer
    addNewLayer(dlg.getOutputLayer());
  }
}
