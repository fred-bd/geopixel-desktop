/*!
  \file geopx-desktop/src/geopixeltools/qt/tools/Eraser.cpp

  \brief This class implements a concrete tool to remove points from layer
*/

#include "Eraser.h"

// TerraLib
#include <terralib/common/STLUtils.h>
#include <terralib/dataaccess/dataset/DataSet.h>
#include <terralib/dataaccess/dataset/DataSetType.h>
#include <terralib/dataaccess/utils/Utils.h>
#include <terralib/datatype/StringProperty.h>
#include <terralib/geometry/Geometry.h>
#include <terralib/geometry/GeometryProperty.h>
#include <terralib/geometry/Utils.h>
#include <terralib/maptools/DataSetLayer.h>
#include <terralib/maptools/MarkRendererManager.h>
#include <terralib/memory/DataSetItem.h>
#include <terralib/se/Fill.h>
#include <terralib/se/Stroke.h>
#include <terralib/se/Mark.h>
#include <terralib/se/Utils.h>
#include <terralib/qt/widgets/canvas/Canvas.h>
#include <terralib/qt/widgets/canvas/MapDisplay.h>
#include <terralib/qt/widgets/canvas/MultiThreadMapDisplay.h>


// Qt
#include <QtCore/QPointF>
#include <QMessageBox>
#include <QMouseEvent>

// STL
#include <cassert>
#include <memory>

geopx::tools::Eraser::Eraser(te::qt::widgets::MapDisplay* display, const QCursor& cursor, te::map::AbstractLayerPtr layer, QObject* parent)
  : AbstractTool(display, parent),
    m_layer(layer),
    m_panStarted(false)
{
  setCursor(cursor);

  display->setFocus();
}

geopx::tools::Eraser::~Eraser()
{
  QPixmap* draft = m_display->getDraftPixmap();
  draft->fill(Qt::transparent);
}

bool geopx::tools::Eraser::eventFilter(QObject* watched, QEvent* e)
{
  if (e->type() == QEvent::MouseButtonRelease)
  {
    QMouseEvent* event = static_cast<QMouseEvent*>(e);

    if (event->button() == Qt::LeftButton)
    {
      selectObjects(event);

      return true;
    }
    else
    {
      return panMouseReleaseEvent(event);
    }
  }
  else if (e->type() == QEvent::MouseButtonPress)
  {
    QMouseEvent* event = static_cast<QMouseEvent*>(e);

    return panMousePressEvent(event);
  }
  else if (e->type() == QEvent::MouseMove)
  {
    QMouseEvent* event = static_cast<QMouseEvent*>(e);

    return panMouseMoveEvent(event);
  }
  else if (e->type() == QEvent::KeyPress)
  {
    QKeyEvent* event = static_cast<QKeyEvent*>(e);

    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Control)
      removeObjects();

    if (event->key() == Qt::Key_Escape)
      cancelOperation();

    return true;
  }
  else if (e->type() == QEvent::Enter)
  {
    if (m_cursor.shape() != Qt::BlankCursor)
      m_display->setCursor(m_cursor);
    return false;
  }

  return false;
}

void geopx::tools::Eraser::setLayer(te::map::AbstractLayerPtr layer)
{
  m_layer = layer;
}

void geopx::tools::Eraser::selectObjects(QMouseEvent* e)
{
  if (!m_layer.get())
    return;

  QPointF pixelOffset(7.0, 7.0);
#if (QT_VERSION >= 0x050000)
  QRectF rect = QRectF(e->localPos() - pixelOffset, e->localPos() + pixelOffset);
#else
  QRectF rect = QRectF(e->posF() - pixelOffset, e->posF() + pixelOffset);
#endif

  // Converts rect boundary to world coordinates
  QPointF ll(rect.left(), rect.bottom());
  QPointF ur(rect.right(), rect.top());
  ll = m_display->transform(ll);
  ur = m_display->transform(ur);

  // Bulding the query box
  te::gm::Envelope envelope(ll.x(), ll.y(), ur.x(), ur.y());

  te::gm::Envelope reprojectedEnvelope(envelope);

  if ((m_layer->getSRID() != TE_UNKNOWN_SRS) && (m_display->getSRID() != TE_UNKNOWN_SRS) && (m_layer->getSRID() != m_display->getSRID()))
    reprojectedEnvelope.transform(m_display->getSRID(), m_layer->getSRID());

  if (!reprojectedEnvelope.intersects(m_layer->getExtent()))
    return;

  // Gets the layer schema
  std::unique_ptr<const te::map::LayerSchema> schema(m_layer->getSchema());

  if (!schema->hasGeom())
    return;

  try
  {
    te::gm::GeometryProperty* gp = te::da::GetFirstGeomProperty(schema.get());

    // Gets the dataset
    std::unique_ptr<te::da::DataSet> dataset = m_layer->getData(gp->getName(), &reprojectedEnvelope, te::gm::INTERSECTS);
    assert(dataset.get());

    // Generates a geometry from the given extent. It will be used to refine the results
    std::unique_ptr<te::gm::Geometry> geometryFromEnvelope(te::gm::GetGeomFromEnvelope(&reprojectedEnvelope, m_layer->getSRID()));

    while (dataset->moveNext())
    {
      std::unique_ptr<te::gm::Geometry> g(dataset->getGeometry(gp->getName()));

      if (g->getSRID() == TE_UNKNOWN_SRS)
        g->setSRID(m_layer->getSRID());

      if (!g->intersects(geometryFromEnvelope.get()))
        continue;

      if (!m_dataSet.get())
      {
        std::unique_ptr<te::da::DataSetType> dsType = m_layer->getSchema();

        m_dataSet.reset(new te::mem::DataSet(dsType.get()));
      }

      //create dataset item
      te::mem::DataSetItem* item = new te::mem::DataSetItem(m_dataSet.get());

      //fid
      item->setInt32(0, dataset->getInt32("FID"));

      //set id
      item->setInt32(1, dataset->getInt32("id"));

      //set origin id
      item->setInt32(2, dataset->getInt32("originId"));

      //set area
      item->setDouble(3, dataset->getDouble("area"));

      //forest type
      item->setString(4, "REMOVED");

      //set geometry
      item->setGeometry(5, g.release());

      m_dataSet->add(item);

    }
  }
  catch (std::exception& e)
  {
    QMessageBox::critical(m_display, tr("Error"), QString(tr("Error erasing geometry. Details:") + " %1.").arg(e.what()));
    return;
  }

  drawSelecteds();

  //repaint the layer
  m_display->repaint();
}

void geopx::tools::Eraser::removeObjects()
{
  if (!m_layer.get())
    return;

  try
  {
    te::map::DataSetLayer* dsLayer = dynamic_cast<te::map::DataSetLayer*>(m_layer.get());

    if (dsLayer)
    {
      std::unique_ptr<const te::map::LayerSchema> schema(m_layer->getSchema());

      te::da::DataSourcePtr dataSource = te::da::GetDataSource(dsLayer->getDataSourceId());

      if (m_dataSet.get())
      {
        std::vector<size_t> ids;
        ids.push_back(0);

        std::vector< std::set<int> > properties;
        std::size_t dsSize = m_dataSet->size();

        for (std::size_t t = 0; t < dsSize; ++t)
        {
          std::set<int> setPos;
          setPos.insert(4);

          properties.push_back(setPos);
        }

        dataSource->update(schema->getName(), m_dataSet.get(), properties, ids);
      }
    }
  }
  catch (std::exception& e)
  {
    QMessageBox::critical(m_display, tr("Error"), QString(tr("Error erasing geometry. Details:") + " %1.").arg(e.what()));
    return;
  }

  m_dataSet.reset();

  //repaint the layer
  te::qt::widgets::MultiThreadMapDisplay* mtmp = dynamic_cast<te::qt::widgets::MultiThreadMapDisplay*>(m_display);
  if (mtmp)
    mtmp->updateLayer(m_layer);
  else
    m_display->refresh();
}

void geopx::tools::Eraser::drawSelecteds()
{
  if (!m_layer.get())
    return;

  if (!m_dataSet.get())
    return;

  std::size_t gmPos = te::da::GetFirstSpatialPropertyPos(m_dataSet.get());

  // Clear draft!
  QPixmap* draft = m_display->getDraftPixmap();
  draft->fill(Qt::transparent);

  const te::gm::Envelope& displayExtent = m_display->getExtent();

  te::qt::widgets::Canvas canvas(draft);
  canvas.setWindow(displayExtent.m_llx, displayExtent.m_lly, displayExtent.m_urx, displayExtent.m_ury);
  canvas.setRenderHint(QPainter::Antialiasing, true);

  std::size_t size = 24;

  te::se::Stroke* stroke = te::se::CreateStroke("#FF0000", "2", "0.5");
  te::se::Fill* fill = te::se::CreateFill("#FFFFFF", "0.5");
  te::se::Mark* mark = te::se::CreateMark("square", stroke, fill);

  te::color::RGBAColor** rgba = te::map::MarkRendererManager::getInstance().render(mark, size);

  canvas.setPointColor(te::color::RGBAColor(0, 0, 0, TE_TRANSPARENT));
  canvas.setPointPattern(rgba, size, size);

  te::common::Free(rgba, size);
  delete mark;


  try
  {
    // Gets the dataset
    if (m_dataSet.get())
    {
      m_dataSet->moveBeforeFirst();

      while (m_dataSet->moveNext())
      {
        std::unique_ptr<te::gm::Geometry> g(m_dataSet->getGeometry(gmPos));

        if (g->getSRID() == TE_UNKNOWN_SRS)
          g->setSRID(m_layer->getSRID());

        canvas.draw(g.get());
      }
    }
  }
  catch (std::exception& e)
  {
    QMessageBox::critical(m_display, tr("Error"), QString(tr("Error erasing geometry. Details:") + " %1.").arg(e.what()));
    return;
  }
}

void geopx::tools::Eraser::cancelOperation()
{
  // Clear draft!
  QPixmap* draft = m_display->getDraftPixmap();
  draft->fill(Qt::transparent);

  m_dataSet.reset();

  //repaint the layer
  m_display->repaint();
}

bool geopx::tools::Eraser::panMousePressEvent(QMouseEvent* e)
{
  if (e->button() != Qt::MiddleButton)
    return false;

  m_panStarted = true;
  m_origin = e->pos();
  m_delta *= 0;
  m_actionCursor.setShape(Qt::SizeAllCursor);

  // Adjusting the action cursor
  if (m_actionCursor.shape() != Qt::BlankCursor)
    m_display->setCursor(m_actionCursor);

  return true;
}

bool geopx::tools::Eraser::panMouseMoveEvent(QMouseEvent* e)
{
  if (!m_panStarted)
    return false;

  // Calculates the delta value
  m_delta = e->pos() - m_origin;

  // Gets the draft map display pixmap
  QPixmap* draft = m_display->getDraftPixmap();
  draft->fill();

  // Gets the current result of map display, i.e. The draw layer composition.
  QPixmap* result = m_display->getDisplayPixmap();

  // Let's draw!
  QPainter painter(draft);
  painter.drawPixmap(0, 0, *result); // Draw the current result.
  painter.save();
  painter.setOpacity(0.3); // Adjusting transparency feedback.
  painter.drawPixmap(m_delta, *result); // Draw the current result translated.
  painter.restore();

  m_display->repaint();

  return true;
}

bool geopx::tools::Eraser::panMouseReleaseEvent(QMouseEvent* e)
{
  m_panStarted = false;

  // Roll back the default tool cursor
  m_display->setCursor(m_cursor);

  if (e->button() != Qt::MiddleButton || m_delta.isNull())
    return false;

  // Clears the draft map display pixmap
  QPixmap* draft = m_display->getDraftPixmap();
  draft->fill(Qt::transparent);

  // Calculates the extent translated
  QRect rec(0, 0, m_display->width(), m_display->height());
  QPoint center = rec.center();
  center -= m_delta;
  rec.moveCenter(center);

  // Conversion to world coordinates
  QPointF ll(rec.left(), rec.bottom());
  QPointF ur(rec.right(), rec.top());
  ll = m_display->transform(ll);
  ur = m_display->transform(ur);

  // Updates the map display with the new extent
  te::gm::Envelope envelope(ll.x(), ll.y(), ur.x(), ur.y());
  m_display->setExtent(envelope);

  drawSelecteds();

  return true;
}

