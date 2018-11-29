/*!
\file geopx-desktop/src/geopixeltools/tileGenerator/core/TileGeneratorService.h

\brief This file implements the service to create tiles over a set of layers.
*/

#ifndef __GEOPXDESKTOP_TOOLS_TILEGENERATOR_TILEGENERATORSERVICE_H
#define __GEOPXDESKTOP_TOOLS_TILEGENERATOR_TILEGENERATORSERVICE_H

#include "../../Config.h"

//TerraLib Includes
#include <terralib/maptools/AbstractLayer.h>
#include <terralib/geometry/Envelope.h>
#include <terralib/qt/widgets/canvas/MapDisplay.h>

//STL Includes
#include <string>

//QT Includes
#include <QPixmap>

namespace geopx
{
  namespace tools
  {
    //forward declarations
    class Tile;

    class TileGeneratorService
    {
      public:

        TileGeneratorService();

        ~TileGeneratorService();

      public:

        void setInputParameters(std::list<te::map::AbstractLayerPtr> layers, te::gm::Envelope env, int srid, int zoomLevelMin, int zoomLevelMax, int tileSize, std::string path, std::string format);

        void runValidation(bool createMissingTiles);

        void runService(bool isRaster);

      protected:

        void checkParameters();

        void buildDisplay();

        QPixmap* drawTile(te::gm::Envelope env);

        QPixmap drawTile(te::gm::Envelope env, int level);

        void validateTile(FILE* file, Tile* tile, int level, int tileIdxX, int tileIdxY, bool& isValid);

        void savePixmap(Tile* tile, QPixmap* pix, int level, int tileIdxX, int tileIdxY);

        std::string getOSMTileFilePath(Tile* tile, int level, int tileIdxX, int tileIdxY, bool save);

      protected:
        te::qt::widgets::MapDisplay* m_display;

        std::list<te::map::AbstractLayerPtr> m_layers;

        te::gm::Envelope m_env;

        int m_srid;

        int m_zoomLevelMin;
        int m_zoomLevelMax;

        int m_tileSize;

        std::string m_path;
        std::string m_format;
        std::string m_logFile;
    };

  } // end namespace tools
} // end namespace geopx

#endif //__GEOPXDESKTOP_TOOLS_TILEGENERATOR_TILEGENERATORSERVICE_H
