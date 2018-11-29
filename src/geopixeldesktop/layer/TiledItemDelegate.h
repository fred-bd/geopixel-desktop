/*!
  \file geopx-desktop/src/geopixeldesktop/layer/TiledItemDelegate.h

  \brief Implements Terralib item delegate for TILEDLAYER.
*/

#ifndef __GEOPXDESKTOP_DESKTOP_LAYER_TILEDITEMDELEGATE_H
#define __GEOPXDESKTOP_DESKTOP_LAYER_TILEDITEMDELEGATE_H

// TerraLib
#include <terralib/common/Decorator.h>

// Qt
#include <QIcon>
#include <QStyledItemDelegate>

namespace geopx
{
  namespace desktop
  {
    namespace layer
    {
      class TiledItemDelegate : public te::common::Decorator<QStyledItemDelegate>
      {

        public:

          TiledItemDelegate(QStyledItemDelegate* decorated, QObject* parent = 0);

          ~TiledItemDelegate();

          void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const;

        protected:

          QIcon m_icon;

      };

    }
  }
}

#endif //__GEOPXDESKTOP_DESKTOP_LAYER_TILEDITEMDELEGATE_H
