/*!
\file geopx-desktop/src/geopixeltools/tileGenerator/core/TileGeneratorService.cpp

\brief This file implements the service to create tiles over a set of layers.
*/

#define GOOGLE_SRID 3857

#include "TileGeneratorService.h"
#include "Tile.h"

//TerraLib Includes
#include <terralib/common/progress/TaskProgress.h>
#include <terralib/common/StringUtils.h>
#include <terralib/core/Exception.h>
#include <terralib/raster/Utils.h>
#include <terralib/qt/widgets/canvas/MapDisplay.h>

//STL Includes
#include <cassert>
#include <exception>
#include <iosfwd>
#include <stdio.h>

//Qt Includes
#include <QDir>
#include <QPainter>

geopx::tools::TileGeneratorService::TileGeneratorService():
  m_display(0),
  m_zoomLevelMin(-1),
  m_zoomLevelMax(-1),
  m_tileSize(0),
  m_path(""),
  m_format(""),
  m_logFile("")
{
}

geopx::tools::TileGeneratorService::~TileGeneratorService()
{
  delete m_display;
}

void geopx::tools::TileGeneratorService::setInputParameters(std::list<te::map::AbstractLayerPtr> layers, te::gm::Envelope env, int srid, int zoomLevelMin, int zoomLevelMax, int tileSize, std::string path, std::string format)
{
  m_layers = layers;

  m_env = env;
  m_srid = srid;
  m_zoomLevelMin = zoomLevelMin;
  m_zoomLevelMax = zoomLevelMax;
  m_tileSize = tileSize;
  m_path = path;
  m_format = format;

  m_env.transform(m_srid, GOOGLE_SRID);

  buildDisplay();
}

void geopx::tools::TileGeneratorService::runValidation(bool createMissingTiles)
{
  //check input parameters
  checkParameters();

  m_logFile = m_path + "/ValidationLog.txt";

  //create file
  FILE* fp = fopen(m_logFile.c_str(), "w");

  if(!fp)
    throw te::core::Exception() << te::ErrorDescription("Error creating log file.");

  fprintf(fp, "\n\n\t\tTiles Validation LOG\n\n");

  fprintf(fp, "Missing Tiles:\n");

  //generate tiles
  for(int i = m_zoomLevelMax; i >= m_zoomLevelMin; --i)
  {
    Tile* tile = new Tile(i, m_tileSize);

    long tIdxX1, tIdxY1, tIdxX2, tIdxY2;

    tile->tileMatrix(m_env, tIdxX1, tIdxY1, tIdxX2, tIdxY2);
    
    for (int k = tIdxY2; k <= tIdxY1; ++k)
    {
      for (int j = tIdxX1; j <= tIdxX2; ++j)
      {
        te::gm::Envelope env = tile->tileBox(j, k);

        if(env.isValid())
        {
          try
          {
            bool isValid;

            validateTile(fp, tile, i, j, k, isValid);

            if(!isValid && createMissingTiles)
            {
              QPixmap* pix = drawTile(env);

              if(pix)
                savePixmap(tile, pix, i, j, k);
            }
          }
          catch(...)
          {
            fprintf(fp, "\n\t Unexpected error. Level: %d Y: %d X: %d \n", i, k, j);
            continue;
          }
        }
        else
        {
          fprintf(fp, "\n\t Invalid Box. Level: %d Y: %d X: %d \n", i, k, j);
        }
      }
    }

    delete tile;
  }
}

void geopx::tools::TileGeneratorService::runService(bool isRaster)
{
  //check input parameters
  checkParameters();

  //progress
  int totalSteps = 0;

  for (int i = m_zoomLevelMax; i >= m_zoomLevelMin; --i)
  {
    std::unique_ptr<Tile> tile(new Tile(i, m_tileSize));

    long tIdxX1, tIdxY1, tIdxX2, tIdxY2;

    tile->tileMatrix(m_env, tIdxX1, tIdxY1, tIdxX2, tIdxY2);

    for (int k = tIdxY2; k <= tIdxY1; ++k)
    {
      for (int j = tIdxX1; j <= tIdxX2; ++j)
      {
        ++totalSteps;
      }
    }
  }

  te::common::TaskProgress progress("Tile Generator");
  progress.setTotalSteps(totalSteps);

  //generate tiles
  for(int i = m_zoomLevelMax; i >= m_zoomLevelMin; --i)
  {
    std::unique_ptr<Tile> tile(new Tile(i, m_tileSize));

    long tIdxX1, tIdxY1, tIdxX2, tIdxY2;

    tile->tileMatrix(m_env, tIdxX1, tIdxY1, tIdxX2, tIdxY2);
    
    for (int k = tIdxY2; k <= tIdxY1; ++k)
    {
      for (int j = tIdxX1; j <= tIdxX2; ++j)
      {
        te::gm::Envelope env = tile->tileBox(j, k);

        if(env.isValid())
        {
          try
          {
            if(isRaster)
            {
              if(i == m_zoomLevelMax)// use original images
              {
                QPixmap* pix = drawTile(env);

                if(pix)
                  savePixmap(tile.get(), pix, i, j, k);
              }
              else //use generated tiles
              {
                QPixmap pix = drawTile(env, i + 1);

                if(!pix.isNull())
                  savePixmap(tile.get(), &pix, i, j, k);
              }
            }
            else
            {
              QPixmap* pix = drawTile(env);

                if(pix)
                  savePixmap(tile.get(), pix, i, j, k);
            }
          }
          catch(...)
          {
            continue;
          }
        }

        progress.pulse();

        if (!progress.isActive())
          return;

      }
    }
  }
}

void geopx::tools::TileGeneratorService::checkParameters()
{
  if(m_layers.empty())
    throw te::core::Exception() << te::ErrorDescription("Select a list of layers firts.");

  if(!m_env.isValid())
    throw te::core::Exception() << te::ErrorDescription("Box defined is not valid.");

  if(m_zoomLevelMin < 0 || m_zoomLevelMax < 0 || m_zoomLevelMax < m_zoomLevelMin)
    throw te::core::Exception() << te::ErrorDescription("Invalid range for zoom levels.");

  if(m_tileSize <= 0)
    throw te::core::Exception() << te::ErrorDescription("Invalid value for tile size.");

  if(m_path.empty())
    throw te::core::Exception() << te::ErrorDescription("Invalid path value.");

  QDir dir(m_path.c_str());
  if(!dir.exists())
    throw te::core::Exception() << te::ErrorDescription("Invalid path value.");

  if(m_format.empty())
    throw te::core::Exception() << te::ErrorDescription("Invalid format value.");
}

void geopx::tools::TileGeneratorService::buildDisplay()
{
  m_display = new te::qt::widgets::MapDisplay(QSize(m_tileSize, m_tileSize));

  m_display->setLayerList(m_layers);

  m_display->setSRID(GOOGLE_SRID, false);
}

QPixmap* geopx::tools::TileGeneratorService::drawTile(te::gm::Envelope env)
{
  m_display->setExtent(env, true);

  return m_display->getDisplayPixmap();
}

QPixmap geopx::tools::TileGeneratorService::drawTile(te::gm::Envelope env, int level)
{
  Tile* tileGroup = new Tile(level, m_tileSize);

  long tIdxX1, tIdxY1, tIdxX2, tIdxY2;

  tileGroup->tileMatrix(env, tIdxX1, tIdxY1, tIdxX2, tIdxY2);

  int width = (tIdxX2 - tIdxX1 + 1) * m_tileSize;
  int height = (tIdxY1 - tIdxY2 + 1) * m_tileSize;

  QPixmap pixGroup(width, height);
  pixGroup.fill(Qt::transparent);

  QPainter painter(&pixGroup);

  int countY = 0;
  for (int k = tIdxY2; k <= tIdxY1; ++k)
  {
    int countX = 0;
    for (int j = tIdxX1; j <= tIdxX2; ++j)
    {
      std::string tileFileName = getOSMTileFilePath(tileGroup, level, j, k, false);

      QPixmap pixGroupItem(tileFileName.c_str());

      if(!pixGroupItem.isNull())
      {
        painter.drawPixmap(countX * m_tileSize, countY * m_tileSize, m_tileSize, m_tileSize, pixGroupItem);
      }

      countX++;
    }

    countY++;
  }

  painter.end();

  delete tileGroup;

  //smoth scale
  return pixGroup.scaled(m_tileSize, m_tileSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

void geopx::tools::TileGeneratorService::validateTile(FILE* file, Tile* tile, int level, int tileIdxX, int tileIdxY, bool& isValid)
{
////change tile index to google system
//  long tileIdxXOSM;
//  long tileIdxYOSM;
//
//  tile->OSMTile(tileIdxX, tileIdxY, tileIdxXOSM, tileIdxYOSM);

  isValid = false;

  //file info
  //std::string pathOSM   = m_path + "/MapquestOSM";
  std::string pathLevel   = m_path + "/" + te::common::Convert2String(level);
  std::string pathColumn  = pathLevel + "/" + te::common::Convert2String(tileIdxX);
  std::string fileName    = pathColumn + "/" + te::common::Convert2String(tileIdxY)+ "." + te::common::Convert2LCase(m_format);

  //check MapquestOSM directory
  //QDir dirOSM(pathOSM.c_str());
  //if(dirOSM.exists())
  //{
    //check dir level
    QDir dirLevel(pathLevel.c_str());
    if(dirLevel.exists())
    {
      //check dir column
      QDir dirColumn(pathColumn.c_str());
      if(dirColumn.exists())
      {
        //check file
        QFile file(fileName.c_str());

        isValid = file.exists();
      }
    }
  //}

  //check validation
  if(!isValid)
  {
    fprintf(file, "\n\t Tile File: %s \n", fileName.c_str());
  }
}

void geopx::tools::TileGeneratorService::savePixmap(Tile* tile, QPixmap* pix, int level, int tileIdxX, int tileIdxY)
{
  //create file
  std::string fileName = getOSMTileFilePath(tile, level, tileIdxX, tileIdxY, true);

  pix->save(fileName.c_str(), m_format.c_str());
}

std::string geopx::tools::TileGeneratorService::getOSMTileFilePath(Tile* tile, int level, int tileIdxX, int tileIdxY, bool save)
{
////change tile index to google system
//  long tileIdxXOSM;
//  long tileIdxYOSM;
//
//  tile->OSMTile(tileIdxX, tileIdxY, tileIdxXOSM, tileIdxYOSM);

  // MapquestOSM directory
  //std::string pathOSM = m_path + "/MapquestOSM";
  //QDir dirOSM(pathOSM.c_str());
  //if(!dirOSM.exists() && save)
  //  dirOSM.mkdir(pathOSM.c_str());

  //create dir level
  std::string pathLevel = m_path + "/" + te::common::Convert2String(level);
  QDir dirLevel(pathLevel.c_str());
  if(!dirLevel.exists() && save)
    dirLevel.mkdir(pathLevel.c_str());

  //create dir column
  std::string pathColumn = pathLevel + "/" + te::common::Convert2String(tileIdxX);
  QDir dirColumn(pathColumn.c_str());
  if(!dirColumn.exists() && save)
    dirColumn.mkdir(pathColumn.c_str());

  //create file
  std::string fileName = pathColumn + "/" + te::common::Convert2String(tileIdxY)+ "." + te::common::Convert2LCase(m_format);

  return fileName;
}

/*
 Tile* tileGroup = new Tile(level, m_tileSize);

  te::gm::Coord2D ll = env.getLowerLeft();

  double stepW = env.getWidth() / 4.;
  double stepH = env.getHeight() / 4.;

  // -------------
  // |  1  |  2  |
  // -------------
  // |  3  |  4  |
  // -------------

  te::gm::Coord2D tile1(ll.getX() + stepW, ll.getY() + (stepH * 2.5));
  te::gm::Coord2D tile2(ll.getX() + stepW + stepW + stepW, ll.getY() + (stepH * 2.5));
  te::gm::Coord2D tile3(ll.getX() + stepW, ll.getY() + stepH);
  te::gm::Coord2D tile4(ll.getX() + stepW + stepW + stepW, ll.getY() + stepH);

  long tIdxX, tIdxY;

  int width = m_tileSize * 2;
  int height = m_tileSize * 2;

  QPixmap pixGroup(width, height);
  pixGroup.fill(Qt::white);

  QPainter painter(&pixGroup);

  //tile 3
  tileGroup->tileIndex(tile3.getX(), tile3.getY(), tIdxX, tIdxY);

  std::string tileFileName = getOSMTileFilePath(tileGroup, level, tIdxX, tIdxY);

  QPixmap pixItem3(tileFileName.c_str());

  if(!pixItem3.isNull())
  {
    painter.drawPixmap(0 * m_tileSize, 1 * m_tileSize, m_tileSize, m_tileSize, pixItem3);
  }

  //tile 1
  tileFileName = getOSMTileFilePath(tileGroup, level, tIdxX, tIdxY + 1);

  QPixmap pixItem1(tileFileName.c_str());

  if(!pixItem1.isNull())
  {
    painter.drawPixmap(0 * m_tileSize, 0 * m_tileSize, m_tileSize, m_tileSize, pixItem1);
  }

  //tile 2
  tileFileName = getOSMTileFilePath(tileGroup, level, tIdxX + 1, tIdxY + 1);

  QPixmap pixItem2(tileFileName.c_str());

  if(!pixItem2.isNull())
  {
    painter.drawPixmap(1 * m_tileSize, 0 * m_tileSize, m_tileSize, m_tileSize, pixItem2);
  }

  //tile 4
  tileFileName = getOSMTileFilePath(tileGroup, level, tIdxX + 1, tIdxY);

  QPixmap pixItem4(tileFileName.c_str());

  if(!pixItem4.isNull())
  {
    painter.drawPixmap(1 * m_tileSize, 1 * m_tileSize, m_tileSize, m_tileSize, pixItem4);
  }

  painter.end();

  delete tileGroup;

  //smoth scale
  return pixGroup.scaled(m_tileSize, m_tileSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
  */
