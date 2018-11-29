/*!
  \file geopx-desktop/src/geopixeldesktop/layer/GeopxItemDelegate.h

  \brief Implements Terralib item delegate for GeopxDataSetLayer.
*/

#ifndef __GEOPXDESKTOP_DESKTOP_LAYER_GEOPXITEMDELEGATE_H
#define __GEOPXDESKTOP_DESKTOP_LAYER_GEOPXITEMDELEGATE_H

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
      class GeopxItemDelegate : public te::common::Decorator<QStyledItemDelegate>
      {

        public:

          GeopxItemDelegate(QStyledItemDelegate* decorated, QObject* parent = 0);

          ~GeopxItemDelegate();

          void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const;

        protected:

          QIcon m_icon;

      };

    }
  }
}

#endif //__GEOPXDESKTOP_DESKTOP_LAYER_GEOPXITEMDELEGATE_H
