/*!
  \file geopx-desktop/src/geopixeldesktop/ConnectLayersTracking.cpp

  \brief This class implements a concrete tool send events for connect layers operation.
*/


#include "ConnectLayersTracking.h"

// Qt
#include <QMouseEvent>

geopx::desktop::ConnectLayersTracking::ConnectLayersTracking(te::qt::widgets::MapDisplay* display, QObject* parent)
  : te::qt::widgets::AbstractTool(display, parent),
  m_mouseEventsEnabled(true)
{
}

geopx::desktop::ConnectLayersTracking::~ConnectLayersTracking()
{
}

void geopx::desktop::ConnectLayersTracking::enableMouseEvents()
{
  m_mouseEventsEnabled = true;
}

void geopx::desktop::ConnectLayersTracking::disableMouseEvents()
{
  m_mouseEventsEnabled = false;
}

bool geopx::desktop::ConnectLayersTracking::eventFilter(QObject* watched, QEvent* e)
{
  switch (e->type())
  {
  case QEvent::KeyPress:
    return keyEvent(static_cast<QKeyEvent*>(e));

  case QEvent::MouseButtonRelease:
    return mouseReleaseEvent(static_cast<QMouseEvent*>(e));

  default:
    return QObject::eventFilter(watched, e);
  }
}

bool geopx::desktop::ConnectLayersTracking::keyEvent(QKeyEvent* e)
{
  Qt::KeyboardModifiers keys = e->modifiers();

  if (e->key() == Qt::Key_Up)
  {
    if (keys == Qt::ControlModifier)
      emit keyCtrlUpTracked();
    else
      emit keyUpTracked();
  }
  else if (e->key() == Qt::Key_Down)
  {
    if (keys == Qt::ControlModifier)
      emit keyCtrlDownTracked();
    else
      emit keyDownTracked();
  }
  if (e->key() == Qt::Key_Left)
  {
    if (keys == Qt::ControlModifier)
      emit keyCtrlLeftTracked();
    else
      emit keyLeftTracked();
  }
  else if (e->key() == Qt::Key_Right)
  {
    if (keys == Qt::ControlModifier)
      emit keyCtrlRightTracked();
    else
      emit keyRightTracked();
  }
  else if (e->key() == Qt::Key_Space)
  {
    emit keySpaceTracked();
  }

  return false;
}

bool geopx::desktop::ConnectLayersTracking::mouseReleaseEvent(QMouseEvent* e)
{
  if (m_mouseEventsEnabled)
  {
    if (e->button() == Qt::MouseButton::RightButton)
    {
      emit keySpaceTracked();
    }
  }

  return false;
}
