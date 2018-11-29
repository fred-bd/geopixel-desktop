/*!
  \file geopx-desktop/src/geopixeldesktop/datasource/GeopixelConnector.cpp

  \brief  Geopixel connector implementation for the Qt data source widget.
*/

#include "GeopixelConnector.h"
#include "GeopixelConnectorDialog.h"

// TerraLib
#include <terralib/dataaccess/datasource/DataSource.h>
#include <terralib/dataaccess/datasource/DataSourceManager.h>
#include <terralib/dataaccess/datasource/DataSourceInfoManager.h>

// Boost
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/filesystem.hpp>

// Qt
#include <QFileDialog>
#include <QMessageBox>

geopx::desktop::GeopixelConnector::GeopixelConnector(QWidget* parent, Qt::WindowFlags f)
  : te::qt::widgets::AbstractDataSourceConnector(parent, f)
{
}

geopx::desktop::GeopixelConnector::~GeopixelConnector() = default;

void geopx::desktop::GeopixelConnector::connect(std::list<te::da::DataSourceInfoPtr>& datasources)
{
  std::unique_ptr<GeopixelConnectorDialog> cdialog(new GeopixelConnectorDialog(static_cast<QWidget*>(parent())));

  int retval = cdialog->exec();

  //if(retval == QDialog::Rejected)
  //  return;

  //te::da::DataSourceInfoPtr ds = cdialog->getDataSource();

  //if(ds.get() != nullptr)
  //{
  //  if(te::da::DataSourceInfoManager::getInstance().add(ds))
  //    datasources.push_back(ds);

  //  te::da::DataSourceManager::getInstance().make(ds->getId(), ds->getType(), ds->getConnInfo());
  //}
}

void geopx::desktop::GeopixelConnector::create(std::list<te::da::DataSourceInfoPtr>& datasources)
{

}

void geopx::desktop::GeopixelConnector::update(std::list<te::da::DataSourceInfoPtr>& datasources)
{

}

void geopx::desktop::GeopixelConnector::remove(std::list<te::da::DataSourceInfoPtr>& datasources)
{
  for(std::list<te::da::DataSourceInfoPtr>::iterator it = datasources.begin(); it != datasources.end(); ++it)
  {
    if(it->get() == nullptr)
      continue;

// first remove driver
    te::da::DataSourcePtr rds = te::da::DataSourceManager::getInstance().find((*it)->getId());

    if(rds.get())
    {
      te::da::DataSourceManager::getInstance().detach(rds);
      rds.reset();
    }

// then remove data source
    te::da::DataSourceInfoManager::getInstance().remove((*it)->getId());
  }
}

