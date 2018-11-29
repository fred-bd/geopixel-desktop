/*!
  \file geopx-desktop/src/geopixeltools/photoindex/core/PhotoIndex.cpp

  \brief This file contains structures and definitions to photo index information.
*/

#include "PhotoIndex.h"

//TerraLib Includes
#include <terralib/common/progress/TaskProgress.h>
#include <terralib/common/STLUtils.h>
#include <terralib/dataaccess/utils/Utils.h>
#include <terralib/geometry/Utils.h>
#include <terralib/memory/DataSetItem.h>

//STL Includes
#include <cassert>

geopx::tools::PhotoIndex::PhotoIndex()
{
}

geopx::tools::PhotoIndex::~PhotoIndex()
{
}

void geopx::tools::PhotoIndex::execute(te::da::DataSourcePtr dataSource, te::mem::DataSet* dataSet)
{
  std::vector<std::string> dataSetNames = dataSource->getDataSetNames();

  te::common::TaskProgress task("Creating Photo Index");
  task.setTotalSteps(static_cast<int>(dataSetNames.size()));

  for(std::size_t t = 0; t < dataSetNames.size(); ++t)
  {
    if(task.isActive() == false)
      break;

    std::string name = dataSetNames[t];

    std::unique_ptr<te::da::DataSet> dataSetRaster = dataSource->getDataSet(name);

    std::size_t rpos = te::da::GetFirstPropertyPos(dataSetRaster.get(), te::dt::RASTER_TYPE);

    std::unique_ptr<te::rst::Raster> inputRst = dataSetRaster->getRaster(rpos);

    te::gm::Geometry* geom = te::gm::GetGeomFromEnvelope(inputRst->getExtent(), inputRst->getSRID());

    //create dataset item
    te::mem::DataSetItem* item = new te::mem::DataSetItem(dataSet);

    //set id
    item->setInt32("id", static_cast<int>(t));

    //set parcel id
    item->setString("fileName", name);

    //set geometry
    item->setGeometry("geom", geom);

    dataSet->add(item);

    task.pulse();
  }
}