/*!
  \file geopx-desktop/src/geopixeldesktop/layer/TiledLayerRenderer.cpp

  \brief  A renderer to draw a Tiled Layer.
*/

#include "TiledLayerRenderer.h"
#include "TiledLayer.h"
#include "../geopixeltools/tileGenerator/core/Tile.h"

//Terralib
#include <terralib/common/STLUtils.h>
#include <terralib/core/Exception.h>
#include <terralib/core/translator/Translator.h>
#include <terralib/geometry/Utils.h>
#include <terralib/maptools/Canvas.h>
#include <terralib/raster/Utils.h>
#include <terralib/qt/widgets/canvas/Canvas.h>

//LibCurl
#include <curl/curl.h>

// STL
#include <stdio.h>
#include <iostream>

//Qt
#include <QFileInfo>
#include <QPainter>
#include <QPixmap>
#include <QStandardPaths>



te::gm::Envelope viewPort(te::gm::Envelope boxA, te::gm::Envelope boxB, int width, int height) 
{
  long c1 = te::rst::Round((width / (boxA.getUpperRightX() - boxA.getLowerLeftX()))*(boxB.getLowerLeftX() - boxA.getLowerLeftX()));
  long l2 = te::rst::Round((height / (boxA.getUpperRightY() - boxA.getLowerLeftY()))*(boxA.getUpperRightY() - boxB.getLowerLeftY()));
  long c2 = te::rst::Round((width / (boxA.getUpperRightX() - boxA.getLowerLeftX()))*(boxB.getUpperRightX() - boxA.getLowerLeftX()));
  long l1 = te::rst::Round((height / (boxA.getUpperRightY() - boxA.getLowerLeftY()))*(boxA.getUpperRightY() - boxB.getUpperRightY()));

  te::gm::Envelope viewBox(c1, l1, c2, l2);

  return viewBox;
}

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) 
{
  size_t written = fwrite(ptr, size, nmemb, stream);
  return written;
}

void DownloadFile(const std::string& url, const std::string& outputFileName)
{
  CURL *curl;
  FILE *fp;
  CURLcode res;

  curl = curl_easy_init();

  if (curl) 
  {
    fp = fopen(outputFileName.c_str(), "wb");
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    res = curl_easy_perform(curl);

    /* always cleanup */
    curl_easy_cleanup(curl);
    fclose(fp);
  }
}

geopx::desktop::layer::TiledLayerRenderer::TiledLayerRenderer() = default;

geopx::desktop::layer::TiledLayerRenderer::~TiledLayerRenderer() = default;

void geopx::desktop::layer::TiledLayerRenderer::draw(te::map::AbstractLayer *layer, te::map::Canvas *canvas, const te::gm::Envelope &bbox, int srid, const double &scale, bool *cancel)
{

  //get tiled layer
  geopx::desktop::layer::TiledLayer* tiledLayer = dynamic_cast<geopx::desktop::layer::TiledLayer*>(layer);

  if (tiledLayer == nullptr)
  {
    throw te::core::Exception() << te::ErrorDescription(TE_TR("Wrong type render type for this layer!"));
  }

  int tileSize = tiledLayer->getTileSize();

  //check envelope
  if (!bbox.isValid())
  {
    return;
  }

  //get canvas qt
  te::qt::widgets::Canvas* canvasQt = dynamic_cast<te::qt::widgets::Canvas*>(canvas);

  if (canvasQt == nullptr)
  {
    throw te::core::Exception() << te::ErrorDescription(TE_TR("Error getting paint device!"));
  }

  // Adjust internal renderer transformer
  canvas->setWindow(bbox.m_llx, bbox.m_lly, bbox.m_urx, bbox.m_ury);

  //reproject envelope to google projection
  int google_srid = 3857;

  te::gm::Envelope reprojectedBBOX(bbox);

  reprojectedBBOX.transform(srid, google_srid);

  //TODO - check if the reprojected box intersects the tiled layer

  //calculate resolution
  double resolution = (std::min)(reprojectedBBOX.getWidth() / canvasQt->getWidth(), reprojectedBBOX.getHeight() / canvasQt->getHeight());

  //adjust box using min resolution and center of box
  double x = (reprojectedBBOX.getUpperRightX() + reprojectedBBOX.getLowerLeftX()) / 2;
  double y = (reprojectedBBOX.getUpperRightY() + reprojectedBBOX.getLowerLeftY()) / 2;

  double x1 = x - ((canvasQt->getWidth() / 2.) * resolution);
  double x2 = x + ((canvasQt->getWidth() / 2.) * resolution);
  double y1 = y - ((canvasQt->getHeight() / 2.) * resolution);
  double y2 = y + ((canvasQt->getHeight() / 2.) * resolution);

  te::gm::Envelope originalReprojectedBBOX = reprojectedBBOX;

  te::gm::Envelope adjustEnv(x1, y1, x2, y2);

  reprojectedBBOX = adjustEnv;

  // use tile class to get x,y range
  geopx::tools::Tile tile(resolution, tileSize);

  int zoomFactor = tile.bestZoomLevel(resolution);

  long tIdxX1, tIdxY1, tIdxX2, tIdxY2;

  tile.tileMatrix(reprojectedBBOX, tIdxX1, tIdxY1, tIdxX2, tIdxY2);

  canvasQt->clear();

  QPainter* p = canvasQt->getPainter();

  p->setWorldMatrixEnabled(false);

  for (int k = tIdxY2; k <= tIdxY1; ++k)
  {
    for (int j = tIdxX1; j <= tIdxX2; ++j)
    {
      std::string urlPath = tiledLayer->getTiledLayerURL(j, k, zoomFactor);

      QString userDataDir = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
      userDataDir.append("/tiledCache/");

      std::string outputFile = userDataDir.toStdString();
      outputFile += te::core::CharEncoding::fromUTF8(layer->getId());
      outputFile += "_";
      outputFile += std::to_string(zoomFactor);
      outputFile += "_";
      outputFile += std::to_string(j);
      outputFile += "_";
      outputFile += std::to_string(k);
      outputFile += ".jpg";

      te::gm::Envelope tileBox = tile.tileBox(j, k);
      te::gm::Envelope subTile = originalReprojectedBBOX.intersection(tileBox);

      if (subTile.isValid())
      {
        te::gm::Envelope s = viewPort(tileBox, subTile, tileSize, tileSize);
        te::gm::Envelope d = viewPort(originalReprojectedBBOX, subTile, canvasQt->getWidth(), canvasQt->getHeight());

        try
        {
          if(QFileInfo::exists(outputFile.c_str()) == false)
          {
            DownloadFile(urlPath, outputFile);
          }

          QPixmap pixItem(QString::fromUtf8(outputFile.c_str()));

          if (!pixItem.isNull())
          {
            p->drawPixmap(d.getLowerLeftX(), d.getLowerLeftY(), d.getWidth(), d.getHeight(),
              pixItem,
              s.getLowerLeftX(), s.getLowerLeftY(), s.getWidth(), s.getHeight());
          }

        }
        catch (...)
        {
          continue;
        }
      }
    }
  }

  p->setWorldMatrixEnabled(true);
}
