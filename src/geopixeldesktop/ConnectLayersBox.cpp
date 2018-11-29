/*!
  \file geopx-desktop/src/geopixeldesktop/ConnectLayersDockWidget.cpp

  \brief This class provides a rectangle that can indicate a boundary to connect layers.
*/

#include "ConnectLayersBox.h"

// Qt
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>

geopx::desktop::ConnectLayersBox::ConnectLayersBox(te::qt::widgets::MapDisplay* display, QObject* parent)
  : AbstractTool(display, parent)
{
  // Setups the rubber band style
  m_pen.setStyle(Qt::SolidLine);
  m_pen.setColor(QColor(100, 177, 216));
  m_pen.setWidth(2);
  m_brush = QColor(100, 177, 216, 80);

  m_connect = false;
  m_buildRect = false;
}

geopx::desktop::ConnectLayersBox::~ConnectLayersBox()
{
}

bool geopx::desktop::ConnectLayersBox::eventFilter(QObject* watched, QEvent* e)
{
  //m_display->setFocus();

  switch (e->type())
  {
    case QEvent::MouseButtonPress:
      return mousePressEvent(static_cast<QMouseEvent*>(e));

    case QEvent::MouseMove:
      return mouseMoveEvent(static_cast<QMouseEvent*>(e));

    case QEvent::MouseButtonRelease:
      return mouseReleaseEvent(static_cast<QMouseEvent*>(e));

    case QEvent::MouseButtonDblClick:
      return mouseDoubleClickEvent(static_cast<QMouseEvent*>(e));

    case QEvent::KeyPress:
      return keyEvent(static_cast<QKeyEvent*>(e));

    case QEvent::Enter:
    {
      if (m_cursor.shape() != Qt::BlankCursor)
        m_display->setCursor(m_cursor);
      return false;
    }

    default:
      break;
  }

  return QObject::eventFilter(watched, e);
}

bool geopx::desktop::ConnectLayersBox::mousePressEvent(QMouseEvent* e)
{
  m_origin = e->pos();

  m_connect = false;
  m_buildRect = true;

  return true;
}

bool geopx::desktop::ConnectLayersBox::mouseMoveEvent(QMouseEvent* e)
{
  if (!m_connect && m_buildRect)
  {
    m_rect = QRect(m_origin, e->pos()).normalized();

    QPixmap* draft = m_display->getDraftPixmap();
    draft->fill(Qt::transparent);

    QPainter painter(draft);
    painter.setPen(m_pen);
    painter.setBrush(m_brush);
    painter.drawRect(m_rect);

    m_display->repaint();
  }
  else if (m_connect)
  {
    m_rect = QRect(m_origin, m_destiny).normalized();

    int w = m_rect.width();
    int h = m_rect.height();

    QPoint curPos = e->pos();

    QPoint topLeft(curPos.x() - (w / 2), curPos.y() - (h / 2));
    QPoint bttomRight(curPos.x() + (w / 2), curPos.y() + (h / 2));

    QRect curRect(topLeft, bttomRight);

    emit connectLayerBoxDefined(curRect);
  }
    
  return true;
}

bool geopx::desktop::ConnectLayersBox::mouseReleaseEvent(QMouseEvent* e)
{
  m_destiny = e->pos();

  QPixmap* draft = m_display->getDraftPixmap();
  draft->fill(Qt::transparent);

  m_display->repaint();

  m_connect = true;
  m_buildRect = false;

  return true;
}

bool geopx::desktop::ConnectLayersBox::keyEvent(QKeyEvent* e)
{
  if (e->key() == Qt::Key_Up)
  {
    emit connectLayerBoxUpLayer();
  }
  else if (e->key() == Qt::Key_Down)
  {
    emit connectLayerBoxDownLayer();
  }
  else if (e->key() == Qt::Key_Space)
  {
    emit connectLayerBoxFlickLayer();
  }

  return true;
}
