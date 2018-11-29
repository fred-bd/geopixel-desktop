/*!
  \file geopx-desktop/src/geopixeltools/qt/ForestMonitorToolBarDialog.h

  \brief This interface is a tool bar for forest monitor classification operations.
*/


#include "ForestMonitorToolBarDialog.h"
#include "ui_ForestMonitorToolBarDialogForm.h"
#include "tools/Creator.h"
#include "tools/Eraser.h"
#include "tools/TrackDeadClassifier.h"
#include "tools/TrackAutoClassifier.h"
#include "tools/UpdateClass.h"

// TerraLib
#include <terralib/qt/af/ApplicationController.h>
#include <terralib/qt/widgets/canvas/MapDisplay.h>



Q_DECLARE_METATYPE(te::map::AbstractLayerPtr);

geopx::tools::ForestMonitorToolBarDialog::ForestMonitorToolBarDialog(QWidget* parent, Qt::WindowFlags f)
  : QDialog(parent, f),
    m_ui(new Ui::ForestMonitorToolBarDialogForm)
{
  // add controls
  m_ui->setupUi(this);

  m_ui->m_distLineEdit->setValidator(new QDoubleValidator(this));
  m_ui->m_distTolLineEdit->setValidator(new QDoubleValidator(this));
  m_ui->m_distTrackLineEdit->setValidator(new QDoubleValidator(this));
  m_ui->m_polyAreaMinLineEdit->setValidator(new QDoubleValidator(this));
  m_ui->m_polyAreaMaxLineEdit->setValidator(new QDoubleValidator(this));
  m_ui->m_maxDeadLineEdit->setValidator(new QIntValidator(this));
  m_ui->m_deadTolLineEdit->setValidator(new QDoubleValidator(this));
  m_ui->m_thresholdLineEdit->setValidator(new QDoubleValidator(this));

  createActions();
}

geopx::tools::ForestMonitorToolBarDialog::~ForestMonitorToolBarDialog()
{
 // if (m_clearTool)
   // m_appDisplay->setCurrentTool(0);
}

void geopx::tools::ForestMonitorToolBarDialog::setLayerList(std::list<te::map::AbstractLayerPtr> list)
{
  //clear combos
  m_ui->m_layerParcelComboBox->clear();
  m_ui->m_layerPointsComboBox->clear();
  m_ui->m_layerPolyComboBox->clear();
  m_ui->m_layerDirComboBox->clear();

  //fill combos
  std::list<te::map::AbstractLayerPtr>::iterator it = list.begin();

  while(it != list.end())
  {
    te::map::AbstractLayerPtr l = *it;

    if(l->isValid())
    {
      std::unique_ptr<te::da::DataSetType> dsType = l->getSchema();

      if (dsType->hasGeom())
      {
        m_ui->m_layerParcelComboBox->addItem(it->get()->getTitle().c_str(), QVariant::fromValue(l));
        m_ui->m_layerPointsComboBox->addItem(it->get()->getTitle().c_str(), QVariant::fromValue(l));
        m_ui->m_layerDirComboBox->addItem(it->get()->getTitle().c_str(), QVariant::fromValue(l));
      }
      else if (dsType->hasRaster())
      {
        m_ui->m_layerPolyComboBox->addItem(it->get()->getTitle().c_str(), QVariant::fromValue(l));
      }
    }

    ++it;
  }
}

void geopx::tools::ForestMonitorToolBarDialog::setMapDisplay(te::qt::af::MapDisplay* mapDisplay)
{
  m_appDisplay = mapDisplay;
}

void geopx::tools::ForestMonitorToolBarDialog::createActions()
{
  QActionGroup* toolsGroup = te::qt::af::AppCtrlSingleton::getInstance().findActionGroup("Map.ToolsGroup");

  //create actions
  m_actionEraser = new QAction(this);
  m_actionEraser->setIcon(QIcon::fromTheme("pointer-remove-selection"));
  m_actionEraser->setToolTip("Erases a existent object.");
  m_actionEraser->setCheckable(true);
  m_actionEraser->setEnabled(true);
  m_actionEraser->setObjectName("eraser_Tool");

  m_actionUpdate = new QAction(this);
  m_actionUpdate->setIcon(QIcon::fromTheme("view-refresh"));
  m_actionUpdate->setToolTip("Updates a class to Live or if Live turns to Dead.");
  m_actionUpdate->setCheckable(true);
  m_actionUpdate->setEnabled(true);
  m_actionUpdate->setObjectName("update_Tool");

  m_actionCreator = new QAction(this);
  m_actionCreator->setIcon(QIcon::fromTheme("bullet-blue"));
  m_actionCreator->setToolTip("Creates a new point with CREATED class.");
  m_actionCreator->setCheckable(true);
  m_actionCreator->setEnabled(true);
  m_actionCreator->setObjectName("created_Tool");

  m_actionCreatorLive = new QAction(this);
  m_actionCreatorLive->setIcon(QIcon::fromTheme("bullet-green"));
  m_actionCreatorLive->setToolTip("Creates a new point with LIVE class.");
  m_actionCreatorLive->setCheckable(true);
  m_actionCreatorLive->setEnabled(true);
  m_actionCreatorLive->setObjectName("live_Tool");

  m_actionCreatorDead = new QAction(this);
  m_actionCreatorDead->setIcon(QIcon::fromTheme("bullet-black"));
  m_actionCreatorDead->setToolTip("Creates a new point with DEAD class.");
  m_actionCreatorDead->setCheckable(true);
  m_actionCreatorDead->setEnabled(true);
  m_actionCreatorDead->setObjectName("dead_Tool");

  m_actionAutoTrack = new QAction(this);
  m_actionAutoTrack->setIcon(QIcon::fromTheme("wand"));
  m_actionAutoTrack->setToolTip("Automatic Track Classifier.");
  m_actionAutoTrack->setCheckable(true);
  m_actionAutoTrack->setEnabled(true);
  m_actionAutoTrack->setObjectName("autoTrack_Tool");

  m_actionDeadTrack = new QAction(this);
  m_actionDeadTrack->setIcon(QIcon::fromTheme("vp-line-length-hint"));
  m_actionDeadTrack->setToolTip("Automatic Dead Track Generator.");
  m_actionDeadTrack->setCheckable(true);
  m_actionDeadTrack->setEnabled(true);
  m_actionDeadTrack->setObjectName("autoDeadTrack_Tool");

  //connects
  connect(m_actionEraser, SIGNAL(triggered(bool)), this, SLOT(onEraserToolButtonClicked(bool)));
  connect(m_actionUpdate, SIGNAL(triggered(bool)), this, SLOT(onUpdateToolButtonClicked(bool)));
  connect(m_actionCreator, SIGNAL(triggered(bool)), this, SLOT(onCreatorToolButtonClicked(bool)));
  connect(m_actionCreatorLive, SIGNAL(triggered(bool)), this, SLOT(onCreatorLiveToolButtonClicked(bool)));
  connect(m_actionCreatorDead, SIGNAL(triggered(bool)), this, SLOT(onCreatorDeadToolButtonClicked(bool)));
  connect(m_actionAutoTrack, SIGNAL(triggered(bool)), this, SLOT(onTrackAutoClassifierToolButtonClicked(bool)));
  connect(m_actionDeadTrack, SIGNAL(triggered(bool)), this, SLOT(onTrackDeadClassifierToolButtonClicked(bool)));
  
  //associate to button
  m_ui->m_eraserToolButton->setDefaultAction(m_actionEraser);
  m_ui->m_updateToolButton->setDefaultAction(m_actionUpdate);
  m_ui->m_creatorToolButton->setDefaultAction(m_actionCreator);
  m_ui->m_creatorLiveToolButton->setDefaultAction(m_actionCreatorLive);
  m_ui->m_creatorDeadToolButton->setDefaultAction(m_actionCreatorDead);
  m_ui->m_trackAutoClassifierToolButton->setDefaultAction(m_actionAutoTrack);
  m_ui->m_trackDeadClassifierToolButton->setDefaultAction(m_actionDeadTrack);
  
  //add to tool group from app
  toolsGroup->addAction(m_actionEraser);
  toolsGroup->addAction(m_actionUpdate);
  toolsGroup->addAction(m_actionCreator);
  toolsGroup->addAction(m_actionCreatorLive);
  toolsGroup->addAction(m_actionCreatorDead);
  toolsGroup->addAction(m_actionAutoTrack);
  toolsGroup->addAction(m_actionDeadTrack);

  m_clearTool = false;
}

void geopx::tools::ForestMonitorToolBarDialog::onEraserToolButtonClicked(bool flag)
{
  if (!flag)
  {
    m_clearTool = false;
    return;
  }

  if (!m_appDisplay)
    return;

  QVariant varLayer = m_ui->m_layerPointsComboBox->itemData(m_ui->m_layerPointsComboBox->currentIndex(), Qt::UserRole);

  te::map::AbstractLayerPtr layer = varLayer.value<te::map::AbstractLayerPtr>();

  geopx::tools::Eraser* tool = new geopx::tools::Eraser(m_appDisplay->getDisplay(), Qt::ArrowCursor, layer);
  m_appDisplay->getDisplay()->setCurrentTool(tool);

  m_clearTool = true;
}

void geopx::tools::ForestMonitorToolBarDialog::onUpdateToolButtonClicked(bool flag)
{
  if (!flag)
  {
    m_clearTool = false;
    return;
  }

  if (!m_appDisplay)
    return;

  QVariant varLayer = m_ui->m_layerPointsComboBox->itemData(m_ui->m_layerPointsComboBox->currentIndex(), Qt::UserRole);

  te::map::AbstractLayerPtr layer = varLayer.value<te::map::AbstractLayerPtr>();

  geopx::tools::UpdateClass* tool = new geopx::tools::UpdateClass(m_appDisplay->getDisplay(), Qt::ArrowCursor, layer);
  m_appDisplay->getDisplay()->setCurrentTool(tool);

  m_clearTool = true;
}

void geopx::tools::ForestMonitorToolBarDialog::onCreatorToolButtonClicked(bool flag)
{
  if (!flag)
  {
    m_clearTool = false;
    return;
  }

  if (!m_appDisplay)
    return;

  QVariant varLayerPoint = m_ui->m_layerPointsComboBox->itemData(m_ui->m_layerPointsComboBox->currentIndex(), Qt::UserRole);
  te::map::AbstractLayerPtr layerPoints = varLayerPoint.value<te::map::AbstractLayerPtr>();

  QVariant varLayerParcel = m_ui->m_layerParcelComboBox->itemData(m_ui->m_layerParcelComboBox->currentIndex(), Qt::UserRole);
  te::map::AbstractLayerPtr layerParcel = varLayerParcel.value<te::map::AbstractLayerPtr>();

  geopx::tools::Creator* tool = new geopx::tools::Creator(m_appDisplay->getDisplay(), Qt::ArrowCursor, layerPoints, layerParcel, geopx::tools::CREATED_TYPE);
  m_appDisplay->getDisplay()->setCurrentTool(tool);

  m_clearTool = true;
}

void geopx::tools::ForestMonitorToolBarDialog::onCreatorLiveToolButtonClicked(bool flag)
{
  if (!flag)
  {
    m_clearTool = false;
    return;
  }

  if (!m_appDisplay)
    return;

  QVariant varLayerPoint = m_ui->m_layerPointsComboBox->itemData(m_ui->m_layerPointsComboBox->currentIndex(), Qt::UserRole);
  te::map::AbstractLayerPtr layerPoints = varLayerPoint.value<te::map::AbstractLayerPtr>();

  QVariant varLayerParcel = m_ui->m_layerParcelComboBox->itemData(m_ui->m_layerParcelComboBox->currentIndex(), Qt::UserRole);
  te::map::AbstractLayerPtr layerParcel = varLayerParcel.value<te::map::AbstractLayerPtr>();

  geopx::tools::Creator* tool = new geopx::tools::Creator(m_appDisplay->getDisplay(), Qt::ArrowCursor, layerPoints, layerParcel, geopx::tools::LIVE_TYPE);
  m_appDisplay->getDisplay()->setCurrentTool(tool);

  m_clearTool = true;
}

void geopx::tools::ForestMonitorToolBarDialog::onCreatorDeadToolButtonClicked(bool flag)
{
  if (!flag)
  {
    m_clearTool = false;
    return;
  }

  if (!m_appDisplay)
    return;

  QVariant varLayerPoint = m_ui->m_layerPointsComboBox->itemData(m_ui->m_layerPointsComboBox->currentIndex(), Qt::UserRole);
  te::map::AbstractLayerPtr layerPoints = varLayerPoint.value<te::map::AbstractLayerPtr>();

  QVariant varLayerParcel = m_ui->m_layerParcelComboBox->itemData(m_ui->m_layerParcelComboBox->currentIndex(), Qt::UserRole);
  te::map::AbstractLayerPtr layerParcel = varLayerParcel.value<te::map::AbstractLayerPtr>();

  geopx::tools::Creator* tool = new geopx::tools::Creator(m_appDisplay->getDisplay(), Qt::ArrowCursor, layerPoints, layerParcel, geopx::tools::DEAD_TYPE);
  m_appDisplay->getDisplay()->setCurrentTool(tool);

  m_clearTool = true;
}

void geopx::tools::ForestMonitorToolBarDialog::onTrackAutoClassifierToolButtonClicked(bool flag)
{
  if (!flag)
  {
    m_clearTool = false;
    return;
  }

  if (!m_appDisplay)
    return;

  QVariant varLayerPoint = m_ui->m_layerPointsComboBox->itemData(m_ui->m_layerPointsComboBox->currentIndex(), Qt::UserRole);
  te::map::AbstractLayerPtr layerPoints = varLayerPoint.value<te::map::AbstractLayerPtr>();

  QVariant varLayerParcel = m_ui->m_layerParcelComboBox->itemData(m_ui->m_layerParcelComboBox->currentIndex(), Qt::UserRole);
  te::map::AbstractLayerPtr layerParcel = varLayerParcel.value<te::map::AbstractLayerPtr>();

  QVariant varLayerPoly = m_ui->m_layerPolyComboBox->itemData(m_ui->m_layerPolyComboBox->currentIndex(), Qt::UserRole);
  te::map::AbstractLayerPtr layerPoly = varLayerPoly.value<te::map::AbstractLayerPtr>();

  QVariant varLayerDir = m_ui->m_layerDirComboBox->itemData(m_ui->m_layerDirComboBox->currentIndex(), Qt::UserRole);
  te::map::AbstractLayerPtr layerDir = varLayerDir.value<te::map::AbstractLayerPtr>();

  geopx::tools::TrackAutoClassifier* tool = new geopx::tools::TrackAutoClassifier(m_appDisplay->getDisplay(), Qt::ArrowCursor, layerPoints, layerParcel, layerPoly, layerDir);
  tool->setLineEditComponents(m_ui->m_distLineEdit, m_ui->m_distTrackLineEdit, m_ui->m_distTolLineEdit, m_ui->m_distTrackTolLineEdit,  m_ui->m_polyAreaMinLineEdit, m_ui->m_polyAreaMaxLineEdit, m_ui->m_maxDeadLineEdit, m_ui->m_deadTolLineEdit, m_ui->m_thresholdLineEdit);
  tool->setAdjustTrack(m_ui->m_adaptTrackCheckBox->isChecked(), m_ui->m_adjustTrackSpinBox->value());
  m_appDisplay->getDisplay()->setCurrentTool(tool);

  m_clearTool = true;
}

void geopx::tools::ForestMonitorToolBarDialog::onTrackDeadClassifierToolButtonClicked(bool flag)
{
  if (!flag)
  {
    m_clearTool = false;
    return;
  }

  if (!m_appDisplay)
    return;

  QVariant varLayerPoint = m_ui->m_layerPointsComboBox->itemData(m_ui->m_layerPointsComboBox->currentIndex(), Qt::UserRole);
  te::map::AbstractLayerPtr layerPoints = varLayerPoint.value<te::map::AbstractLayerPtr>();

  QVariant varLayerParcel = m_ui->m_layerParcelComboBox->itemData(m_ui->m_layerParcelComboBox->currentIndex(), Qt::UserRole);
  te::map::AbstractLayerPtr layerParcel = varLayerParcel.value<te::map::AbstractLayerPtr>();

  QVariant varLayerPoly = m_ui->m_layerPolyComboBox->itemData(m_ui->m_layerPolyComboBox->currentIndex(), Qt::UserRole);
  te::map::AbstractLayerPtr layerPoly = varLayerPoly.value<te::map::AbstractLayerPtr>();

  geopx::tools::TrackDeadClassifier* tool = new geopx::tools::TrackDeadClassifier(m_appDisplay->getDisplay(), Qt::ArrowCursor, layerPoints, layerParcel, layerPoly);
  tool->setLineEditComponents(m_ui->m_distLineEdit, m_ui->m_distTrackLineEdit, m_ui->m_distTolLineEdit, m_ui->m_distTrackTolLineEdit, m_ui->m_polyAreaMinLineEdit, m_ui->m_polyAreaMaxLineEdit, m_ui->m_deadTolLineEdit, m_ui->m_thresholdLineEdit);
  m_appDisplay->getDisplay()->setCurrentTool(tool);

  m_clearTool = true;
}
