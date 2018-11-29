/*!
  \file geopx-desktop/src/geopixeltools/photoindex/core/PhotoIndexService.cpp

  \brief This file implements the service to photo index information.
*/

#include "PhotoIndexService.h"
#include "PhotoIndex.h"

//TerraLib Includes
#include <terralib/core/Exception.h>
#include <terralib/dataaccess/datasource/DataSource.h>
#include <terralib/dataaccess/datasource/DataSourceFactory.h>
#include <terralib/dataaccess/utils/Utils.h>
#include <terralib/datatype/SimpleProperty.h>
#include <terralib/geometry/GeometryProperty.h>


//STL Includes
#include <cassert>

geopx::tools::PhotoIndexService::PhotoIndexService() :
  m_path(""),
  m_outputDataSetName("")
{
}

geopx::tools::PhotoIndexService::~PhotoIndexService()
{
}

void geopx::tools::PhotoIndexService::setInputParameters(std::string path)
{
  m_path = path;
}

void geopx::tools::PhotoIndexService::setOutputParameters(te::da::DataSourcePtr ds, std::string outputDataSetName)
{
  m_ds = ds;

  m_outputDataSetName = outputDataSetName;
}

void geopx::tools::PhotoIndexService::runService()
{
  //check input parameters
  checkParameters();

  //get input data source
  std::string connInfo("file://");
  connInfo += m_path;

  te::da::DataSourcePtr inputDataSource = te::da::DataSourceFactory::make("GDAL", connInfo);
  inputDataSource->open();

  //get srid
  int srid = getSRID(inputDataSource);

  //create output dataset
  std::unique_ptr<te::da::DataSetType> dsType = createDataSetType(srid);

  std::unique_ptr<te::mem::DataSet> ds(new te::mem::DataSet(dsType.get()));

  //generate tracks
  geopx::tools::PhotoIndex pi;

  pi.execute(inputDataSource, ds.get());

  //save output information
  saveDataSet(ds.get(), dsType.get());
}

void geopx::tools::PhotoIndexService::checkParameters()
{
  if(!m_ds.get())
    throw te::core::Exception() << te::ErrorDescription("Data Source not defined.");

  if(m_outputDataSetName.empty())
    throw te::core::Exception() << te::ErrorDescription("Data Source name not defined.");

  if(m_path.empty())
    throw te::core::Exception() << te::ErrorDescription("Path not defined.");
}

std::unique_ptr<te::da::DataSetType> geopx::tools::PhotoIndexService::createDataSetType(int srid)
{
  std::unique_ptr<te::da::DataSetType> dsType(new te::da::DataSetType(m_outputDataSetName));

  //create id property
  te::dt::SimpleProperty* idProperty = new te::dt::SimpleProperty("id", te::dt::INT32_TYPE);
  dsType->add(idProperty);

  //create parcel id property
  te::dt::SimpleProperty* parcelIdProperty = new te::dt::SimpleProperty("fileName", te::dt::STRING_TYPE);
  dsType->add(parcelIdProperty);

  //create geometry property
  te::gm::GeometryProperty* geomProperty = new te::gm::GeometryProperty("geom", srid, te::gm::PolygonType);
  dsType->add(geomProperty);

  //create primary key
  std::string pkName = "pk_" + m_outputDataSetName;
  te::da::PrimaryKey* pk = new te::da::PrimaryKey(pkName, dsType.get());
  pk->add(idProperty);

  return dsType;
}

void geopx::tools::PhotoIndexService::saveDataSet(te::mem::DataSet* dataSet, te::da::DataSetType* dsType)
{
  assert(dataSet);
  assert(dsType);

  //save dataset
  dataSet->moveBeforeFirst();

  std::map<std::string, std::string> options;

  m_ds->createDataSet(dsType, options);

  m_ds->add(m_outputDataSetName, dataSet, options);
}

int geopx::tools::PhotoIndexService::getSRID(te::da::DataSourcePtr ds)
{
  std::vector<std::string> dsNames = ds->getDataSetNames();

  std::unique_ptr<te::da::DataSet> firstDataSet = ds->getDataSet(dsNames[0]);

  std::size_t rpos = te::da::GetFirstPropertyPos(firstDataSet.get(), te::dt::RASTER_TYPE);

  std::unique_ptr<te::rst::Raster> inputRst = firstDataSet->getRaster(rpos);

  return inputRst->getSRID();
}
