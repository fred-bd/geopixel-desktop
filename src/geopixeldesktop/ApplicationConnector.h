/*!
  \file geopx-desktop/src/geopixeldesktop/ApplicationConnector.h

  \brief A connector for the TerraLib application items to the Geopixel App
*/

#ifndef __GEOPXDESKTOP_DESKTOP_APPLICATIONCONNECTOR_H
#define __GEOPXDESKTOP_DESKTOP_APPLICATIONCONNECTOR_H

// Terralib
#include "../Config.h"
#ifndef Q_MOC_RUN
#include <terralib/maptools/AbstractLayer.h>
#endif

#include <terralib/qt/af/events/Event.h>
#include <terralib/qt/widgets/layer/explorer/LayerItemView.h>
#include <terralib/qt/widgets/se/StyleDockWidget.h>

// Qt
#include <QtCore/QObject>

// STL
#include <vector>
#include <list>

namespace geopx
{
  namespace desktop
  {
    class ApplicationConnector : public QObject
    {
      Q_OBJECT

      public:

        /*!
          \brief Constructor.

          \param explorer te::qt::widgets::LayerExplorer to be listened.
        */
        ApplicationConnector(te::qt::widgets::LayerItemView* layerExplorer, te::qt::widgets::StyleDockWidget* styleExplorer, QObject* parent = 0);

        /*! \brief Destructor. */
        ~ApplicationConnector();

      protected slots:

        void onLayerVisibilityChanged();

        void styleChanged(te::map::AbstractLayer* l);

      signals:

        void drawLayers();


      protected:

        te::qt::widgets::LayerItemView* m_layerExplorer;      //!< Pointer to a component te::qt::widgets::LayerExplorer.

        te::qt::widgets::StyleDockWidget* m_styleExplorer;    //!< Pointer to a component te::qt::widgets::StyleDockWidget.
    };

  }   // end namespace desktop
}   // end namespace geopx

#endif // __GEOPXDESKTOP_DESKTOP_APPLICATIONCONNECTOR_H

