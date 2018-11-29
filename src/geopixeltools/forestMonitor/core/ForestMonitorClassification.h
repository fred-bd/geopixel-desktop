/*!
  \file geopx-desktop/src/geopixeltools/core/ForestMonitorClassification.h

  \brief This file contains structures and definitions for forest monitor classification operation.
*/

#ifndef __GEOPXDESKTOP_TOOLS_FORESTMONITOR_FORESTMONITORCLASSIFICATION_H
#define __GEOPXDESKTOP_TOOLS_FORESTMONITOR_FORESTMONITORCLASSIFICATION_H

// TerraLib
#include <terralib/core/uri/URI.h>
#include <terralib/maptools/AbstractLayer.h>
#include <terralib/rp/Filter.h>
#include "../../Config.h"

//STL Includes
#include <map>
#include <memory>

namespace te
{
  namespace rst { class Raster; }
}

namespace geopx
{
  namespace tools
  {

    enum ForetType
    {
      FOREST_UNKNOWN,
      FOREST_LIVE,
      FOREST_DEAD
    };

    struct CentroidInfo
    {
      te::gm::Point* m_point;
      int m_parentId;
      double m_area;
      geopx::tools::ForetType type;
    };



    std::unique_ptr<te::rst::Raster> GenerateFilterRaster(te::rst::Raster* raster, int band, int nIter, te::rp::Filter::InputParameters::FilterType fType,
                                                        std::string type, std::map<std::string, std::string> rinfo);

    std::unique_ptr<te::rst::Raster> GenerateThresholdRaster(te::rst::Raster* raster, int band, double value,
                                                            std::string type, std::map<std::string, std::string> rinfo);


    void ExportRaster(te::rst::Raster* rasterIn, std::string fileName);

    std::vector<te::gm::Geometry*> Raster2Vector(te::rst::Raster* raster, int band);

    void ExtractCentroids(std::vector<te::gm::Geometry*>& geomVec, std::vector<CentroidInfo*>& centroids, int parcelId);

    void AssociateObjects(te::map::AbstractLayer* layer, std::vector<geopx::tools::CentroidInfo*>& points, int srid);

    void ExportVector(std::vector<geopx::tools::CentroidInfo*>& ciVec, std::string dataSetName, std::string dsType, const te::core::URI& connInfo, int srid);

    void ExportPolyVector(std::vector<te::gm::Geometry*>& geomVec, std::string dataSetName, std::string dsType, const te::core::URI& connInfo, int srid);

    void ClearData(te::map::AbstractLayerPtr layer);

  }     // end namespace qt
}       // end namespace te

#endif //__GEOPXDESKTOP_TOOLS_FORESTMONITOR_FORESTMONITORCLASSIFICATION_H
