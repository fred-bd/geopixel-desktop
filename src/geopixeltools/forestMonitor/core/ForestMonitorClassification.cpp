/*!
  \file geopx-desktop/src/geopixeltools/core/ForestMonitorClassification.cpp

  \brief This file contains structures and definitions for forest monitor classification operation.
*/

#include "ForestMonitorClassification.h"

//TerraLib Includes
#include <terralib/common/progress/TaskProgress.h>
#include <terralib/common/Exception.h>
#include <terralib/common/STLUtils.h>
#include <terralib/dataaccess/dataset/DataSetType.h>
#include <terralib/dataaccess/dataset/PrimaryKey.h>
#include <terralib/dataaccess/datasource/DataSource.h>
#include <terralib/dataaccess/datasource/DataSourceInfoManager.h>
#include <terralib/dataaccess/datasource/DataSourceManager.h>
#include <terralib/dataaccess/datasource/DataSourceFactory.h>
#include <terralib/dataaccess/utils/Utils.h>
#include <terralib/datatype/SimpleProperty.h>
#include <terralib/datatype/StringProperty.h>
#include <terralib/geometry/GeometryProperty.h>
#include <terralib/geometry/MultiPoint.h>
#include <terralib/geometry/MultiPolygon.h>
#include <terralib/geometry/Utils.h>
#include <terralib/memory/DataSet.h>
#include <terralib/memory/DataSetItem.h>
#include <terralib/raster/Band.h>
#include <terralib/raster/BandProperty.h>
#include <terralib/raster/Grid.h>
#include <terralib/raster/Raster.h>
#include <terralib/raster/RasterFactory.h>
#include <terralib/raster/Utils.h>

//STL Includes
#include <cassert>

// Boost
#include <boost/filesystem.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

std::unique_ptr<te::rst::Raster> geopx::tools::GenerateFilterRaster(te::rst::Raster* raster, int band, int nIter,
  te::rp::Filter::InputParameters::FilterType fType, std::string type, std::map<std::string, std::string> rinfo)
{
  std::unique_ptr<te::rst::Raster> rasterOut;

  te::rp::Filter algorithmInstance;

  te::rp::Filter::InputParameters algoInputParams;
  algoInputParams.m_iterationsNumber = nIter;
  algoInputParams.m_filterType = fType;
  algoInputParams.m_inRasterBands.push_back(band);
  algoInputParams.m_inRasterPtr = raster;
  algoInputParams.m_enableProgress = false;

  te::rp::Filter::OutputParameters algoOutputParams;
  algoOutputParams.m_rType = type;
  algoOutputParams.m_rInfo = rinfo;

  if (algorithmInstance.initialize(algoInputParams))
  {
    if (algorithmInstance.execute(algoOutputParams))
    {
      rasterOut = std::move(algoOutputParams.m_outputRasterPtr);
    }
  }

  return rasterOut;
}

std::unique_ptr<te::rst::Raster> geopx::tools::GenerateThresholdRaster(te::rst::Raster* raster, int band, double value,
  std::string type, std::map<std::string, std::string> rinfo)
{
  std::unique_ptr<te::rst::Raster> rasterOut;

  //create raster out
  std::vector<te::rst::BandProperty*> bandsProperties;
  te::rst::BandProperty* bandProp = new te::rst::BandProperty(0, te::dt::UCHAR_TYPE);
  bandProp->m_nblocksx = raster->getBand(band)->getProperty()->m_nblocksx;
  bandProp->m_nblocksy = raster->getBand(band)->getProperty()->m_nblocksy;
  bandProp->m_blkh = raster->getBand(band)->getProperty()->m_blkh;
  bandProp->m_blkw = raster->getBand(band)->getProperty()->m_blkw;
  bandsProperties.push_back(bandProp);

  te::rst::Grid* grid = new te::rst::Grid(*(raster->getGrid()));

  te::rst::Raster* rOut = te::rst::RasterFactory::make(type, grid, bandsProperties, rinfo);

  rasterOut.reset(rOut);

  //fill threshold raster
  for (unsigned int i = 0; i < raster->getNumberOfRows(); ++i)
  {
    for (unsigned int j = 0; j < raster->getNumberOfColumns(); ++j)
    {
      double curValue;

      raster->getValue(j, i, curValue);

      if (curValue <= value)
      {
        rasterOut->setValue(j, i, 255.);
      }
      else
      {
        rasterOut->setValue(j, i, 0.);
      }
    }
  }

  return rasterOut;
}

void geopx::tools::ExportRaster(te::rst::Raster* rasterIn, std::string fileName)
{
  assert(rasterIn);

  te::rst::CreateCopy(*rasterIn, fileName);
}

std::vector<te::gm::Geometry*> geopx::tools::Raster2Vector(te::rst::Raster* raster, int band)
{
  assert(raster);

  //create vectors
  std::vector<te::gm::Geometry*> geomVec;

  raster->vectorize(geomVec, band, 0);

  return geomVec;
}

void geopx::tools::ExtractCentroids(std::vector<te::gm::Geometry*>& geomVec, std::vector<geopx::tools::CentroidInfo*>& centroids, int parcelId)
{
  for(std::size_t t = 0; t < geomVec.size(); ++t)
  {
    te::gm::Geometry* geom = geomVec[t];

    te::gm::Point* point = 0;

    double area = 0.;

    if(geom->getGeomTypeId() == te::gm::MultiPolygonType)
    {
      te::gm::MultiPolygon* mp = dynamic_cast<te::gm::MultiPolygon*>(geom);

      te::gm::Polygon* p = dynamic_cast<te::gm::Polygon*>(mp->getGeometryN(0));

      point = p->getCentroid();

      area = p->getArea();
    }
    else if(geom->getGeomTypeId() == te::gm::PolygonType)
    {
      te::gm::Polygon* p = dynamic_cast<te::gm::Polygon*>(geom);

      point = p->getCentroid();

      area = p->getArea();
    }

    if (point)
    {
      geopx::tools::CentroidInfo* ci = new geopx::tools::CentroidInfo();

      ci->m_point = point;
      ci->m_area = area;
      ci->m_parentId = parcelId;
      ci->type = geopx::tools::FOREST_UNKNOWN;

      centroids.push_back(ci);
    }
  }
}

void geopx::tools::AssociateObjects(te::map::AbstractLayer* layer, std::vector<geopx::tools::CentroidInfo*>& points, int srid)
{
  std::unique_ptr<te::da::DataSet> dataSet = layer->getData();
  std::unique_ptr<te::da::DataSetType> dataSetType = layer->getSchema();

  std::size_t gpos = te::da::GetFirstPropertyPos(dataSet.get(), te::dt::GEOMETRY_TYPE);
  te::gm::GeometryProperty* geomProp = te::da::GetFirstGeomProperty(dataSetType.get());

  bool remap = false;

  if (layer->getSRID() != srid)
    remap = true;

  te::da::PrimaryKey* pk = dataSetType->getPrimaryKey();
  std::string name = pk->getProperties()[0]->getName();

  std::size_t size = dataSet->size();

  dataSet->moveBeforeFirst();

  te::common::TaskProgress task("Associating Centroids");
  task.setTotalSteps(size);

  //get geometries
  while (dataSet->moveNext())
  {
    if (!task.isActive())
    {
      break;
    }

    std::unique_ptr<te::gm::Geometry> g(dataSet->getGeometry(gpos));

    if (!g->isValid())
    {
      continue;
    }

    g->setSRID(layer->getSRID());

    if (remap)
      g->transform(srid);

    int id = dataSet->getInt32(name);

    for (std::size_t t = 0; t < points.size(); ++t)
    {
      if (g->contains(points[t]->m_point))
      {
        points[t]->m_parentId = id;
      }
    }

    task.pulse();
  }

  return;
}

void geopx::tools::ExportVector(std::vector<geopx::tools::CentroidInfo*>& ciVec, std::string dataSetName, std::string dsType, const te::core::URI& connInfo, int srid)
{
  assert(!ciVec.empty());

  //create dataset type
  std::unique_ptr<te::da::DataSetType> dataSetType(new te::da::DataSetType(dataSetName));

  //create id property
  te::dt::SimpleProperty* idProperty = new te::dt::SimpleProperty("id", te::dt::INT32_TYPE);
  dataSetType->add(idProperty);

  //create origin id property
  te::dt::SimpleProperty* originIdProperty = new te::dt::SimpleProperty("originId", te::dt::INT32_TYPE);
  dataSetType->add(originIdProperty);

  //create area property
  te::dt::SimpleProperty* areaProperty = new te::dt::SimpleProperty("area", te::dt::DOUBLE_TYPE);
  dataSetType->add(areaProperty);

  //create forest type
  te::dt::StringProperty* typeProperty = new te::dt::StringProperty("type");
  dataSetType->add(typeProperty);

  //create geometry property
  te::gm::GeometryProperty* geomProperty = new te::gm::GeometryProperty("geom", srid, te::gm::PointType);
  dataSetType->add(geomProperty);

  //create primary key
  std::string pkName = "pk_id";
  pkName += "_" + dataSetName;
  te::da::PrimaryKey* pk = new te::da::PrimaryKey(pkName, dataSetType.get());
  pk->add(idProperty);

  //create data set
  std::unique_ptr<te::mem::DataSet> dataSetMem(new te::mem::DataSet(dataSetType.get()));

  te::common::TaskProgress task("Exporting Centroids");
  task.setTotalSteps(ciVec.size());

  for (std::size_t t = 0; t < ciVec.size(); ++t)
  {
    if (!task.isActive())
    {
      break;
    }

    if (ciVec[t]->m_parentId == -1)
      continue;

    //create dataset item
    te::mem::DataSetItem* item = new te::mem::DataSetItem(dataSetMem.get());

    //set id
    item->setInt32("id", (int)t);

    //set origin id
    item->setInt32("originId", ciVec[t]->m_parentId);

    //set area
    item->setDouble("area", ciVec[t]->m_area);

    //forest type
    if (ciVec[t]->type == geopx::tools::FOREST_UNKNOWN)
    {
      item->setString("type", "UNKNOWN");
    }

    //set geometry
    te::gm::Point* pClone = new te::gm::Point(*ciVec[t]->m_point);

    item->setGeometry("geom", pClone);

    dataSetMem->add(item);

    task.pulse();
  }

  dataSetMem->moveBeforeFirst();

  //save data set
  std::unique_ptr<te::da::DataSource> dataSource = te::da::DataSourceFactory::make(dsType, connInfo);
  dataSource->open();

  std::map<std::string, std::string> options;
  dataSource->createDataSet(dataSetType.get(), options);
  dataSource->add(dataSetName, dataSetMem.get(), options);
}

void geopx::tools::ExportPolyVector(std::vector<te::gm::Geometry*>& geomVec, std::string dataSetName, std::string dsType, const te::core::URI& connInfo, int srid)
{
  assert(!geomVec.empty());

  //create dataset type
  std::unique_ptr<te::da::DataSetType> dataSetType(new te::da::DataSetType(dataSetName));

  //create id property
  te::dt::SimpleProperty* idProperty = new te::dt::SimpleProperty("id", te::dt::INT32_TYPE);
  dataSetType->add(idProperty);

  //create geometry property
  te::gm::GeometryProperty* geomProperty = new te::gm::GeometryProperty("geom", srid, te::gm::PolygonType);
  dataSetType->add(geomProperty);

  //create primary key
  std::string pkName = "pk_id";
  pkName += "_" + dataSetName;
  te::da::PrimaryKey* pk = new te::da::PrimaryKey(pkName, dataSetType.get());
  pk->add(idProperty);

  //create data set
  std::unique_ptr<te::mem::DataSet> dataSetMem(new te::mem::DataSet(dataSetType.get()));

  te::common::TaskProgress task("Exporting Polygons");
  task.setTotalSteps(geomVec.size());

  for (std::size_t t = 0; t < geomVec.size(); ++t)
  {
    if (!task.isActive())
    {
      break;
    }

    //create dataset item
    te::mem::DataSetItem* item = new te::mem::DataSetItem(dataSetMem.get());

    //set id
    item->setInt32("id", (int)t);

    //set geometry
    te::gm::Polygon* pClone = new te::gm::Polygon(*dynamic_cast<te::gm::Polygon*>(geomVec[t]));

    item->setGeometry("geom", pClone);

    dataSetMem->add(item);

    task.pulse();
  }

  dataSetMem->moveBeforeFirst();

  //save data set
  std::unique_ptr<te::da::DataSource> dataSource = te::da::DataSourceFactory::make(dsType, connInfo);
  dataSource->open();

  std::map<std::string, std::string> options;
  dataSource->createDataSet(dataSetType.get(), options);
  dataSource->add(dataSetName, dataSetMem.get(), options);
}

void geopx::tools::ClearData(te::map::AbstractLayerPtr layer)
{
  // create output dataset
  std::string dataSetName = "testFix";
  std::string repository = "d:/testFix.shp";
  std::string dsType = "OGR";
  std::map<std::string, std::string> connInfo;
  connInfo["URI"] = repository;

  boost::uuids::basic_random_generator<boost::mt19937> gen;
  boost::uuids::uuid u = gen();
  std::string id_ds = boost::uuids::to_string(u);

  te::da::DataSourceInfoPtr dsInfoPtr(new te::da::DataSourceInfo);
  //dsInfoPtr->setConnInfo(connInfo);
  //dsInfoPtr->setTitle(repository);
  //dsInfoPtr->setAccessDriver("OGR");
  //dsInfoPtr->setType("OGR");
  //dsInfoPtr->setDescription(repository);
  //dsInfoPtr->setId(id_ds);

  te::da::DataSourceInfoManager::getInstance().add(dsInfoPtr);

  te::da::DataSourcePtr outputDataSource = te::da::DataSourceManager::getInstance().get(id_ds, "OGR", dsInfoPtr->getConnInfo());

  //create rtree
  std::unique_ptr<const te::map::LayerSchema> schema(layer->getSchema());
  std::unique_ptr<te::da::DataSet> ds(layer->getData());

  te::gm::GeometryProperty* gmProp = te::da::GetFirstGeomProperty(schema.get());

  int geomIdx = te::da::GetPropertyPos(schema.get(), gmProp->getName());

  te::da::PrimaryKey* pk = schema->getPrimaryKey();

  int idIdx = te::da::GetPropertyPos(schema.get(), pk->getProperties()[0]->getName());

  ds->moveBeforeFirst();

  te::sam::rtree::Index<int> rtree;

  struct CentroidInfo
  {
    te::gm::Point* point;
    int parentId;
    double area;
    std::string type;
  };

  std::map<int, CentroidInfo> centroidInfoMap;

  while (ds->moveNext())
  {
    std::string strId = ds->getAsString(idIdx);

    int id = atoi(strId.c_str());

    te::gm::Geometry* g = ds->getGeometry(geomIdx).release();
    const te::gm::Envelope* box = g->getMBR();

    rtree.insert(*box, id);

    te::gm::Point* point = 0;

    if (g->getGeomTypeId() == te::gm::MultiPointType)
    {
      te::gm::MultiPoint* mPoint = dynamic_cast<te::gm::MultiPoint*>(g);
      point = dynamic_cast<te::gm::Point*>(mPoint->getGeometryN(0));
    }
    else if (g->getGeomTypeId() == te::gm::PointType)
    {
      point = dynamic_cast<te::gm::Point*>(g);
    }

    //save centroid info
    CentroidInfo ci;
    ci.parentId = ds->getInt32(2);
    ci.area = ds->getDouble(3);
    ci.type = ds->getString(4);
    ci.point = point;

    centroidInfoMap.insert(std::map<int, CentroidInfo>::value_type(id, ci));
  }

  //get all elements and check into rtree
  std::set<int> rightValues;

  ds->moveBeforeFirst();

  while (ds->moveNext())
  {
    std::string strId = ds->getAsString(idIdx);

    int id = atoi(strId.c_str());

    std::unique_ptr<te::gm::Geometry> g = ds->getGeometry(geomIdx);
    const te::gm::Envelope* box = g->getMBR();

    std::vector<int> resultsTree;

    rtree.search(*box, resultsTree);

    int rightValue = -1;

    for (std::size_t t = 0; t < resultsTree.size(); ++t)
    {
      if (resultsTree[t] > rightValue)
      {
        rightValue = resultsTree[t];
      }
    }

    if (rightValue != -1)
    {
      rightValues.insert(rightValue);
    }
  }

  //create dataset type
  std::unique_ptr<te::da::DataSetType> dataSetType(new te::da::DataSetType(dataSetName));

  te::dt::SimpleProperty* idProperty = new te::dt::SimpleProperty("id", te::dt::INT32_TYPE);
  dataSetType->add(idProperty);

  te::dt::SimpleProperty* originIdProperty = new te::dt::SimpleProperty("originId", te::dt::INT32_TYPE);
  dataSetType->add(originIdProperty);

  te::dt::SimpleProperty* areaProperty = new te::dt::SimpleProperty("area", te::dt::DOUBLE_TYPE);
  dataSetType->add(areaProperty);

  te::dt::StringProperty* typeProperty = new te::dt::StringProperty("type");
  dataSetType->add(typeProperty);

  te::gm::GeometryProperty* geomProperty = new te::gm::GeometryProperty("geom", layer->getSRID(), te::gm::PointType);
  dataSetType->add(geomProperty);

  std::string pkName = "pk_id";
  pkName += "_" + dataSetName;
  te::da::PrimaryKey* pkNew = new te::da::PrimaryKey(pkName, dataSetType.get());
  pkNew->add(idProperty);

  //create data set
  std::unique_ptr<te::mem::DataSet> dataSetMem(new te::mem::DataSet(dataSetType.get()));

  int count = 0;

  for (std::set<int>::iterator it = rightValues.begin(); it != rightValues.end(); ++it)
  {
    CentroidInfo ci = centroidInfoMap[*it];

    //create dataset item
    te::mem::DataSetItem* item = new te::mem::DataSetItem(dataSetMem.get());

    //set id
    item->setInt32("id", count);

    //set origin id
    item->setInt32("originId", ci.parentId);

    //set area
    item->setDouble("area", ci.area);

    //forest type
    item->setString("type", ci.type);

    //set geometry
    item->setGeometry("geom", ci.point);

    dataSetMem->add(item);

    ++count;
  }

  dataSetMem->moveBeforeFirst();

  //save dataset
  std::map<std::string, std::string> options;
  try
  {
    outputDataSource->createDataSet(dataSetType.get(), options);
    outputDataSource->add(dataSetName, dataSetMem.get(), options);
  }
  catch (const std::exception& e)
  {
    return;
  }
  catch (...)
  {
    return;
  }

  centroidInfoMap.clear();
}



