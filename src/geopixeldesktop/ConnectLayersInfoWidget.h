/*!
  \file geopx-desktop/src/geopixeldesktop/ConnectLayersInfoWidget.h

  \brief This class implements a status bar widget for connect layers information.
*/

#ifndef __GEOPXDESKTOP_DESKTOP_CONNECTLAYERSINFOWIDGET_H
#define __GEOPXDESKTOP_DESKTOP_CONNECTLAYERSINFOWIDGET_H

// STL
#include <string>

// Qt
#include <QLabel>
#include <QToolButton>
#include <QString>
#include <QWidget>

namespace geopx
{
  namespace desktop
  {

      class ConnectLayersInfoWidget : public QWidget
      {
        Q_OBJECT

        public:

          ConnectLayersInfoWidget(QWidget* parent);

          ~ConnectLayersInfoWidget();

        public:

          void setConnectLayerStatus(bool status);

          void setNumberOfLayersConnected(std::size_t nLayers);

          void setFlickerStatus(bool status);

          void setCurrentLayerName(const std::string& name);

          void clear();

        protected:

          void buildUI();

          void update();

        protected:

          QString m_currentLayer;
          bool m_connectLayerStatus;
          bool m_flickerStatus;
          std::size_t m_nLayers;

          QLabel* m_connectLabel;
          QLabel* m_connectInfoLabel;
          QLabel* m_flickLabel;
          QLabel* m_flickStatusLabel;
          QLabel* m_layerLabel;
          QLabel* m_layerNameLabel;
      };

  } // end namespace desktop
} // end namespace geopx

#endif  // __GEOPXDESKTOP_DESKTOP_CONNECTLAYERSINFOWIDGET_H
