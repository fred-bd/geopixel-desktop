/*!
  \file geopx-desktop/src/geopixeltools/qt/tools/Creator.cpp

  \brief This class implements a concrete tool to create points from layer
*/

#include "Creator.h"

// TerraLib
#include <terralib/common/STLUtils.h>
#include <terralib/dataaccess/dataset/DataSet.h>
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

geopx::tools::Creator::Creator(te::qt::widgets::MapDisplay* display, const QCursor& cursor, te::map::AbstractLayerPtr coordLayer, te::map::AbstractLayerPtr parcelLayer, geopx::tools::CreatorType type , QObject* parent)
  : AbstractTool(display, parent),
  m_coordLayer(coordLayer),
  m_parcelLayer(parcelLayer),
  m_starterId(0),
  m_panStarted(false)
{
  getStartIdValue();

  setCursor(cursor);

  display->setFocus();

  m_type = type;
}

geopx::tools::Creator::~Creator()
{
  QPixmap* draft = m_display->getDraftPixmap();
  draft->fill(Qt::transparent);
}

bool geopx::tools::Creator::eventFilter(QObject* watched, QEvent* e)
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
      saveObjects();

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

void geopx::tools::Creator::selectObjects(QMouseEvent* e)
{
  if (!m_coordLayer.get() || !m_parcelLayer.get())
    return;

  QPointF pixelOffset(4.0, 4.0);
#if (QT_VERSION >= 0x050000)
  QPointF qtPoint = e->localPos();
#else
  QPointF qtPoint = e->posF();
#endif

  // Converts point to world coordinates
  QPointF qtWorldPoint = m_display->transform(qtPoint);

  // Bulding the query box
  te::gm::Point* point = new te::gm::Point(qtWorldPoint.x(), qtWorldPoint.y(), m_display->getSRID());

  if ((m_coordLayer->getSRID() != TE_UNKNOWN_SRS) && (m_display->getSRID() != TE_UNKNOWN_SRS) && (m_coordLayer->getSRID() != m_display->getSRID()))
    point->transform(m_coordLayer->getSRID());

  //get parcel parent id
  int parcelId;
  if (!getParcelParentId(point, parcelId))
  {
    delete point;
    return;
  }

  if (!m_dataSet.get())
  {
    std::unique_ptr<te::da::DataSetType> dsType = m_coordLayer->getSchema();

    //create dataset type
    std::unique_ptr<te::da::DataSetType> dataSetType(new te::da::DataSetType(dsType->getName()));

    //create id property
    te::dt::SimpleProperty* idProperty = new te::dt::SimpleProperty("id", te::dt::INT32_TYPE);
    dataSetType->add(idProperty);

    //create origin id property
    te::dt::SimpleProperty* originIdProperty = new te::dt::SimpleProperty("originId", te::dt::INT32_TYPE);
    dataSetType->add(originIdProperty);

    //create area property
    te::dt::SimpleProperty* areaProperty = new te::dt::SimpleProperty("area", te::dt::DOUBLE_TYPE);
    dataSetType->add(areaProperty);

    //create forest type
    te::dt::StringProperty* typeProperty = new te::dt::StringProperty("type");
    dataSetType->add(typeProperty);

    //create geometry property
    te::gm::GeometryProperty* geomProperty = new te::gm::GeometryProperty("geom", m_coordLayer->getSRID(), te::gm::PointType);
    dataSetType->add(geomProperty);

    m_dataSet.reset(new te::mem::DataSet(dataSetType.get()));
  }

  //create dataset item
  te::mem::DataSetItem* item = new te::mem::DataSetItem(m_dataSet.get());

  //set id
  item->setInt32(0, m_starterId); 

  //set origin id
  int originIdPos = te::da::GetPropertyIndex(m_dataSet.get(), "originId");
  item->setInt32(1, parcelId);

  //set area
  int areaId = te::da::GetPropertyIndex(m_dataSet.get(), "area");
  item->setDouble(2, 0.);

  //forest type
  int typeId = te::da::GetPropertyIndex(m_dataSet.get(), "type");

  if (m_type == geopx::tools::CREATED_TYPE)
    item->setString(3, "CREATED");
  else if (m_type == geopx::tools::LIVE_TYPE)
    item->setString(3, "LIVE");
  else if (m_type == geopx::tools::DEAD_TYPE)
    item->setString(3, "DEAD");

  //set geometry
  item->setGeometry(4, point);

  m_dataSet->add(item);

  ++m_starterId;

  drawSelecteds();

  //repaint the layer
  m_display->repaint();
}

void geopx::tools::Creator::saveObjects()
{
  if (!m_coordLayer.get())
    return;

  try
  {
    //remove entries
    if (m_dataSet.get())
    {
      m_dataSet->moveBeforeFirst();

      std::unique_ptr<te::da::DataSetType> dsType = m_coordLayer->getSchema();

      te::map::DataSetLayer* dsLayer = dynamic_cast<te::map::DataSetLayer*>(m_coordLayer.get());

      if (dsLayer)
      {
        te::da::DataSourcePtr dataSource = te::da::GetDataSource(dsLayer->getDataSourceId());

        std::map<std::string, std::string> options;

        dataSource->add(dsType->getName(), m_dataSet.get(), options);
      }
    }

  }
  catch (std::exception& e)
  {
    QMessageBox::critical(m_display, tr("Error"), QString(tr("Error saving geometry. Details:") + " %1.").arg(e.what()));
    return;
  }

  m_dataSet.reset();

  //repaint the layer
  te::qt::widgets::MultiThreadMapDisplay* mtmp = dynamic_cast<te::qt::widgets::MultiThreadMapDisplay*>(m_display);
  if (mtmp)
    mtmp->updateLayer(m_coordLayer);
  else
    m_display->refresh();
}

void geopx::tools::Creator::drawSelecteds()
{
  if (!m_dataSet.get())
    return;

  // Gets the layer schema
  std::size_t gmPos = te::da::GetFirstSpatialPropertyPos(m_dataSet.get());

  // Clear draft!
  QPixmap* draft = m_display->getDraftPixmap();
  draft->fill(Qt::transparent);

  const te::gm::Envelope& displayExtent = m_display->getExtent();

  te::qt::widgets::Canvas canvas(draft);
  canvas.setWindow(displayExtent.m_llx, displayExtent.m_lly, displayExtent.m_urx, displayExtent.m_ury);
  canvas.setRenderHint(QPainter::Antialiasing, true);

  //set visual
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
    m_dataSet->moveBeforeFirst();

    while (m_dataSet->moveNext())
    {
      std::unique_ptr<te::gm::Geometry> g(m_dataSet->getGeometry(gmPos));

      if (g->getSRID() == TE_UNKNOWN_SRS)
        g->setSRID(m_coordLayer->getSRID());

      if (g->getSRID() != m_display->getSRID())
        g->transform(m_display->getSRID());

      canvas.draw(g.get());
    }
  }
  catch (std::exception& e)
  {
    QMessageBox::critical(m_display, tr("Error"), QString(tr("Error erasing geometry. Details:") + " %1.").arg(e.what()));
    return;
  }
}

bool geopx::tools::Creator::getParcelParentId(te::gm::Point* point, int& id)
{
  if (!m_parcelLayer.get())
    throw;

  // Bulding the query box
  te::gm::Envelope envelope(point->getX(), point->getY(), point->getX(), point->getY());

  // Gets the layer schema
  std::unique_ptr<const te::map::LayerSchema> schema(m_parcelLayer->getSchema());

  if (!schema->hasGeom())
    throw;

  te::gm::GeometryProperty* gp = te::da::GetFirstGeomProperty(schema.get());

  te::da::PrimaryKey* pk = schema->getPrimaryKey();
  std::string name = pk->getProperties()[0]->getName();
  
  if (m_coordLayer->getSRID() != m_parcelLayer->getSRID())
    envelope.transform(m_coordLayer->getSRID(), m_parcelLayer->getSRID());

  // Gets the dataset
  std::unique_ptr<te::da::DataSet> dataset = m_parcelLayer->getData(gp->getName(), &envelope, te::gm::INTERSECTS);

  assert(dataset.get());

  dataset->moveBeforeFirst();

  std::unique_ptr<te::gm::Geometry> g;

  while (dataset->moveNext())
  {
    g = dataset->getGeometry(gp->getName());

    if (g->getSRID() == TE_UNKNOWN_SRS)
      g->setSRID(m_parcelLayer->getSRID());

    if (g->getSRID() != point->getSRID())
      g->transform(point->getSRID());

    if (g->covers(point))
    {
      id = dataset->getInt32(name);

      return true;
    }
  }

  return false;
}

void geopx::tools::Creator::getStartIdValue()
{
  if (!m_coordLayer.get())
    throw;

  std::unique_ptr<te::da::DataSet> dataset = m_coordLayer->getData();
  std::unique_ptr<te::da::DataSetType> dataSetType = m_coordLayer->getSchema();

  te::da::PrimaryKey* pk = dataSetType->getPrimaryKey();
  std::string name = pk->getProperties()[0]->getName();

  dataset->moveBeforeFirst();

  while (dataset->moveNext())
  {
    int id = dataset->getInt32(name);

    if (id > m_starterId)
      m_starterId = id;
  }

  ++m_starterId;
}

void geopx::tools::Creator::cancelOperation()
{
  // Clear draft!
  QPixmap* draft = m_display->getDraftPixmap();
  draft->fill(Qt::transparent);

  m_dataSet.reset();

  //repaint the layer
  m_display->repaint();
}

bool geopx::tools::Creator::panMousePressEvent(QMouseEvent* e)
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

bool geopx::tools::Creator::panMouseMoveEvent(QMouseEvent* e)
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

bool geopx::tools::Creator::panMouseReleaseEvent(QMouseEvent* e)
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
