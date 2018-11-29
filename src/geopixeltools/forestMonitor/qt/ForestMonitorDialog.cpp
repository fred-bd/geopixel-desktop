/*!
  \file geopx-desktop/src/geopixeltools/qt/ForestMonitorDialog.cpp

  \brief This interface is used to get the input parameters for forest monitor information.
*/

#include "ForestMonitorDialog.h"
#include "ui_ForestMonitorDialogForm.h"
#include "../core/ForestMonitorService.h"

// TerraLib
#include <terralib/common/progress/ProgressManager.h>
#include <terralib/dataaccess/datasource/DataSource.h>
#include <terralib/dataaccess/datasource/DataSourceInfoManager.h>
#include <terralib/dataaccess/datasource/DataSourceManager.h>
#include <terralib/dataaccess/utils/Utils.h>
#include <terralib/qt/widgets/datasource/selector/DataSourceSelectorDialog.h>
#include <terralib/qt/widgets/layer/utils/DataSet2Layer.h>
#include <terralib/qt/widgets/progress/ProgressViewerDialog.h>

// Qt
#include <QFileDialog>
#include <QMessageBox>
#include <QValidator>

// Boost
#include <boost/filesystem.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

Q_DECLARE_METATYPE(te::map::AbstractLayerPtr);

geopx::tools::ForestMonitorDialog::ForestMonitorDialog(QWidget* parent, Qt::WindowFlags f)
  : QDialog(parent, f),
    m_ui(new Ui::ForestMonitorDialogForm)
{
  // add controls
  m_ui->setupUi(this);

  m_ui->m_targetDatasourceToolButton->setIcon(QIcon::fromTheme("datasource"));

  // connectors
  connect(m_ui->m_okPushButton, SIGNAL(clicked()), this, SLOT(onOkPushButtonClicked()));
  connect(m_ui->m_targetDatasourceToolButton, SIGNAL(pressed()), this, SLOT(onTargetDatasourceToolButtonPressed()));
  connect(m_ui->m_targetFileToolButton, SIGNAL(pressed()), this,  SLOT(onTargetFileToolButtonPressed()));

  //validators
  m_ui->m_angleTolLineEdit->setValidator(new QDoubleValidator(this));
  m_ui->m_centroidDistLineEdit->setValidator(new QDoubleValidator(this));
  m_ui->m_distTolLineEdit->setValidator(new QDoubleValidator(this));
}

geopx::tools::ForestMonitorDialog::~ForestMonitorDialog()
{

}

void geopx::tools::ForestMonitorDialog::setLayerList(std::list<te::map::AbstractLayerPtr> list)
{
  //clear combos
  m_ui->m_centroidLayerComboBox->clear();
  m_ui->m_parceLayerComboBox->clear();
  m_ui->m_angleLayerComboBox->clear();

  //fill combos
  std::list<te::map::AbstractLayerPtr>::iterator it = list.begin();

  while(it != list.end())
  {
    te::map::AbstractLayerPtr l = *it;

    if(l->isValid())
    {
      std::unique_ptr<te::da::DataSetType> dsType = l->getSchema();

      if(dsType->hasGeom())
      {
        m_ui->m_centroidLayerComboBox->addItem(it->get()->getTitle().c_str(), QVariant::fromValue(l));
        m_ui->m_parceLayerComboBox->addItem(it->get()->getTitle().c_str(), QVariant::fromValue(l));
        m_ui->m_angleLayerComboBox->addItem(it->get()->getTitle().c_str(), QVariant::fromValue(l));
      }
    }

    ++it;
  }
}

te::map::AbstractLayerPtr geopx::tools::ForestMonitorDialog::getOutputLayer()
{
  return m_outputLayer;
}

void geopx::tools::ForestMonitorDialog::onOkPushButtonClicked()
{  
  // check input parameters
  if(m_ui->m_repositoryLineEdit->text().isEmpty())
  {
    QMessageBox::information(this, tr("Warning"), tr("Define a repository for the result."));
    return;
  }
       
  if(m_ui->m_newLayerNameLineEdit->text().isEmpty())
  {
    QMessageBox::information(this, tr("Warning"), tr("Define a name for the resulting layer."));
    return;
  }

  double angleTol;
  if(m_ui->m_angleTolLineEdit->text().isEmpty())
  {
    QMessageBox::information(this, tr("Warning"), tr("Angle tolerance not defined."));
    return;
  }
  angleTol = m_ui->m_angleTolLineEdit->text().toDouble();

  double centroidDist;
  if(m_ui->m_centroidDistLineEdit->text().isEmpty())
  {
    QMessageBox::information(this, tr("Warning"), tr("Centroid Distance not defined."));
    return;
  }
  centroidDist = m_ui->m_centroidDistLineEdit->text().toDouble();

  double distTol;
  if(m_ui->m_distTolLineEdit->text().isEmpty())
  {
    QMessageBox::information(this, tr("Warning"), tr("Distance tolerance not defined."));
    return;
  }
  distTol = m_ui->m_distTolLineEdit->text().toDouble();

  //get centroid layer
  QVariant centroidVarLayer = m_ui->m_centroidLayerComboBox->itemData(m_ui->m_centroidLayerComboBox->currentIndex(), Qt::UserRole);
  te::map::AbstractLayerPtr centroidLayer = centroidVarLayer.value<te::map::AbstractLayerPtr>();

  //get parcel layer
  QVariant parcelVarLayer = m_ui->m_parceLayerComboBox->itemData(m_ui->m_parceLayerComboBox->currentIndex(), Qt::UserRole);
  te::map::AbstractLayerPtr parcelLayer = parcelVarLayer.value<te::map::AbstractLayerPtr>();

  //get angle layer
  QVariant angleVarLayer = m_ui->m_angleLayerComboBox->itemData(m_ui->m_angleLayerComboBox->currentIndex(), Qt::UserRole);
  te::map::AbstractLayerPtr angleLayer = angleVarLayer.value<te::map::AbstractLayerPtr>();

  //get datasource
  te::da::DataSourcePtr outputDataSource;

  if(m_toFile)
  {
    std::string repository = m_ui->m_repositoryLineEdit->text().toStdString();

    //create new data source
    boost::filesystem::path uri(repository);

    std::string dsinfo("file://" + uri.string());

    boost::uuids::basic_random_generator<boost::mt19937> gen;
    boost::uuids::uuid u = gen();
    std::string id_ds = boost::uuids::to_string(u);

    te::da::DataSourceInfoPtr dsInfoPtr(new te::da::DataSourceInfo);
    dsInfoPtr->setConnInfo(dsinfo);
    dsInfoPtr->setTitle(uri.stem().string());
    dsInfoPtr->setAccessDriver("OGR");
    dsInfoPtr->setType("OGR");
    dsInfoPtr->setDescription(uri.string());
    dsInfoPtr->setId(id_ds);

    te::da::DataSourceInfoManager::getInstance().add(dsInfoPtr);

    outputDataSource = te::da::DataSourceManager::getInstance().get(id_ds, "OGR", dsInfoPtr->getConnInfo());

    m_outputDatasource = dsInfoPtr;
  }
  else
  {
    outputDataSource = te::da::GetDataSource(m_outputDatasource->getId());
  }

  //create datasource to save the output information
  std::string dataSetName = m_ui->m_newLayerNameLineEdit->text().toStdString();

  std::size_t idx = dataSetName.find(".");
  if (idx != std::string::npos)
        dataSetName=dataSetName.substr(0,idx);


  //progress
  te::qt::widgets::ProgressViewerDialog v(this);
  int id = te::common::ProgressManager::getInstance().addViewer(&v);

  QApplication::setOverrideCursor(Qt::WaitCursor);

  try
  {
    geopx::tools::ForestMonitorService fms;

    fms.setInputParameters(centroidLayer, parcelLayer, angleLayer, angleTol, centroidDist, distTol);

    fms.setOutputParameters(outputDataSource, dataSetName);

    fms.runService();

    //create layer
    te::da::DataSourcePtr outDataSource = te::da::GetDataSource(m_outputDatasource->getId());
    
    te::qt::widgets::DataSet2Layer converter(m_outputDatasource->getId());
      
    te::da::DataSetTypePtr dt(outDataSource->getDataSetType(dataSetName).release());

    m_outputLayer = converter(dt);
  }
  catch(const std::exception& e)
  {
    QMessageBox::warning(this, tr("Warning"), e.what());

    te::common::ProgressManager::getInstance().removeViewer(id);

    QApplication::restoreOverrideCursor();

    return;
  }
  catch(...)
  {
    QMessageBox::warning(this, tr("Warning"), tr("Internal Error."));

    te::common::ProgressManager::getInstance().removeViewer(id);

    QApplication::restoreOverrideCursor();

    return;
  }

  te::common::ProgressManager::getInstance().removeViewer(id);

  QApplication::restoreOverrideCursor();

  accept();
}

void geopx::tools::ForestMonitorDialog::onTargetDatasourceToolButtonPressed()
{
  m_ui->m_newLayerNameLineEdit->clear();
  m_ui->m_newLayerNameLineEdit->setEnabled(true);

  te::qt::widgets::DataSourceSelectorDialog dlg(this);
  dlg.exec();

  std::list<te::da::DataSourceInfoPtr> dsPtrList = dlg.getSelecteds();

  if(dsPtrList.size() <= 0)
    return;

  std::list<te::da::DataSourceInfoPtr>::iterator it = dsPtrList.begin();

  m_ui->m_repositoryLineEdit->setText(QString(it->get()->getTitle().c_str()));

  m_outputDatasource = *it;
  
  m_toFile = false;
}

void geopx::tools::ForestMonitorDialog::onTargetFileToolButtonPressed()
{
  m_ui->m_newLayerNameLineEdit->clear();
  m_ui->m_repositoryLineEdit->clear();
  
  QString fileName = QFileDialog::getSaveFileName(this, tr("Save as..."), QString(), tr("Shapefile (*.shp *.SHP);;"),0, QFileDialog::DontConfirmOverwrite);
  
  if (fileName.isEmpty())
    return;
  
  boost::filesystem::path outfile(fileName.toStdString());

  m_ui->m_repositoryLineEdit->setText(outfile.string().c_str());

  m_ui->m_newLayerNameLineEdit->setText(outfile.leaf().string().c_str());

  m_ui->m_newLayerNameLineEdit->setEnabled(false);
  
  m_toFile = true;
}
