/*!
  \file geopx-desktop/src/geopixeldesktop/layer/GeopxDataSetLayer.h

  \brief Implements Terralib layer GeopxDataSetLayer.
*/

#ifndef __GEOPXDESKTOP_DESKTOP_LAYER_GEOPXDATASETLAYER_H
#define __GEOPXDESKTOP_DESKTOP_LAYER_GEOPXDATASETLAYER_H

// TerraLib
#include <terralib/maptools/DataSetLayer.h>

namespace geopx
{
  namespace desktop
  {
    namespace layer
    {

      class GeopxDataSetLayer : public te::map::DataSetLayer
      {
        public:

        /*!
          \brief It initializes a new layer.

          \param parent The parent layer (NULL if it has no parent).
        */
        GeopxDataSetLayer(AbstractLayer* parent = 0);

        /*!
          \brief It initializes a new layer.

          \param id     The layer id.
          \param parent The parent layer (NULL if it has no parent).
        */
        GeopxDataSetLayer(const std::string& id, AbstractLayer* parent = 0);

        /*!
          \brief It initializes a new Layer.

          \param id     The layer id.
          \param title  The title is a brief description about the layer.
          \param parent The parent layer (NULL if it has no parent).
        */
        GeopxDataSetLayer(const std::string& id, const std::string& title, AbstractLayer* parent = 0);

        /*! \brief Destructor. */
        ~GeopxDataSetLayer();

        AbstractLayer* clone();

        /*!
          \brief It returns the layer type: DATASET_LAYER.

          \return The layer type: DATASET_LAYER.
        */
        const std::string& getType() const;

        const std::string getUser() const;

        void setUser(const std::string& user);

        const std::string getProfile() const;

        void setProfile(const std::string& profile);

        const int getThemeId() const;

        void setThemeId(const int& themeId);

      private:

        std::string m_user;
        std::string m_profile;
        int m_themeId;

        static const std::string sm_type;  //!< A static data member used in the implementation of getType method.
      };

      typedef boost::intrusive_ptr<GeopxDataSetLayer> GeopxDataSetLayerPtr;
    }

  } // end namespace map
}   // end namespace te

#endif  // __GEOPXDESKTOP_DESKTOP_LAYER_GEOPXDATASETLAYER_H
