/*!
\file geopx-desktop/src/geopixeltools/tileGenerator/qt/TileGeneratorAction.cpp

\brief This interface i used to get the input parameters for  tile generation.
*/

#include "TileGeneratorDialog.h"
#include "ui_TileGeneratorDialogForm.h"
#include "../core/TileGeneratorService.h"

// TerraLib
#include <terralib/qt/af/ApplicationController.h>
#include <terralib/qt/widgets/tools/ExtentAcquire.h>
#include <terralib/qt/widgets/progress/ProgressViewerDialog.h>

// Qt
#include <QFileDialog>
#include <QImageWriter>
#include <QMessageBox>

geopx::tools::TileGeneratorDialog::TileGeneratorDialog(QWidget* parent, Qt::WindowFlags f)
  : QDialog(parent, f),
    m_ui(new Ui::TileGeneratorDialogForm)
{
  // add controls
  m_ui->setupUi(this);

  m_ui->m_dirToolButton->setIcon(QIcon::fromTheme("folder-open"));
  m_ui->m_toolButton->setIcon(QIcon::fromTheme("pointer"));

  //set zoom level range
  m_ui->m_zoomMinSpinBox->setMinimum(0);
  m_ui->m_zoomMinSpinBox->setMaximum(25);
  m_ui->m_zoomMinSpinBox->setValue(0);

  m_ui->m_zoomMaxSpinBox->setMinimum(0);
  m_ui->m_zoomMaxSpinBox->setMaximum(25);
  m_ui->m_zoomMaxSpinBox->setValue(25);

  //tile info
  m_ui->m_tileSizeLineEdit->setValidator(new QIntValidator(this));
  m_ui->m_tileSizeLineEdit->setText("256");

  //output file formats
  QList<QByteArray> list = QImageWriter::supportedImageFormats();

  m_ui->m_formatComboBox->clear();
  for(int i = 0; i < list.size(); ++i)
    m_ui->m_formatComboBox->addItem(list[i].constData());

  //create action
  m_action = new QAction(this);
  m_action->setIcon(QIcon::fromTheme("pointer"));
  m_action->setToolTip("Acquire extent from map display.");
  m_action->setCheckable(true);
  m_action->setEnabled(true);
  m_action->setObjectName("tileGen_boxTool");

  //connects
  connect(m_action, SIGNAL(triggered(bool)), this, SLOT(onToolButtonClicked(bool)));
  connect(m_ui->m_dirToolButton, SIGNAL(clicked()), this, SLOT(onDirToolButtonClicked()));
  connect(m_ui->m_validatePushButton, SIGNAL(clicked()), this, SLOT(onValidatePushButtonClicked()));
  connect(m_ui->m_okPushButton, SIGNAL(clicked()), this, SLOT(onOkPushButtonClicked()));

  // Get the action group of map tools.
  m_ui->m_toolButton->setDefaultAction(m_action);

  QActionGroup* toolsGroup = te::qt::af::AppCtrlSingleton::getInstance().findActionGroup("Map.ToolsGroup");
  
  if (toolsGroup)
    toolsGroup->addAction(m_action);
  
  m_clearTool = false;
}

geopx::tools::TileGeneratorDialog::~TileGeneratorDialog()
{
  if (m_clearTool)
    m_appDisplay->getDisplay()->setCurrentTool(0);
}

void geopx::tools::TileGeneratorDialog::setExtentInfo(te::gm::Envelope env, int srid)
{
  m_srid = srid;

  m_env = env;

  m_ui->m_llxLineEdit->setText(QString::number(env.getLowerLeftX(), 'f', 5));
  m_ui->m_llyLineEdit->setText(QString::number(env.getLowerLeftY(), 'f', 5));
  m_ui->m_urxLineEdit->setText(QString::number(env.getUpperRightX(), 'f', 5));
  m_ui->m_uryLineEdit->setText(QString::number(env.getUpperRightY(), 'f', 5));
}

void geopx::tools::TileGeneratorDialog::setMapDisplay(te::qt::af::MapDisplay* mapDisplay)
{
  m_appDisplay = mapDisplay;
}

void geopx::tools::TileGeneratorDialog::setLayerList(std::list<te::map::AbstractLayerPtr> list)
{
  m_layerList = list;
}

void geopx::tools::TileGeneratorDialog::onEnvelopeAcquired(te::gm::Envelope env)
{
  m_env = env;

  m_ui->m_llxLineEdit->setText(QString::number(env.getLowerLeftX(), 'f', 5));
  m_ui->m_llyLineEdit->setText(QString::number(env.getLowerLeftY(), 'f', 5));
  m_ui->m_urxLineEdit->setText(QString::number(env.getUpperRightX(), 'f', 5));
  m_ui->m_uryLineEdit->setText(QString::number(env.getUpperRightY(), 'f', 5));
}

void geopx::tools::TileGeneratorDialog::onToolButtonClicked(bool flag)
{
  if (!flag)
  {
    m_clearTool = false;
    return;
  }

  if (!m_appDisplay)
    return;

  te::qt::widgets::ExtentAcquire* ea = new te::qt::widgets::ExtentAcquire(m_appDisplay->getDisplay(), Qt::BlankCursor);
  m_appDisplay->getDisplay()->setCurrentTool(ea);

  connect(ea, SIGNAL(extentAcquired(te::gm::Envelope)), this, SLOT(onEnvelopeAcquired(te::gm::Envelope)));

  m_clearTool = true;
}

void geopx::tools::TileGeneratorDialog::onDirToolButtonClicked()
{
  QString dirName = QFileDialog::getExistingDirectory(this, tr("Select a directory to save tiles"), "", QFileDialog::ShowDirsOnly);

  if(!dirName.isEmpty())
    m_ui->m_dirLineEdit->setText(dirName);
}

void geopx::tools::TileGeneratorDialog::onValidatePushButtonClicked()
{
  //get dir info
  if(m_ui->m_dirLineEdit->text().isEmpty())
  {
    QMessageBox::warning(this, tr("Warning"), tr("Output path not defined."));
    return;
  }
  std::string path = m_ui->m_dirLineEdit->text().toStdString();

  //get tile size
  if(m_ui->m_tileSizeLineEdit->text().isEmpty())
  {
    QMessageBox::warning(this, tr("Warning"), tr("Tile Size defined."));
    return;
  }
  int tileSize = m_ui->m_tileSizeLineEdit->text().toInt();

  //get format
  std::string format = m_ui->m_formatComboBox->currentText().toStdString();

  bool createMissingTiles = false;
  //create missing tiles
  int response = QMessageBox::question(this, tr("Tile Generator"), tr("Create missing tiles?"), tr("Yes"), tr("No"));

  if(response == 0)
    createMissingTiles = true;

  //progress
  te::qt::widgets::ProgressViewerDialog v(this);

  //run operation
  geopx::tools::TileGeneratorService service;

  try
  {
    service.setInputParameters(m_layerList, m_env, m_srid, m_ui->m_zoomMinSpinBox->value(), m_ui->m_zoomMaxSpinBox->value(), tileSize, path, format);

    service.runValidation(createMissingTiles);
  }
  catch(std::exception e)
  {
    QMessageBox::warning(this, tr("Warning"), e.what());
    return;
  }
  catch(...)
  {
     QMessageBox::warning(this, tr("Warning"), tr("Internal Error."));
    return;
  }

  QMessageBox::information(this, tr("Information"), tr("Tile Validation done!"));
}

void geopx::tools::TileGeneratorDialog::onOkPushButtonClicked()
{
  //get dir info
  if(m_ui->m_dirLineEdit->text().isEmpty())
  {
    QMessageBox::warning(this, tr("Warning"), tr("Output path not defined."));
    return;
  }
  std::string path = m_ui->m_dirLineEdit->text().toStdString();

  //get tile size
  if(m_ui->m_tileSizeLineEdit->text().isEmpty())
  {
    QMessageBox::warning(this, tr("Warning"), tr("Tile Size defined."));
    return;
  }
  int tileSize = m_ui->m_tileSizeLineEdit->text().toInt();

  //get format
  std::string format = m_ui->m_formatComboBox->currentText().toStdString();

  //get tile export type
  bool isRaster = true;

  if(m_ui->m_rasterRadioButton->isChecked())
    isRaster = true;

  if(m_ui->m_vectorRadioButton->isChecked())
    isRaster = false;

  //progress
  te::qt::widgets::ProgressViewerDialog v(this);

  //run operation
  geopx::tools::TileGeneratorService service;

  try
  {
    service.setInputParameters(m_layerList, m_env, m_srid, m_ui->m_zoomMinSpinBox->value(), m_ui->m_zoomMaxSpinBox->value(), tileSize, path, format);

    service.runService(isRaster);
  }
  catch(std::exception e)
  {
    QMessageBox::warning(this, tr("Warning"), e.what());
    return;
  }
  catch(...)
  {
     QMessageBox::warning(this, tr("Warning"), tr("Internal Error."));
    return;
  }

  QMessageBox::information(this, tr("Information"), tr("Tile Generation done!"));

  accept();
}
