/*!
  \file geopx-desktop/src/geopixeltools/qt/tools/TrackClassifier.cpp

  \brief This class implements a concrete tool to track classifier
*/

#include "TrackClassifier.h"

// TerraLib
#include <terralib/common/STLUtils.h>
#include <terralib/dataaccess/dataset/DataSet.h>
#include <terralib/dataaccess/dataset/DataSetType.h>
#include <terralib/dataaccess/dataset/ObjectId.h>
#include <terralib/dataaccess/utils/Utils.h>
#include <terralib/datatype/StringProperty.h>
#include <terralib/geometry/Geometry.h>
#include <terralib/geometry/GeometryProperty.h>
#include <terralib/geometry/LineString.h>
#include <terralib/geometry/MultiPoint.h>
#include <terralib/geometry/Point.h>
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

// Qt
#include <QApplication>
#include <QtCore/QPointF>
#include <QMessageBox>
#include <QMouseEvent>

// STL
#include <cassert>
#include <memory>

#define DISTANCE_BUFFER 1.5
#define TOLERANCE_FACTOR 0.2
#define ANGLE_TOL 20

geopx::tools::TrackClassifier::TrackClassifier(te::qt::widgets::MapDisplay* display, const QCursor& cursor, te::map::AbstractLayerPtr coordLayer, te::map::AbstractLayerPtr parcelLayer, te::map::AbstractLayerPtr polyLayer, QObject* parent)
  : AbstractTool(display, parent),
  m_coordLayer(coordLayer),
  m_parcelLayer(parcelLayer),
  m_polyLayer(polyLayer),
  m_buffer(0),
  m_track(0),
  m_point0(0),
  m_objId0(0),
  m_point1(0),
  m_objId1(0),
  m_point2(0),
  m_objId2(0),
  m_starterId(0)
{
  setCursor(cursor);
  
  display->setFocus();
  
  createRTree();

  getStartIdValue();
}

geopx::tools::TrackClassifier::~TrackClassifier()
{
  QPixmap* draft = m_display->getDraftPixmap();
  draft->fill(Qt::transparent);

  m_polyRtree.clear();
  te::common::FreeContents(m_polyGeomMap);
  
  m_centroidRtree.clear();
  te::common::FreeContents(m_centroidGeomMap);
  te::common::FreeContents(m_centroidObjIdMap);

  delete m_buffer;

  delete m_point0;
  delete m_point1;
  delete m_point2;
}

bool geopx::tools::TrackClassifier::eventFilter(QObject* watched, QEvent* e)
{
  if (e->type() == QEvent::MouseButtonRelease)
  {
    m_display->setFocus();

    QMouseEvent* event = static_cast<QMouseEvent*>(e);

    if (event->button() == Qt::LeftButton)
    {
      selectObjects(event);

      return true;
    }
  }
  else if (e->type() == QEvent::KeyPress)
  {
    QKeyEvent* event = static_cast<QKeyEvent*>(e);

    if ((event->key() == Qt::Key_Return || event->key() == Qt::Key_Control ) && m_point2)
      classifyObjects();

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

void geopx::tools::TrackClassifier::selectObjects(QMouseEvent* e)
{
  if (!m_coordLayer.get())
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

  if ((m_coordLayer->getSRID() != TE_UNKNOWN_SRS) && (m_display->getSRID() != TE_UNKNOWN_SRS) && (m_coordLayer->getSRID() != m_display->getSRID()))
    reprojectedEnvelope.transform(m_display->getSRID(), m_coordLayer->getSRID());

  if (!reprojectedEnvelope.intersects(m_coordLayer->getExtent()))
    return;

  // Gets the layer schema
  std::unique_ptr<const te::map::LayerSchema> schema(m_coordLayer->getSchema());

  if (!schema->hasGeom())
    return;

  te::da::ObjectIdSet* oids = 0;

  try
  {
    te::gm::GeometryProperty* gp = te::da::GetFirstGeomProperty(schema.get());

    // Gets the dataset
    std::unique_ptr<te::da::DataSet> dataset = m_coordLayer->getData(gp->getName(), &reprojectedEnvelope, te::gm::INTERSECTS);
    assert(dataset.get());

    // Let's generate the oid
    std::vector<std::string> pnames;
    te::da::GetOIDPropertyNames(schema.get(), pnames);

    // Generates a geometry from the given extent. It will be used to refine the results
    std::unique_ptr<te::gm::Geometry> geometryFromEnvelope(te::gm::GetGeomFromEnvelope(&reprojectedEnvelope, m_coordLayer->getSRID()));

    while (dataset->moveNext())
    {
      std::unique_ptr<te::gm::Geometry> g(dataset->getGeometry(gp->getName()));

      if (g->getSRID() == TE_UNKNOWN_SRS)
        g->setSRID(m_coordLayer->getSRID());

      if (!g->intersects(geometryFromEnvelope.get()))
        continue;

      if (!m_point0)
      {
        m_point0 = getPoint(dynamic_cast<te::gm::Geometry*>(g->clone()));
        m_objId0 = te::da::GenerateOID(dataset.get(), pnames);
        break;
      }

      if (!m_point1)
      {
        m_point1 = getPoint(dynamic_cast<te::gm::Geometry*>(g->clone()));
        m_objId1 = te::da::GenerateOID(dataset.get(), pnames);
        break;
      }

      if (!m_point2)
      {
        m_point2 = getPoint(dynamic_cast<te::gm::Geometry*>(g->clone()));
        m_objId2 = te::da::GenerateOID(dataset.get(), pnames);
        break;
      }
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

void geopx::tools::TrackClassifier::classifyObjects()
{
  if (!m_coordLayer.get())
    return;

  if (!m_buffer)
    return;

  QApplication::setOverrideCursor(Qt::WaitCursor);

  try
  {
    //create dataset type
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

    te::mem::DataSet* liveDS = 0;
    te::mem::DataSet* intruderDS = 0;

    getClassDataSets(dsType.get(), liveDS, intruderDS);

    //class
    te::map::DataSetLayer* dsLayer = dynamic_cast<te::map::DataSetLayer*>(m_coordLayer.get());

    if (dsLayer)
    {
      std::unique_ptr<const te::map::LayerSchema> schema(m_coordLayer->getSchema());

      te::da::DataSourcePtr dataSource = te::da::GetDataSource(dsLayer->getDataSourceId());

      //update live dataset
      std::vector<size_t> ids;
      ids.push_back(0);

      if (liveDS)
      {
        std::vector< std::set<int> > properties;
        std::size_t dsSize = liveDS->size();

        for (std::size_t t = 0; t < dsSize; ++t)
        {
          std::set<int> setPos;
          setPos.insert(4);

          properties.push_back(setPos);
        }

        dataSource->update(schema->getName(), liveDS, properties, ids);
      }

      //update intruder dataset
      if (intruderDS)
      {
        std::vector< std::set<int> > properties;
        std::size_t dsSize = intruderDS->size();

        for (std::size_t t = 0; t < dsSize; ++t)
        {
          std::set<int> setPos;
          setPos.insert(4);

          properties.push_back(setPos);
        }

        dataSource->update(schema->getName(), intruderDS, properties, ids);
      }

      //add dead dataset
      if (m_dataSet.get())
      {
        std::map<std::string, std::string> options;

        dataSource->add(schema->getName(), m_dataSet.get(), options);
      }
    }
  }
  catch (std::exception& e)
  {
    QApplication::restoreOverrideCursor();

    QMessageBox::critical(m_display, tr("Error"), QString(tr("Error classifing track. Details:") + " %1.").arg(e.what()));
    return;
  }

  QApplication::restoreOverrideCursor();

  delete m_point0;
  m_point0 = 0;
  delete m_objId0;
  m_objId0 = 0;

  delete m_point1;
  m_point1 = 0;
  delete m_objId1;
  m_objId1 = 0;

  delete m_point2;
  m_point2 = 0;
  delete m_objId2;
  m_objId2 = 0;

  createRTree();

  m_dataSet.reset();

  //repaint the layer
  m_display->refresh();
}

void geopx::tools::TrackClassifier::cancelOperation()
{
  // Clear draft!
  QPixmap* draft = m_display->getDraftPixmap();
  draft->fill(Qt::transparent);

  delete m_point0;
  m_point0 = 0;
  delete m_objId0;
  m_objId0 = 0;

  delete m_point1;
  m_point1 = 0;
  delete m_objId1;
  m_objId1 = 0;

  delete m_point2;
  m_point2 = 0;
  delete m_objId2;
  m_objId2 = 0;

  m_dataSet.reset();

  std::unique_ptr<te::da::DataSetType> schema = m_coordLayer->getSchema();

  te::da::GetEmptyOIDSet(schema.get(), m_track);

  //repaint the layer
  m_display->repaint();
}

void geopx::tools::TrackClassifier::drawSelecteds()
{
  if (!m_coordLayer.get())
    return;

  // Gets the layer schema
  std::unique_ptr<const te::map::LayerSchema> schema(m_coordLayer->getSchema());

  te::gm::GeometryProperty* gp = te::da::GetFirstGeomProperty(schema.get());

  // Clear draft!
  QPixmap* draft = m_display->getDraftPixmap();
  draft->fill(Qt::transparent);

  const te::gm::Envelope& displayExtent = m_display->getExtent();

  te::qt::widgets::Canvas canvas(draft);
  canvas.setWindow(displayExtent.m_llx, displayExtent.m_lly, displayExtent.m_urx, displayExtent.m_ury);
  canvas.setRenderHint(QPainter::Antialiasing, true);

  //configure for polygons
  canvas.setPolygonContourWidth(1);
  canvas.setPolygonContourColor(te::color::RGBAColor(0, 0, 0, 128));
  canvas.setPolygonFillColor(te::color::RGBAColor(255, 255, 255, 128));
   
  //configure for lines
  canvas.setLineColor(te::color::RGBAColor(255, 0, 0, 128));
  canvas.setLineWidth(6);
   
  //configure for points
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
    if (m_point0)
      canvas.draw(m_point0);

    if (m_point1)
      canvas.draw(m_point1);

    if (m_point2)
      canvas.draw(m_point2);

    if (m_point2)
    {
      delete m_buffer;

      te::gm::LineString* line = 0;

      std::list<te::gm::Point*> track;

      m_buffer = createBuffer(m_coordLayer->getSRID(), gp->getName(), line, track);

      if (m_buffer)
      {
        te::gm::Polygon* poly = dynamic_cast<te::gm::Polygon*>(m_buffer);

        if (poly && poly->isValid() && poly->getNumRings() > 0)
        {
          canvas.draw(line);

          canvas.draw(m_buffer);

          for (std::list<te::gm::Point*>::iterator it = track.begin(); it != track.end(); ++it)
          {
            canvas.draw(*it);
          }
        }
      }
    }
  }
  catch (std::exception& e)
  {
    QMessageBox::critical(m_display, tr("Error"), QString(tr("Error creating buffer. Details:") + " %1.").arg(e.what()));
    return;
  }
}

te::gm::Geometry* geopx::tools::TrackClassifier::createBuffer(int srid, std::string gpName, te::gm::LineString*& lineBuffer, std::list<te::gm::Point*>& track)
{
  std::unique_ptr<te::da::DataSetType> schema = m_coordLayer->getSchema();

  std::vector<std::string> pnames;
  te::da::GetOIDPropertyNames(schema.get(), pnames);
  te::da::ObjectId* objIdRoot = m_objId0;

  //get sample info
  double distance, dx, dy;

  getTrackInfo(distance, dx, dy);

  //get parcel geom
  int parcelId;
  std::unique_ptr<te::gm::Geometry> parcelGeom = getParcelGeeom(m_point0, parcelId);

  parcelGeom->setSRID(srid);

  te::da::GetEmptyOIDSet(schema.get(), m_track);

  bool insideParcel = parcelGeom->covers(m_point0);

  te::gm::Point* rootPoint = m_point0;
  
  track.push_back(new te::gm::Point(*rootPoint));

  te::gm::Point* starter = new te::gm::Point(*rootPoint);

  m_track->add(objIdRoot);

  bool invert = false;

  QApplication::setOverrideCursor(Qt::WaitCursor);

  while (insideParcel)
  {
    //te::gm::Point* guestPoint = createGuessPoint(rootPoint, distance, angle, srid);

    te::gm::Point* guestPoint = createGuessPoint(rootPoint, dx, dy, srid);
    
    //create envelope to find if guest point exist
    te::gm::Envelope ext(guestPoint->getX(), guestPoint->getY(), guestPoint->getX(), guestPoint->getY());

    ext.m_llx -= (distance * TOLERANCE_FACTOR);
    ext.m_lly -= (distance * TOLERANCE_FACTOR);
    ext.m_urx += (distance * TOLERANCE_FACTOR);
    ext.m_ury += (distance * TOLERANCE_FACTOR);

    //check on tree
    std::vector<int> resultsTree;

    m_centroidRtree.search(ext, resultsTree);

    if (resultsTree.empty())
    {
      //dead point
      rootPoint = guestPoint;

      insideParcel = parcelGeom->covers(rootPoint);

      if (insideParcel)
      {
        if (!invert)
        {
          track.push_back(new te::gm::Point(*rootPoint));
        }
        else
        {
          track.push_front(new te::gm::Point(*rootPoint));
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

        std::string forestType = "UNKNOWN";

        te::gm::Envelope extPoint(guestPoint->getX(), guestPoint->getY(), guestPoint->getX(), guestPoint->getY());

        std::vector<int> resultsPolyTree;

        m_polyRtree.search(extPoint, resultsPolyTree);

        if (resultsPolyTree.empty())
        {
          forestType = "DEAD";
        }
        else
        {
          //bool found = false;
          //for (std::size_t t = 0; t < resultsPolyTree.size(); ++t)
          //{
          //  std::map<int, te::gm::Geometry*>::iterator it = m_polyGeomMap.find(resultsPolyTree[t]);

          //  bool covers = false;

          //  if (it->second->isValid())
          //    covers = it->second->covers(guestPoint);

          //  if (covers)
          //  {
          //    found = true;
          //    break;
          //  }
          //}

          //if (found)
          //  forestType = "LIVE";
          //else
            forestType = "LIVE";
        }

        //create dataset item
        te::mem::DataSetItem* item = new te::mem::DataSetItem(m_dataSet.get());

        //set id
        item->setInt32(0, m_starterId);

        //set origin id
        item->setInt32(1, parcelId);

        //set area
        item->setDouble(2, 0.);

        //forest type
        item->setString(3, forestType);

        //set geometry
        item->setGeometry(4, new te::gm::Point(*rootPoint));

        m_dataSet->add(item);

        ++m_starterId;

      }
      else
      {
        if (!invert)
        {
          invert = true;
          insideParcel = true;
          dx = dx * -1;
          dy = dy * -1;
          rootPoint = starter;
        }
      }
       
    }
    else
    {
      //live point
      te::gm::Point* pCandidate = 0;

      bool found = false;

      double lowerDistance = std::numeric_limits<double>::max();
      te::gm::Point* newCandidate = 0;
      te::da::ObjectId* newObjIdCandidate = 0;

      for (std::size_t t = 0; t < resultsTree.size(); ++t)
      {
        std::map<int, te::gm::Geometry*>::iterator it = m_centroidGeomMap.find(resultsTree[t]);

        std::map<int, te::da::ObjectId*>::iterator itObjId = m_centroidObjIdMap.find(resultsTree[t]);

        if (!isClassified(itObjId->second))
        {
          pCandidate = getPoint(it->second);

          pCandidate->setSRID(srid);

          if (rootPoint->getX() != pCandidate->getX() || rootPoint->getY() != pCandidate->getY())
          {
            //check for lower distance from guest point
            double dist = std::abs(guestPoint->distance(pCandidate));

            if (dist < lowerDistance)
            {
              lowerDistance = dist;
              newCandidate = pCandidate;
              newObjIdCandidate = itObjId->second;
              found = true;
            }
          }
        }
      }

      if (!found)
      {
        rootPoint = guestPoint;
      }
      else
      {
        rootPoint = newCandidate;
        m_track->add(newObjIdCandidate);
      }

      insideParcel = parcelGeom->covers(rootPoint);

      if (insideParcel)
      {
        if (!invert)
        {
          track.push_back(new te::gm::Point(*rootPoint));
        }
        else
        {
          track.push_front(new te::gm::Point(*rootPoint));
        }
      }
      else
      {
        if (!invert)
        {
          invert = true;
          insideParcel = true;
          dx = dx * -1;
          dy = dy * -1;
          rootPoint = starter;
        }
      }
    }
  }

  QApplication::restoreOverrideCursor();

  delete starter;

  if (track.size() < 2)
  {
    return 0;
  }

  //create buffer
  lineBuffer = new te::gm::LineString(track.size(), te::gm::LineStringType, srid);

  int count = 0;
  std::list<te::gm::Point*>::iterator it = track.begin();
  while (it != track.end())
  {
    lineBuffer->setPoint(count, (*it)->getX(), (*it)->getY());

    ++count;
    ++it;
  }

  return lineBuffer->buffer(DISTANCE_BUFFER, 16, te::gm::CapButtType);
}

void geopx::tools::TrackClassifier::getTrackInfo(double& distance, double& dx, double& dy)
{
  distance = m_point0->distance(m_point1);
  double bigDistance = m_point0->distance(m_point2);

  double big_dx = m_point2->getX() - m_point0->getX();
  double big_dy = m_point2->getY() - m_point0->getY();

  dx = distance * big_dx / bigDistance;
  dy = distance * big_dy / bigDistance;
}

std::unique_ptr<te::gm::Geometry> geopx::tools::TrackClassifier::getParcelGeeom(te::gm::Geometry* root, int& parcelId)
{
  if (!m_parcelLayer.get())
    throw;

  // Bulding the query box
  te::gm::Envelope envelope(*root->getMBR());

  te::gm::Envelope reprojectedEnvelope(envelope);

  if ((m_parcelLayer->getSRID() != TE_UNKNOWN_SRS) && (m_display->getSRID() != TE_UNKNOWN_SRS) && (m_parcelLayer->getSRID() != m_display->getSRID()))
    reprojectedEnvelope.transform(m_display->getSRID(), m_parcelLayer->getSRID());

  if (!reprojectedEnvelope.intersects(m_parcelLayer->getExtent()))
    throw;

  // Gets the layer schema
  std::unique_ptr<const te::map::LayerSchema> schema(m_parcelLayer->getSchema());

  if (!schema->hasGeom())
    throw;

  te::gm::GeometryProperty* gp = te::da::GetFirstGeomProperty(schema.get());

  te::da::PrimaryKey* pk = schema->getPrimaryKey();
  std::string name = pk->getProperties()[0]->getName();

  // Gets the dataset
  std::unique_ptr<te::da::DataSet> dataset = m_parcelLayer->getData(gp->getName(), &reprojectedEnvelope, te::gm::INTERSECTS);

  assert(dataset.get());

  dataset->moveBeforeFirst();

  std::unique_ptr<te::gm::Geometry> g;

  while (dataset->moveNext())
  {
    g = dataset->getGeometry(gp->getName());

    if (g->getSRID() == TE_UNKNOWN_SRS)
      g->setSRID(m_coordLayer->getSRID());

    if (g->covers(root))
    {
      parcelId = dataset->getInt32(name);

      break;
    }
  }

  return g;
}

te::gm::Point* geopx::tools::TrackClassifier::createGuessPoint(te::gm::Point* p, double dx, double dy, int srid)
{
  return new te::gm::Point(p->getX() + dx, p->getY() + dy, srid);
}

te::da::ObjectIdSet* geopx::tools::TrackClassifier::getBufferObjIdSet()
{
  // Bulding the query box
  te::gm::Envelope envelope(*m_buffer->getMBR());

  te::gm::Envelope reprojectedEnvelope(envelope);

  if ((m_coordLayer->getSRID() != TE_UNKNOWN_SRS) && (m_display->getSRID() != TE_UNKNOWN_SRS) && (m_coordLayer->getSRID() != m_display->getSRID()))
    reprojectedEnvelope.transform(m_display->getSRID(), m_coordLayer->getSRID());

  if (!reprojectedEnvelope.intersects(m_coordLayer->getExtent()))
    throw;

  // Gets the layer schema
  std::unique_ptr<const te::map::LayerSchema> schema(m_coordLayer->getSchema());

  if (!schema->hasGeom())
    throw;

  te::da::ObjectIdSet* oids = 0;

  try
  {
    te::gm::GeometryProperty* gp = te::da::GetFirstGeomProperty(schema.get());

    // Gets the dataset
    std::unique_ptr<te::da::DataSet> dataset = m_coordLayer->getData(gp->getName(), &reprojectedEnvelope, te::gm::INTERSECTS);
    assert(dataset.get());

    // Let's generate the oids
    te::da::GetEmptyOIDSet(schema.get(), oids);
    assert(oids);

    std::vector<std::string> pnames;
    te::da::GetOIDPropertyNames(schema.get(), pnames);

    // Generates a geometry from the given extent. It will be used to refine the results
    std::unique_ptr<te::gm::Geometry> geometryFromEnvelope(te::gm::GetGeomFromEnvelope(&reprojectedEnvelope, m_coordLayer->getSRID()));

    while (dataset->moveNext())
    {
      std::unique_ptr<te::gm::Geometry> g(dataset->getGeometry(gp->getName()));

      if (g->getSRID() == TE_UNKNOWN_SRS)
        g->setSRID(m_coordLayer->getSRID());

      if (!m_buffer->contains(g.get()))
        continue;

      // Feature found
      oids->add(te::da::GenerateOID(dataset.get(), pnames));
    }
  }
  catch (std::exception& e)
  {
    QMessageBox::critical(m_display, tr("Error"), QString(tr("Error getting geometry. Details:") + " %1.").arg(e.what()));
    throw;
  }

  return oids;
}

void geopx::tools::TrackClassifier::getClassDataSets(te::da::DataSetType* dsType, te::mem::DataSet*& liveDataSet, te::mem::DataSet*& intruderDataSet)
{
  // Bulding the query box
  te::gm::Envelope envelope(*m_buffer->getMBR());

  te::gm::Envelope reprojectedEnvelope(envelope);

  if ((m_coordLayer->getSRID() != TE_UNKNOWN_SRS) && (m_display->getSRID() != TE_UNKNOWN_SRS) && (m_coordLayer->getSRID() != m_display->getSRID()))
    reprojectedEnvelope.transform(m_display->getSRID(), m_coordLayer->getSRID());

  if (!reprojectedEnvelope.intersects(m_coordLayer->getExtent()))
    throw;

  // Gets the layer schema
  std::unique_ptr<const te::map::LayerSchema> schema(m_coordLayer->getSchema());

  if (!schema->hasGeom())
    throw;

  try
  {
    te::gm::GeometryProperty* gp = te::da::GetFirstGeomProperty(schema.get());

    // Gets the dataset
    std::unique_ptr<te::da::DataSet> dataset = m_coordLayer->getData(gp->getName(), &reprojectedEnvelope, te::gm::INTERSECTS);
    assert(dataset.get());

    std::vector<std::string> pnames;
    te::da::GetOIDPropertyNames(schema.get(), pnames);

    // Generates a geometry from the given extent. It will be used to refine the results
    std::unique_ptr<te::gm::Geometry> geometryFromEnvelope(te::gm::GetGeomFromEnvelope(&reprojectedEnvelope, m_coordLayer->getSRID()));

    while (dataset->moveNext())
    {
      std::unique_ptr<te::gm::Geometry> g(dataset->getGeometry(gp->getName()));

      if (g->getSRID() == TE_UNKNOWN_SRS)
        g->setSRID(m_coordLayer->getSRID());

      if (!m_buffer->contains(g.get()))
        continue;

      // Feature found
      te::da::ObjectId* objId = te::da::GenerateOID(dataset.get(), pnames);

      if (m_track->contains(objId))//live
      {
        if (!liveDataSet)
        {
          liveDataSet = new te::mem::DataSet(dsType);
        }

        //create dataset item
        te::mem::DataSetItem* item = new te::mem::DataSetItem(liveDataSet);

        //fid
        item->setInt32(0, dataset->getInt32("FID"));

        //set id
        item->setInt32(1, dataset->getInt32("id"));

        //set origin id
        item->setInt32(2, dataset->getInt32("originId"));

        //set area
        item->setDouble(3, dataset->getDouble("area"));

        //forest type
        item->setString(4, "LIVE");

        //set geometry
        item->setGeometry(5, g.release());

        liveDataSet->add(item);
      }
      else//intruder
      {
        if (!intruderDataSet)
        {
          intruderDataSet = new te::mem::DataSet(dsType);
        }

        //create dataset item
        te::mem::DataSetItem* item = new te::mem::DataSetItem(intruderDataSet);

        //fid
        item->setInt32(0, dataset->getInt32("FID"));

        //set id
        item->setInt32(1, dataset->getInt32("id"));

        //set origin id
        item->setInt32(2, dataset->getInt32("originId"));

        //set area
        item->setDouble(3, dataset->getDouble("area"));

        //forest type
        item->setString(4, "INTRUDER");

        //set geometry
        item->setGeometry(5, g.release());

        intruderDataSet->add(item);
      }
    }
  }
  catch (std::exception& e)
  {
    QMessageBox::critical(m_display, tr("Error"), QString(tr("Error getting geometry. Details:") + " %1.").arg(e.what()));
    throw;
  }
}

void geopx::tools::TrackClassifier::createRTree()
{
  QApplication::setOverrideCursor(Qt::WaitCursor);

  te::common::FreeContents(m_centroidGeomMap);
  te::common::FreeContents(m_centroidObjIdMap);

  m_centroidRtree.clear();
  m_centroidGeomMap.clear();
  m_centroidObjIdMap.clear();

  //create rtree
  std::unique_ptr<const te::map::LayerSchema> schema(m_coordLayer->getSchema());
  std::unique_ptr<te::da::DataSet> ds(m_coordLayer->getData());

  std::vector<std::string> pnames;
  te::da::GetOIDPropertyNames(schema.get(), pnames);

  //geom property info
  te::gm::GeometryProperty* gmProp = te::da::GetFirstGeomProperty(schema.get());

  int geomIdx = te::da::GetPropertyPos(schema.get(), gmProp->getName());

  //id info
  te::da::PrimaryKey* pk = schema->getPrimaryKey();

  int idIdx = te::da::GetPropertyPos(schema.get(), pk->getProperties()[0]->getName());

  ds->moveBeforeFirst();

  while (ds->moveNext())
  {
    std::string strId = ds->getAsString(idIdx);

    int id = atoi(strId.c_str());

    te::gm::Geometry* g = ds->getGeometry(geomIdx).release();
    const te::gm::Envelope* box = g->getMBR();

    m_centroidRtree.insert(*box, id);

    m_centroidGeomMap.insert(std::map<int, te::gm::Geometry*>::value_type(id, g));

    m_centroidObjIdMap.insert(std::map<int, te::da::ObjectId*>::value_type(id, te::da::GenerateOID(ds.get(), pnames)));
  }

  //create polygons rtree
  if (m_polyGeomMap.empty())
  {
    std::unique_ptr<const te::map::LayerSchema> schema(m_polyLayer->getSchema());
    std::unique_ptr<te::da::DataSet> ds(m_polyLayer->getData());

    //geom property info
    te::gm::GeometryProperty* gmProp = te::da::GetFirstGeomProperty(schema.get());

    int geomIdx = te::da::GetPropertyPos(schema.get(), gmProp->getName());

    //id info
    te::da::PrimaryKey* pk = schema->getPrimaryKey();

    int idIdx = te::da::GetPropertyPos(schema.get(), pk->getProperties()[0]->getName());

    ds->moveBeforeFirst();

    while (ds->moveNext())
    {
      std::string strId = ds->getAsString(idIdx);

      int id = atoi(strId.c_str());

      te::gm::Geometry* g = ds->getGeometry(geomIdx).release();
      const te::gm::Envelope* box = g->getMBR();

      m_polyRtree.insert(*box, id);

      m_polyGeomMap.insert(std::map<int, te::gm::Geometry*>::value_type(id, g));
    }
  }

  QApplication::restoreOverrideCursor();
}

te::gm::Point* geopx::tools::TrackClassifier::getPoint(te::gm::Geometry* g)
{
  te::gm::Point* point = 0;

  if (g->getGeomTypeId() == te::gm::MultiPointType)
  {
    te::gm::MultiPoint* mPoint = dynamic_cast<te::gm::MultiPoint*>(g);
    point = dynamic_cast<te::gm::Point*>(mPoint->getGeometryN(0));
  }
  else if (g->getGeomTypeId() == te::gm::PointType)
  {
    point = dynamic_cast<te::gm::Point*>(g);
  }

  return point;
}

void geopx::tools::TrackClassifier::getStartIdValue()
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

bool geopx::tools::TrackClassifier::isClassified(te::da::ObjectId* objId)
{
  if (!m_coordLayer.get())
    throw;

  std::unique_ptr<te::da::DataSetType> schema = m_coordLayer->getSchema();

  te::da::ObjectIdSet* objIdSet;

  te::da::GetEmptyOIDSet(schema.get(), objIdSet);

  objIdSet->add(objId);

  std::unique_ptr<te::da::DataSet> ds = m_coordLayer->getData(objIdSet);

  if (!ds->isEmpty())
  {
    ds->moveFirst();

    std::string classValue = ds->getString("type");

    if (classValue != "UNKNOWN" && classValue != "CREATED")
      return true;
  }

  return false;
}
