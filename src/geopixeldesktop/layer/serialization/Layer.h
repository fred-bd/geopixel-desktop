/*!
  \file geopx-desktop/src/geopixeldesktop/layer/serialization/Layer.h

  \brief  Auxiliary classes and functions to read/write Tiled layers using XML document.
*/

#ifndef __GEOPXDESKTOP_DESKTOP_LAYER_SERIALIZATION_LAYER_H
#define __GEOPXDESKTOP_DESKTOP_LAYER_SERIALIZATION_LAYER_H

#include "../../Config.h"

// TerraLib
#include <terralib/maptools/serialization/xml/Layer.h>

namespace geopx
{
  namespace desktop
  {
    namespace layer
    {
      namespace serialize
      {

        te::map::AbstractLayer* LayerReader(te::xml::Reader& reader);

        te::map::AbstractLayer* GeopxDataSetLayerReader(te::xml::Reader& reader);

        void LayerWriter(const te::map::AbstractLayer* layer, te::xml::AbstractWriter& writer);

        void GeopxDataSetLayerWriter(const te::map::AbstractLayer* layer, te::xml::AbstractWriter& writer);

      }
    }
  }
}

#endif // __GEOPXDESKTOP_DESKTOP_LAYER_SERIALIZATION_LAYER_H
