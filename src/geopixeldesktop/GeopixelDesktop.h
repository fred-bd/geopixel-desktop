/*!
  \file geopx-desktop/src/geopixeldesktop/GeopixelDesktop.h

  \brief The main class of GeopixelDesktop.
*/

#ifndef __GEOPXDESKTOP_DESKTOP_GEOPIXELDESKTOP_H
#define __GEOPXDESKTOP_DESKTOP_GEOPIXELDESKTOP_H

#include "ApplicationConnector.h"
#include "GeopixelDesktopController.h"
#include "ConnectLayersDockWidget.h"
//#include "DisplaysDockWidget.h"

#include "layer/GeopxItemDelegate.h"
#include "layer/TiledItemDelegate.h"

// Geopixel Tools
#include "../geopixeltools/forestMonitor/ForestMonitorAction.h"
#include "../geopixeltools/forestMonitor/ForestMonitorClassAction.h"
#include "../geopixeltools/forestMonitor/ForestMonitorToolBarAction.h"
#include "../geopixeltools/forestMonitor/NDVIAction.h"
#include "../geopixeltools/photoIndex/PhotoIndexAction.h"
#include "../geopixeltools/tileGenerator/TileGeneratorAction.h"

// TerraLib
#include <terralib/qt/af/BaseApplication.h>

// STL
#include <string>

// Forward declarations
class QWidget;
class QMenu;

namespace te
{
  namespace qt
  {
    namespace af
    {
      class InterfaceController;
    }
    namespace widgets
    {
      class ChartDisplayWidget;
      class CompositionModeMenuWidget;
      class HelpManagerImpl;
      class QueryDialog;
      class ProgressViewerBar;
      class ProgressViewerWidget;
    }
  }
}

/*!
  \brief The main class of GeopixelDesktop.

  \sa te::qt::af::BaseApplication
*/
namespace geopx
{
  namespace desktop
  {
    struct ProjectMetadata;

    class GeopixelDesktop : public te::qt::af::BaseApplication
    {
      Q_OBJECT

      public:

        GeopixelDesktop(QWidget* parent = 0);

        ~GeopixelDesktop();

        void init(const QString& cfgFile);

        void startProject(const QString& projectFileName);

      protected:

        virtual void makeDialog();

        virtual void initActions();

        virtual void initMenus();

        virtual void initToolbars();

        virtual void initSlotsConnections();

        virtual void createDefaultSettings();

        void addMenusActions();

        void addPopUpMenu();

        void addGeopxMenuActions();

        void removeGeopxMenuActions();

      protected slots:

        void showAboutDialog();

        void onLayerShowTableTriggered();

        void onApplicationTriggered(te::qt::af::evt::Event* e);


        void onRestartSystemTriggered();

        void onNewProjectTriggered();

        void onOpenProjectTriggered();

        void onSaveProjectTriggered(bool save_as = false);

        void onSaveProjectAsTriggered();

        void onSaveAsImageTriggered();


        void onHelpTriggered();


        void onLinkTriggered();

        void onLayerHistogramTriggered();

        void onLayerScatterTriggered();

        void onLayerChartTriggered();

        void onLayerDuplicateLayerTriggered();

        void onLayerGroupingTriggered();

        void onLayerCompositionModeTriggered();

        void onSpatialQueryLayerTriggered();

        void onAttrQueryLayerTriggered();

        void onQueryLayerCreateLayer(te::map::AbstractLayerPtr layer);

        void onAddDataSetLayerTriggered();

        void onAddQueryLayerTriggered();

        void onAddTabularLayerTriggered();

        void onAddFolderLayerTriggered();

        void onUpdateLayerDataSourceTriggered();

        void onRecentProjectsTriggered(QAction* proj);

        void onProjectAddGeomGeopxLayerTriggered();

        void onProjectAddTileGeopxLayerTriggered();


        void onPluginsManagerTriggered();

        void onToolsCustomizeTriggered();

        void onToolsDataExchangerTriggered();

        void onToolsDataExchangerDirectTriggered();

        void onToolsDataExchangerDirectPopUpTriggered();

        void onToolsQueryDataSourceTriggered();

        void onToolsRasterMultiResolutionTriggered();


        void onDataSourceExplorerTriggered();


        void showProgressDockWidget();

        void onHighlightLayerObjects(const te::map::AbstractLayerPtr& layer, te::da::DataSet* dataset, te::se::Style* style);

        void onCreateNewLayer(te::map::AbstractLayerPtr layer);

      protected:

        void projectChanged();

        bool checkAndSaveProject();

        void openProject(const QString& prjFileName);

        void resetComponents();

        void closeEvent(QCloseEvent * event);

        void addActions(const QString& name, const QString& category, const QList<QAction*>& acts);

        QAction* m_fileNewProject;
        QAction* m_fileSaveProject;
        QAction* m_fileSaveProjectAs;
        QAction* m_fileOpenProject;
        QAction* m_fileExit;
        QAction* m_fileSaveAsImage;
        QAction* m_fileRestartSystem;

        QAction* m_helpAbout;
        QAction* m_helpContents;
        QAction* m_helpUpdate;
   
        QAction* m_layerChartsHistogram;
        QAction* m_layerChartsScatter;
        QAction* m_layerChart;
        QAction* m_layerDuplicateLayer;
        QAction* m_layerLinkTable;
        QAction* m_layerObjectGrouping;
        QAction* m_layerCompositionMode;
        QAction* m_layerSpatialQuery;
        QAction* m_layerAttrQuery;

        QAction* m_pluginsManager;

        QAction* m_projectAddLayerDataset;
        QAction* m_projectAddLayerQueryDataSet;
        QAction* m_projectAddLayerTabularDataSet;
        QAction* m_projectAddLayerGraph;
        QAction* m_projectAddFolderLayer;
        QAction* m_projectUpdateLayerDataSource;

        QAction* m_projectAddGeomGeopxLayer;
        QAction* m_projectAddTileGeopxLayer;

        QAction* m_toolsCustomize;
        QAction* m_toolsDataExchanger;
        QAction* m_toolsDataExchangerDirect;
        QAction* m_toolsDataExchangerDirectPopUp;
        QAction* m_toolsDataSourceExplorer;
        QAction* m_toolsDataSourceManagement;
        QAction* m_toolsQueryDataSource;
        QAction* m_toolsRasterMultiResolution;

        QAction* m_showConnectLayers;
        //QAction* m_showDisplays;

        QMenu* m_fileMenu;
        QMenu* m_helpMenu;
        QMenu* m_layerMenu;
        QMenu* m_mapMenu;
        QMenu* m_pluginsMenu;
        QMenu* m_projectMenu;
        QMenu* m_projectAddLayerMenu;
        QMenu* m_recentProjectsMenu;
        QMenu* m_toolsMenu;
        QMenu* m_toolsExchangerMenu;
        QMenu* m_viewMenu;
        QMenu* m_viewToolBarsMenu;


        QDockWidget* m_progressDockWidget;       //!< Dock widget used to show progress information

        te::qt::widgets::HelpManagerImpl* m_helpManager;

        te::qt::af::InterfaceController* m_iController;

        te::qt::widgets::QueryDialog* m_queryDlg;

        te::qt::widgets::CompositionModeMenuWidget* m_compModeMenu;

        geopx::desktop::ProjectMetadata* m_project;

        geopx::desktop::GeopixelDesktopController* m_tvController;

        te::qt::widgets::ProgressViewerBar* m_pvb;

        te::qt::widgets::ProgressViewerWidget* m_pvw;

        //Geopixel new docks
        geopx::desktop::ConnectLayersDockWidget* m_connectLayersDock;
        //geopx::desktop::DisplaysDockWidget* m_displaysDock;
        geopx::desktop::ApplicationConnector* m_appConnector;

        //Geopixel tools Actions
        geopx::tools::ForestMonitorAction* m_forestMonitorAction;
        geopx::tools::ForestMonitorClassAction* m_forestMonitorClassAction;
        geopx::tools::ForestMonitorToolBarAction* m_forestMonitorToolBarAction;
        geopx::tools::NDVIAction* m_ndviAction;
        geopx::tools::PhotoIndexAction* m_photoIndexAction;
        geopx::tools::TileGeneratorAction* m_tileGeneratorAction;

        geopx::desktop::layer::GeopxItemDelegate* m_geopxDelegate;
        geopx::desktop::layer::TiledItemDelegate* m_tiledDelegate;

    };
  } // end namespace desktop
}   // end namespace geopx

#endif  // __GEOPXDESKTOP_DESKTOP_GEOPIXELDESKTOP_H
