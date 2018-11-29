/*!
  \file geopx-desktop/src/geopixeldesktop/layer/TiledLayer.h

  \brief A layer with reference to a Tiled Layer.
*/

#ifndef __GEOPXDESKTOP_DESKTOP_LAYER_TILEDLAYER_H
#define __GEOPXDESKTOP_DESKTOP_LAYER_TILEDLAYER_H

#include "../Config.h"

//TerraLib
#include <terralib/maptools/AbstractLayer.h>


namespace geopx
{
  namespace desktop
  {
    namespace layer
    {

      /*!
        \class TiledLayer

        \brief A layer with reference to a Tiled Layer.
      */
      class TiledLayer : public te::map::AbstractLayer
      {
        
      public:

        /*!
          \brief It initializes a new layer.

          \param parent The parent layer (NULL if it has no parent).
        */
        TiledLayer(te::map::AbstractLayer* parent = 0);

        /*!
          \brief It initializes a new layer.

          \param id     The layer id.
          \param parent The parent layer (NULL if it has no parent).
        */
        TiledLayer(const std::string& id, te::map::AbstractLayer* parent = 0);

        /*!
          \brief It initializes a new Layer.

          \param id     The layer id.
          \param title  The title is a brief description about the layer.
          \param parent The parent layer (NULL if it has no parent).
        */
        TiledLayer(const std::string& id, const std::string& title, te::map::AbstractLayer* parent = 0);

        /*! \brief Destructor. */
        ~TiledLayer();

        AbstractLayer* clone();

        void setTiledLayerURL(const std::string& url);

        std::string getTiledLayerURL(const int& x, const int& y, const int& z) const;

        std::string getTiledLayerURL() const;

        void setTileSize(const int& size);

        int getTileSize() const;

        virtual std::unique_ptr<te::map::LayerSchema> getSchema() const;

        std::unique_ptr<te::da::DataSet> getData(te::common::TraverseType travType = te::common::FORWARDONLY,
                                               const te::common::AccessPolicy accessPolicy = te::common::RAccess) const;

        std::unique_ptr<te::da::DataSet> getData(const std::string& propertyName,
                                               const te::gm::Envelope* e,
                                               te::gm::SpatialRelation r = te::gm::INTERSECTS,
                                               te::common::TraverseType travType = te::common::FORWARDONLY,
                                               const te::common::AccessPolicy accessPolicy = te::common::RAccess) const;

        std::unique_ptr<te::da::DataSet> getData(const std::string& propertyName,
                                               const te::gm::Geometry* g,
                                               te::gm::SpatialRelation r,
                                               te::common::TraverseType travType = te::common::FORWARDONLY,
                                               const te::common::AccessPolicy accessPolicy = te::common::RAccess) const;

        std::unique_ptr<te::da::DataSet> getData(te::da::Expression* restriction,
                                               te::common::TraverseType travType = te::common::FORWARDONLY,
                                               const te::common::AccessPolicy accessPolicy = te::common::RAccess) const;

        std::unique_ptr<te::da::DataSet> getData(const te::da::ObjectIdSet* oids,
                                               te::common::TraverseType travType = te::common::FORWARDONLY,
                                               const te::common::AccessPolicy accessPolicy = te::common::RAccess) const;

        bool isValid() const;

        void draw(te::map::Canvas* canvas, const te::gm::Envelope& bbox, int srid, const double& scale, bool* cancel);

        const std::string& getType() const;

        const std::string& getDataSourceId() const;

        void setDataSourceId(const std::string& datadourceId);

        const std::string& getRendererType() const;

        void setRendererType(const std::string& rendererType);

        
      private:

        std::string                        m_datasourceId;   //!< The DataSource associated to this layer.
        std::string                        m_rendererType;   //!< A pointer to the internal renderer used to paint this layer.
        
        te::gm::Envelope                   m_currentExtent;  //!< Layer extent in current SRID.

        mutable std::unique_ptr<te::map::LayerSchema> m_schema; //!< The Tiled layer schema.

        static const std::string sm_type;                    //!< A static data member used in the implementation of getType method.

        std::string m_tiledLayerURL;
        int m_tileSize;
      };

      typedef boost::intrusive_ptr<TiledLayer> TiledLayerPtr;
      
    }
  }
}

#endif // __GEOPXDESKTOP_DESKTOP_LAYER_TILEDLAYER_H
