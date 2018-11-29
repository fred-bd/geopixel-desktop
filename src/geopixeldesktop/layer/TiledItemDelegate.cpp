/*!
  \file geopx-desktop/src/geopixeldesktop/layer/TiledItemDelegate.cpp

  \brief Implements Terralib item delegate for TILEDLAYER.
*/

#include "../Utils.h"
#include "TiledItemDelegate.h"

//TerraLib
#include <terralib/qt/widgets/layer/explorer/LayerItem.h>

geopx::desktop::layer::TiledItemDelegate::TiledItemDelegate(QStyledItemDelegate *decorated, QObject *parent)
  : te::common::Decorator<QStyledItemDelegate>(decorated, false)
{
  setParent(parent);

  std::string iconGeopixelTile = geopx::desktop::FindInPath("share/geopixeldesktop/icons/svg/geopixel_tile.svg").c_str();

  m_icon = QIcon(iconGeopixelTile.c_str());
}

geopx::desktop::layer::TiledItemDelegate::~TiledItemDelegate() = default;

void geopx::desktop::layer::TiledItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  te::qt::widgets::TreeItem* item = static_cast<te::qt::widgets::TreeItem*>(index.internalPointer());

  if(item->getType() == "LAYER")
  {
    te::qt::widgets::LayerItem* li = (te::qt::widgets::LayerItem*)item;

    if(li->getLayer()->isValid() && li->getLayer()->getType() == "TILEDLAYER")
    {
      QStyleOptionViewItem opt = option;
      opt.decorationSize = QSize(20, 20);

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
      opt.features |= QStyleOptionViewItem::HasDecoration;
      opt.icon = m_icon;
#endif

      QStyledItemDelegate::paint(painter, opt, index);

      return;
    }
  }

  if(m_decorated != nullptr)
    m_decorated->paint(painter, option, index);
  else
    QStyledItemDelegate::paint(painter, option, index);
}
