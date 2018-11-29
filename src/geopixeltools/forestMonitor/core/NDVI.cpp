/*!
  \file geopx-desktop/src/geopixeltools/core/NDVI.cpp

  \brief This file contains structures and definitions NDVI operation.
*/

#include "NDVI.h"

//TerraLib Includes
#include <terralib/common/progress/TaskProgress.h>
#include <terralib/common/Exception.h>
#include <terralib/common/STLUtils.h>
#include <terralib/memory/ExpansibleRaster.h>
#include <terralib/raster/BandProperty.h>
#include <terralib/raster/Grid.h>
#include <terralib/raster/Raster.h>
#include <terralib/raster/RasterFactory.h>
#include <terralib/raster/RasterIterator.h>


//STL Includes
#include <cassert>
#include <numeric>

std::unique_ptr<te::rst::Raster> geopx::tools::GenerateNDVIRaster(te::rst::Raster* rasterNIR, int bandNIR,
                                                                               te::rst::Raster* rasterVIS, int bandVIS, 
                                                                               double gain, double offset, bool normalize, 
                                                                               std::map<std::string, std::string> rInfo,
                                                                               std::string type, int srid,
                                                                               bool invert, bool rgbVIS)
{
  //check input parameters
  if(!rasterNIR || ! rasterVIS)
  {
    throw te::common::Exception("Invalid input rasters.");
  }

  if(rasterNIR->getNumberOfColumns() != rasterVIS->getNumberOfColumns() ||
     rasterNIR->getNumberOfRows() != rasterVIS->getNumberOfRows())
  {
    throw te::common::Exception("Incompatible rasters.");
  }

  std::string typeNDVI = type;

  if(normalize)
    typeNDVI = "MEM";

  //create raster out
  std::vector<te::rst::BandProperty*> bandsProperties;
  te::rst::BandProperty* bandProp = new te::rst::BandProperty(0, te::dt::DOUBLE_TYPE);
  bandProp->m_nblocksx = rasterNIR->getBand(bandNIR)->getProperty()->m_nblocksx;
  bandProp->m_nblocksy = rasterNIR->getBand(bandNIR)->getProperty()->m_nblocksy;
  bandProp->m_blkh = rasterNIR->getBand(bandNIR)->getProperty()->m_blkh;
  bandProp->m_blkw = rasterNIR->getBand(bandNIR)->getProperty()->m_blkw;
  bandsProperties.push_back(bandProp);

  te::rst::Grid* grid = new te::rst::Grid(*(rasterNIR->getGrid()));
  grid->setSRID(srid);

  te::rst::Raster* rasterNDVI = 0;

  if(normalize)
  {
    rasterNDVI = new te::mem::ExpansibleRaster(10, grid, bandsProperties);
  }
  else
  {
    rasterNDVI = te::rst::RasterFactory::make(typeNDVI, grid, bandsProperties, rInfo);
  }

  /*if(invert)
  {
    rasterNIR = InvertRaster(rasterNIR, bandNIR);
    bandNIR = 0;
  }*/

  //start NDVI operation
  std::size_t nRows = rasterNDVI->getNumberOfRows();
  std::size_t nCols = rasterNDVI->getNumberOfColumns();

  double nirValue = 0.;
  double visValue = 0.;

  double minValue = std::numeric_limits<double>::max();
  double maxValue = -std::numeric_limits<double>::max();

  {
    te::common::TaskProgress task("Calculating NDVI.");
    task.setTotalSteps(nRows);

    for(std::size_t t = 0; t < nRows; ++t)
    {
      if(task.isActive() == false)
        throw te::common::Exception("Operation Canceled.");

      for(std::size_t q = 0; q < nCols; ++q)
      {
        try
        {
          rasterNIR->getValue(q, t, nirValue, bandNIR);

          if (invert)
            nirValue = (nirValue * (-1.)) + 255.;

          if (rgbVIS)
          {
            std::vector<double> visValueVec;

            rasterVIS->getValues(q, t, visValueVec);

            for (std::size_t a = 0; a < visValueVec.size(); ++a)
            {
              visValue += visValueVec[a];
            }

            if (visValue != 0.)
              visValue = visValue / (double)rasterVIS->getNumberOfBands();
          }
          else
          {
            rasterVIS->getValue(q, t, visValue, bandVIS);
          }
          
          double value = 0.;

          if(nirValue + visValue != 0.)
          {
            value = (gain *((nirValue - visValue) / (nirValue + visValue))) + offset;
          }

          rasterNDVI->setValue(q, t, value, 0);

          if(value > maxValue)
          {
            maxValue = value;
          }

          if(value < minValue)
          {
            minValue = value;
          }
        }
        catch (...)
        {
          continue;
        }
      }

      task.pulse();
    }
  }

  //if (invert)
  //{
  //  delete rasterNIR;
  //}

  std::unique_ptr<te::rst::Raster> rasterOut;

  if(normalize)
  {
    rasterOut = NormalizeRaster(rasterNDVI, minValue, maxValue, 0., 255., rInfo, type);

    delete rasterNDVI;
  }
  else
  {
    rasterOut.reset(rasterNDVI);
  }

  rasterOut->getGrid()->setSRID(srid);

  return rasterOut;
}

te::rst::Raster* geopx::tools::InvertRaster(te::rst::Raster* rasterNIR, int bandNIR)
{
  //create raster out
  std::vector<te::rst::BandProperty*> bandsProperties;
  te::rst::BandProperty* bandProp = new te::rst::BandProperty(0, te::dt::UCHAR_TYPE);
  bandProp->m_nblocksx = rasterNIR->getBand(bandNIR)->getProperty()->m_nblocksx;
  bandProp->m_nblocksy = rasterNIR->getBand(bandNIR)->getProperty()->m_nblocksy;
  bandProp->m_blkh = rasterNIR->getBand(bandNIR)->getProperty()->m_blkh;
  bandProp->m_blkw = rasterNIR->getBand(bandNIR)->getProperty()->m_blkw;
  bandsProperties.push_back(bandProp);

  te::rst::Grid* grid = new te::rst::Grid(*(rasterNIR->getGrid()));

  te::rst::Raster* rasterInverted = new te::mem::ExpansibleRaster(10, grid, bandsProperties);

  te::common::TaskProgress task("Invert Raster NIR.");
  task.setTotalSteps(rasterNIR->getNumberOfRows());

  double value;

  for (std::size_t t = 0; t < rasterNIR->getNumberOfRows(); ++t)
  {
    for (std::size_t q = 0; q < rasterNIR->getNumberOfColumns(); ++q)
    {
      try
      {
        double value;

        rasterNIR->getValue(q, t, value, bandNIR);

        double invertedValue = (value * -1) + 255.;

        rasterInverted->setValue(q, t, invertedValue, 0);
      }
      catch (...)
      {
        continue;
      }
    }

    task.pulse();
  }

  return rasterInverted;
}

std::unique_ptr<te::rst::Raster> geopx::tools::NormalizeRaster(te::rst::Raster* inraster, double min, double max, double nmin, double nmax,
                                                                            std::map<std::string, std::string> rInfo, std::string type)
{
//create raster out
  std::vector<te::rst::BandProperty*> bandsProperties;
  te::rst::BandProperty* bandProp = new te::rst::BandProperty(0, te::dt::UCHAR_TYPE);
  bandsProperties.push_back(bandProp);

  te::rst::Grid* grid = new te::rst::Grid(*(inraster->getGrid()));

  te::rst::Raster* rasterNormalized = te::rst::RasterFactory::make(type, grid, bandsProperties, rInfo);

  //start Normalize operation
  std::size_t nRows = inraster->getNumberOfRows();
  std::size_t nCols = inraster->getNumberOfColumns();

  te::common::TaskProgress task("Normalize NDVI.");
  task.setTotalSteps(nRows);

  double gain = (double)(nmax-nmin)/(max-min);
  double offset = -1*gain*min+nmin;

  double value;

  for(std::size_t t = 0; t < nRows; ++t)
  {
    if(task.isActive() == false)
      throw te::common::Exception("Operation Canceled.");

    for(std::size_t q = 0; q < nCols; ++q)
    {
      try
      {
        inraster->getValue(q, t, value, 0);

        double normalizeValue = (value * gain + offset);

        rasterNormalized->setValue(q, t, normalizeValue, 0);
      }
      catch (...)
      {
        continue;
      }
    }

    task.pulse();
  }

  std::unique_ptr<te::rst::Raster> rOut(rasterNormalized);

  return rOut;
}
