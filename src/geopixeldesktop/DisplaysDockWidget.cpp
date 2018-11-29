/*!
  \file geopx-desktop/src/geopixeldesktop/DisplaysDockWidget.cpp

  \brief This class implements a widget for displays management.
*/

#include "DisplaysDockWidget.h"
#include "ui_DisplaysDockWidgetForm.h"

//Qt
#include <QGridLayout>

geopx::desktop::DisplaysDockWidget::DisplaysDockWidget(te::qt::widgets::MapDisplay* mapDisplay, te::qt::af::MapDisplay* mapDisplayConnector, QWidget* parent)
  : QDockWidget(parent),
  m_ui(new Ui::DisplaysDockWidgetForm)
{
  // add controls
  m_ui->setupUi(this);

  QGridLayout* eyeBirdDisplayLayout = new QGridLayout(m_ui->m_eyeBirdWidget);
  m_eyeBirdMapDisplay = new te::qt::widgets::EyeBirdMapDisplayWidget(mapDisplay, m_ui->m_eyeBirdWidget);
  eyeBirdDisplayLayout->addWidget(m_eyeBirdMapDisplay);
  eyeBirdDisplayLayout->setContentsMargins(0, 0, 0, 0);

  QGridLayout* zoomInDisplayLayout = new QGridLayout(m_ui->m_zoomInWidget);
  m_zoomInMapDisplay = new te::qt::widgets::ZoomInMapDisplayWidget(mapDisplay, m_ui->m_zoomInWidget);
  zoomInDisplayLayout->addWidget(m_zoomInMapDisplay);
  zoomInDisplayLayout->setContentsMargins(0, 0, 0, 0);

  //mapDisplayConnector->setEyeBirdDisplay(m_eyeBirdMapDisplay);
  //mapDisplayConnector->setZoomInDisplay(m_zoomInMapDisplay);

  this->setAllowedAreas(Qt::DockWidgetArea::LeftDockWidgetArea | Qt::DockWidgetArea::RightDockWidgetArea);
}

geopx::desktop::DisplaysDockWidget::~DisplaysDockWidget()
{

}

void geopx::desktop::DisplaysDockWidget::resizeEvent(QResizeEvent* e)
{
//  m_eyeBirdMapDisplay->resizeEvent(e);
//  m_zoomInMapDisplay->resizeEvent(e);
}

void geopx::desktop::DisplaysDockWidget::onShowDisplaysToggled(bool flag)
{
  setVisible(flag);
}
