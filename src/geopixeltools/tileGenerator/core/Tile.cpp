/*!
  \file geopx-desktop/src/geopixeltools/tileGenerator/core/TileGeneratorService.h

  \brief This file contains structures and definitions to build a Tile box information.
*/

#include "Tile.h"

//TerraLib Includes
#include <terralib/raster/Utils.h>

//STL
#include <cmath>

geopx::tools::Tile::Tile(double resolution, int tileSize):
  m_worldExtent(40075016.686),
  m_resolution(resolution),
  m_tileSize(tileSize)
{
  m_zoomLevel = bestZoomLevel(m_resolution);
}

geopx::tools::Tile::Tile(int zoomLevel, int tileSize):
  m_worldExtent(40075016.686),
  m_zoomLevel(zoomLevel),
  m_tileSize(tileSize)
{
  m_resolution = resolution(m_zoomLevel);
}

geopx::tools::Tile::~Tile()
{
}

double geopx::tools::Tile::resolution(int zoomLevel)
{
  return m_worldExtent / (pow(2., zoomLevel) * m_tileSize);
}

int geopx::tools::Tile::bestZoomLevel(double resolution)
{
  double zoom = log(m_worldExtent / m_tileSize / resolution) / log(2);
  return int(zoom);
}

void geopx::tools::Tile::tileIndex(double xPos, double yPos, long& tileIndexX, long& tileIndexY)
{
  tileIndexX = floor((0.5 + xPos / m_worldExtent) * pow(2., m_zoomLevel));
  tileIndexY = floor((0.5 - yPos / m_worldExtent) * pow(2., m_zoomLevel));
}

te::gm::Envelope geopx::tools::Tile::tileBox(long tileIndexX, long tileIndexY)
{
  double x1 = tileIndexX * m_worldExtent / pow(2., m_zoomLevel) - m_worldExtent / 2;
  double y1 = -(tileIndexY + 1) * m_worldExtent / pow(2., m_zoomLevel) + m_worldExtent / 2;
  double x2 = x1 + m_worldExtent / pow(2., m_zoomLevel);
  double y2 = y1 + m_worldExtent / pow(2., m_zoomLevel);

  te::gm::Envelope env(x1, y1, x2, y2);

  return env;
}

long geopx::tools::Tile::tileMatrixDimension(int zoomLevel)
{
  return (long)pow(2., zoomLevel);
}

void geopx::tools::Tile::tileMatrix(te::gm::Envelope env, long& tileIndexX1, long& tileIndexY1, long& tileIndexX2, long& tileIndexY2)
{
  tileIndex(env.getLowerLeftX(), env.getLowerLeftY(), tileIndexX1, tileIndexY1);
  tileIndex(env.getUpperRightX(), env.getUpperRightY(), tileIndexX2, tileIndexY2);
}

//void geopx::tools::Tile::OSMTile(long tileX, long tileY, long& OSMTileX, long& OSMTileY)
//{
//  OSMTileX = (tileMatrixDimension(m_zoomLevel) / 2.) + tileX;
//  OSMTileY = (tileMatrixDimension(m_zoomLevel) / 2.) - (tileY + 1);
//}
//
//void geopx::tools::Tile::TMSTile(long tileX, long tileY, long& TMSTileX, long& TMSTileY)
//{
//  TMSTileX = (tileMatrixDimension(m_zoomLevel) / 2.) + tileX;
//  TMSTileY = (tileMatrixDimension(m_zoomLevel) / 2.) + tileY;
//}
