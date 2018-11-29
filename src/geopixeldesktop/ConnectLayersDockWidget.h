/*!
  \file geopx-desktop/src/geopixeldesktop/ConnectLayersDockWidget.h

  \brief This class implements a widget for connect layers management.
*/

#ifndef __GEOPXDESKTOP_DESKTOP_CONNECTLAYERSDOCKWIDGET_H
#define __GEOPXDESKTOP_DESKTOP_CONNECTLAYERSDOCKWIDGET_H

#include "Config.h"
#include "ConnectLayersTracking.h"
#include "ConnectLayersInfoWidget.h"

// TerraLib
#include <terralib/maptools/AbstractLayer.h>
#include <terralib/qt/af/ApplicationController.h>
#include <terralib/qt/widgets/canvas/MapDisplay.h>
#include <terralib/qt/widgets/layer/explorer/LayerItemView.h>

// STL
#include <list>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

// Qt
#include <QDockWidget>
#include <QLabel>
#include <QTreeWidgetItem>
#include <QMenu>

namespace Ui { class ConnectLayersDockWidgetForm; }

namespace geopx
{
  namespace desktop
  {

    class ConnectLayersDockWidget : public QDockWidget
    {
      Q_OBJECT

    public:

      ConnectLayersDockWidget(QWidget* parent = 0);

      ~ConnectLayersDockWidget();

      void setApplicationComponents(te::qt::widgets::LayerItemView* layerExplorer, te::qt::widgets::MapDisplay* mapDisplay, QMenu* mapMenu);

      void setLayers();

      void clear();

      QWidget* getInfoWidget();

    public slots:

      void onMapDisplayExtentChanged();

      void onDisplayPaintEvent(QPainter* painter);

      void onDrawToolButtonClicked();

      void onUpdateToolButtonClicked();

      void onUpToolButtonClicked();

      void onDownToolButtonClicked();

      void onAlphaSliderChanged(int value);

      void onVSliderChanged(int value);

      void onHSliderChanged(int value);

      void onFlickerTolButtonClicked(bool flag);

      void onBoxToolButtonClicked(bool flag);

      void onConnectLayerBoxDefined(QRect rect);

      void onConnectLayerBoxUpLayer();

      void onConnectLayerBoxCtrlUpLayer();

      void onConnectLayerBoxDownLayer();

      void onConnectLayerBoxCtrlDownLayer();

      void onConnectLayerBoxLeftLayer();

      void onConnectLayerBoxCtrlLeftLayer();

      void onConnectLayerBoxRightLayer();

      void onConnectLayerBoxCtrlRightLayer();

      void onConnectLayerBoxFlickLayer();

      void onShowConnectLayersToggled(bool flag);

      void enableMouseEvents();

      void disableMouseEvents();

      void onResetTool(QAction* action);

      void clearTrackTool();

    protected:

      void drawLayers();

      void drawConnectedLayers();

      void updateSliderBarSize();

      void clearPixmaps();

      bool findLayer(std::string id);

      void resetTreeItemsColor();

    private:

      std::unique_ptr<Ui::ConnectLayersDockWidgetForm> m_ui;

      geopx::desktop::ConnectLayersTracking* m_tracking;

      te::qt::widgets::LayerItemView* m_layerExplorer;

      te::qt::widgets::MapDisplay* m_mapDisplay;

      std::vector< std::pair<QTreeWidgetItem*, QPixmap*> > m_layersPixmaps;

      std::vector< std::pair<QTreeWidgetItem*, std::list< te::map::AbstractLayerPtr> > > m_vecLayerList;

      bool m_isConnected;

      bool m_flick;

      bool m_toolChecked;

      std::size_t m_curLayerPix;

      QMenu* m_mapMenu;

      QRect m_rect;

      QAction* m_currentAction;

      ConnectLayersInfoWidget* m_infoWidget;

      QTreeWidgetItem* m_currentItem;

      QImage m_img;
    };

  } // end namespace desktop
} // end namespace geopx


#endif  // __GEOPXDESKTOP_DESKTOP_CONNECTLAYERSDOCKWIDGET_H
