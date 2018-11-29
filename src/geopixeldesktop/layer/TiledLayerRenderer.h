/*!
  \file geopx-desktop/src/geopixeldesktop/layer/TiledLayerRenderer.h

  \brief  A renderer to draw a Tiled Layer.
*/

#ifndef __GEOPXDESKTOP_DESKTOP_LAYER_TILEDLAYERRENDERER_H
#define __GEOPXDESKTOP_DESKTOP_LAYER_TILEDLAYERRENDERER_H

#include "../Config.h"

//Terralib
#include <terralib/geometry/Envelope.h>
#include <terralib/maptools/AbstractRenderer.h>

//STL
#include <string>

//Qt
#include <QPixmap>

namespace geopx
{
  namespace desktop
  {
    namespace layer
    {
      struct TiledPixmap
      {
        QPixmap m_pix;
        int m_x;
        int m_y;
        int m_z;
        te::gm::Envelope m_env;
      };

      /*!
        \class TiledLayerRenderer

        \brief It renders the data associated to a Tiled layer.
      */
      class TiledLayerRenderer : public te::map::AbstractRenderer
      {
        public:

          TiledLayerRenderer();

          ~TiledLayerRenderer();

          void draw(te::map::AbstractLayer* layer, te::map::Canvas* canvas, const te::gm::Envelope& bbox, int srid, const double& scale, bool* cancel);

      };

    }
  }
}

#endif
