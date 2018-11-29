/*!
  \file geopx-desktop/src/geopixeltools/core/NDVI.h

  \brief This file contains structures and definitions NDVI operation.
*/

#ifndef __GEOPXDESKTOP_TOOLS_FORESTMONITOR_NDVI_H
#define __GEOPXDESKTOP_TOOLS_FORESTMONITOR_NDVI_H

// TerraLib
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

    std::unique_ptr<te::rst::Raster> GenerateNDVIRaster(te::rst::Raster* rasterNIR, int bandNIR,
                                                      te::rst::Raster* rasterVIS, int bandVIS, 
                                                      double gain, double offset, bool normalize, 
                                                      std::map<std::string, std::string> rInfo,
                                                      std::string type, int srid,
                                                      bool invert, bool rgbVIS);

    te::rst::Raster* InvertRaster(te::rst::Raster* rasterNIR, int bandNIR);

    std::unique_ptr<te::rst::Raster> NormalizeRaster(te::rst::Raster* inraster, double min, double max, double nmin, double nmax,
                                                    std::map<std::string, std::string> rInfo, std::string type);


  } // end namespace tools
} // end namespace geopx

#endif //__GEOPXDESKTOP_TOOLS_FORESTMONITOR_NDVI_H
