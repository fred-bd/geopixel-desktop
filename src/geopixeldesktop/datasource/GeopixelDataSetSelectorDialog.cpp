/*!
  \file geopx-desktop/src/geopixeldesktop/datasource/GeopixelDataSetSelectorDialog.cpp

  \brief  A dialog window for showing the data sets from a geopixel data source.
*/

#include "../Utils.h"
#include "GeopixelDataSetSelectorDialog.h"
#include "ui_GeopixelDataSetSelectorDialogForm.h"

#include "GeopixelDataModelUtils.h"
#include "../Utils.h"
#include "../layer/GeopxDataSetLayer.h"
#include "../layer/TiledLayer.h"

// Terralib
#include <terralib/dataaccess/utils/Utils.h>
#include <terralib/dataaccess/dataset/DataSetType.h>
#include <terralib/geometry/GeometryProperty.h>
#include <terralib/qt/af/ApplicationController.h>
#include <terralib/qt/af/events/LayerEvents.h>
#include <terralib/qt/widgets/layer/utils/DataSet2Layer.h>
#include <terralib/se/Utils.h>

// Boost
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

// Qt
#include <QMessageBox>


geopx::desktop::GeopixelDataSetSelectorDialog::GeopixelDataSetSelectorDialog(QWidget* parent, Qt::WindowFlags f)
  : QDialog(parent, f)
  , m_ui(new Ui::GeopixelDataSetSelectorDialogForm)
{
  // add controls
  m_ui->setupUi(this);

  m_ui->m_logo->setPixmap(QIcon(geopx::desktop::FindInPath("share/geopixeldesktop/images/png/geopixeldesktop-icon-transp-2.png").c_str()).pixmap(48, 48));

  //connects
  connect(m_ui->m_addPushButton, SIGNAL(clicked()), this, SLOT(onAddPushButtonClicked()));
}

geopx::desktop::GeopixelDataSetSelectorDialog::~GeopixelDataSetSelectorDialog() =  default;

void geopx::desktop::GeopixelDataSetSelectorDialog::setConnection(te::da::DataSource* ds, std::string user, std::string profile, bool XYZDataSet)
{
  m_dataSource = ds;

  m_dataSource->open();

  m_user = user;

  m_profile = profile;

  m_xyzDataSet = XYZDataSet;

  if (m_xyzDataSet)
    listLayersXYZ();
  else
    listLayers();
}

void geopx::desktop::GeopixelDataSetSelectorDialog::onAddPushButtonClicked()
{
  for (int i = 0; i < m_ui->m_tableWidget->rowCount(); ++i)
  {
    if (m_ui->m_tableWidget->item(i, 0)->isSelected())
    {
      static boost::uuids::basic_random_generator<boost::mt19937> gen;
      boost::uuids::uuid u = gen();

      if (m_xyzDataSet)
      {
        std::string garbage = "\r\n";
        std::string layerName = m_ui->m_tableWidget->item(i, 0)->text().toUtf8().data();
        std::string url = m_ui->m_tableWidget->item(i, 1)->text().toUtf8().data();
        std::string id = boost::uuids::to_string(u);

        //fix url name
        size_t pos = url.find(garbage);
        if (pos != std::string::npos)
        {
          url = url.substr(0, url.size() - garbage.size());
        }

        geopx::desktop::layer::TiledLayer* layer = new geopx::desktop::layer::TiledLayer(id, layerName);

        layer->setTiledLayerURL(url);

        te::qt::af::evt::LayerAdded evt(layer);

        te::qt::af::AppCtrlSingleton::getInstance().trigger(&evt);
      }
      else
      {
        int cmd_id = m_ui->m_tableWidget->item(i, 0)->data(Qt::UserRole).toInt();
        int themeId = geopx::desktop::datasource::GetThemeId(m_dataSource, m_profile, cmd_id);
        std::string layerName = m_ui->m_tableWidget->item(i, 0)->text().toUtf8().data();
        std::string tableName = m_ui->m_tableWidget->item(i, 1)->text().toUtf8().data();
        std::string id = boost::uuids::to_string(u);

        try
        {
          std::unique_ptr<te::da::DataSetType> dsType = m_dataSource->getDataSetType(tableName);

          geopx::desktop::layer::GeopxDataSetLayerPtr layer(new geopx::desktop::layer::GeopxDataSetLayer(id, layerName));
          layer->setDataSetName(tableName);
          layer->setDataSourceId(m_dataSource->getId());
          layer->setVisibility(te::map::NOT_VISIBLE);
          layer->setRendererType("ABSTRACT_LAYER_RENDERER");
          layer->setUser(m_user);
          layer->setProfile(m_profile);
          layer->setThemeId(themeId);

          te::gm::GeometryProperty* gp = te::da::GetFirstGeomProperty(dsType.get());

          std::unique_ptr<te::gm::Envelope> mbr(m_dataSource->getExtent(dsType->getName(), gp->getName()));

          layer->setSRID(gp->getSRID());

          if (mbr.get() != nullptr)
            layer->setExtent(*mbr);

          layer->setStyle(te::se::CreateFeatureTypeStyle(gp->getGeometryType()));

          te::qt::af::evt::LayerAdded evt(layer);

          te::qt::af::AppCtrlSingleton::getInstance().trigger(&evt);
        }
        catch (...)
        {
          QString errorMsg = tr("The layer: ");
          errorMsg.append(layerName.c_str());
          errorMsg.append(tr(" could not be created."));

          QMessageBox::warning(this, tr("Warning"), errorMsg);
          continue;
        }
      }
    }
  }
}

void geopx::desktop::GeopixelDataSetSelectorDialog::listLayersXYZ()
{
  m_ui->m_tableWidget->setRowCount(0);

  std::string query = geopx::desktop::datasource::GetQueryLayersXYZ(m_profile);
  std::unique_ptr<te::da::DataSet> datasetProfile = m_dataSource->query(query);
  datasetProfile->moveBeforeFirst();

  while (datasetProfile->moveNext())
  {
    std::string layerName = datasetProfile->getString("nome");
    std::string url = datasetProfile->getString("url");

    int newrow = m_ui->m_tableWidget->rowCount();
    m_ui->m_tableWidget->insertRow(newrow);

    QTableWidgetItem* itemName = new QTableWidgetItem(QString::fromUtf8(layerName.c_str()));
    m_ui->m_tableWidget->setItem(newrow, 0, itemName);

    QTableWidgetItem* itemURL = new QTableWidgetItem(QString::fromUtf8(url.c_str()));
    m_ui->m_tableWidget->setItem(newrow, 1, itemURL);
  }

  m_ui->m_tableWidget->resizeColumnToContents(0);

  //m_ui->m_tableWidget->resizeRowsToContents();
}

void geopx::desktop::GeopixelDataSetSelectorDialog::listLayers()
{
  m_ui->m_tableWidget->setRowCount(0);

  std::string query = geopx::desktop::datasource::GetQueryLayersVec(m_profile);
  std::unique_ptr<te::da::DataSet> datasetProfile = m_dataSource->query(query);
  datasetProfile->moveBeforeFirst();

  while (datasetProfile->moveNext())
  {
    std::string layerName = datasetProfile->getString("nome");
    std::string tableName = datasetProfile->getString("nome_tabela_geo");
    int cmd_id = datasetProfile->getInt32("cmd_id");

    int newrow = m_ui->m_tableWidget->rowCount();
    m_ui->m_tableWidget->insertRow(newrow);

    QTableWidgetItem* itemName = new QTableWidgetItem(QString::fromUtf8(layerName.c_str()));
    m_ui->m_tableWidget->setItem(newrow, 0, itemName);
    itemName->setData(Qt::UserRole, QVariant(cmd_id));

    QTableWidgetItem* itemTable = new QTableWidgetItem(QString::fromUtf8(tableName.c_str()));
    m_ui->m_tableWidget->setItem(newrow, 1, itemTable);
  }

  m_ui->m_tableWidget->resizeColumnToContents(0);

  m_ui->m_tableWidget->resizeRowsToContents();
}
