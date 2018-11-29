/*!
  \file geopx-desktop/src/geopixeltools/tileGenerator/core/TileGeneratorService.h

  \brief This file contains structures and definitions to build a Tile box information.

  \note Classe a ser generalizada no futuro para qualquer proojeção e area de interesse
  O codigo a seguir funciona corretamente para dados em projeção mercator e
  elipsoide WMS esferico (Terralib VirualEarthProjection ou coisa parecida)
  O sistema de numeração de tiles pode variar:  Google:(0,0)=upper-left ou
  TMS:(0,0)=lower-left ou Outros:(0,0)=(w0,n0).
  Os metodos da classe geram o numero do tile sendo (0,0)=(w0,n0).
  Neste caso se for recuperar dados do Google ou TMS deve ajustar as origens
*/

#ifndef __GEOPXDESKTOP_TOOLS_TILEGENERATOR_TILE_H
#define __GEOPXDESKTOP_TOOLS_TILEGENERATOR_TILE_H

#include "../../Config.h"

//TerraLib Includes
#include <terralib/geometry/Envelope.h>


namespace geopx
{
  namespace tools
  {

    class GEOPXTOOLSEXPORT Tile
    {
      public:
    
        /**
          \brief Constructor.
          \param resolution
          \param tileSize
        */
        Tile(double resolution, int tileSize);

        /**
          \brief Constructor.
          \param zoomLevel
          \param tileSize
        */
        Tile(int zoomLevel, int tileSize);

        /**
          \brief Destructor
        */
        ~Tile();

      public:

        /** 
          \brief Calculates tile resolution given the zoom level.
        */
        double resolution(int zoomLevel);

        /** 
          \brief The zoom level which generates the minimum resolution, ajusted to a zoom level,  greater than a given resolution.
        */
        int bestZoomLevel(double resolution);

        /** 
          /brief Calculates the tile index given a planar coordinates of a location.
          */
        void tileIndex(double xPos, double yPos, long& tileIndexX, long& tileIndexY);

        /**
          /brief Calculates the tile box.
        */
        te::gm::Envelope tileBox(long tileIndexX, long tileIndexY);

        /** 
          /brief Calculates the tile matrix dimension for world extent.
        */
        long tileMatrixDimension(int zoomLevel);

        /** 
          /brief Calculates the range of tiles for a given box in planar coordinates.
        */
        void tileMatrix(te::gm::Envelope env, long& tileIndexX1, long& tileIndexY1, long& tileIndexX2, long& tileIndexY2);

        ///** 
        //  /brief Calculates the Google and Open Streetmap tile given normalized tile (0,0=n0,w0).
        //*/
        //void OSMTile(long tileX, long tileY, long& OSMTileX, long& OSMTileY);

        ///** 
        //  /brief Calculates the TMS tile  given normalized tile (0,0=n0,w0).
        //*/
        //void TMSTile(long tileX, long tileY, long& TMSTileX, long& TMSTileY);

      protected:
        double m_worldExtent;
        double m_resolution;
        int m_tileSize;
        int m_zoomLevel;
    };

  } // end namespace tools
}  // end namespace geopx

#endif //__GEOPXDESKTOP_TOOLS_TILEGENERATOR_TILE_H
