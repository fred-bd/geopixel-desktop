/*!
  \file geopx-desktop/src/geopixeltools/core/ForestMonitorService.cpp

  \brief This file implements the service to monitor the forest information.

*/

#include "ForestMonitorService.h"
#include "ForestMonitor.h"

//TerraLib Includes
#include <terralib/core/Exception.h>
#include <terralib/dataaccess/utils/Utils.h>
#include <terralib/datatype/SimpleProperty.h>
#include <terralib/geometry/GeometryProperty.h>
#include <terralib/memory/DataSet.h>


//STL Includes
#include <cassert>

geopx::tools::ForestMonitorService::ForestMonitorService() :
  m_angleTol(0.),
  m_centroidDist(0.),
  m_distTol(0.),
  m_outputDataSetName("")
{
}

geopx::tools::ForestMonitorService::~ForestMonitorService()
{
}

void geopx::tools::ForestMonitorService::setInputParameters(te::map::AbstractLayerPtr centroidLayer, 
                                                                           te::map::AbstractLayerPtr parcelLayer,
                                                                           te::map::AbstractLayerPtr angleLayer,
                                                                           double angleTol, double centroidDist, double distanceTol)
{
  m_centroidLayer = centroidLayer;

  m_parcelLayer = parcelLayer;

  m_angleLayer = angleLayer;

  m_angleTol = angleTol;

  m_centroidDist = centroidDist;

  m_distTol = distanceTol;
}

void geopx::tools::ForestMonitorService::setOutputParameters(te::da::DataSourcePtr ds, std::string outputDataSetName)
{
  m_ds = ds;

  m_outputDataSetName = outputDataSetName;
}

void geopx::tools::ForestMonitorService::runService()
{
  //check input parameters
  checkParameters();

  //get input data
  std::unique_ptr<te::da::DataSet> parcelDataSet = m_parcelLayer->getData();
  std::unique_ptr<te::da::DataSetType> parcelDsType = m_parcelLayer->getSchema();
  int parcelIdIdx, parcelGeomIdx;
  getDataSetTypeInfo(parcelDsType.get(), parcelIdIdx, parcelGeomIdx);

  std::unique_ptr<te::da::DataSet> angleDataSet = m_angleLayer->getData();
  std::unique_ptr<te::da::DataSetType> angleDsType = m_angleLayer->getSchema();
  int angleIdIdx, angleGeomIdx;
  getDataSetTypeInfo(angleDsType.get(), angleIdIdx, angleGeomIdx);

  std::unique_ptr<te::da::DataSet> centroidDataSet = m_centroidLayer->getData();
  std::unique_ptr<te::da::DataSetType> centroidDsType = m_centroidLayer->getSchema();
  int centroidIdIdx, centroidGeomIdx;
  getDataSetTypeInfo(centroidDsType.get(), centroidIdIdx, centroidGeomIdx);

  //get srid
  int srid = getParcelSRID();

  //create output dataset
  std::unique_ptr<te::da::DataSetType> dsType = createDataSetType(srid);

  std::unique_ptr<te::mem::DataSet> ds(new te::mem::DataSet(dsType.get()));

  //generate tracks
  geopx::tools::ForestMonitor fm(m_angleTol, m_centroidDist, m_distTol,ds.get());

  fm.execute(std::move(parcelDataSet), parcelGeomIdx, parcelIdIdx, 
             std::move(angleDataSet), angleGeomIdx, angleIdIdx, 
             std::move(centroidDataSet), centroidGeomIdx, centroidIdIdx);

  //save output information
  saveDataSet(ds.get(), dsType.get());
}

void geopx::tools::ForestMonitorService::checkParameters()
{
  if(!m_centroidLayer.get())
    throw te::core::Exception() << te::ErrorDescription("Centroid Layer not defined.");

  if(!m_parcelLayer.get())
    throw te::core::Exception() << te::ErrorDescription("Parcel Layer not defined.");

  if(!m_angleLayer.get())
    throw te::core::Exception() << te::ErrorDescription("Angle Layer not defined.");

  if(!m_ds.get())
    throw te::core::Exception() << te::ErrorDescription("Data Source not defined.");

  if(m_outputDataSetName.empty())
    throw te::core::Exception() << te::ErrorDescription("Data Source name not defined.");
}

std::unique_ptr<te::da::DataSetType> geopx::tools::ForestMonitorService::createDataSetType(int srid)
{
  std::unique_ptr<te::da::DataSetType> dsType(new te::da::DataSetType(m_outputDataSetName));

  //create id property
  te::dt::SimpleProperty* idProperty = new te::dt::SimpleProperty("trackId", te::dt::INT32_TYPE);
  dsType->add(idProperty);

  //create parcel id property
  te::dt::SimpleProperty* parcelIdProperty = new te::dt::SimpleProperty("parcelId", te::dt::INT32_TYPE);
  dsType->add(parcelIdProperty);

  //create geometry property
  te::gm::GeometryProperty* geomProperty = new te::gm::GeometryProperty("geom", srid, te::gm::LineStringType);
  dsType->add(geomProperty);

  //create primary key
  std::string pkName = "pk_" + m_outputDataSetName;
  te::da::PrimaryKey* pk = new te::da::PrimaryKey(pkName, dsType.get());
  pk->add(idProperty);

  return dsType;
}

void geopx::tools::ForestMonitorService::saveDataSet(te::mem::DataSet* dataSet, te::da::DataSetType* dsType)
{
  assert(dataSet);
  assert(dsType);

  //save dataset
  dataSet->moveBeforeFirst();

  std::map<std::string, std::string> options;

  m_ds->createDataSet(dsType, options);

  m_ds->add(m_outputDataSetName, dataSet, options);
}

void geopx::tools::ForestMonitorService::getDataSetTypeInfo(te::da::DataSetType* dsType, int& idIdx, int& geomIdx)
{
  //geom property info
  te::gm::GeometryProperty* gmProp = te::da::GetFirstGeomProperty(dsType);

  if(gmProp)
  {
    geomIdx = te::da::GetPropertyPos(dsType, gmProp->getName());
  }

  //id info
  te::da::PrimaryKey* pk = dsType->getPrimaryKey();

  if(pk && !pk->getProperties().empty())
  {
    idIdx = te::da::GetPropertyPos(dsType, pk->getProperties()[0]->getName());
  }
}

int geopx::tools::ForestMonitorService::getParcelSRID()
{
  assert(m_parcelLayer);

  std::unique_ptr<te::da::DataSetType> parcelDsType = m_parcelLayer->getSchema();

  te::gm::GeometryProperty* gmProp = te::da::GetFirstGeomProperty(parcelDsType.get());

  int srid = 0;

  if(gmProp)
  {
    srid = gmProp->getSRID();
  }

  return srid;
}
