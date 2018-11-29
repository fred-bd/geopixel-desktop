/*!
\file geopx-desktop/src/geopixeldesktop/GeopixelDesktop.cpp

\brief The main class of GeopixelDesktop.
*/

#include "GeopixelDesktop.h"

// GeopixelDesktop
#include "../Config.h"
#include "AboutDialog.h"
#include "Project.h"
#include "Utils.h"
#include "XMLFormatter.h"
#include "datasource/GeopixelConnectorDialog.h"
#include "datasource/GeopixelDataSetSelectorDialog.h"
#include "datasource/GeopixelDataModelUtils.h"
#include "layer/GeopxDataSetLayer.h"
#include "layer/serialization/Layer.h"

// TerraLib
#include <terralib/common/Exception.h>
#include <terralib/common/progress/ProgressManager.h>
#include <terralib/core/filesystem/FileSystem.h>
#include <terralib/core/utils/Platform.h>
#include <terralib/core/translator/Translator.h>
#include <terralib/dataaccess/utils/Utils.h>
#include <terralib/dataaccess/datasource/DataSourceCatalogManager.h>
#include <terralib/dataaccess/datasource/DataSourceInfoManager.h>
#include <terralib/dataaccess/datasource/DataSourceManager.h>
#include <terralib/maptools/Utils.h>
#include <terralib/maptools/serialization/xml/Layer.h>
#include <terralib/qt/af/ApplicationController.h>
#include <terralib/qt/af/connectors/ChartDisplayDockWidget.h>
#include <terralib/qt/af/connectors/DataSetTableDockWidget.h>
#include <terralib/qt/af/connectors/InterfaceController.h>
#include <terralib/qt/af/connectors/LayerExplorer.h>
#include <terralib/qt/af/connectors/MapDisplay.h>
#include <terralib/qt/af/connectors/StyleExplorer.h>
#include <terralib/qt/af/events/ApplicationEvents.h>
#include <terralib/qt/af/events/LayerEvents.h>
#include <terralib/qt/af/events/MapEvents.h>
#include <terralib/qt/af/events/ToolEvents.h>
#include <terralib/qt/af/settings/SettingsDialog.h>
#include <terralib/qt/af/Utils.h>
#include <terralib/qt/af/XMLFormatter.h>
#include <terralib/qt/widgets/canvas/EyeBirdMapDisplayWidget.h>
#include <terralib/qt/widgets/canvas/MultiThreadMapDisplay.h>
#include <terralib/qt/widgets/canvas/ZoomInMapDisplayWidget.h>
#include <terralib/qt/widgets/charts/ChartLayerDialog.h>
#include <terralib/qt/widgets/charts/HistogramDialog.h>
#include <terralib/qt/widgets/charts/ScatterDialog.h>
#include <terralib/qt/widgets/datasource/core/DataSourceType.h>
#include <terralib/qt/widgets/datasource/core/DataSourceTypeManager.h>
#include <terralib/qt/widgets/datasource/connector/AbstractDataSourceConnector.h>
#include <terralib/qt/widgets/datasource/selector/DataSourceExplorerDialog.h>
#include <terralib/qt/widgets/datasource/selector/DataSourceSelectorDialog.h>
#include <terralib/qt/widgets/exchanger/DataExchangerWizard.h>
#include <terralib/qt/widgets/exchanger/DirectExchangerDialog.h>
#include <terralib/qt/widgets/externalTable/DataPropertiesDialog.h>
#include <terralib/qt/widgets/externalTable/TableLinkDialog.h>
#include <terralib/qt/widgets/help/HelpManager.h>
#include <terralib/qt/widgets/Utils.h>
#include <terralib/qt/widgets/layer/explorer/LayerItem.h>
#include <terralib/qt/widgets/layer/explorer/LayerItemView.h>
#include <terralib/qt/widgets/layer/selector/AbstractLayerSelector.h>
#include <terralib/qt/widgets/layer/utils/CompositionModeMenuWidget.h>
#include <terralib/qt/widgets/plugin/manager/PluginManagerDialog.h>
#include <terralib/qt/widgets/progress/ProgressViewerBar.h>
#include <terralib/qt/widgets/progress/ProgressViewerDialog.h>
#include <terralib/qt/widgets/progress/ProgressViewerWidget.h>
#include <terralib/qt/widgets/query/QueryBuilderDialog.h>
#include <terralib/qt/widgets/query/QueryDataSourceDialog.h>
#include <terralib/qt/widgets/query/QueryDialog.h>
#include <terralib/qt/widgets/query/QueryLayerBuilderWizard.h>
#include <terralib/qt/widgets/raster/MultiResolutionDialog.h>
#include <terralib/qt/widgets/rp/Utils.h>
#include <terralib/qt/widgets/se/GroupingDialog.h>
#include <terralib/qt/widgets/se/StyleDockWidget.h>
#include <terralib/qt/widgets/se/StyleControllerWidget.h>
#include <terralib/qt/widgets/srs/SRSManagerDialog.h>
#include <terralib/qt/widgets/tools/Measure.h>

// STL
#include <memory>
#include <fstream>

// Qt
#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QFileDialog>
#include <QImageWriter> 
#include <QInputDialog>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QModelIndex>
#include <QStandardPaths>
#include <QToolBar>

// Boost
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

te::qt::af::DataSetTableDockWidget* GetLayerDock(const te::map::AbstractLayer* layer, const std::vector<te::qt::af::DataSetTableDockWidget*>& docs)
{
  std::vector<te::qt::af::DataSetTableDockWidget*>::const_iterator it;

  for (it = docs.begin(); it != docs.end(); ++it)
    if ((*it)->getLayer() == layer)
      return *it;

  return nullptr;
}

QString GetWindowTitle(const geopx::desktop::ProjectMetadata& project, te::qt::af::ApplicationController* app)
{
  QString title = app->getAppTitle() + " - ";
  title += TE_TR("Project:");
  title += " ";
  title += project.m_title;
  title += " - ";

  boost::filesystem::path p(project.m_fileName.toUtf8().data());

  std::string filename = p.filename().string();

  title += filename.c_str();

  return title;
}

void GetProjectsFromSettings(QStringList& prjTitles, QStringList& prjPaths)
{
  QSettings s(QSettings::IniFormat,
              QSettings::UserScope,
              qApp->organizationName(),
              qApp->applicationName());

  s.beginGroup("projects");

  s.beginGroup("most_recent");
  QString path = s.value("path").toString();
  QString title = s.value("title").toString();

  s.endGroup();

  if(path.isEmpty() || title.isEmpty())
    return;

  prjPaths.append(path);
  prjTitles.append(title);

  int size = s.beginReadArray("recents");

  for(int i=0; i<size; i++)
  {
    s.setArrayIndex(i);

    path = s.value("project/path").toString();
    title = s.value("project/title").toString();

    prjPaths.append(path);
    prjTitles.append(title);
  }

  s.endArray();
  s.endGroup();
}

void WriteProjectsToSettings(const QStringList& prjTitles, const QStringList& prjPaths)
{
  QSettings s(QSettings::IniFormat,
              QSettings::UserScope,
              qApp->organizationName(),
              qApp->applicationName());

  s.beginGroup("projects");
  s.beginGroup("most_recent");
  s.setValue("path", *prjPaths.begin());
  s.setValue("title", *prjTitles.begin());
  s.endGroup();

  s.beginWriteArray("recents");
  for(int i=1; i<prjTitles.size(); i++)
  {
    s.setArrayIndex(i-1);

    s.setValue("project/path", prjPaths.at(i));
    s.setValue("project/title", prjTitles.at(i));
  }
  s.endArray();
  s.endGroup();
}

void AddRecentProjectToSettings(const QString& prjTitle, const QString& prjPath)
{
  QStringList prjPaths, prjTitles;

  GetProjectsFromSettings(prjTitles, prjPaths);

  if(!prjPaths.contains(prjPath))
  {
    prjTitles.prepend(prjTitle);
    prjPaths.prepend(prjPath);
  }
  else
  {
    int prjPathIdx = prjPaths.indexOf(prjPath);

    if(!prjTitles.contains(prjTitle))
      prjTitles.replace(prjPathIdx, prjTitle);

    prjTitles.move(prjPathIdx, 0);
    prjPaths.move(prjPathIdx, 0);
  }

  WriteProjectsToSettings(prjTitles, prjPaths);
}

QModelIndex GetParent(QTreeView* view)
{
  QModelIndex res;

  QModelIndexList idxs = view->selectionModel()->selectedIndexes();

  if(idxs.size() == 1)
  {
    QModelIndex idx = idxs.at(0);
    te::qt::widgets::TreeItem* item = static_cast<te::qt::widgets::TreeItem*>(idx.internalPointer());

    if(item->getType() == "FOLDER")
      res = idx;
  }

  return res;
}

void ResetProject(geopx::desktop::ProjectMetadata* p)
{
  p->m_author = "GEOPIXEL";
  p->m_title = QObject::tr("Default project");
  p->m_changed = false;
}

geopx::desktop::GeopixelDesktop::GeopixelDesktop(QWidget* parent)
  : te::qt::af::BaseApplication(parent),
  m_helpManager(nullptr),
  m_iController(nullptr),
  m_queryDlg(nullptr),
  m_compModeMenu(nullptr),
  m_tvController(nullptr),
  m_pvb(nullptr),
  m_pvw(nullptr),
  m_connectLayersDock(nullptr),
  //m_displaysDock(nullptr),
  m_appConnector(nullptr),
  m_geopxDelegate(nullptr),
  m_tiledDelegate(nullptr)
{
  m_project = new ProjectMetadata;

  ResetProject(m_project);
}

geopx::desktop::GeopixelDesktop::~GeopixelDesktop()
{
  if (m_iController)
  {
    m_iController->removeInteface(m_queryDlg);
    m_iController->removeInteface(m_styleExplorer->getExplorer());
  }

  //remove connectlayer label
  m_statusbar->removeWidget(m_connectLayersDock->getInfoWidget());
  m_connectLayersDock->clearTrackTool();

  removeGeopxMenuActions();

  delete m_iController;
  delete m_compModeMenu;
  delete m_queryDlg;
  delete m_project;
  delete m_tvController;

  delete m_appConnector;

  te::common::ProgressManager::getInstance().clearAll();
}

void geopx::desktop::GeopixelDesktop::init(const QString& cfgFile)
{
  //init base application
  BaseApplication::init(cfgFile);

  m_tvController = new GeopixelDesktopController(m_app, cfgFile.toUtf8().data());

  m_tvController->initializeProjectMenus();

  QStringList prjTitles, prjPaths;

  GetProjectsFromSettings(prjTitles, prjPaths);

  if(!prjPaths.empty())
    openProject(*prjPaths.begin());
  else
    setWindowTitle(windowTitle() + " - " + m_project->m_title);

  setWindowIcon(QIcon(geopx::desktop::FindInPath("share/geopixeldesktop/images/png/geopixeldesktop-icon.png").c_str()));

  te::qt::af::XMLFormatter::formatDataSourceInfos(false);
}

void geopx::desktop::GeopixelDesktop::startProject(const QString& projectFileName)
{
  openProject(projectFileName);
}

void geopx::desktop::GeopixelDesktop::makeDialog()
{
  te::qt::af::BaseApplication::makeDialog();

  addMenusActions();

  addPopUpMenu();

  addGeopxMenuActions();

  //composition mode
  m_compModeMenu = new te::qt::widgets::CompositionModeMenuWidget();
  m_layerCompositionMode->setMenu(m_compModeMenu->getMenu());

  //interface controller
  m_iController = new te::qt::af::InterfaceController(this);
  m_app->addListener(m_iController);

  //progress support
  m_pvb = new te::qt::widgets::ProgressViewerBar(this);
  m_pvb->setFixedWidth(220);

  m_pvw = new te::qt::widgets::ProgressViewerWidget(this);

  m_statusbar->addPermanentWidget(m_pvb);

  connect(m_pvb, SIGNAL(clicked()), this, SLOT(showProgressDockWidget()));

  m_progressDockWidget = new QDockWidget(this);
  m_progressDockWidget->setObjectName("ProgressDockWidget");
  m_progressDockWidget->setWidget(m_pvw);
  m_progressDockWidget->setMinimumHeight(300);
  m_progressDockWidget->setAllowedAreas(Qt::RightDockWidgetArea);
  m_progressDockWidget->setWindowTitle(tr("Tasks Progress"));
  addDockWidget(Qt::RightDockWidgetArea, m_progressDockWidget);
  m_progressDockWidget->setVisible(false);

  //add styleExplorer to interface controller
  if (m_iController)
    m_iController->addInterface(m_styleExplorer->getExplorer());

  //insert connect layers dock
  m_connectLayersDock = new geopx::desktop::ConnectLayersDockWidget(this);
  m_connectLayersDock->setApplicationComponents(getLayerExplorer(), m_display->getDisplay(), m_mapMenu);
  this->addDockWidget(Qt::LeftDockWidgetArea, m_connectLayersDock);
  

  m_statusbar->addWidget(m_connectLayersDock->getInfoWidget());

 // connect(m_showConnectLayers, SIGNAL(toggled(bool)), m_connectLayersDock, SLOT(onShowConnectLayersToggled(bool)));
  connect(m_showConnectLayers, SIGNAL(toggled(bool)), m_connectLayersDock, SLOT(setVisible(bool)));
 
  m_showConnectLayers->setChecked(true);
  m_connectLayersDock->setVisible(true);
 // m_connectLayersDock->onShowConnectLayersToggled(true);

  //insert displays dock
 // m_displaysDock = new geopx::desktop::DisplaysDockWidget(m_display->getDisplay(), m_display, this);
 // this->addDockWidget(Qt::LeftDockWidgetArea, m_displaysDock);

 // connect(m_showDisplays, SIGNAL(toggled(bool)), m_displaysDock, SLOT(onShowDisplaysToggled(bool)));

 // m_showDisplays->setChecked(true);
 // m_displaysDock->onShowDisplaysToggled(true);

  //adjust docks
  this->tabifyDockWidget(m_connectLayersDock, getLayerExplorerDock());
}

void geopx::desktop::GeopixelDesktop::initActions()
{
  te::qt::af::BaseApplication::initActions();

  // Menu -Tools- actions
  initAction(m_toolsCustomize, "preferences-system", "Tools.Customize", tr("&Customize..."), tr("Customize the system preferences"), true, false, true, m_menubar);
  initAction(m_toolsDataExchanger, "datasource-exchanger", "Tools.Exchanger.All to All", tr("&Advanced..."), tr("Exchange data sets between data sources"), true, false, true, m_menubar);
  initAction(m_toolsDataExchangerDirect, "data-exchange-direct-icon", "Tools.Exchanger.Direct", tr("&Layer..."), tr("Exchange data sets from layers"), true, false, true, m_menubar);
  initAction(m_toolsDataExchangerDirectPopUp, "data-exchange-direct-icon", "Tools.Exchanger.Direct", tr("&Exchange..."), tr("Exchange data sets from layers"), true, false, true, m_menubar);
  initAction(m_toolsDataSourceExplorer, "datasource-explorer", "Tools.Data Source Explorer", tr("&Data Source Explorer..."), tr("Show or hide the data source explorer"), true, false, true, m_menubar);
  initAction(m_toolsQueryDataSource, "datasource-query", "Tools.Query Data Source", tr("&Query Data Source..."), tr("Allows you to query data in a data source"), true, false, true, m_menubar);
  initAction(m_toolsRasterMultiResolution, "raster-multiresolution-icon", "Tools.Raster Multi Resolution", tr("&Raster Multi Resolution..."), tr("Creates multi resolution over a raster..."), true, false, true, m_menubar);

  // Menu -Plugins- actions
  initAction(m_pluginsManager, "plugin", "Plugins.Management", tr("&Manage Plugins..."), tr("Manage the application plugins"), true, false, true, m_menubar);

  // Menu -Help- actions
  initAction(m_helpContents, "help-browser", "Help.View Help", tr("&View Help..."), tr("Shows help dialog"), true, false, true, m_menubar);
  initAction(m_helpAbout, "help-about-browser", "Help.About", tr("&About..."), tr(""), true, false, true, m_menubar);

  // Menu -Project- actions
  initAction(m_projectAddGeomGeopxLayer, "datasource", "Project.Add Layer.GeopxVector", tr("Geopixel Vector DataSet..."), tr("Add a vectorial layer from a Geopixel Data source"), true, false, true, m_menubar);
  initAction(m_projectAddTileGeopxLayer, "datasource", "Project.Add Layer.GeopxTile", tr("Geopixel Tiled Layer..."), tr("Add a tiled layer from a Geopixel Data source"), true, false, true, m_menubar);

  initAction(m_projectAddLayerDataset, "datasource", "Project.Add Layer.All Sources", tr("&From Data Source..."), tr("Add a new layer from all available data sources"), true, false, true, m_menubar);
  initAction(m_projectAddFolderLayer, "folderlayer-new", "Project.New Folder Layer", tr("Add &Folder Layer..."), tr("Add a new folder layer"), true, false, true, m_menubar);
  initAction(m_projectAddLayerQueryDataSet, "view-filter", "Project.Add Layer.Query Dataset", tr("&Query Dataset..."), tr("Add a new layer from a queried dataset"), true, false, true, m_menubar);
  initAction(m_projectAddLayerTabularDataSet, "view-data-table", "Project.Add Layer.Tabular File", tr("&Tabular File..."), tr("Add a new layer from a Tabular file"), true, false, true, m_menubar);
  initAction(m_projectUpdateLayerDataSource, "", "Project.Update Layer Data Source", tr("&Update Layer Data Source"), tr("Update layer Data Source"), true, false, true, this);

  // Menu -Layer- actions
  initAction(m_layerObjectGrouping, "grouping", "Layer.ObjectGrouping", tr("&Edit Legend..."), tr(""), true, false, true, m_menubar);
  initAction(m_layerChartsHistogram, "chart-bar", "Layer.Charts.Histogram", tr("&Histogram..."), tr(""), true, false, true, m_menubar);
  initAction(m_layerChartsScatter, "chart-scatter", "Layer.Charts.Scatter", tr("&Scatter..."), tr(""), true, false, true, m_menubar);
  initAction(m_layerChart, "chart-pie", "Layer.Charts.Chart", tr("&Pie/Bar Chart..."), tr(""), true, false, true, m_menubar);
  initAction(m_layerAttrQuery, "view-filter", "Layer.AttributeQuery", tr("Attribute Query..."), tr(""), true, false, true, m_menubar);
  initAction(m_layerSpatialQuery, "view-filter", "Layer.SpatialQuery", tr("Spatial Query..."), tr(""), true, false, true, m_menubar);
  initAction(m_layerLinkTable, "layer-link", "Layer.Link", tr("&Link..."), tr(""), true, false, true, m_menubar);
  initAction(m_layerCompositionMode, "layer-compose", "Layer.Composition Mode", tr("&Composition Mode..."), tr("Set the composition mode to renderer the selected layer"), true, false, true, m_menubar);
  initAction(m_layerDuplicateLayer, "layer-duplicate", "Layer.Duplicate Layer", tr("&Duplicate Layer"), tr("Create a copy of a layer"), true, false, true, m_menubar);

  // Menu -File- actions
  initAction(m_fileNewProject, "document-new", "File.New Project", tr("&New Project..."), tr(""), true, false, true, m_menubar);
  initAction(m_fileSaveProject, "document-save", "File.Save Project", tr("&Save Project"), tr(""), true, false, true, m_menubar);
  initAction(m_fileSaveProjectAs, "document-save-as", "File.Save Project As", tr("Save Project &As..."), tr(""), true, false, false, m_menubar);
  initAction(m_fileOpenProject, "document-open", "File.Open Project", tr("&Open Project..."), tr(""), true, false, true, m_menubar);
  initAction(m_fileRestartSystem, "", "File.Restart System", tr("&Restart System..."), tr("Restart the system."), true, false, true, m_menubar);
  initAction(m_fileExit, "system-log-out", "File.Exit", tr("E&xit"), tr(""), true, false, true, m_menubar);
  initAction(m_fileSaveAsImage, "", "File.Save As Image", tr("&Save Display Image"), tr("Saves the screen drawing as an image ."), true, true, true, m_menubar);

  // Show Menu
  initAction(m_showConnectLayers, "", "Show.Connect Layers", tr("&Connect Layers"), tr("Show or hide the connection layers"), true, true, true, m_menubar);
  //initAction(m_showDisplays, "view-map-display", "Show.Displays", tr("&Displays"), tr("Show or hide the displays"), true, true, true, m_menubar);

  std::string iconConnectLayer = geopx::desktop::FindInPath("share/geopixeldesktop/icons/svg/connect-eye.svg").c_str();
  m_showConnectLayers->setIcon(QIcon(iconConnectLayer.c_str()));

  m_mapShowGeographicGrid->setIcon(QIcon(geopx::desktop::FindInPath("share/geopixeldesktop/icons/png/map-grid.png").c_str()));
  m_mapShowGraphicScale->setIcon(QIcon(geopx::desktop::FindInPath("share/geopixeldesktop/icons/png/scale.png").c_str()));

  std::string iconGeopixelGeom = geopx::desktop::FindInPath("share/geopixeldesktop/icons/svg/geopixel_geom.svg").c_str();
  m_projectAddGeomGeopxLayer->setIcon(QIcon(iconGeopixelGeom.c_str()));

  std::string iconGeopixelTile = geopx::desktop::FindInPath("share/geopixeldesktop/icons/svg/geopixel_tile.svg").c_str();
  m_projectAddTileGeopxLayer->setIcon(QIcon(iconGeopixelTile.c_str()));
}

void geopx::desktop::GeopixelDesktop::initSlotsConnections()
{
  te::qt::af::BaseApplication::initSlotsConnections();

  connect(m_fileNewProject, SIGNAL(triggered()), SLOT(onNewProjectTriggered()));
  connect(m_fileOpenProject, SIGNAL(triggered()), SLOT(onOpenProjectTriggered()));
  connect(m_fileSaveProject, SIGNAL(triggered()), SLOT(onSaveProjectTriggered()));
  connect(m_fileSaveProjectAs, SIGNAL(triggered()), SLOT(onSaveProjectAsTriggered()));
  connect(m_fileExit, SIGNAL(triggered()), SLOT(close()));
  connect(m_fileRestartSystem, SIGNAL(triggered()), SLOT(onRestartSystemTriggered()));
  connect(m_fileSaveAsImage, SIGNAL(triggered()), SLOT(onSaveAsImageTriggered()));

  connect(m_helpContents, SIGNAL(triggered()), SLOT(onHelpTriggered()));
  connect(m_helpAbout, SIGNAL(triggered()), SLOT(showAboutDialog()));

  connect(m_layerChartsHistogram, SIGNAL(triggered()), SLOT(onLayerHistogramTriggered()));
  connect(m_layerLinkTable, SIGNAL(triggered()), SLOT(onLinkTriggered()));
  connect(m_layerChartsScatter, SIGNAL(triggered()), SLOT(onLayerScatterTriggered()));
  connect(m_layerChart, SIGNAL(triggered()), SLOT(onLayerChartTriggered()));
  connect(m_layerObjectGrouping, SIGNAL(triggered()), SLOT(onLayerGroupingTriggered()));
  connect(m_layerCompositionMode, SIGNAL(hovered()), SLOT(onLayerCompositionModeTriggered()));
  connect(m_layerAttrQuery, SIGNAL(triggered()), SLOT(onAttrQueryLayerTriggered()));
  connect(m_layerSpatialQuery, SIGNAL(triggered()), SLOT(onSpatialQueryLayerTriggered()));
  connect(m_layerDuplicateLayer, SIGNAL(triggered()), SLOT(onLayerDuplicateLayerTriggered()));

  connect(m_projectAddGeomGeopxLayer, SIGNAL(triggered()), SLOT(onProjectAddGeomGeopxLayerTriggered()));
  connect(m_projectAddTileGeopxLayer, SIGNAL(triggered()), SLOT(onProjectAddTileGeopxLayerTriggered()));

  connect(m_projectAddLayerDataset, SIGNAL(triggered()), SLOT(onAddDataSetLayerTriggered()));
  connect(m_projectAddLayerQueryDataSet, SIGNAL(triggered()), SLOT(onAddQueryLayerTriggered()));
  connect(m_projectAddLayerTabularDataSet, SIGNAL(triggered()), SLOT(onAddTabularLayerTriggered()));
  connect(m_projectAddFolderLayer, SIGNAL(triggered()), SLOT(onAddFolderLayerTriggered()));
  connect(m_projectUpdateLayerDataSource, SIGNAL(triggered()), SLOT(onUpdateLayerDataSourceTriggered()));
  connect(m_recentProjectsMenu, SIGNAL(triggered(QAction*)), SLOT(onRecentProjectsTriggered(QAction*)));

  connect(m_pluginsManager, SIGNAL(triggered()), SLOT(onPluginsManagerTriggered()));

  connect(m_toolsCustomize, SIGNAL(triggered()), SLOT(onToolsCustomizeTriggered()));
  connect(m_toolsDataExchanger, SIGNAL(triggered()), SLOT(onToolsDataExchangerTriggered()));
  connect(m_toolsDataExchangerDirect, SIGNAL(triggered()), SLOT(onToolsDataExchangerDirectTriggered()));
  connect(m_toolsDataExchangerDirectPopUp, SIGNAL(triggered()), SLOT(onToolsDataExchangerDirectPopUpTriggered()));
  connect(m_toolsQueryDataSource, SIGNAL(triggered()), SLOT(onToolsQueryDataSourceTriggered()));
  connect(m_toolsRasterMultiResolution, SIGNAL(triggered()), SLOT(onToolsRasterMultiResolutionTriggered()));
  connect(m_toolsDataSourceExplorer, SIGNAL(triggered()), SLOT(onDataSourceExplorerTriggered()));

}

void geopx::desktop::GeopixelDesktop::createDefaultSettings()
{
  QSettings sett(QSettings::IniFormat, QSettings::UserScope, qApp->organizationName(), qApp->applicationName());

  sett.beginGroup("toolbars");

  sett.beginGroup("File Tool Bar");
  sett.setValue("name", "File Tool Bar");
  sett.beginWriteArray("Actions");
  sett.setArrayIndex(0);
  sett.setValue("action", "File.New Project");
  sett.setArrayIndex(1);
  sett.setValue("action", "File.Open Project");
  sett.setArrayIndex(2);
  sett.setValue("action", "File.Save Project");
  sett.setArrayIndex(3);
  sett.setValue("action", "");
  sett.setArrayIndex(4);
  sett.setValue("action", "Project.New Folder");
  sett.setArrayIndex(5);
  sett.setValue("action", "Project.Add Layer.All Sources");
  sett.setArrayIndex(6);
  sett.setValue("action", "Project.Add Layer.GeopxVector");
  sett.setArrayIndex(7);
  sett.setValue("action", "Project.Add Layer.GeopxTile");
  sett.endArray();
  sett.endGroup();

  sett.beginGroup("View Tool Bar");
  sett.setValue("name", "View Tool Bar");
  sett.beginWriteArray("Actions");
  sett.setArrayIndex(0);
  sett.setValue("action", "View.Layer Explorer");
  sett.setArrayIndex(1);
  sett.setValue("action", "View.Map Display");
  sett.setArrayIndex(2);
  sett.setValue("action", "View.Data Table");
  sett.setArrayIndex(3);
  sett.setValue("action", "View.Style Explorer");
  sett.setArrayIndex(4);
  sett.setValue("action", "Show.Connect Layers");
  sett.setArrayIndex(5);
  sett.setValue("action", "Show.Displays");
  sett.setArrayIndex(6);
  sett.setValue("action", "");
  sett.setArrayIndex(7);
  sett.setValue("action", "Map.Show graphic scale");
  sett.endArray();
  sett.endGroup();

  sett.beginGroup("Map Tool Bar");
  sett.setValue("name", "Map Tool Bar");
  sett.beginWriteArray("Actions");
  sett.setArrayIndex(0);
  sett.setValue("action", "Map.SRID");
  sett.setArrayIndex(1);
  sett.setValue("action", "");
  sett.setArrayIndex(2);
  sett.setValue("action", "Map.Draw");
  sett.setArrayIndex(3);
  sett.setValue("action", "Map.Previous Extent");
  sett.setArrayIndex(4);
  sett.setValue("action", "Map.Next Extent");
  sett.setArrayIndex(5);
  sett.setValue("action", "Map.Zoom Extent");
  sett.setArrayIndex(6);
  sett.setValue("action", "");
  sett.setArrayIndex(7);
  sett.setValue("action", "Map.Zoom In");
  sett.setArrayIndex(8);
  sett.setValue("action", "Map.Zoom Out");
  sett.setArrayIndex(9);
  sett.setValue("action", "Map.Pan");
  sett.setArrayIndex(10);
  sett.setValue("action", "");
  sett.setArrayIndex(11);
  sett.setValue("action", "Map.Info");
  sett.setArrayIndex(12);
  sett.setValue("action", "Map.Selection");
  sett.setArrayIndex(13);
  sett.setValue("action", "Layer.Invert Selection");
  sett.endArray();
  sett.endGroup();

  sett.endGroup();

  sett.beginGroup("projects");

  sett.setValue("author_name", "");
  sett.setValue("recents_history_size", "8");

  sett.endGroup();
}

void geopx::desktop::GeopixelDesktop::addMenusActions()
{
  // File menu
  m_fileMenu->setObjectName("File");
  m_fileMenu->setTitle(tr("&File"));

  m_recentProjectsMenu->setObjectName("File.Recent Projects");
  m_recentProjectsMenu->setTitle(tr("Recent &Projects"));

  m_fileMenu->addAction(m_fileNewProject);
  m_fileMenu->addAction(m_fileOpenProject);
  m_fileMenu->addAction(m_fileSaveProject);
  m_fileMenu->addAction(m_fileSaveProjectAs);
  m_fileMenu->addSeparator();
  m_fileMenu->addMenu(m_recentProjectsMenu);
  m_recentProjectsMenu->setEnabled(false);
  m_fileMenu->addSeparator();
  m_fileMenu->addAction(m_fileSaveAsImage);
  m_fileMenu->addSeparator();
  m_fileMenu->addAction(m_fileExit);

  // View menu
  m_viewMenu->setObjectName("View");
  m_viewMenu->setTitle(tr("&View"));

  m_viewToolBarsMenu->setObjectName("View.Toolbars");
  m_viewToolBarsMenu->setTitle(tr("&Toolbars"));

  m_viewMenu->addAction(m_viewDataTable);
  m_viewMenu->addAction(m_viewLayerExplorer);
  m_viewMenu->addAction(m_viewStyleExplorer);
  m_viewMenu->addAction(m_showConnectLayers);
 //m_viewMenu->addAction(m_showDisplays);
  m_viewMenu->addSeparator();
  m_viewMenu->addAction(m_viewFullScreen);
  m_viewMenu->addSeparator();
  m_viewMenu->addMenu(m_viewToolBarsMenu);

  // Project menu
  m_projectMenu->setObjectName("Project");
  m_projectMenu->setTitle(tr("&Project"));

  m_projectAddLayerMenu->setObjectName("Project.Add Layer");
  m_projectAddLayerMenu->setTitle(tr("&Add Layer"));
  m_projectAddLayerMenu->setIcon(QIcon::fromTheme("layer-add"));
  m_projectAddLayerMenu->addAction(m_projectAddGeomGeopxLayer);
  m_projectAddLayerMenu->addAction(m_projectAddTileGeopxLayer);
  m_projectAddLayerMenu->addSeparator();
  m_projectAddLayerMenu->addAction(m_projectAddLayerTabularDataSet);
  m_projectAddLayerMenu->addAction(m_projectAddLayerDataset);
  m_projectAddLayerMenu->addSeparator();
  m_projectAddLayerMenu->addAction(m_projectAddLayerQueryDataSet);
  m_projectAddLayerMenu->addSeparator();
  m_projectMenu->addAction(m_projectAddFolderLayer);
  m_projectMenu->addSeparator();
  m_projectMenu->addAction(m_layerRemove);
  m_projectMenu->addAction(m_layerRename);

  m_layerMenu->setObjectName("Layer");
  m_layerMenu->setTitle(tr("&Layer"));

  m_layerMenu->addAction(m_layerObjectGrouping);
  m_layerMenu->addAction(m_layerChartsHistogram);
  m_layerMenu->addAction(m_layerChart);
  m_layerMenu->addAction(m_layerChartsScatter);
  m_layerMenu->addAction(m_layerAttrQuery);
  m_layerMenu->addAction(m_layerSpatialQuery);
  m_layerMenu->addSeparator();
  m_layerMenu->addAction(m_layerFitOnMapDisplay);
  m_layerMenu->addAction(m_layerFitSelectedOnMapDisplay);
  m_layerMenu->addAction(m_layerPanToSelectedOnMapDisplay);
  m_layerMenu->addSeparator();
  m_layerMenu->addAction(m_layerShowTable);
  m_layerMenu->addAction(m_viewStyleExplorer);
  m_layerMenu->addSeparator();
  m_layerMenu->addAction(m_layerRemoveObjectSelection);
  m_layerMenu->addAction(m_layerInvertObjectSelection);
  m_layerMenu->addSeparator();
  m_layerMenu->addAction(m_layerCheckSelectedLayer);
  m_layerMenu->addAction(m_layerClearSelectedLayer);
  m_layerMenu->addSeparator();
  m_layerMenu->addAction(m_layerSRS);
  m_layerMenu->addSeparator();
  m_layerMenu->addAction(m_layerProperties);


  // Map Menu
  m_mapMenu->setObjectName("Map");
  m_mapMenu->setTitle(tr("&Map"));

  m_mapMenu->addAction(m_mapDraw);
  m_mapMenu->addAction(m_mapStopDrawing);
  m_mapMenu->addSeparator();
  m_mapMenu->addAction(m_mapInfo);
  m_mapMenu->addAction(m_mapRemoveSelection);
  m_mapMenu->addAction(m_mapSelection);
  m_mapMenu->addSeparator();
  m_mapMenu->addAction(m_mapPan);
  m_mapMenu->addAction(m_mapZoomExtent);
  m_mapMenu->addAction(m_mapZoomIn);
  m_mapMenu->addAction(m_mapZoomOut);
  m_mapMenu->addSeparator();
  m_mapMenu->addAction(m_mapNextExtent);
  m_mapMenu->addAction(m_mapPreviousExtent);
  m_mapMenu->addSeparator();
  m_mapMenu->addAction(m_mapMeasureAngle);
  m_mapMenu->addAction(m_mapMeasureArea);
  m_mapMenu->addAction(m_mapMeasureDistance);
  m_mapMenu->addSeparator();
  m_mapMenu->addAction(m_mapSRID);
  m_mapMenu->addAction(m_mapUnknownSRID);
  m_mapMenu->addSeparator();
  m_mapMenu->addAction(m_mapShowGraphicScale);
  m_mapMenu->addAction(m_mapEditGraphicScale);
  m_mapMenu->addSeparator();
  //m_mapMenu->addAction(m_mapShowGeographicGrid);
  //m_mapMenu->addAction(m_mapEditGrid);
  //m_mapMenu->addSeparator();
  m_mapMenu->addAction(m_mapPanToCoordinateTool);

  // Tools menu
  m_toolsMenu->setObjectName("Tools");
  m_toolsMenu->setTitle(tr("&Tools"));

  m_toolsExchangerMenu->setObjectName("Tools.Exchanger");
  m_toolsExchangerMenu->setTitle(tr("&Data Exchanger"));
  m_toolsExchangerMenu->setIcon(QIcon::fromTheme("datasource-exchanger"));
  m_toolsExchangerMenu->addAction(m_toolsDataExchangerDirect);
  m_toolsExchangerMenu->addAction(m_toolsDataExchanger);

  m_toolsMenu->addAction(m_toolsDataSourceExplorer);
  m_toolsMenu->addAction(m_toolsQueryDataSource);
  m_toolsMenu->addSeparator();
  m_toolsMenu->addAction(m_toolsRasterMultiResolution);
  m_toolsMenu->addSeparator();
  m_toolsMenu->addAction(m_toolsCustomize);
  m_toolsMenu->addSeparator();

  // Plugins menu
  m_pluginsMenu->setObjectName("Plugins");
  m_pluginsMenu->setTitle(tr("Pl&ugins"));

  m_pluginsMenu->addSeparator()->setObjectName("ManagePluginsSeparator");
  m_pluginsMenu->addAction(m_pluginsManager);

  // Help menu
  m_helpMenu->setObjectName("Help");
  m_helpMenu->setTitle(tr("&Help"));

  m_helpMenu->addAction(m_helpContents);
  m_helpMenu->addAction(m_helpAbout);
}

void geopx::desktop::GeopixelDesktop::addPopUpMenu()
{
  te::qt::widgets::LayerItemView* treeView = m_layerExplorer->getExplorer();
  treeView->setAnimated(true);

  //// Actions to be added to the context menu when there is no item selected
  treeView->addNoLayerAction(m_projectAddLayerMenu->menuAction());

  QAction* noItemSelectedSep = new QAction(this);
  noItemSelectedSep->setSeparator(true);
  treeView->addNoLayerAction(noItemSelectedSep);

  treeView->addNoLayerAction(m_projectAddFolderLayer);

  //// Actions to be added to the context menu when there is a unique item selected

  treeView->addAllLayerAction(m_layerRemove);

  //// Actions for the folder layer item
  treeView->addFolderLayerAction(m_layerRename);
  treeView->addFolderLayerAction(m_projectAddLayerMenu->menuAction());

  QAction* folderSep1 = new QAction(this);
  folderSep1->setSeparator(true);
  treeView->addFolderLayerAction(folderSep1);

  treeView->addFolderLayerAction(m_projectAddFolderLayer);

  QAction* folderSep2 = new QAction(this);
  folderSep2->setSeparator(true);
  treeView->addFolderLayerAction(folderSep2);

  treeView->addFolderLayerAction(m_layerDuplicateLayer);

  //// Actions for the single layer item that is not a raster layer
  treeView->addVectorLayerAction(m_layerRemoveObjectSelection);
  treeView->addVectorLayerAction(m_layerInvertObjectSelection);
  treeView->addVectorLayerAction(m_layerRename);
  treeView->addVectorLayerAction(m_layerDuplicateLayer);

  QAction* actionStyleSep1 = new QAction(this);
  actionStyleSep1->setSeparator(true);
  treeView->addVectorLayerAction(actionStyleSep1);

  treeView->addVectorLayerAction(m_layerObjectGrouping);
  treeView->addVectorLayerAction(m_toolsDataExchangerDirectPopUp);
  treeView->addVectorLayerAction(m_layerChartsHistogram);
  treeView->addVectorLayerAction(m_layerChart);
  treeView->addVectorLayerAction(m_layerAttrQuery);
  treeView->addVectorLayerAction(m_layerSpatialQuery);
  treeView->addVectorLayerAction(m_layerChartsScatter);
  treeView->addVectorLayerAction(m_layerLinkTable);

  QAction* actionChartSep = new QAction(this);
  actionChartSep->setSeparator(true);
  treeView->addVectorLayerAction(actionChartSep);

  treeView->addVectorLayerAction(m_layerShowTable);
  treeView->addVectorLayerAction(m_viewStyleExplorer);

  QAction* actionStyleSep = new QAction(this);
  actionStyleSep->setSeparator(true);
  treeView->addVectorLayerAction(actionStyleSep);

  QAction* actionRemoveSep = new QAction(this);
  actionRemoveSep->setSeparator(true);
  treeView->addVectorLayerAction(actionRemoveSep);

  treeView->addVectorLayerAction(m_layerFitOnMapDisplay);
  treeView->addVectorLayerAction(m_layerFitSelectedOnMapDisplay);
  treeView->addVectorLayerAction(m_layerPanToSelectedOnMapDisplay);

  QAction* actionFitSep = new QAction(this);
  actionFitSep->setSeparator(true);
  treeView->addVectorLayerAction(actionFitSep);

  treeView->addVectorLayerAction(m_layerSaveSelectedObjects);

  QAction* actionSaveAsSep = new QAction(this);
  actionSaveAsSep->setSeparator(true);
  treeView->addVectorLayerAction(actionSaveAsSep);

  treeView->addVectorLayerAction(m_layerSRS);
  treeView->addVectorLayerAction(m_layerCharEncoding);

  QAction* actionSRSSep = new QAction(this);
  actionSRSSep->setSeparator(true);
  treeView->addVectorLayerAction(actionSRSSep);

  treeView->addVectorLayerAction(m_layerCompositionMode);
  treeView->addVectorLayerAction(m_layerProperties);

  //// Actions for the raster layer item
  treeView->addRasterLayerAction(m_layerRename);

  actionStyleSep1 = new QAction(this);
  actionStyleSep1->setSeparator(true);
  treeView->addVectorLayerAction(actionStyleSep1);

  treeView->addRasterLayerAction(m_layerObjectGrouping);
  treeView->addRasterLayerAction(m_layerChartsHistogram);
  treeView->addRasterLayerAction(m_layerChartsScatter);

  QAction* rasterSep1 = new QAction(this);
  rasterSep1->setSeparator(true);
  treeView->addRasterLayerAction(rasterSep1);

  treeView->addRasterLayerAction(m_viewStyleExplorer);

  QAction* rasterSep2 = new QAction(this);
  rasterSep2->setSeparator(true);
  treeView->addRasterLayerAction(rasterSep2);

  QAction* rasterSep3 = new QAction(this);
  rasterSep3->setSeparator(true);
  treeView->addRasterLayerAction(rasterSep3);

  treeView->addRasterLayerAction(m_layerFitOnMapDisplay);

  QAction* rasterSep4 = new QAction(this);
  rasterSep4->setSeparator(true);
  treeView->addRasterLayerAction(rasterSep4);
  
  treeView->addRasterLayerAction(m_layerSRS);

  QAction* rasterSep5 = new QAction(this);
  rasterSep5->setSeparator(true);
  treeView->addRasterLayerAction(rasterSep5);

  treeView->addRasterLayerAction(m_layerCompositionMode);
  treeView->addRasterLayerAction(m_layerProperties);

  //// Actions for tabular layers
  treeView->addTabularLayerAction(m_layerRename);

  actionStyleSep1 = new QAction(this);
  actionStyleSep1->setSeparator(true);
  treeView->addTabularLayerAction(actionStyleSep1);

  treeView->addTabularLayerAction(m_toolsDataExchangerDirectPopUp);
  treeView->addTabularLayerAction(m_layerChartsHistogram);
  treeView->addTabularLayerAction(m_layerChartsScatter);
  treeView->addTabularLayerAction(m_layerLinkTable);

  actionChartSep = new QAction(this);
  actionChartSep->setSeparator(true);
  treeView->addTabularLayerAction(actionChartSep);

  treeView->addTabularLayerAction(m_layerShowTable);

  actionSaveAsSep = new QAction(this);
  actionSaveAsSep->setSeparator(true);
  treeView->addTabularLayerAction(actionSaveAsSep);

  treeView->addTabularLayerAction(m_layerCharEncoding);

  actionSRSSep = new QAction(this);
  actionSRSSep->setSeparator(true);
  treeView->addTabularLayerAction(actionSRSSep);
 
  treeView->addTabularLayerAction(m_layerProperties);

  //// Actions for invalid layers
  treeView->addInvalidLayerAction(m_projectUpdateLayerDataSource);

  //// Action for multi selected layers
  treeView->addMultipleSelectionAction(m_layerSRS);
  treeView->addMultipleSelectionAction(m_layerCheckSelectedLayer);
  treeView->addMultipleSelectionAction(m_layerClearSelectedLayer);
}

void geopx::desktop::GeopixelDesktop::addGeopxMenuActions()
{
  m_photoIndexAction = new geopx::tools::PhotoIndexAction(m_toolsMenu);
  m_tileGeneratorAction = new geopx::tools::TileGeneratorAction(m_toolsMenu);

  QMenu* forestMenu = new QMenu(m_toolsMenu);
  forestMenu->setTitle(tr("&Forest Monitor..."));

  m_toolsMenu->addMenu(forestMenu);

  m_forestMonitorAction = new geopx::tools::ForestMonitorAction(forestMenu);
  m_forestMonitorClassAction = new geopx::tools::ForestMonitorClassAction(forestMenu);
  m_forestMonitorToolBarAction = new geopx::tools::ForestMonitorToolBarAction(forestMenu);
  m_ndviAction = new geopx::tools::NDVIAction(forestMenu);

  ////register actions into application tool bar
  //QToolBar* toolBar = te::qt::af::AppCtrlSingleton::getInstance().getToolBar("Map Tool Bar");

  //if (toolBar)
  //{
  //  toolBar->addSeparator();
  //  toolBar->addWidget(m_scaleCmbBox);
  //}

  //m_scaleCmbBox->setFixedHeight(26);

  //application connector
  m_appConnector = new geopx::desktop::ApplicationConnector(m_layerExplorer->getExplorer(), m_styleExplorer->getExplorer());
 // m_app->addListener(m_appConnector);

  connect(m_appConnector, SIGNAL(drawLayers()), this, SLOT(onDrawTriggered()));

  //adjust status bar
//  m_statusbar->removeWidget(m_internalSettingsToolButton);
 // m_statusbar->removeWidget(m_mapUnknownSRIDToolButton);
//  m_statusbar->removeWidget(m_mapSRIDToolButton);
  //m_statusbar->removeWidget(m_stopDrawToolButton);
  //m_statusbar->removeWidget(m_scaleCmbBox);

  //register TiledLayer serializer
  te::map::serialize::Layer::getInstance().reg("TILEDLAYER", std::make_pair(te::map::serialize::Layer::LayerReadFnctType(&geopx::desktop::layer::serialize::LayerReader),
                                                                            te::map::serialize::Layer::LayerWriteFnctType(&geopx::desktop::layer::serialize::LayerWriter)));

  te::map::serialize::Layer::getInstance().reg("GEOPXDATASETLAYER", std::make_pair(te::map::serialize::Layer::LayerReadFnctType(&geopx::desktop::layer::serialize::GeopxDataSetLayerReader),
                                                                            te::map::serialize::Layer::LayerWriteFnctType(&geopx::desktop::layer::serialize::GeopxDataSetLayerWriter)));

  //GeopxDataSetLayer Item delegate
  te::qt::widgets::LayerItemView* view = getLayerExplorer();

  m_geopxDelegate = new geopx::desktop::layer::GeopxItemDelegate((QStyledItemDelegate*)view->itemDelegate(), this);

  view->setItemDelegate(m_geopxDelegate);
  

  //TiledLayer Item delegate
  m_tiledDelegate = new geopx::desktop::layer::TiledItemDelegate((QStyledItemDelegate*)view->itemDelegate(), this);
  
  view->setItemDelegate(m_tiledDelegate);

  //TiledLayer cache
  QString userDataDir = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
  userDataDir.append("/tiledCache");
  
  if (te::core::FileSystem::exists(userDataDir.toUtf8().data()))
  {
    QDir dir(userDataDir);

    dir.setNameFilters(QStringList() << "*.*");
    dir.setFilter(QDir::Files);

    foreach(QString dirFile, dir.entryList())
    {
      dir.remove(dirFile);
    }
  }
  else
  {
    te::core::FileSystem::createDirectories(userDataDir.toUtf8().data());
  }
}

void geopx::desktop::GeopixelDesktop::removeGeopxMenuActions()
{
  delete m_photoIndexAction;
  delete m_tileGeneratorAction;

  delete m_forestMonitorAction;
  delete m_forestMonitorClassAction;
  delete m_forestMonitorToolBarAction;
  delete m_ndviAction;

  //remove TiledLayer Item delegate
  te::qt::widgets::LayerItemView* view = getLayerExplorer();
  view->removeDelegate(m_geopxDelegate);
  view->removeDelegate(m_tiledDelegate);

  //geopixel datasource
  te::qt::widgets::DataSourceTypeManager::getInstance().remove("GEOPIXEL");
}

void geopx::desktop::GeopixelDesktop::initMenus()
{
  te::qt::af::BaseApplication::initMenus();

  m_fileMenu = new QMenu(m_menubar);
  m_recentProjectsMenu = new QMenu(m_fileMenu);
  m_menubar->addAction(m_fileMenu->menuAction());
  m_viewMenu = new QMenu(m_menubar);
  m_menubar->addAction(m_viewMenu->menuAction());
  m_viewToolBarsMenu = new QMenu(m_viewMenu);
  m_viewMenu->addMenu(m_viewToolBarsMenu);
  m_projectMenu = new QMenu(m_menubar);
  m_projectAddLayerMenu = new QMenu(m_projectMenu);
  m_menubar->addAction(m_projectMenu->menuAction());
  m_projectMenu->addMenu(m_projectAddLayerMenu);
  m_layerMenu = new QMenu(m_menubar);
  m_menubar->addAction(m_layerMenu->menuAction());
  m_mapMenu = new QMenu(m_menubar);
  m_mapMenu->setObjectName("Map");
  m_menubar->addAction(m_mapMenu->menuAction());
  m_toolsMenu = new QMenu(m_menubar);
  m_toolsExchangerMenu = new QMenu(m_toolsMenu);
  m_toolsMenu->addAction(m_toolsExchangerMenu->menuAction());
  m_menubar->addAction(m_toolsMenu->menuAction());
  m_pluginsMenu = new QMenu(m_menubar);
  m_pluginsMenu->setObjectName("Plugins");
  m_menubar->addMenu(m_pluginsMenu);
  m_helpMenu = new QMenu(m_menubar);
  m_helpMenu->setObjectName("Help");
  m_menubar->addAction(m_helpMenu->menuAction());
}

void geopx::desktop::GeopixelDesktop::initToolbars()
{
  te::qt::af::BaseApplication::initToolbars();

  std::vector<QToolBar*> toolBars = m_app->getToolBars();


  for (std::size_t t = 0; t < toolBars.size(); ++t)
  {
    QToolBar* bar = toolBars[t];

    m_viewToolBarsMenu->addAction(bar->toggleViewAction());
  }
}

void geopx::desktop::GeopixelDesktop::showAboutDialog()
{
  AboutDialog dialog(this);

  dialog.exec();
}

void geopx::desktop::GeopixelDesktop::onLayerShowTableTriggered()
{
  std::list<te::map::AbstractLayerPtr> layers = te::qt::widgets::GetSelectedLayersOnly(getLayerExplorer());

  if (layers.empty())
  {
    QMessageBox::warning(this, m_app->getAppTitle(), tr("There's no selected layer."));
    return;
  }
  else
  {
    for (std::list<te::map::AbstractLayerPtr>::iterator it = layers.begin(); it != layers.end(); ++it)
    {
      if (!(*it)->isValid())
      {
        QMessageBox::warning(this, m_app->getAppTitle(), tr("There are invalid layers selected!"));
        return;
      }
    }
  }


  te::map::AbstractLayerPtr lay = *layers.begin();

  if (lay->getSchema()->hasRaster())
    return;

  bool editable = true;

  geopx::desktop::layer::GeopxDataSetLayer* geopxLayer = dynamic_cast<geopx::desktop::layer::GeopxDataSetLayer*>(lay.get());

  if (geopxLayer)
  {
    te::da::DataSourcePtr ds = te::da::GetDataSource(geopxLayer->getDataSourceId());

    editable = geopx::desktop::datasource::IsLayerEditionEnabled(ds.get(), geopxLayer->getThemeId());
  }

  te::qt::af::DataSetTableDockWidget* doc = GetLayerDock(lay.get(), m_tables);

  if (doc)
  {
    onLayerTableClose(doc);
    delete doc;
  }

  doc = new te::qt::af::DataSetTableDockWidget(this);
  doc->setLayer(lay.get(), editable);
  doc->setHighlightColor(m_app->getSelectionColor());
  addDockWidget(Qt::BottomDockWidgetArea, doc);

  connect(doc, SIGNAL(closed(te::qt::af::DataSetTableDockWidget*)), SLOT(onLayerTableClose(te::qt::af::DataSetTableDockWidget*)));
  connect(doc, SIGNAL(createChartDisplay(te::qt::widgets::ChartDisplayWidget*, te::map::AbstractLayer*)), SLOT(onChartDisplayCreated(te::qt::widgets::ChartDisplayWidget*, te::map::AbstractLayer*)));

  if (!m_tables.empty())
    tabifyDockWidget(m_tables[m_tables.size() - 1], doc);

  m_tables.push_back(doc);

  m_app->addListener(doc);

  doc->show();
  doc->raise();

  m_viewDataTable->setChecked(true);

  m_viewDataTable->setEnabled(true);
}

void geopx::desktop::GeopixelDesktop::onApplicationTriggered(te::qt::af::evt::Event* e)
{
  switch (e->m_id)
  {
    case te::qt::af::evt::NEW_ACTIONS_AVAILABLE:
    {
      te::qt::af::evt::NewActionsAvailable* evt = static_cast<te::qt::af::evt::NewActionsAvailable*>(e);

      if(!evt->m_actions.empty())
        addActions(evt->m_plgName.c_str(), evt->m_category.c_str(), evt->m_actions);
      else if(evt->m_toolbar != nullptr)
        QMainWindow::addToolBar(Qt::TopToolBarArea, evt->m_toolbar);
    }
      break;

    case te::qt::af::evt::LAYER_REMOVED:
    {
      std::list<te::map::AbstractLayerPtr> layers;
      te::qt::widgets::GetValidLayers(getLayerExplorer()->model(), QModelIndex(), layers);

      getMapDisplay()->setLayerList(layers);
      getMapDisplay()->refresh();

      projectChanged();
    }
      break;

    case te::qt::af::evt::LAYER_ADDED:
    case te::qt::af::evt::LAYER_CHANGED:
    case te::qt::af::evt::LAYER_VISIBILITY_CHANGED:
      projectChanged();
      break;

    default:
      BaseApplication::onApplicationTriggered(e);
  }
}


void geopx::desktop::GeopixelDesktop::onRestartSystemTriggered()
{
  QMessageBox msgBox(this);

  msgBox.setText(tr("The system will be restarted."));
  msgBox.setInformativeText(tr("Do you want to continue?"));
  msgBox.setWindowTitle(tr("Restart system"));

  msgBox.addButton(QMessageBox::No);
  msgBox.addButton(QMessageBox::Yes);

  msgBox.setDefaultButton(QMessageBox::Yes);

  if (msgBox.exec() == QMessageBox::Yes)
  {
    te::qt::af::SaveDataSourcesFile(m_app);
    qApp->exit(1000);
  }
}

void geopx::desktop::GeopixelDesktop::onNewProjectTriggered()
{
  bool status = checkAndSaveProject();

  if (!status)
    return;

  resetComponents();

  m_project->m_title = tr("Default Project");

  setWindowTitle(m_app->getAppName() + " - " + m_project->m_title);
}

void geopx::desktop::GeopixelDesktop::onOpenProjectTriggered()
{
  bool status = checkAndSaveProject();

  if (!status)
    return;

  resetComponents();

  QString file = QFileDialog::getOpenFileName(this, tr("Open project file"), qApp->applicationDirPath(), m_tvController->getExtensionFilter());

  if (file.isEmpty())
    return;

  try
  {
    openProject(file);
  }
  catch (const te::common::Exception& e)
  {
    QString msg = tr("Fail to open project.");
    msg += " ";
    msg += e.what();
    QMessageBox::warning(this, m_app->getAppTitle(), msg);
  }
}

void geopx::desktop::GeopixelDesktop::onSaveProjectTriggered(bool save_as)
{
  QString projFile = m_project->m_fileName;

  if (projFile.isEmpty() || save_as)
  {
    QFileDialog d(this);

    QString filter = tr("Geopixel Desktop project(*.") + m_tvController->getAppProjectExtension() + ")";

    QString fileName = d.getSaveFileName(this, tr("Save project"), qApp->applicationDirPath(), filter);

    if(fileName.isEmpty())
      return;

    QFileInfo info(fileName);

    if(info.suffix().isEmpty())
      fileName.append("." + m_tvController->getAppProjectExtension());

    m_project->m_fileName = fileName;
    m_project->m_title = info.baseName();
  }

  std::list<te::map::AbstractLayerPtr> lays = getLayerExplorer()->getAllLayers();

  m_project->m_changed = false;

  setWindowTitle(m_app->getAppName() + " - " + m_project->m_title);

  AddRecentProjectToSettings(m_project->m_title, m_project->m_fileName);

  m_tvController->updateRecentProjects(m_project->m_fileName, m_project->m_title);

  geopx::desktop::XMLFormatter::format(m_project, lays, true);
  te::qt::af::XMLFormatter::formatDataSourceInfos(true);

  SaveProject(*m_project, lays);

  te::qt::af::SaveDataSourcesFile(m_app);

  geopx::desktop::XMLFormatter::format(m_project, lays, false);
  te::qt::af::XMLFormatter::formatDataSourceInfos(false);
}

void geopx::desktop::GeopixelDesktop::onSaveProjectAsTriggered()
{
  onSaveProjectTriggered(true);
}

void geopx::desktop::GeopixelDesktop::onSaveAsImageTriggered()
{
  QFileDialog d(this);

  QString filter = tr("BMP Raster File (*.bmp *.BMP);;JPEG/JPG Raster File (*.jpeg *.JPEG *.jpg *.JPG);;PNG Raster File (*.png *.PNG)");
  QString selectedFilter;

  QString fileName = d.getSaveFileName(this, tr("Save image"), te::qt::widgets::GetFilePathFromSettings("raster"),
    filter, &selectedFilter);

  if (fileName.isEmpty())
    return;

  te::qt::widgets::MapDisplay* mapd = getMapDisplay();
  QPixmap* pix = mapd->getDisplayPixmap();
  if (pix->save(fileName))
  {
    if (selectedFilter == "JPEG/JPG Raster File (*.jpeg *.JPEG *.jpg *.JPG)")
    {
      QString fileNameJpw(fileName);
      fileNameJpw.replace(".jpg", ".jgw");
      fileNameJpw.replace(".jpeg", ".jgw");

      double rx, ry;
      std::ofstream ofsjpw;
      ofsjpw.open(fileNameJpw.toStdString(), std::ofstream::out );
      rx = mapd->getExtent().getWidth() / mapd->getWidth();
      ry = mapd->getExtent().getHeight() / mapd->getHeight();

      ofsjpw << rx << std::endl << "0.0" << std::endl << "0.0" << std::endl << -ry << std::endl;
      ofsjpw << mapd->getExtent().getLowerLeftX() << std::endl << mapd->getExtent().getUpperRightY();

      ofsjpw.close();

      int reply = QMessageBox::question(this, "Geopixel Desktop", tr("Would you like to add the layer to the project?"), QMessageBox::No, QMessageBox::Yes);

      if (reply == QMessageBox::Yes)
      {
        std::map<std::string, std::string> outdsinfo;
        outdsinfo["URI"] = fileName.toStdString();
        te::map::AbstractLayerPtr outputLayer = te::qt::widgets::createLayer("GDAL", outdsinfo);
        std::list<te::map::AbstractLayerPtr> layers;
        outputLayer->setSRID(mapd->getSRID());
        outputLayer->setExtent(mapd->getExtent());
        layers.push_back(outputLayer);

        if (layers.empty())
          return;

        QModelIndex pF = GetParent(getLayerExplorer());

        getLayerExplorer()->addLayers(layers, pF);
      }
    }
  }
}

void geopx::desktop::GeopixelDesktop::onHelpTriggered()
{
  te::qt::widgets::HelpManager::getInstance().showHelp("geopixeldesktop/frontpage.html", "geopixel.geopixeldesktop");
}


void geopx::desktop::GeopixelDesktop::onLinkTriggered()
{
  try
  {
    // Get the parent layer where the dataset layer(s) will be added.
    QModelIndex par = GetParent(getLayerExplorer());

    std::list<te::map::AbstractLayerPtr> selectedLayers = te::qt::widgets::GetSelectedLayersOnly(getLayerExplorer());

    if(selectedLayers.empty())
    {
      QMessageBox::warning(this, m_app->getAppTitle(), tr("Select a layer in the layer explorer!"));
      return;
    }
    else
    {
      for(std::list<te::map::AbstractLayerPtr>::iterator it = selectedLayers.begin(); it != selectedLayers.end(); ++it)
      {
        if(!it->get()->isValid())
        {
          QMessageBox::warning(this, m_app->getAppTitle(), tr("There are invalid layers selected!"));
          return;
        }
      }
    }

    te::map::AbstractLayerPtr selectedLayer = *(selectedLayers.begin());

    std::unique_ptr<te::qt::widgets::TableLinkDialog> elb(new te::qt::widgets::TableLinkDialog(this));
    elb->setInputLayer(selectedLayer);

    int retval = elb->exec();

    if(retval == QDialog::Rejected)
      return;

    te::map::AbstractLayerPtr layer = elb->getQueryLayer();
    std::list<te::map::AbstractLayerPtr> layers;

    layers.push_back(layer);

    getLayerExplorer()->addLayers(layers, par);

    projectChanged();
  }
  catch(const std::exception& e)
  {
    QMessageBox::warning(this, m_app->getAppTitle(), e.what());
  }
  catch(...)
  {
    QMessageBox::warning(this, m_app->getAppTitle(), tr("Unknown error while trying to add a layer from a queried dataset!"));
  }
}

void geopx::desktop::GeopixelDesktop::onLayerHistogramTriggered()
{
  try
  {
    std::list<te::map::AbstractLayerPtr> selectedLayers = te::qt::widgets::GetSelectedLayersOnly(getLayerExplorer());

    if(selectedLayers.empty())
    {
      QMessageBox::warning(this, m_app->getAppTitle(),
                           tr("Select a layer in the layer explorer!"));
      return;
    }
    else
    {
      for(std::list<te::map::AbstractLayerPtr>::iterator it = selectedLayers.begin(); it != selectedLayers.end(); ++it)
      {
        if(!it->get()->isValid())
        {
          QMessageBox::warning(this, m_app->getAppTitle(),
                               tr("There are invalid layers selected!"));
          return;
        }
      }
    }

    // The histogram will be created based on the first selected layer
    te::map::AbstractLayerPtr selectedLayer = *(selectedLayers.begin());

    te::qt::widgets::HistogramDialog dlg(selectedLayer, this);

    dlg.setWindowTitle(dlg.windowTitle() + " (" + tr("Layer") + ":" + selectedLayer->getTitle().c_str() + ")");

    int res = dlg.exec();
    if(res == QDialog::Accepted)
    {
      te::qt::af::ChartDisplayDockWidget* doc = new te::qt::af::ChartDisplayDockWidget(dlg.getDisplayWidget(), this);
      doc->setSelectionColor(m_app->getSelectionColor());
      doc->setWindowTitle(tr("Histogram"));
      doc->setWindowIcon(QIcon::fromTheme("chart-bar"));
      doc->setLayer(selectedLayer.get());
      doc->setAppController(m_app);

      m_app->addListener(doc);
      addDockWidget(Qt::RightDockWidgetArea, doc, Qt::Horizontal);
      doc->show();
    }
  }
  catch(const std::exception& e)
  {
    QMessageBox::warning(this, m_app->getAppTitle(), e.what());
  }
}

void geopx::desktop::GeopixelDesktop::onLayerScatterTriggered()
{
  try
  {
    std::list<te::map::AbstractLayerPtr> selectedLayers = te::qt::widgets::GetSelectedLayersOnly(getLayerExplorer());

    if(selectedLayers.empty())
    {
      QMessageBox::warning(this, m_app->getAppTitle(), tr("Select a layer in the layer explorer!"));
      return;
    }
    else
    {
      for(std::list<te::map::AbstractLayerPtr>::iterator it = selectedLayers.begin(); it != selectedLayers.end(); ++it)
      {
        if(!it->get()->isValid())
        {
          QMessageBox::warning(this, m_app->getAppTitle(), tr("There are invalid layers selected!"));
          return;
        }
      }
    }

    // The scatter will be created based on the first selected layer
    te::map::AbstractLayerPtr selectedLayer = *(selectedLayers.begin());

    const te::map::LayerSchema* schema = selectedLayer->getSchema().release();

    te::da::DataSet* dataset = selectedLayer->getData().release();
    te::da::DataSetType* dataType = (te::da::DataSetType*) schema;

    te::qt::widgets::ScatterDialog dlg(dataset, dataType, this);

    dlg.setWindowTitle(dlg.windowTitle() + " (" + tr("Layer") + ":" + selectedLayer->getTitle().c_str() + ")");

    int res = dlg.exec();
    if(res == QDialog::Accepted)
    {
      te::qt::af::ChartDisplayDockWidget* doc = new te::qt::af::ChartDisplayDockWidget(dlg.getDisplayWidget(), this);

      doc->setSelectionColor(m_app->getSelectionColor());
      doc->setWindowTitle(tr("Scatter"));
      doc->setWindowIcon(QIcon::fromTheme("chart-scatter"));
      m_app->addListener(doc);
      doc->setLayer(selectedLayer.get());
      doc->setAppController(m_app);

      addDockWidget(Qt::RightDockWidgetArea, doc, Qt::Horizontal);
      doc->show();
    }
  }
  catch(const std::exception& e)
  {
    QMessageBox::warning(this, m_app->getAppTitle(), e.what());
  }
}

void geopx::desktop::GeopixelDesktop::onLayerChartTriggered()
{
  try
  {
    std::list<te::map::AbstractLayerPtr> selectedLayers = te::qt::widgets::GetSelectedLayersOnly(getLayerExplorer());

    if(selectedLayers.empty())
    {
      QMessageBox::warning(this, m_app->getAppTitle(),
                           tr("Select a single layer in the layer explorer!"));
      return;
    }
    else
    {
      for(std::list<te::map::AbstractLayerPtr>::iterator it = selectedLayers.begin(); it != selectedLayers.end(); ++it)
      {
        if(!(*it)->isValid())
        {
          QMessageBox::warning(this, m_app->getAppTitle(),
                               tr("There are invalid layers selected!"));
          return;
        }
      }
    }

    // The chart will be accomplished only on the first single layer selected
    te::map::AbstractLayerPtr selectedLayer = *selectedLayers.begin();

    te::qt::widgets::ChartLayerDialog dlg(this);

    dlg.setWindowTitle(dlg.windowTitle() + " (" + tr("Layer") + ":" + selectedLayer->getTitle().c_str() + ")");

    dlg.setLayer(selectedLayer);

    // If the selected layer has a chart associated to it, set the chart layer
    // dialog for initializing with this chart.
    te::map::Chart* chart = selectedLayer->getChart();

    if(chart)
      dlg.setChart(chart);

    if(dlg.exec() == QDialog::Accepted)
    {
      getLayerExplorer()->updateChart(*getLayerExplorer()->selectionModel()->selectedIndexes().begin());

      m_display->getDisplay()->refresh();

      projectChanged();
    }
  }
  catch(const std::exception& e)
  {
    QMessageBox::warning(this, m_app->getAppTitle(), e.what());
  }
}

void geopx::desktop::GeopixelDesktop::onLayerDuplicateLayerTriggered()
{
  try
  {
    std::list<te::map::AbstractLayerPtr> selectedLayers = te::qt::widgets::GetSelectedLayersOnly(getLayerExplorer(), true);

    if (selectedLayers.empty())
    {
      QMessageBox::warning(this, m_app->getAppTitle(), tr("Select a single layer in the layer explorer!"));
      return;
    }
    else
    {
      for (std::list<te::map::AbstractLayerPtr>::iterator it = selectedLayers.begin(); it != selectedLayers.end(); ++it)
      {
        te::map::AbstractLayer* lptr = it->get();

        if (!lptr->isValid())
        {
          QMessageBox::warning(this, m_app->getAppTitle(), tr("There are invalid layers selected!"));

          return;
        }
      }
    }

    te::map::AbstractLayerPtr selectedLayer = *selectedLayers.begin();

    te::map::AbstractLayerPtr newLayer = selectedLayer->clone();

    QModelIndex res;
    QModelIndexList idxs = getLayerExplorer()->selectionModel()->selectedIndexes();

    if (idxs.size() == 1)
    {
      res = idxs.at(0);
      te::qt::widgets::TreeItem* item = static_cast<te::qt::widgets::TreeItem*>(res.internalPointer());

      if (item->getType() == "FOLDER")
        res = res.parent();
    }

    std::list<te::map::AbstractLayerPtr> layers;
    layers.push_back(newLayer);

    getLayerExplorer()->addLayers(layers, res);

    projectChanged();
  }
  catch (const std::exception& e)
  {
    QMessageBox::warning(this, m_app->getAppTitle(), e.what());
  }
}

void geopx::desktop::GeopixelDesktop::onLayerGroupingTriggered()
{
  try
  {
    std::list<te::map::AbstractLayerPtr> selectedLayers = te::qt::widgets::GetSelectedLayersOnly(getLayerExplorer());

    if(selectedLayers.empty())
    {
      QMessageBox::warning(this, m_app->getAppTitle(), tr("Select a single layer in the layer explorer!"));
      return;
    }
    else
    {
      for(std::list<te::map::AbstractLayerPtr>::iterator it = selectedLayers.begin(); it != selectedLayers.end(); ++it)
      {
        if(!(*it)->isValid())
        {
          QMessageBox::warning(this, m_app->getAppTitle(), tr("There are invalid layers selected!"));

          return;
        }
      }
    }

    // The object grouping will be accomplished only on the first layer selected
    te::map::AbstractLayerPtr selectedLayer = *selectedLayers.begin();

    // Get all layer with grouping to dispose to import
    std::vector<te::map::AbstractLayerPtr> allLayers;
    te::qt::widgets::GetValidLayers(getLayerExplorer()->model(), QModelIndex(), allLayers);

    te::qt::widgets::GroupingDialog dlg(this);
    dlg.setLayers(selectedLayer, allLayers);


    if(dlg.exec() == QDialog::Accepted)
    {
      getLayerExplorer()->updateLegend(selectedLayer.get());

      projectChanged();

      te::qt::af::evt::LayerChanged e2(selectedLayer.get());
      emit triggered(&e2);

      m_display->getDisplay()->refresh();
    }
  }
  catch(const std::exception& e)
  {
    QMessageBox::warning(this, m_app->getAppTitle(), e.what());
  }
}

void geopx::desktop::GeopixelDesktop::onLayerCompositionModeTriggered()
{
  std::list<te::map::AbstractLayerPtr> selectedLayers = te::qt::widgets::GetSelectedLayersOnly(getLayerExplorer());

  if(!selectedLayers.empty())
  {
    std::list<te::map::AbstractLayerPtr>::iterator it = selectedLayers.begin();

    m_compModeMenu->setLayer(*it);
  }
}

void geopx::desktop::GeopixelDesktop::onSpatialQueryLayerTriggered()
{
  if(!m_queryDlg)
  {
    m_queryDlg = new te::qt::widgets::QueryDialog(this);

    connect(m_queryDlg, SIGNAL(layerSelectedObjectsChanged(const te::map::AbstractLayerPtr&)),
            SLOT(onLayerSelectedObjectsChanged(const te::map::AbstractLayerPtr&)));

    connect(m_queryDlg, SIGNAL(createLayer(te::map::AbstractLayerPtr)), SLOT(onQueryLayerCreateLayer(te::map::AbstractLayerPtr)));

    if(m_iController)
      m_iController->addInterface(m_queryDlg);
  }

  std::list<te::map::AbstractLayerPtr> allLayersList;
  te::qt::widgets::GetValidLayers(getLayerExplorer()->model(), QModelIndex(), allLayersList);

  m_queryDlg->setLayerList(allLayersList);

  std::list<te::map::AbstractLayerPtr> selectedLayers = GetSelectedLayersOnly(getLayerExplorer());

  if (!selectedLayers.empty())
  {
    te::map::AbstractLayerPtr layer = *selectedLayers.begin();

    if (layer->isValid())
    {
      m_queryDlg->setCurrentLayer(layer);
    }
  }

  m_queryDlg->show();
}

void geopx::desktop::GeopixelDesktop::onAttrQueryLayerTriggered()
{
  te::qt::widgets::QueryBuilderDialog dlg(this);

  connect(&dlg, SIGNAL(layerSelectedObjectsChanged(const te::map::AbstractLayerPtr&)),
    SLOT(onLayerSelectedObjectsChanged(const te::map::AbstractLayerPtr&)));

  connect(&dlg, SIGNAL(createLayer(te::map::AbstractLayerPtr)), SLOT(onQueryLayerCreateLayer(te::map::AbstractLayerPtr)));

  std::list<te::map::AbstractLayerPtr> allLayersList;
  te::qt::widgets::GetValidLayers(getLayerExplorer()->model(), QModelIndex(), allLayersList);

  dlg.setLayerList(allLayersList);

  std::list<te::map::AbstractLayerPtr> selectedLayers = te::qt::widgets::GetSelectedLayersOnly(getLayerExplorer());

  if (!selectedLayers.empty())
  {
    te::map::AbstractLayerPtr layer = *selectedLayers.begin();

    if (layer->isValid())
    {
      dlg.setCurrentLayer(layer);
    }
  }

  dlg.exec();
}

void geopx::desktop::GeopixelDesktop::onQueryLayerCreateLayer(te::map::AbstractLayerPtr layer)
{
  if (layer)
  {
    std::list<te::map::AbstractLayerPtr> layers;
    layers.push_back(layer);

    if (layers.empty())
      return;
    
    QModelIndex pF = GetParent(getLayerExplorer());

    getLayerExplorer()->addLayers(layers, pF);

    projectChanged();
  }
}

void geopx::desktop::GeopixelDesktop::onAddDataSetLayerTriggered()
{
  try
  {
    if(m_project == nullptr)
      throw te::common::Exception(TE_TR("Error: there is no opened project!"));

    QApplication::setOverrideCursor(Qt::WaitCursor);

    // Get the parent layer where the dataset layer(s) will be added.
    QModelIndex pF = GetParent(getLayerExplorer());

    // Get the layer(s) to be added
    std::unique_ptr<te::qt::widgets::DataSourceSelectorDialog> dselector(new te::qt::widgets::DataSourceSelectorDialog(this));

    QString dsTypeSett = te::qt::af::GetLastDatasourceFromSettings();

    if(!dsTypeSett.isNull() && !dsTypeSett.isEmpty())
      dselector->setDataSourceToUse(dsTypeSett);

    QApplication::restoreOverrideCursor();

    int retval = dselector->exec();

    QApplication::setOverrideCursor(Qt::WaitCursor);

    if(retval == QDialog::Rejected)
    {
      QApplication::restoreOverrideCursor();
      return;
    }

    std::list<te::da::DataSourceInfoPtr> selectedDatasources = dselector->getSelecteds();

    if(selectedDatasources.empty())
    {
      QApplication::restoreOverrideCursor();
      return;
    }

    dselector.reset(nullptr);

    const std::string& dsTypeId = selectedDatasources.front()->getType();

    const te::qt::widgets::DataSourceType* dsType = te::qt::widgets::DataSourceTypeManager::getInstance().get(dsTypeId);

    std::unique_ptr<QWidget> lselectorw(dsType->getWidget(te::qt::widgets::DataSourceType::WIDGET_LAYER_SELECTOR, this));

    if(lselectorw.get() == nullptr)
    {
      QApplication::restoreOverrideCursor();
      throw te::common::Exception((boost::format(TE_TR("No layer selector widget found for this type of data source: %1%!")) % dsTypeId).str());
    }

    te::qt::widgets::AbstractLayerSelector* lselector = dynamic_cast<te::qt::widgets::AbstractLayerSelector*>(lselectorw.get());

    if(lselector == nullptr)
    {
      QApplication::restoreOverrideCursor();
      throw te::common::Exception(TE_TR("Wrong type of object for layer selection!"));
    }

    lselector->set(selectedDatasources);

    QApplication::restoreOverrideCursor();

    std::list<te::map::AbstractLayerPtr> layers = lselector->getLayers();

    if(layers.empty())
      return;

    QApplication::setOverrideCursor(Qt::WaitCursor);

    lselectorw.reset(nullptr);

    getLayerExplorer()->addLayers(layers, pF);

    te::qt::af::SaveLastDatasourceOnSettings(dsTypeId.c_str());

    projectChanged();
  }
  catch(const std::exception& e)
  {
    QApplication::restoreOverrideCursor();
    QMessageBox::warning(this, m_app->getAppTitle(), e.what());
  }
  catch(...)
  {
    QApplication::restoreOverrideCursor();
    QMessageBox::warning(this,
                         m_app->getAppTitle(),
                         tr("Unknown error while trying to add a layer from a dataset!"));
  }

  QApplication::restoreOverrideCursor();
}

void geopx::desktop::GeopixelDesktop::onAddQueryLayerTriggered()
{
  try
  {
    if(m_project == nullptr)
      throw te::common::Exception(TE_TR("Error: there is no opened project!"));

    // Get the parent layer where the dataset layer(s) will be added.
    QModelIndex par = GetParent(getLayerExplorer());

    std::unique_ptr<te::qt::widgets::QueryLayerBuilderWizard> qlb(new te::qt::widgets::QueryLayerBuilderWizard(this));

    std::list<te::map::AbstractLayerPtr> layers;
    te::qt::widgets::GetValidLayers(getLayerExplorer()->model(), QModelIndex(), layers);

    qlb->setLayerList(layers);

    int retval = qlb->exec();

    if(retval == QDialog::Rejected)
      return;

    te::map::AbstractLayerPtr layer = qlb->getQueryLayer();

    if((m_layerExplorer != nullptr) && (m_layerExplorer->getExplorer() != nullptr))
    {
      std::list<te::map::AbstractLayerPtr> ls;
      ls.push_back(layer);

      getLayerExplorer()->addLayers(ls, par);

      projectChanged();
    }
  }
  catch(const std::exception& e)
  {
    QMessageBox::warning(this, m_app->getAppTitle(), e.what());
  }
  catch(...)
  {
    QMessageBox::warning(this,
                         m_app->getAppTitle(),
                         tr("Unknown error while trying to add a layer from a queried dataset!"));
  }

}

void geopx::desktop::GeopixelDesktop::onAddTabularLayerTriggered()
{
  try
  {
    if(m_project == nullptr)
      throw te::common::Exception(TE_TR("Error: there is no opened project!"));

    // Get the parent layer where the tabular layer will be added.
    QModelIndex par = GetParent(getLayerExplorer());

    te::qt::widgets::DataPropertiesDialog dlg(this);
    int res = dlg.exec();

    if(res == QDialog::Accepted)
    {
      if((m_layerExplorer != nullptr) && (m_layerExplorer->getExplorer() != nullptr))
      {
        std::list<te::map::AbstractLayerPtr> ls;
        ls.push_back(dlg.getDataSetAdapterLayer());

        getLayerExplorer()->addLayers(ls, par);

        projectChanged();
      }
    }
  }
  catch(const std::exception& e)
  {
    QMessageBox::warning(this, m_app->getAppTitle(), e.what());
  }
  catch(...)
  {
    QMessageBox::warning(this,
                         m_app->getAppTitle(),
                         tr("Unknown error while trying to add a layer from a queried dataset!"));
  }
}

void geopx::desktop::GeopixelDesktop::onAddFolderLayerTriggered()
{
  // Get the parent item where the folder layer will be added.
  QModelIndex idx = GetParent(getLayerExplorer());

  // Get the folder layer to be added
  bool ok;
  QString text = QInputDialog::getText(this, m_app->getAppTitle(),
                                       tr("Folder layer name:"), QLineEdit::Normal,
                                       tr("Enter folder layer name"), &ok);

  if(!ok)
    return;

  if(text.isEmpty())
  {
    QMessageBox::warning(this, m_app->getAppTitle(), tr("Enter the layer name!"));
    return;
  }

  getLayerExplorer()->addFolder(text.toUtf8().data(), idx);

  projectChanged();
}

void geopx::desktop::GeopixelDesktop::onUpdateLayerDataSourceTriggered()
{
  try
  {
    std::list<te::qt::widgets::TreeItem*> selectedLayerItems = getLayerExplorer()->getSelectedItems();

    if(selectedLayerItems.empty())
      return;

    te::map::AbstractLayerPtr layer = ((te::qt::widgets::LayerItem*)selectedLayerItems.front())->getLayer();

    te::map::DataSetLayer* dsl = (te::map::DataSetLayer*)layer.get();

    if(!dsl)
      return;

    std::list<te::da::DataSourceInfoPtr> selecteds;

    te::da::DataSourceInfoPtr ds = te::da::DataSourceInfoManager::getInstance().get(dsl->getDataSourceId());

    if (ds)
    {
      selecteds.push_back(ds);

      const std::string& dsTypeId = selecteds.front()->getType();

      const te::qt::widgets::DataSourceType* dsType = te::qt::widgets::DataSourceTypeManager::getInstance().get(dsTypeId);

      std::unique_ptr<QWidget> connectorw(dsType->getWidget(te::qt::widgets::DataSourceType::WIDGET_DATASOURCE_CONNECTOR, this));

      if (connectorw.get() == nullptr)
      {
        throw te::common::Exception((boost::format(TE_TR("No layer selector widget found for this type of data source: %1%!")) % dsTypeId).str());
      }

      te::qt::widgets::AbstractDataSourceConnector* connector = dynamic_cast<te::qt::widgets::AbstractDataSourceConnector*>(connectorw.get());

      if (connector == nullptr)
      {
        throw te::common::Exception(TE_TR("Wrong type of object for layer selection!"));
      }

      connector->update(selecteds);

    }
    else
    {
      te::qt::widgets::DataSourceSelectorDialog dlg;
      
      if (dlg.exec() == QDialog::Rejected)
        return;

      te::da::DataSourceInfoPtr selected = dlg.getSelecteds().front();

      dsl->setDataSourceId(selected->getId());

    }
  }
  catch(const std::exception& e)
  {
    QMessageBox::warning(this, m_app->getAppTitle(), e.what());
  }
  catch(...)
  {
    QMessageBox::warning(this,
                         m_app->getAppTitle(),
                         tr("Unknown error while trying to update a layer data source!"));
  }
}

void geopx::desktop::GeopixelDesktop::onRecentProjectsTriggered(QAction* proj)
{
  bool status = checkAndSaveProject();

  if (!status)
    return;

  QString projFile = proj->data().toString();

  resetComponents();

  openProject(projFile);
}

void geopx::desktop::GeopixelDesktop::onProjectAddGeomGeopxLayerTriggered()
{
  geopx::desktop::GeopixelConnectorDialog dlg(this);

  if (dlg.exec() == QDialog::Accepted)
  {
    std::string userName = dlg.getUser();
    std::string profileName = dlg.getProfile();

    te::da::DataSourceInfoPtr dsInfo = dlg.getDataSource();

    if (dsInfo.get() != nullptr)
    {
      te::da::DataSourceInfoManager::getInstance().add(dsInfo);

      te::da::DataSourceManager::getInstance().make(dsInfo->getId(), dsInfo->getAccessDriver(), dsInfo->getConnInfo());
    }

    geopx::desktop::GeopixelDataSetSelectorDialog dlgSelector(this);

    te::da::DataSourcePtr dsPtr = te::da::GetDataSource(dsInfo->getId(), true);

    dlgSelector.setConnection(dsPtr.get(), userName, profileName, false);

    dlgSelector.exec();
  }
}

void geopx::desktop::GeopixelDesktop::onProjectAddTileGeopxLayerTriggered()
{
  geopx::desktop::GeopixelConnectorDialog dlg(this);

  if (dlg.exec() == QDialog::Accepted)
  {
    std::string userName = dlg.getUser();
    std::string profileName = dlg.getProfile();

    te::da::DataSourceInfoPtr dsInfo = dlg.getDataSource();

    if (dsInfo.get() != nullptr)
    {
      te::da::DataSourceInfoManager::getInstance().add(dsInfo);

      te::da::DataSourceManager::getInstance().make(dsInfo->getId(), dsInfo->getAccessDriver(), dsInfo->getConnInfo());
    }

    geopx::desktop::GeopixelDataSetSelectorDialog dlgSelector(this);

    te::da::DataSourcePtr dsPtr = te::da::GetDataSource(dsInfo->getId(), true);

    dlgSelector.setConnection(dsPtr.get(), userName, profileName, true);

    dlgSelector.exec();
  }
}


void geopx::desktop::GeopixelDesktop::onPluginsManagerTriggered()
{
  try
  {
    te::qt::widgets::PluginManagerDialog dlg(this);
    dlg.exec();
  }
  catch (const std::exception& e)
  {
    QMessageBox::warning(this, m_app->getAppTitle(), e.what());
  }
}

void geopx::desktop::GeopixelDesktop::onToolsCustomizeTriggered()
{
  try
  {
    te::qt::af::SettingsDialog dlg(this);
    dlg.setApplicationController(m_app);
    dlg.exec();
  }
  catch (const std::exception& e)
  {
    QMessageBox::warning(this, m_app->getAppTitle(), e.what());
  }
}

void geopx::desktop::GeopixelDesktop::onToolsDataExchangerTriggered()
{
  try
  {
    te::qt::widgets::DataExchangerWizard dlg(this);
    dlg.exec();
  }
  catch (const std::exception& e)
  {
    QMessageBox::warning(this, m_app->getAppTitle(), e.what());
  }
}

void geopx::desktop::GeopixelDesktop::onToolsDataExchangerDirectTriggered()
{
  try
  {
    te::qt::widgets::DirectExchangerDialog dlg(this);

    std::list<te::map::AbstractLayerPtr> layers;
    te::qt::widgets::GetValidLayers(getLayerExplorer()->model(), QModelIndex(), layers);

    dlg.setLayers(layers);

    QString dsTypeSett = te::qt::af::GetLastDatasourceFromSettings();

    if(!dsTypeSett.isNull() && !dsTypeSett.isEmpty())
      dlg.setLastDataSource(dsTypeSett.toUtf8().data());

    dlg.exec();
  }
  catch(const std::exception& e)
  {
    QMessageBox::warning(this, m_app->getAppTitle(), e.what());
  }
}

void geopx::desktop::GeopixelDesktop::onToolsDataExchangerDirectPopUpTriggered()
{
  try
  {
    te::qt::widgets::DirectExchangerDialog dlg(this);

    std::list<te::map::AbstractLayerPtr> selectedLayerItems = te::qt::widgets::GetSelectedLayersOnly(getLayerExplorer());

    if(selectedLayerItems.empty())
    {
      QMessageBox::warning(this, m_app->getAppTitle(),
                           tr("Select a single layer in the layer explorer!"));
      return;
    }

    std::list<te::map::AbstractLayerPtr> layers;
    layers.push_back(selectedLayerItems.front());

    dlg.setLayers(layers);

    dlg.exec();
  }
  catch(const std::exception& e)
  {
    QMessageBox::warning(this, m_app->getAppTitle(), e.what());
  }
}

void geopx::desktop::GeopixelDesktop::onToolsQueryDataSourceTriggered()
{
  try
  {
    te::qt::widgets::QueryDataSourceDialog dlg(this, Qt::WindowMaximizeButtonHint);

    connect(&dlg, SIGNAL(createNewLayer(te::map::AbstractLayerPtr)), this, SLOT(onCreateNewLayer(te::map::AbstractLayerPtr)));

    std::list<te::map::AbstractLayerPtr> layers;
    te::qt::widgets::GetValidLayers(getLayerExplorer()->model(), QModelIndex(), layers);

    dlg.setLayerList(layers);
    dlg.setAppMapDisplay(m_display->getDisplay());

    dlg.exec();
  }
  catch(const std::exception& e)
  {
    QMessageBox::warning(this, m_app->getAppTitle(), e.what());
  }
}

void geopx::desktop::GeopixelDesktop::onToolsRasterMultiResolutionTriggered()
{
  try
  {
    te::qt::widgets::MultiResolutionDialog dlg(this);

    std::list<te::map::AbstractLayerPtr> layers;
    te::qt::widgets::GetValidLayers(getLayerExplorer()->model(), QModelIndex(), layers);

    dlg.setLayerList(layers);

    dlg.exec();
  }
  catch(const std::exception& e)
  {
    QMessageBox::warning(this, m_app->getAppTitle(), e.what());
  }
}

void geopx::desktop::GeopixelDesktop::onDataSourceExplorerTriggered()
{
  try
  {
    std::unique_ptr<te::qt::widgets::DataSourceExplorerDialog> dExplorer(new te::qt::widgets::DataSourceExplorerDialog(this));

    QString dsTypeSett = te::qt::af::GetLastDatasourceFromSettings();

    if (!dsTypeSett.isNull() && !dsTypeSett.isEmpty())
      dExplorer->setDataSourceToUse(dsTypeSett);


    int retval = dExplorer->exec();

    if (retval == QDialog::Rejected)
      return;

    std::list<te::da::DataSourceInfoPtr> selectedDatasources = dExplorer->getSelecteds();

    if (selectedDatasources.empty())
      return;

    dExplorer.reset(nullptr);

    const std::string& dsTypeId = selectedDatasources.front()->getType();

    te::qt::af::SaveLastDatasourceOnSettings(dsTypeId.c_str());
  }
  catch (const std::exception& e)
  {
    QMessageBox::warning(this, m_app->getAppTitle(), e.what());
  }
  catch (...)
  {
    QMessageBox::warning(this,
                         m_app->getAppTitle(),
                         tr("DataSetExplorer Error!"));
  }
}


void geopx::desktop::GeopixelDesktop::showProgressDockWidget()
{
  m_progressDockWidget->setVisible(true);
}

void geopx::desktop::GeopixelDesktop::onHighlightLayerObjects(const te::map::AbstractLayerPtr& layer, te::da::DataSet* dataset, te::se::Style* style)
{
  assert(layer.get());
  assert(dataset);

  te::qt::af::evt::HighlightLayerObjects e(layer, dataset, style);
  m_app->trigger(&e);
}

void geopx::desktop::GeopixelDesktop::onCreateNewLayer(te::map::AbstractLayerPtr layer)
{
  te::qt::af::evt::LayerAdded evt(layer);
  m_app->trigger(&evt);
}


void geopx::desktop::GeopixelDesktop::projectChanged()
{
  m_project->m_changed = true;

  setWindowTitle(qApp->applicationName() + " - " + m_project->m_title + " *");
}

bool geopx::desktop::GeopixelDesktop::checkAndSaveProject()
{
  if(m_project->m_changed)
  {
    QString msg = tr("The current project has unsaved changes. Do you want to save them?");
    QMessageBox::StandardButton btn = QMessageBox::question(this, windowTitle(), msg, QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);

    if (btn == QMessageBox::Save)
      onSaveProjectTriggered();

    if (btn == QMessageBox::Cancel)
      return false;
  }

  return true;
}

void geopx::desktop::GeopixelDesktop::openProject(const QString& prjFileName)
{
  std::list<te::map::AbstractLayerPtr> lst;

  try
  {
    LoadProject(prjFileName, *m_project, lst);

    geopx::desktop::XMLFormatter::format(m_project, lst, false);
    te::qt::af::XMLFormatter::formatDataSourceInfos(false);

    getLayerExplorer()->setLayers(lst);

    m_project->m_changed = false;

    setWindowTitle(m_app->getAppName() + " - " + m_project->m_title);

    AddRecentProjectToSettings(m_project->m_title, m_project->m_fileName);
  }
  catch(te::common::Exception&)
  {
    QString msg = tr("The project ") + m_project->m_title + tr(" is invalid or from an older Geopixel Desktop version. \nAn empty project will be created.");
    QMessageBox::warning(this, m_app->getAppTitle(), msg);

    ResetProject(m_project);
  }
  catch (const boost::exception&)
  {
    QString msg = tr("The project ") + m_project->m_title + tr(" is invalid or from an older Geopixel Desktop version. \nAn empty project will be created.");
    QMessageBox::warning(this, m_app->getAppTitle(), msg);

    ResetProject(m_project);
  }
}

void geopx::desktop::GeopixelDesktop::resetComponents()
{
  m_project->m_fileName = "";
  m_project->m_title = "";
  m_project->m_changed = false;

  // Closing tables
  for (std::size_t i = 0; i < m_tables.size(); ++i)
  {
    m_tables[i]->close();
  }

  setWindowTitle(m_app->getAppName() + " - " + m_project->m_title);

  std::list<te::map::AbstractLayerPtr> ls;

  getLayerExplorer()->setLayers(ls);

  getMapDisplay()->setLayerList(ls);

  getMapDisplay()->refresh(true);

  m_styleExplorer->getExplorer()->clear();

  //clear connect layers
  m_connectLayersDock->clear();

  // Set map srid 0
  te::qt::af::evt::MapSRIDChanged mapSRIDChagned(std::pair<int, std::string>(0, ""));
  m_app->trigger(&mapSRIDChagned);

  m_display->getDisplay()->setSRID(0);
}

void geopx::desktop::GeopixelDesktop::closeEvent(QCloseEvent* event)
{
  te::qt::af::SaveDataSourcesFile(m_app);

  if (checkAndSaveProject() == false)
  {
    event->ignore();
    return;
  }

  if (m_connectLayersDock)
    m_connectLayersDock->clear();

  event->accept();
}

void geopx::desktop::GeopixelDesktop::addActions(const QString& plgName, const QString& category, const QList<QAction*>& acts)
{
  if(category == "Processing")
  {
    QMenu* mnu = m_app->getMenu("Processing");

    if(mnu == nullptr)
      return;

    QMenu* p = new QMenu(plgName, mnu);

    for(QList<QAction*>::const_iterator it = acts.begin(); it != acts.end(); ++it)
      p->addAction(*it);

    mnu->addMenu(p);
  }
  else if(category == "Dataaccess")
  {
    for(QList<QAction*>::const_iterator it = acts.begin(); it != acts.end(); ++it)
      m_projectAddLayerMenu->addAction(*it);
  }
  else
  {
    for(QList<QAction*>::const_iterator it = acts.begin(); it != acts.end(); ++it)
      m_pluginsMenu->insertAction(m_app->findAction("ManagePluginsSeparator"), *it);
  }
}

