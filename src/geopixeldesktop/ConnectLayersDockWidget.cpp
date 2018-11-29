/*!
\file geopx-desktop/src/geopixeldesktop/ConnectLayersDockWidget.cpp

\brief This class implements a widget for connect layers management.
*/

#include "ConnectLayersDockWidget.h"
#include "ConnectLayersBox.h"
#include "Utils.h"
#include "ui_ConnectLayersDockWidgetForm.h"

// TerraLib
#include <terralib/common/STLUtils.h>
#include <terralib/maptools/FolderLayer.h>
#include <terralib/qt/af/ApplicationController.h>
#include <terralib/qt/af/events/LayerEvents.h>
#include <terralib/qt/widgets/canvas/Canvas.h>
#include <terralib/qt/widgets/layer/explorer/LayerItem.h>
#include <terralib/qt/widgets/utils/ScopedCursor.h>

// Qt
#include <QKeyEvent>
#include <QMessageBox>

Q_DECLARE_METATYPE(te::map::AbstractLayerPtr);

void BuildLayersTree(QTreeWidget* treeWidget, std::list<te::map::AbstractLayerPtr> layerList, QTreeWidgetItem* itemParent = 0)
{
  for (std::list<te::map::AbstractLayerPtr>::const_iterator it = layerList.begin(); it != layerList.end(); ++it)
  {
    te::map::AbstractLayerPtr layer = *it;

    if (layer->getType() == "FOLDERLAYER")
    {
      QTreeWidgetItem* item;

      if (itemParent)
        item = new QTreeWidgetItem(itemParent);
      else
        item = new QTreeWidgetItem(treeWidget);

      item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsTristate | Qt::ItemIsEnabled);
      item->setCheckState(0, Qt::Unchecked);
      item->setText(0, layer->getTitle().c_str());
      item->setIcon(0, QIcon::fromTheme("folder"));
      item->setData(0, Qt::UserRole, QVariant::fromValue(layer));

      if (itemParent)
        itemParent->insertChild(0, item);
      else
        treeWidget->insertTopLevelItem(0, item);

      std::vector<te::map::AbstractLayer*> layers = layer->getDescendants();

      std::list<te::map::AbstractLayerPtr> subLayerList;

      for (std::size_t t = 0; t < layers.size(); ++t)
        subLayerList.push_back(layers[t]);

      BuildLayersTree(treeWidget, subLayerList, item);
    }
    else
    {
      QTreeWidgetItem* item;
      
      if (itemParent)
        item = new QTreeWidgetItem(itemParent);
      else
        item = new QTreeWidgetItem(treeWidget);

      item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
      item->setCheckState(0, Qt::Unchecked);
      item->setText(0, layer->getTitle().c_str());
      item->setIcon(0, QIcon::fromTheme("layer"));
      item->setData(0, Qt::UserRole, QVariant::fromValue(layer));

      if (itemParent)
        itemParent->insertChild(0, item);
      else
        treeWidget->insertTopLevelItem(0, item);
    }
  }
}

std::vector< std::pair<QTreeWidgetItem*, std::list< te::map::AbstractLayerPtr > > > GetVecLayerList(QTreeWidget* treeWidget, bool groupFolder)
{
  std::vector< std::pair<QTreeWidgetItem*, std::list< te::map::AbstractLayerPtr > > > vecLayerList;

  int nTopLevels = treeWidget->topLevelItemCount();
  
  for (int i = 0; i < nTopLevels; ++i)
  {
    QTreeWidgetItem* item = treeWidget->topLevelItem(i);

    QVariant varLayer = item->data(0, Qt::UserRole);
    te::map::AbstractLayerPtr layer = varLayer.value<te::map::AbstractLayerPtr>();

    if (layer->getType() == "FOLDERLAYER")
    {
      int childCount = item->childCount();

      std::list< te::map::AbstractLayerPtr > list;

      for (int j = 0; j < childCount; ++j)
      {
        QTreeWidgetItem* subItem = item->child(j);

        if (!groupFolder)
        {
          list.clear();
        }

        if (subItem->checkState(0) == Qt::Checked)
        {
          QVariant subVarLayer = subItem->data(0, Qt::UserRole);
          te::map::AbstractLayerPtr subLayer = subVarLayer.value<te::map::AbstractLayerPtr>();

          list.push_back(subLayer);
        }

        if (!list.empty() && !groupFolder)
        {
          std::pair < QTreeWidgetItem*, std::list< te::map::AbstractLayerPtr > > pair(subItem, list);

          vecLayerList.push_back(pair);
        }
      }

      if (!list.empty() && groupFolder)
      {
        std::pair < QTreeWidgetItem*, std::list< te::map::AbstractLayerPtr > > pair(item, list);

        vecLayerList.push_back(pair);
      }
    }
    else
    {
      if (item->checkState(0) == Qt::Checked)
      {
        std::list< te::map::AbstractLayerPtr > list;
        list.push_back(layer);

        std::pair < QTreeWidgetItem*, std::list< te::map::AbstractLayerPtr > > pair(item, list);

        vecLayerList.push_back(pair);
      }
    }
  }

  return vecLayerList;
}

geopx::desktop::ConnectLayersDockWidget::ConnectLayersDockWidget(QWidget* parent)
  : QDockWidget(parent),
  m_ui(new Ui::ConnectLayersDockWidgetForm),
  m_tracking(nullptr),
  m_isConnected(false),
  m_flick(false),
  m_toolChecked(false),
  m_currentAction(0),
  m_mapMenu(0), 
  m_currentItem(0)
{
  // add controls
  m_ui->setupUi(this);

  m_ui->m_activeToolButton->setIcon(QIcon(geopx::desktop::FindInPath("share/geopixeldesktop/icons/svg/connect-eye.svg").c_str()));
  m_ui->m_flickToolButton->setIcon(QIcon(geopx::desktop::FindInPath("share/geopixeldesktop/icons/svg/connect-flick.svg").c_str()));
  m_ui->m_boxToolButton->setIcon(QIcon(geopx::desktop::FindInPath("share/geopixeldesktop/icons/svg/connect-box.svg").c_str()));
  m_ui->m_upToolButton->setIcon(QIcon::fromTheme("go-up"));
  m_ui->m_downToolButton->setIcon(QIcon::fromTheme("go-down"));
  m_ui->m_drawToolButton->setIcon(QIcon::fromTheme("map-draw"));
  m_ui->m_updateToolButton->setIcon(QIcon::fromTheme("view-refresh"));
  m_ui->m_groupFoderToolButton->setIcon(QIcon::fromTheme("folder"));
  
  m_ui->m_upToolButton->setEnabled(false);
  m_ui->m_downToolButton->setEnabled(false);


  //connects
  connect(m_ui->m_activeToolButton, SIGNAL(toggled(bool)), this, SLOT(onShowConnectLayersToggled(bool)));
  connect(m_ui->m_drawToolButton, SIGNAL(clicked()), this, SLOT(onDrawToolButtonClicked()));
  connect(m_ui->m_updateToolButton, SIGNAL(clicked()), this, SLOT(onUpdateToolButtonClicked()));
  connect(m_ui->m_upToolButton, SIGNAL(clicked()), this, SLOT(onUpToolButtonClicked()));
  connect(m_ui->m_downToolButton, SIGNAL(clicked()), this, SLOT(onDownToolButtonClicked()));
  connect(m_ui->m_alphaSlider, SIGNAL(valueChanged(int)), this, SLOT(onAlphaSliderChanged(int)));
  connect(m_ui->m_verticalSlider, SIGNAL(valueChanged(int)), this, SLOT(onVSliderChanged(int)));
  connect(m_ui->m_horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(onHSliderChanged(int)));
  connect(m_ui->m_flickToolButton, SIGNAL(toggled(bool)), this, SLOT(onFlickerTolButtonClicked(bool)));

  //create tool for connect layers
  QActionGroup* toolsGroup = te::qt::af::AppCtrlSingleton::getInstance().findActionGroup("Map.ToolsGroup");
  assert(toolsGroup);

  QAction* action = new QAction(this);
  action->setIcon(QIcon(geopx::desktop::FindInPath("share/geopixeldesktop/icons/svg/connect-box.svg").c_str()));
  action->setToolTip(tr("Create box to connect layers."));
  action->setCheckable(true);
  action->setEnabled(true);
  action->setObjectName("ConnectLayersBoxAction");

  m_ui->m_boxToolButton->setDefaultAction(action);

  toolsGroup->addAction(m_ui->m_boxToolButton->defaultAction());

  connect(toolsGroup, SIGNAL(triggered(QAction*)), this, SLOT(onResetTool(QAction*)));
  connect(m_ui->m_boxToolButton->defaultAction(), SIGNAL(triggered(bool)), this, SLOT(onBoxToolButtonClicked(bool)));

  //info
  m_infoWidget = new ConnectLayersInfoWidget(this);

  //start component off
  onShowConnectLayersToggled(false);
}

geopx::desktop::ConnectLayersDockWidget::~ConnectLayersDockWidget()
{
  clearPixmaps();

  delete m_tracking;
}

void geopx::desktop::ConnectLayersDockWidget::setApplicationComponents(te::qt::widgets::LayerItemView* layerExplorer, te::qt::widgets::MapDisplay* mapDisplay, QMenu* mapMenu)
{
  m_layerExplorer = layerExplorer;
  m_mapDisplay = mapDisplay;
  m_mapMenu = mapMenu;

  connect(m_mapDisplay, SIGNAL(extentChanged()), this, SLOT(onMapDisplayExtentChanged()));
  connect(m_mapDisplay, SIGNAL(displayPaintEvent(QPainter*)), this, SLOT(onDisplayPaintEvent(QPainter*)));

  setLayers();

  m_tracking = new geopx::desktop::ConnectLayersTracking(m_mapDisplay, this);

  connect(m_tracking, SIGNAL(keyUpTracked()), this, SLOT(onConnectLayerBoxUpLayer()));
  connect(m_tracking, SIGNAL(keyCtrlUpTracked()), this, SLOT(onConnectLayerBoxCtrlUpLayer()));
  connect(m_tracking, SIGNAL(keyDownTracked()), this, SLOT(onConnectLayerBoxDownLayer()));
  connect(m_tracking, SIGNAL(keyCtrlDownTracked()), this, SLOT(onConnectLayerBoxCtrlDownLayer()));
  connect(m_tracking, SIGNAL(keyLeftTracked()), this, SLOT(onConnectLayerBoxLeftLayer()));
  connect(m_tracking, SIGNAL(keyCtrlLeftTracked()), this, SLOT(onConnectLayerBoxCtrlLeftLayer()));
  connect(m_tracking, SIGNAL(keyRightTracked()), this, SLOT(onConnectLayerBoxRightLayer()));
  connect(m_tracking, SIGNAL(keyCtrlRightTracked()), this, SLOT(onConnectLayerBoxCtrlRightLayer()));
  connect(m_tracking, SIGNAL(keySpaceTracked()), this, SLOT(onConnectLayerBoxFlickLayer()));
}

void geopx::desktop::ConnectLayersDockWidget::setLayers()
{
  m_ui->m_upToolButton->setEnabled(false);
  m_ui->m_downToolButton->setEnabled(false);

  m_ui->m_treeWidget->clear();

  std::list<te::map::AbstractLayerPtr> layerList = m_layerExplorer->getAllLayers();
  
  BuildLayersTree(m_ui->m_treeWidget, layerList);
}

void geopx::desktop::ConnectLayersDockWidget::clear()
{
  clearPixmaps();

  m_isConnected = false;

  m_ui->m_treeWidget->clear();

  m_ui->m_upToolButton->setEnabled(false);
  m_ui->m_downToolButton->setEnabled(false);

  m_currentItem = 0;
}

QWidget* geopx::desktop::ConnectLayersDockWidget::getInfoWidget()
{
  return m_infoWidget;
}

void geopx::desktop::ConnectLayersDockWidget::onMapDisplayExtentChanged()
{
  if (!m_isConnected)
    return;

  const te::gm::Envelope& env = m_mapDisplay->getExtent();

  if (!env.isValid())
    return;

  clearPixmaps();

  resetTreeItemsColor();

  m_ui->m_flickToolButton->setChecked(true);

  m_vecLayerList = GetVecLayerList(m_ui->m_treeWidget, m_ui->m_groupFoderToolButton->isChecked());

  for (std::size_t tVec = 0; tVec < m_vecLayerList.size(); ++tVec)
  {
    QPixmap* pix = nullptr;

    std::pair<QTreeWidgetItem*, std::list< te::map::AbstractLayerPtr > > pairItemLayers = m_vecLayerList[tVec];

    QTreeWidgetItem* item = pairItemLayers.first;

    std::pair<QTreeWidgetItem*, QPixmap*> pair(item, pix);

    m_layersPixmaps.push_back(pair);
  }

  m_currentItem = 0;

  m_ui->m_upToolButton->setEnabled(false);

  if (m_layersPixmaps.size() > 1)
    m_ui->m_downToolButton->setEnabled(true);
  else
    m_ui->m_downToolButton->setEnabled(false);
  
  m_infoWidget->setNumberOfLayersConnected(m_layersPixmaps.size());

  updateSliderBarSize();
}

void geopx::desktop::ConnectLayersDockWidget::onDisplayPaintEvent(QPainter* painter)
{
  if (m_vecLayerList.empty() || !m_isConnected)
    return;


  if (m_curLayerPix < 0 || m_curLayerPix > m_layersPixmaps.size() - 1)
    return;

  double opacity = painter->opacity();

  painter->setOpacity(m_ui->m_alphaSlider->value() / 255.);
  painter->drawImage(m_rect.topLeft().x(), m_rect.topLeft().y(), m_img, m_rect.topLeft().x(), m_rect.topLeft().y(), m_rect.width(), m_rect.height());
  painter->setOpacity(opacity);
}

void geopx::desktop::ConnectLayersDockWidget::onDrawToolButtonClicked()
{
  onMapDisplayExtentChanged();
}

void  geopx::desktop::ConnectLayersDockWidget::onUpdateToolButtonClicked()
{
  setLayers();

  int nTopLevels = m_ui->m_treeWidget->topLevelItemCount();

  for (int i = 0; i < nTopLevels; ++i)
  {
    QTreeWidgetItem* item = m_ui->m_treeWidget->topLevelItem(i);

    QVariant varLayer = item->data(0, Qt::UserRole);
    te::map::AbstractLayerPtr layer = varLayer.value<te::map::AbstractLayerPtr>();

    if (layer->getType() == "FOLDERLAYER")
    {
      int childCount = item->childCount();

      for (int j = 0; j < childCount; ++j)
      {
        QTreeWidgetItem* subItem = item->child(j);

        QVariant subVarLayer = subItem->data(0, Qt::UserRole);
        te::map::AbstractLayerPtr subLayer = subVarLayer.value<te::map::AbstractLayerPtr>();

        bool check = findLayer(subLayer->getId());

        if (check)
          subItem->setCheckState(0, Qt::Checked);
        else
          subItem->setCheckState(0, Qt::Unchecked);
      }
    }
    else
    {
      bool check = findLayer(layer->getId());

      if (check)
        item->setCheckState(0, Qt::Checked);
      else
        item->setCheckState(0, Qt::Unchecked);
    }
  }
 
  onDrawToolButtonClicked();
}

void geopx::desktop::ConnectLayersDockWidget::onUpToolButtonClicked()
{
  if (m_layersPixmaps.empty())
    return;

  --m_curLayerPix;

  if (m_curLayerPix == 0)
    m_ui->m_upToolButton->setEnabled(false);

  if (m_layersPixmaps.size() > 1)
    m_ui->m_downToolButton->setEnabled(true);

  drawLayers();
}

void geopx::desktop::ConnectLayersDockWidget::onDownToolButtonClicked()
{
  if (m_layersPixmaps.empty() || m_curLayerPix >= m_layersPixmaps.size() - 1)
    return;

  ++m_curLayerPix;

  if (m_curLayerPix == m_layersPixmaps.size() - 1)
    m_ui->m_downToolButton->setEnabled(false);

  if (m_layersPixmaps.size() > 1)
    m_ui->m_upToolButton->setEnabled(true);

  drawLayers();
}

void geopx::desktop::ConnectLayersDockWidget::onAlphaSliderChanged(int /*value*/)
{
  drawLayers();
}

void geopx::desktop::ConnectLayersDockWidget::onVSliderChanged(int value)
{
  QPoint bottomRight = m_rect.bottomRight();

  m_rect.setTopLeft(QPoint(0, 0));
  m_rect.setBottomRight(QPoint(bottomRight.x(), value));

  drawLayers();
}

void geopx::desktop::ConnectLayersDockWidget::onHSliderChanged(int value)
{
  QPoint bottomRight = m_rect.bottomRight();

  m_rect.setTopLeft(QPoint(0, 0));
  m_rect.setBottomRight(QPoint(value, bottomRight.y()));

  drawLayers();
}

void geopx::desktop::ConnectLayersDockWidget::onFlickerTolButtonClicked(bool flag)
{
  m_infoWidget->setFlickerStatus(flag);

  m_flick = flag;

  drawLayers();
}

void geopx::desktop::ConnectLayersDockWidget::onBoxToolButtonClicked(bool flag)
{
  onResetTool(m_ui->m_boxToolButton->defaultAction());

  m_ui->m_boxToolButton->defaultAction()->blockSignals(true);

  if(m_toolChecked)
  {
    m_ui->m_boxToolButton->defaultAction()->setChecked(false);
    m_ui->m_boxToolButton->defaultAction()->blockSignals(false);

    m_toolChecked = false;
    m_mapDisplay->setCurrentTool(0, false);

    updateSliderBarSize();
    m_ui->m_flickToolButton->setChecked(false);
    m_ui->m_flickToolButton->setEnabled(true);
    return;
  }

  m_ui->m_boxToolButton->defaultAction()->setChecked(true);
  m_ui->m_boxToolButton->defaultAction()->blockSignals(false);

  m_toolChecked = true;

  if (!flag)
  {
    updateSliderBarSize();

    m_ui->m_flickToolButton->setChecked(false);
    m_ui->m_flickToolButton->setEnabled(true);

    return;
  }

  m_currentAction = m_ui->m_boxToolButton->defaultAction();

  geopx::desktop::ConnectLayersBox* tool = new geopx::desktop::ConnectLayersBox(m_mapDisplay);
  m_mapDisplay->setCurrentTool(tool);

  connect(tool, SIGNAL(connectLayerBoxDefined(QRect)), this, SLOT(onConnectLayerBoxDefined(QRect)));
  connect(tool, SIGNAL(connectLayerBoxUpLayer()), this, SLOT(onConnectLayerBoxUpLayer()));
  connect(tool, SIGNAL(connectLayerBoxDownLayer()), this, SLOT(onConnectLayerBoxDownLayer()));
  connect(tool, SIGNAL(connectLayerBoxFlickLayer()), this, SLOT(onConnectLayerBoxFlickLayer()));

  m_ui->m_flickToolButton->setChecked(false);
  m_ui->m_flickToolButton->setEnabled(false);
  
}

void geopx::desktop::ConnectLayersDockWidget::onConnectLayerBoxDefined(QRect rect)
{
  m_rect = rect;

  disconnect(m_ui->m_verticalSlider, SIGNAL(valueChanged(int)), this, SLOT(onVSliderChanged(int)));
  disconnect(m_ui->m_horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(onHSliderChanged(int)));

  m_ui->m_horizontalSlider->setValue(m_rect.bottomRight().x());
  m_ui->m_verticalSlider->setValue(m_rect.bottomRight().y());

  connect(m_ui->m_verticalSlider, SIGNAL(valueChanged(int)), this, SLOT(onVSliderChanged(int)));
  connect(m_ui->m_horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(onHSliderChanged(int)));

  drawLayers();
}

void geopx::desktop::ConnectLayersDockWidget::onConnectLayerBoxUpLayer()
{
  if (!m_ui->m_upToolButton->isEnabled())
    return;

  onUpToolButtonClicked();
}

void geopx::desktop::ConnectLayersDockWidget::onConnectLayerBoxCtrlUpLayer()
{
  int value = m_ui->m_verticalSlider->value();

  double step = (m_ui->m_verticalSlider->maximum() - m_ui->m_verticalSlider->minimum()) / 20.;

  int newValue = value - step;

  if (newValue > m_ui->m_verticalSlider->maximum())
    newValue = m_ui->m_verticalSlider->maximum();

  m_ui->m_verticalSlider->setValue(newValue);
}

void geopx::desktop::ConnectLayersDockWidget::onConnectLayerBoxDownLayer()
{
  if (!m_ui->m_downToolButton->isEnabled())
    return;

  onDownToolButtonClicked();
}

void geopx::desktop::ConnectLayersDockWidget::onConnectLayerBoxCtrlDownLayer()
{
  int value = m_ui->m_verticalSlider->value();

  double step = (m_ui->m_verticalSlider->maximum() - m_ui->m_verticalSlider->minimum()) / 20.;

  int newValue = value + step;

  if (newValue < m_ui->m_verticalSlider->minimum())
    newValue = m_ui->m_verticalSlider->minimum();

  m_ui->m_verticalSlider->setValue(newValue);
}

void geopx::desktop::ConnectLayersDockWidget::onConnectLayerBoxLeftLayer()
{
  int value = m_ui->m_alphaSlider->value();

  if (value < 0)
    return;

  value = value - 10;

  if (value < 0)
    value = 0;

  m_ui->m_alphaSlider->setValue(--value);
}

void geopx::desktop::ConnectLayersDockWidget::onConnectLayerBoxCtrlLeftLayer()
{
  int value = m_ui->m_horizontalSlider->value();

  double step = (m_ui->m_horizontalSlider->maximum() - m_ui->m_horizontalSlider->minimum()) / 20.;

  int newValue = value - step;

  if (newValue < m_ui->m_horizontalSlider->minimum())
    newValue = m_ui->m_horizontalSlider->minimum();

  m_ui->m_horizontalSlider->setValue(newValue);
}

void geopx::desktop::ConnectLayersDockWidget::onConnectLayerBoxRightLayer()
{
  int value = m_ui->m_alphaSlider->value();

  if (value > 255)
    return;

  value = value + 10;

  if (value > 255)
    value = 255;

  m_ui->m_alphaSlider->setValue(value);
}

void geopx::desktop::ConnectLayersDockWidget::onConnectLayerBoxCtrlRightLayer()
{
  int value = m_ui->m_horizontalSlider->value();

  double step = (m_ui->m_horizontalSlider->maximum() - m_ui->m_horizontalSlider->minimum()) / 20.;

  int newValue = value + step;

  if (newValue > m_ui->m_horizontalSlider->maximum())
    newValue = m_ui->m_horizontalSlider->maximum();

  m_ui->m_horizontalSlider->setValue(newValue);
}

void geopx::desktop::ConnectLayersDockWidget::onConnectLayerBoxFlickLayer()
{
  bool flag = m_ui->m_flickToolButton->isChecked();

  m_ui->m_flickToolButton->setChecked(!flag);

  onFlickerTolButtonClicked(!flag);
}

void geopx::desktop::ConnectLayersDockWidget::onShowConnectLayersToggled(bool flag)
{
  m_infoWidget->setConnectLayerStatus(flag);

  m_isConnected = flag;

  if (m_isConnected)
  {
    setLayers();

    m_ui->m_drawToolButton->setEnabled(true);
    m_ui->m_updateToolButton->setEnabled(true);
    m_ui->m_groupFoderToolButton->setEnabled(true);
    m_ui->m_flickToolButton->setEnabled(true);
    m_ui->m_boxToolButton->setEnabled(true);

    m_ui->m_horizontalSlider->setEnabled(true);
    m_ui->m_verticalSlider->setEnabled(true);
    m_ui->m_alphaSlider->setEnabled(true);

    if (m_tracking)
      m_mapDisplay->installEventFilter(m_tracking);

    if (m_mapMenu)
      m_mapMenu->setObjectName("NoMap");
  }
  else
  {
    clearPixmaps();

    m_ui->m_treeWidget->clear();

    
    m_ui->m_drawToolButton->setEnabled(false);
    m_ui->m_updateToolButton->setEnabled(false);
    m_ui->m_groupFoderToolButton->setEnabled(false);
    m_ui->m_flickToolButton->setEnabled(false);
    m_ui->m_boxToolButton->setEnabled(false);

    m_ui->m_horizontalSlider->setEnabled(false);
    m_ui->m_verticalSlider->setEnabled(false);
    m_ui->m_alphaSlider->setEnabled(false);

    m_ui->m_upToolButton->setEnabled(false);
    m_ui->m_downToolButton->setEnabled(false);

    if (m_tracking)
      m_mapDisplay->removeEventFilter(m_tracking);

    if (m_mapMenu)
      m_mapMenu->setObjectName("Map");
  }
}

void geopx::desktop::ConnectLayersDockWidget::enableMouseEvents()
{
  if (m_tracking)
    m_tracking->enableMouseEvents();
}

void geopx::desktop::ConnectLayersDockWidget::disableMouseEvents()
{
  if (m_tracking)
    m_tracking->disableMouseEvents();
}

void geopx::desktop::ConnectLayersDockWidget::onResetTool(QAction *action)
{
  if(!m_currentAction)
    return;

  if(m_currentAction->objectName() != action->objectName())
    m_toolChecked = false;
}

void geopx::desktop::ConnectLayersDockWidget::clearTrackTool()
{
  delete m_tracking;
  m_tracking = 0;
}

void geopx::desktop::ConnectLayersDockWidget::drawLayers()
{
  if (m_vecLayerList.empty() || !m_isConnected)
    return;


  if (m_curLayerPix < 0 || m_curLayerPix > m_layersPixmaps.size() - 1)
    return;

  //disconnect all
  m_mapDisplay->removeEventFilter(m_tracking);

  disconnect(m_mapDisplay, SIGNAL(extentChanged()), this, SLOT(onMapDisplayExtentChanged()));
  disconnect(m_mapDisplay, SIGNAL(displayPaintEvent(QPainter*)), this, SLOT(onDisplayPaintEvent(QPainter*)));

  this->setEnabled(false);

  if (!m_flick)
  {
    if (m_layersPixmaps[m_curLayerPix].second == nullptr)
    {
      QTreeWidgetItem* item = m_layersPixmaps[m_curLayerPix].first;

      for (std::size_t tVec = 0; tVec < m_vecLayerList.size(); ++tVec)
      {
        if (m_vecLayerList[tVec].first == item)
        {
          const te::gm::Envelope& env = m_mapDisplay->getExtent();

          QPixmap* pix = new QPixmap(m_mapDisplay->width(), m_mapDisplay->height());

          pix->fill(Qt::white);

          te::qt::widgets::Canvas canvas(m_mapDisplay->width(), m_mapDisplay->height());
          canvas.setDevice(pix, false);
          canvas.setWindow(env.m_llx, env.m_lly, env.m_urx, env.m_ury);

          std::pair<QTreeWidgetItem*, std::list< te::map::AbstractLayerPtr > > pairItemLayers = m_vecLayerList[tVec];

          QTreeWidgetItem* item = pairItemLayers.first;
          std::list< te::map::AbstractLayerPtr > layerList = pairItemLayers.second;

          bool cancel = false;

          te::qt::widgets::ScopedCursor sc(Qt::WaitCursor);

          for (std::list< te::map::AbstractLayerPtr >::reverse_iterator it = layerList.rbegin(); it != layerList.rend(); ++it)
          {
            te::map::AbstractLayerPtr layer = *it;

            layer->draw(&canvas, env, m_mapDisplay->getSRID(), m_mapDisplay->getScale(), &cancel);
          }

          m_layersPixmaps[m_curLayerPix].second = pix;

          m_img = pix->toImage();
        }
      }
    }
    else
    {
      m_img = m_layersPixmaps[m_curLayerPix].second->toImage();
    }
  }
  else
  {
    QPixmap pix(m_mapDisplay->width(), m_mapDisplay->height());
    pix.fill(Qt::transparent);
    m_img = pix.toImage();
  }

  //check layer list item
  if (m_currentItem)
    m_currentItem->setTextColor(0, Qt::black);

  QTreeWidgetItem* item = m_layersPixmaps[m_curLayerPix].first;

  if (m_flick)
    item->setTextColor(0, Qt::red);
  else
    item->setTextColor(0, Qt::blue);

  m_currentItem = item;

  m_infoWidget->setCurrentLayerName(item->text(0).toStdString());

  //connect all
  m_mapDisplay->installEventFilter(m_tracking);

  connect(m_mapDisplay, SIGNAL(extentChanged()), this, SLOT(onMapDisplayExtentChanged()));
  connect(m_mapDisplay, SIGNAL(displayPaintEvent(QPainter*)), this, SLOT(onDisplayPaintEvent(QPainter*)));

  this->setEnabled(true);

  m_mapDisplay->repaint();
}

void geopx::desktop::ConnectLayersDockWidget::updateSliderBarSize()
{
  m_ui->m_horizontalSlider->setMaximum(m_mapDisplay->getWidth() - 1);
  m_ui->m_horizontalSlider->setValue(m_mapDisplay->getWidth() - 1);
  m_ui->m_verticalSlider->setMaximum(m_mapDisplay->getHeight() - 1);
  m_ui->m_verticalSlider->setValue(m_mapDisplay->getHeight() - 1);

  m_rect.setBottomRight(QPoint(m_mapDisplay->getWidth() - 1, m_mapDisplay->getHeight() - 1));

  drawLayers();
}

void geopx::desktop::ConnectLayersDockWidget::clearPixmaps()
{
  m_vecLayerList.clear();

  for (std::size_t t = 0; t < m_layersPixmaps.size(); ++t)
    delete m_layersPixmaps[t].second;
  
  m_layersPixmaps.clear();

  m_curLayerPix = 0;

  m_currentItem = 0;
}

bool geopx::desktop::ConnectLayersDockWidget::findLayer(std::string id)
{
  for (std::size_t t = 0; t < m_vecLayerList.size(); ++t)
  {
    std::pair<QTreeWidgetItem*, std::list< te::map::AbstractLayerPtr > > pair = m_vecLayerList[t];

    std::list< te::map::AbstractLayerPtr > list = pair.second;

    for (std::list<te::map::AbstractLayerPtr>::const_iterator it = list.begin(); it != list.end(); ++it)
    {
      te::map::AbstractLayerPtr layer = *it;

      if (layer->getId() == id)
        return true;
    }
  }
  
  return false;
}

void  geopx::desktop::ConnectLayersDockWidget::resetTreeItemsColor()
{
  QTreeWidgetItemIterator it(m_ui->m_treeWidget);

  while (*it)
  {
    (*it)->setTextColor(0, Qt::black);

    ++it;
  }
}
