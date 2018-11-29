/*!
  \file geopx-desktop/src/geopixeldesktop/layer/TiledLayerRendererFactory.h

  \brief  This is the concrete factory for renderers of a TiledLayer.
*/

#ifndef __GEOPXDESKTOP_DESKTOP_LAYER_TILEDLAYERRENDERERFACTORY_H
#define __GEOPXDESKTOP_DESKTOP_LAYER_TILEDLAYERRENDERERFACTORY_H

#include "../Config.h"

//Terralib
#include <terralib/maptools/RendererFactory.h>

namespace geopx
{
  namespace desktop
  {
    namespace layer
    {

      /*!
        \class TiledLayerRendererFactory

        \brief This is the concrete factory for renderers of a TiledLayer.
      */
      class TiledLayerRendererFactory : public te::map::RendererFactory
      {
      public:

        ~TiledLayerRendererFactory();

      protected:

        te::map::AbstractRenderer* build();

        TiledLayerRendererFactory();

      private:

        static TiledLayerRendererFactory sm_factory; //!< A pointer to the global renderer factory.

      };

    }
  }
}

#endif //__GEOPXDESKTOP_DESKTOP_LAYER_TILEDLAYERRENDERERFACTORY_H
