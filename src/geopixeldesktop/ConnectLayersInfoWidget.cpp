/*!
  \file geopx-desktop/src/geopixeldesktop/ConnectLayersInfoWidget.cpp

  \brief This class implements a status bar widget for connect layers information.
*/

#include "ConnectLayersInfoWidget.h"
#include "Utils.h"

// Qt
#include <QGridLayout>
#include <QIcon>
#include <QMouseEvent>
#include <QToolTip>

geopx::desktop::ConnectLayersInfoWidget::ConnectLayersInfoWidget(QWidget* parent)
: QWidget(parent)
{
  buildUI();
}

geopx::desktop::ConnectLayersInfoWidget::~ConnectLayersInfoWidget()
{

}

void geopx::desktop::ConnectLayersInfoWidget::setConnectLayerStatus(bool status)
{
  m_connectLayerStatus = status;

  if (!m_connectLayerStatus)
    clear();
  else
    update();
}

void geopx::desktop::ConnectLayersInfoWidget::setNumberOfLayersConnected(std::size_t nLayers)
{
  m_nLayers = nLayers;

  update();
}

void geopx::desktop::ConnectLayersInfoWidget::setFlickerStatus(bool status)
{
  m_flickerStatus = status;

  update();
}

void geopx::desktop::ConnectLayersInfoWidget::setCurrentLayerName(const std::string& name)
{
  m_currentLayer = name.c_str();

  update();
}

void geopx::desktop::ConnectLayersInfoWidget::clear()
{
  m_currentLayer.clear();
  m_connectLayerStatus = false;
  m_flickerStatus = false;
  m_nLayers = 0;

  update();
}

void geopx::desktop::ConnectLayersInfoWidget::buildUI()
{
  QGridLayout* layout = new QGridLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);

  //connection icon
  m_connectLabel = new QLabel(this);
  m_connectLabel->setPixmap(QIcon(geopx::desktop::FindInPath("share/geopixeldesktop/icons/svg/connect-eye.svg").c_str()).pixmap(16, 16));
  layout->addWidget(m_connectLabel, 0, 0);

  //connection info (n layers)
  m_connectInfoLabel = new QLabel(this);
  m_connectInfoLabel->setTextFormat(Qt::RichText);
  layout->addWidget(m_connectInfoLabel, 0, 1);

  //flick icon
  m_flickLabel = new QLabel(this);
  m_flickLabel->setPixmap(QIcon(geopx::desktop::FindInPath("share/geopixeldesktop/icons/svg/connect-flick.svg").c_str()).pixmap(16, 16));
  layout->addWidget(m_flickLabel, 0, 2);

  //flick status icon
  m_flickStatusLabel = new QLabel(this);
  layout->addWidget(m_flickStatusLabel, 0, 3);

  //layer icon
  m_layerLabel = new QLabel(this);
  m_layerLabel->setPixmap(QIcon::fromTheme("layer").pixmap(16, 16));
  layout->addWidget(m_layerLabel, 0, 4);

  //flick status icon
  m_layerNameLabel = new QLabel(this);
  m_layerNameLabel->setTextFormat(Qt::RichText);
  layout->addWidget(m_layerNameLabel, 0, 5);

  clear();
}

void geopx::desktop::ConnectLayersInfoWidget::update()
{
  m_connectLabel->setEnabled(m_connectLayerStatus);

  if (!m_connectLayerStatus)
  {
    m_connectInfoLabel->setVisible(false);

    m_flickLabel->setVisible(false);
    m_flickStatusLabel->setVisible(false);

    m_layerLabel->setVisible(false);
    m_layerNameLabel->setVisible(false);

    return;
  }

  m_connectInfoLabel->setVisible(true);

  {
    std::string richStr = "<b>";
    richStr += QString::number(m_nLayers).toStdString();
    richStr += " layer(s) connected</b>";
    richStr += "</b>";
    m_connectInfoLabel->setText(richStr.c_str());
  }

  m_flickLabel->setVisible(true);
  m_flickStatusLabel->setVisible(true);

  m_layerLabel->setVisible(true);
  m_layerNameLabel->setVisible(true);

  if (!m_flickerStatus)
    m_flickStatusLabel->setPixmap(QIcon::fromTheme("check").pixmap(16, 16));
  else
    m_flickStatusLabel->setPixmap(QIcon::fromTheme("delete").pixmap(16, 16));

  if (m_currentLayer.isEmpty())
    m_layerNameLabel->setText("");
  else
  {
    std::string richStr = "";

    if (m_flickerStatus)
    {
      richStr = "<b><font color = \"red\">";
      richStr += m_currentLayer.toStdString();
      richStr += "</font></b>";
    }
    else
    {
      richStr = "<b><font color = \"blue\">";
      richStr += m_currentLayer.toStdString();
      richStr += "</font></b>";
    }
    
    m_layerNameLabel->setText(richStr.c_str());
  }

  
}
