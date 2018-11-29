/*!
  \file geopx-desktop/src/geopixeldesktop/DisplaysDockWidget.h

  \brief This class implements a widget for displays management.
*/

#ifndef __GEOPXDESKTOP_DESKTOP_DISPLAYSDOCKWIDGET_H
#define __GEOPXDESKTOP_DESKTOP_DISPLAYSDOCKWIDGET_H

#include "Config.h"

// TerraLib
#include <terralib/qt/af/connectors/MapDisplay.h>
#include <terralib/qt/widgets/canvas/MapDisplay.h>
#include <terralib/qt/widgets/canvas/EyeBirdMapDisplayWidget.h>
#include <terralib/qt/widgets/canvas/ZoomInMapDisplayWidget.h>

// Qt
#include <QDockWidget>

namespace Ui { class DisplaysDockWidgetForm; }

namespace geopx
{
  namespace desktop
  {

    class DisplaysDockWidget : public QDockWidget
    {
      Q_OBJECT

    public:

      DisplaysDockWidget(te::qt::widgets::MapDisplay* mapDisplay, te::qt::af::MapDisplay* mapDisplayConnector , QWidget* parent = 0);

      ~DisplaysDockWidget();

      void resizeEvent(QResizeEvent* e);

    public slots:

      void onShowDisplaysToggled(bool flag);

    private:

      std::unique_ptr<Ui::DisplaysDockWidgetForm> m_ui;

      te::qt::widgets::ZoomInMapDisplayWidget* m_zoomInMapDisplay;

      te::qt::widgets::EyeBirdMapDisplayWidget* m_eyeBirdMapDisplay;

    };

  } // end namespace desktop
} // end namespace geopx


#endif  // __GEOPXDESKTOP_DESKTOP_DISPLAYSDOCKWIDGET_H
