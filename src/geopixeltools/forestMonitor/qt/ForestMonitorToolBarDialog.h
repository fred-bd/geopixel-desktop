/*!
  \file geopx-desktop/src/geopixeltools/qt/ForestMonitorToolBarDialog.h

  \brief This interface is a tool bar for forest monitor classification operations.
*/

#ifndef __GEOPXDESKTOP_TOOLS_FORESTMONITOR_FORESTMONITORTOOLBARDIALOG_H
#define __GEOPXDESKTOP_TOOLS_FORESTMONITOR_FORESTMONITORTOOLBARDIALOG_H

#include "../../Config.h"

// TerraLib
#include <terralib/qt/af/connectors/MapDisplay.h>
#include <terralib/maptools/AbstractLayer.h>

// STL
#include <memory>

// Qt
#include <QDialog>

namespace Ui { class ForestMonitorToolBarDialogForm; }

namespace geopx
{
  namespace tools
  {
    /*!
      \class ForestMonitorToolBarDialog

      \brief This interface is a tool bar for forest monitor classification operations.
    */
    class ForestMonitorToolBarDialog : public QDialog
    {
      Q_OBJECT

      public:

        ForestMonitorToolBarDialog(QWidget* parent = 0, Qt::WindowFlags f = 0);

        ~ForestMonitorToolBarDialog();

      public:

        void setLayerList(std::list<te::map::AbstractLayerPtr> list);

        void setMapDisplay(te::qt::af::MapDisplay* mapDisplay);

      protected:

        void createActions();

      protected slots:

        void onEraserToolButtonClicked(bool flag);

        void onUpdateToolButtonClicked(bool flag);

        void onCreatorToolButtonClicked(bool flag);

        void onCreatorLiveToolButtonClicked(bool flag);

        void onCreatorDeadToolButtonClicked(bool flag);

        void onTrackAutoClassifierToolButtonClicked(bool flag);

        void onTrackDeadClassifierToolButtonClicked(bool flag);

      private:

        std::unique_ptr<Ui::ForestMonitorToolBarDialogForm> m_ui;

        te::qt::af::MapDisplay* m_appDisplay;

        QAction* m_actionEraser;
        QAction* m_actionUpdate;
        QAction* m_actionCreator;
        QAction* m_actionCreatorLive;
        QAction* m_actionCreatorDead;
        QAction* m_actionAutoTrack;
        QAction* m_actionDeadTrack;
            
        bool m_clearTool;
    };

  }  // end namespace tools
}  // end namespace geopx

#endif  // __GEOPXDESKTOP_TOOLS_FORESTMONITOR_FORESTMONITORTOOLBARDIALOG_H

