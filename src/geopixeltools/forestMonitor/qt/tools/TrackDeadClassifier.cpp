/*!
  \file geopx-desktop/src/geopixeltools/qt/tools/TrackDeadClassifier.cpp

  \brief This class implements a concrete tool to track classifier
*/

#include "TrackDeadClassifier.h"

// TerraLib
#include <terralib/common/STLUtils.h>
#include <terralib/common/progress/TaskProgress.h>
#include <terralib/dataaccess/dataset/DataSet.h>
#include <terralib/dataaccess/dataset/DataSetType.h>
#include <terralib/dataaccess/dataset/ObjectId.h>
#include <terralib/dataaccess/query_h.h>
#include <terralib/dataaccess/utils/Utils.h>
#include <terralib/datatype/StringProperty.h>
#include <terralib/geometry/Geometry.h>
#include <terralib/geometry/GeometryProperty.h>
#include <terralib/geometry/LineString.h>
#include <terralib/geometry/MultiLineString.h>
#include <terralib/geometry/MultiPoint.h>
#include <terralib/geometry/MultiPolygon.h>
#include <terralib/geometry/Point.h>
#include <terralib/geometry/Utils.h>
#include <terralib/maptools/DataSetLayer.h>
#include <terralib/maptools/MarkRendererManager.h>
#include <terralib/memory/DataSetItem.h>
#include <terralib/raster/Grid.h>
#include <terralib/raster/Raster.h>
#include <terralib/se/Fill.h>
#include <terralib/se/Stroke.h>
#include <terralib/se/Mark.h>
#include <terralib/se/Utils.h>
#include <terralib/qt/widgets/canvas/Canvas.h>
#include <terralib/qt/widgets/canvas/MapDisplay.h>
#include <terralib/qt/widgets/canvas/MultiThreadMapDisplay.h>


// Qt
#include <QApplication>
#include <QtCore/QPointF>
#include <QMessageBox>
#include <QMouseEvent>

// STL
#include <cassert>
#include <memory>

#define DISTANCE 2.0
#define DISTANCE_TRACK 3.0
#define TOLERANCE_FACTOR 0.2
#define TRACK_TOLERANCE_FACTOR 0.3
#define POLY_AREA_MIN 0.1
#define POLY_AREA_MAX 2.0
#define DELTA_TOL 0.1
#define NDVI_THRESHOLD 120.0

double TePerpendicularDistance(const te::gm::Point& first, const te::gm::Point& last, te::gm::Point& pin, te::gm::Point*& pinter);

geopx::tools::TrackDeadClassifier::TrackDeadClassifier(te::qt::widgets::MapDisplay* display, const QCursor& cursor, te::map::AbstractLayerPtr coordLayer, te::map::AbstractLayerPtr parcelLayer, te::map::AbstractLayerPtr rasterLayer, QObject* parent)
  : AbstractTool(display, parent),
  m_coordLayer(coordLayer),
  m_parcelLayer(parcelLayer),
  m_track(0),
  m_point0(0),
  m_objId0(0),
  m_point1(0),
  m_starterId(0),
  m_panStarted(false)
{
  m_distLineEdit = 0;
  m_distanceTrackLineEdit = 0;
  m_distanceToleranceFactorLineEdit = 0;

  m_dx = 0.;
  m_dy = 0.;
  m_distance = DISTANCE;

  m_deadCount = 0;
  m_deltaTol = DELTA_TOL;

  m_classify = false;

  setCursor(cursor);
  
  display->setFocus();
  
  createRTree();

  getStartIdValue();

  //get raster
  std::unique_ptr<te::da::DataSet> ds = rasterLayer->getData();

  m_ndviRaster = ds->getRaster(0).release();

  m_totalDistance = 0;
}

geopx::tools::TrackDeadClassifier::~TrackDeadClassifier()
{
  QPixmap* draft = m_display->getDraftPixmap();
  draft->fill(Qt::transparent);

  m_centroidRtree.clear();
  te::common::FreeContents(m_centroidGeomMap);
  te::common::FreeContents(m_centroidObjIdMap);

  delete m_point0;
  delete m_point1;

  delete m_ndviRaster;
}

void geopx::tools::TrackDeadClassifier::setLineEditComponents(QLineEdit* distLineEdit, QLineEdit* distanceTrackLineEdit, QLineEdit* distanceToleranceFactorLineEdit, QLineEdit* distanceTrackToleranceFactorLineEdit, QLineEdit* polyAreaMin, QLineEdit* polyAreaMax, QLineEdit* deadTol, QLineEdit* threshold)
{
  m_distLineEdit = distLineEdit;
  m_distanceTrackLineEdit = distanceTrackLineEdit;
  m_distanceToleranceFactorLineEdit = distanceToleranceFactorLineEdit;
  m_distanceTrackToleranceFactorLineEdit = distanceTrackToleranceFactorLineEdit;
  m_polyAreaMin = polyAreaMin;
  m_polyAreaMax = polyAreaMax;
  m_deadTolLineEdit = deadTol;
  m_thresholdLineEdit = threshold;
}



bool geopx::tools::TrackDeadClassifier::eventFilter(QObject* watched, QEvent* e)
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

    panMouseMoveEvent(event);

    deadTrackMouseMove(event);

    return true;
  }
  else if (e->type() == QEvent::KeyPress)
  {
    QKeyEvent* event = static_cast<QKeyEvent*>(e);

    if ((event->key() == Qt::Key_Return || event->key() == Qt::Key_Control) && m_point1 && m_point0)
      classifyObjects();

    if (event->key() == Qt::Key_Escape || event->key() == Qt::Key_Backspace)
      cancelOperation();

    if (event->key() == Qt::Key_Delete && m_point1 && m_point0)
      deleteOperation();

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

void geopx::tools::TrackDeadClassifier::selectObjects(QMouseEvent* e)
{
  if (!m_coordLayer.get())
    return;

  QPointF pixelOffset(7.0, 7.0);
#if (QT_VERSION >= 0x050000)
  QPointF qtPoint = e->localPos();
  QRectF rect = QRectF(e->localPos() - pixelOffset, e->localPos() + pixelOffset);
#else
  QPointF qtPoint = e->posF();
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

    te::gm::Point* pClicked = 0;

    te::da::ObjectId* objId = 0;

    if (dataset->isEmpty())
    {
      QPointF qtWorldPoint = m_display->transform(qtPoint);

      pClicked = new te::gm::Point(qtWorldPoint.x(), qtWorldPoint.y(), m_display->getSRID());

      if ((m_coordLayer->getSRID() != TE_UNKNOWN_SRS) && (m_display->getSRID() != TE_UNKNOWN_SRS) && (m_coordLayer->getSRID() != m_display->getSRID()))
        pClicked->transform(m_coordLayer->getSRID());
    }
    else
    {
      while (dataset->moveNext())
      {
        std::unique_ptr<te::gm::Geometry> g(dataset->getGeometry(gp->getName()));

        if (g->getSRID() == TE_UNKNOWN_SRS)
          g->setSRID(m_coordLayer->getSRID());

        if (!g->intersects(geometryFromEnvelope.get()))
          continue;

        pClicked = getPoint(dynamic_cast<te::gm::Geometry*>(g->clone()));
        objId = te::da::GenerateOID(dataset.get(), pnames);

        break;
      }
    }

    if (pClicked)
    {
      if (!m_point0)
      {
        m_point0 = pClicked;
        m_objId0 = objId;
      }
      else if (!m_point1)
      {
        m_point1 = pClicked;
        m_totalDistance = m_point0->distance(m_point1);

        delete objId;
      }
      else
      {
        delete pClicked;
        delete objId;
      }
    }
  }
  catch (std::exception& e)
  {
    QMessageBox::critical(m_display, tr("Error"), QString(tr("Error erasing geometry. Details:") + " %1.").arg(e.what()));
    return;
  }

  drawSelecteds();

  m_display->repaint();
}

void geopx::tools::TrackDeadClassifier::classifyObjects()
{
  processDataSet();

  //clear data
  cancelOperation();

  //repaint the layer
  te::qt::widgets::MultiThreadMapDisplay* mtmp = dynamic_cast<te::qt::widgets::MultiThreadMapDisplay*>(m_display);
  if (mtmp)
    mtmp->updateLayer(m_coordLayer);
  else
    m_display->repaint();
}

void geopx::tools::TrackDeadClassifier::cancelOperation()
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

  m_classify = false;

  m_dataSet.reset();

  std::unique_ptr<te::da::DataSetType> schema = m_coordLayer->getSchema();

  te::da::GetEmptyOIDSet(schema.get(), m_track);

  //repaint the layer
  m_display->repaint();
}

void geopx::tools::TrackDeadClassifier::deleteOperation()
{
  std::unique_ptr<te::da::DataSetType> dsType = m_coordLayer->getSchema();

  //create dataset type
  m_dataSet.reset(new te::mem::DataSet(dsType.get()));
  

  //get dead tolerance info
  if (m_deadTolLineEdit->text().isEmpty())
  {
    m_deltaTol = DELTA_TOL;
  }
  else
  {
    m_deltaTol = m_deadTolLineEdit->text().toDouble();
  }

  //get distance buffer info
  double distanceTrack = 0.;

  if (m_distanceTrackLineEdit->text().isEmpty())
  {
    distanceTrack = DISTANCE_TRACK;
  }
  else
  {
    distanceTrack = m_distanceTrackLineEdit->text().toDouble();
  }

  double distanceTrackTol = 0.;

  if (m_distanceTrackToleranceFactorLineEdit->text().isEmpty())
  {
    distanceTrackTol = TRACK_TOLERANCE_FACTOR;
  }
  else
  {
    distanceTrackTol = m_distanceTrackToleranceFactorLineEdit->text().toDouble();
  }

  //create line buffer
  std::unique_ptr<te::gm::LineString> lineSearchBuffer(new te::gm::LineString(2, te::gm::LineStringType, m_coordLayer->getSRID()));
  lineSearchBuffer->setPoint(0, m_point0->getX(), m_point0->getY());
  lineSearchBuffer->setPoint(1, m_point1->getX(), m_point1->getY());
  te::gm::Geometry* geomLineSearchBuffer = lineSearchBuffer->buffer(distanceTrack / 2., 4, te::gm::CapButtType);

  //check on tree
  te::gm::Envelope ext(*geomLineSearchBuffer->getMBR());

  std::vector<int> resultsTreeObjs;

  m_centroidRtree.search(ext, resultsTreeObjs);

  std::unique_ptr<const te::map::LayerSchema> schema(m_coordLayer->getSchema());

  te::gm::GeometryProperty* gp = te::da::GetFirstGeomProperty(schema.get());

  for (std::size_t t = 0; t < resultsTreeObjs.size(); ++t)
  {
    std::map<int, te::gm::Geometry*>::iterator it = m_centroidGeomMap.find(resultsTreeObjs[t]);

    if (it != m_centroidGeomMap.end())
    {
      te::gm::Geometry* g = it->second;

      if (geomLineSearchBuffer->covers(g))
      {
        // Gets the dataset
        std::unique_ptr<te::da::DataSet> dataset = m_coordLayer->getData(gp->getName(), g, te::gm::INTERSECTS);
        assert(dataset.get());

        while (dataset->moveNext())
        {
          std::string type = dataset->getString(4);

          if (type == "DEAD")
          {
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
            item->setGeometry(5, dataset->getGeometry(gp->getName()).release());

            m_dataSet->add(item);
          }
        }
      }
    }
  }

  removeObjects();

  createRTree();

  cancelOperation();

  //repaint the layer
  te::qt::widgets::MultiThreadMapDisplay* mtmp = dynamic_cast<te::qt::widgets::MultiThreadMapDisplay*>(m_display);
  if (mtmp)
    mtmp->updateLayer(m_coordLayer);
  else
    m_display->refresh();
}

void geopx::tools::TrackDeadClassifier::removeObjects()
{
  if (!m_coordLayer.get())
    return;

  try
  {
    te::map::DataSetLayer* dsLayer = dynamic_cast<te::map::DataSetLayer*>(m_coordLayer.get());

    if (dsLayer)
    {
      std::unique_ptr<const te::map::LayerSchema> schema(m_coordLayer->getSchema());

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
}

void geopx::tools::TrackDeadClassifier::drawSelecteds()
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

  try
  {
    std::size_t size = 24;

    te::se::Stroke* stroke = te::se::CreateStroke("#FF0000", "2", "0.5");
    te::se::Fill* fill = te::se::CreateFill("#FFFFFF", "0.5");
    te::se::Mark* mark = te::se::CreateMark("square", stroke, fill);

    te::color::RGBAColor** rgba = te::map::MarkRendererManager::getInstance().render(mark, size);

    canvas.setPointColor(te::color::RGBAColor(0, 0, 0, TE_TRANSPARENT));
    canvas.setPointPattern(rgba, size, size);

    te::common::Free(rgba, size);
    delete mark;

    // Gets the dataset
    if (m_point0)
      canvas.draw(m_point0);

    if (m_point1)
      canvas.draw(m_point1);
  }
  catch (std::exception& e)
  {
    QMessageBox::critical(m_display, tr("Error"), QString(tr("Error creating buffer. Details:") + " %1.").arg(e.what()));
    return;
  }
}

te::gm::Geometry* geopx::tools::TrackDeadClassifier::createBuffer(te::gm::Point* rootPoint, te::da::ObjectId* objIdRoot, int srid, std::string gpName, te::gm::LineString*& lineBuffer, std::list<te::gm::Point*>& track)
{
  std::unique_ptr<te::da::DataSetType> schema = m_coordLayer->getSchema();

  std::vector<std::string> pnames;
  te::da::GetOIDPropertyNames(schema.get(), pnames);

  //get dead tolerance info
  if (m_deadTolLineEdit->text().isEmpty())
  {
    m_deltaTol = DELTA_TOL;
  }
  else
  {
    m_deltaTol = m_deadTolLineEdit->text().toDouble();
  }

  //get distance buffer info
  double distanceTrack = 0.;

  if (m_distanceTrackLineEdit->text().isEmpty())
  {
    distanceTrack = DISTANCE_TRACK;
  }
  else
  {
    distanceTrack = m_distanceTrackLineEdit->text().toDouble();
  }

  double distanceTrackTol = 0.;

  if (m_distanceTrackToleranceFactorLineEdit->text().isEmpty())
  {
    distanceTrackTol = TRACK_TOLERANCE_FACTOR;
  }
  else
  {
    distanceTrackTol = m_distanceTrackToleranceFactorLineEdit->text().toDouble();
  }

  //get parcel geom
  int parcelId;
  std::unique_ptr<te::gm::Geometry> parcelGeom = getParcelGeeom(rootPoint, parcelId);

  if (!parcelGeom.get())
    return 0;

  parcelGeom->setSRID(srid);

  te::da::GetEmptyOIDSet(schema.get(), m_track);

  track.push_back(new te::gm::Point(*rootPoint));

  if (objIdRoot)
  {
    m_track->add(objIdRoot);
  }
  else
  {
    calculateGuessPoint(rootPoint, parcelId);
  }

  QApplication::setOverrideCursor(Qt::WaitCursor);

  //calculate track info angle
  if (m_point0 && m_point1)
  {
    getTrackInfo(m_point0, m_point1);
  }
 
  double dx = m_dx;
  double dy = m_dy;

  m_deadCount = 0;

  if (parcelGeom->getSRID() != rootPoint->getSRID())
    parcelGeom->transform(rootPoint->getSRID());

  te::gm::Point* newRoot = 0;

  while (true)
  {
    te::gm::Point* guestPoint = createGuessPoint(rootPoint, dx, dy, srid);

    //create envelope to find if guest point exist
    double toleranceFactor = 0.;

    if (m_distanceToleranceFactorLineEdit->text().isEmpty())
    {
      toleranceFactor = TOLERANCE_FACTOR;
    }
    else
    {
      toleranceFactor = m_distanceToleranceFactorLineEdit->text().toDouble();
    }

    //adjust tolerance for dead trees
    toleranceFactor = toleranceFactor + (m_deadCount * m_deltaTol);

    //stop criteria
    double curDistance = m_point0->distance(guestPoint);

    if (curDistance > (m_totalDistance + toleranceFactor))
      break;

    //filter using a line buffer
    std::unique_ptr<te::gm::LineString> lineSearchBuffer(new te::gm::LineString(2, te::gm::LineStringType, srid));
    lineSearchBuffer->setPoint(0, rootPoint->getX() + (dx - (dx*toleranceFactor)), rootPoint->getY() + (dy - (dy*toleranceFactor)));
    lineSearchBuffer->setPoint(1, rootPoint->getX() + (dx + (dx*toleranceFactor)), rootPoint->getY() + (dy + (dy*toleranceFactor)));
    te::gm::Geometry* geomLineSearchBuffer = lineSearchBuffer->buffer(distanceTrack * distanceTrackTol, 4, te::gm::CapButtType);

    //check on tree
    te::gm::Envelope ext(*geomLineSearchBuffer->getMBR());

    std::vector<int> resultsTreeObjs;

    m_centroidRtree.search(ext, resultsTreeObjs);

    std::vector<int> resultsTree;

    for (std::size_t t = 0; t < resultsTreeObjs.size(); ++t)
    {
      std::map<int, te::gm::Geometry*>::iterator it = m_centroidGeomMap.find(resultsTreeObjs[t]);

      if (it != m_centroidGeomMap.end())
      {
        te::gm::Geometry* g = it->second;

        if (geomLineSearchBuffer->covers(g))
        {
          resultsTree.push_back(resultsTreeObjs[t]);
        }
      }
    }

    if (resultsTree.empty())
    {
      //dead point
      rootPoint = guestPoint;

      te::gm::Point* pGuessCalculated = calculateGuessPoint(rootPoint, parcelId);

      if (pGuessCalculated)
      {
        rootPoint = pGuessCalculated;

        track.push_back(new te::gm::Point(*rootPoint));
      }
    }
    else
    {
      //live point
      te::da::ObjectId* objIdCandidate = 0;

      te::gm::Point* pCandidate = getCandidatePoint(rootPoint, guestPoint, srid, resultsTree, objIdCandidate);

      if (!pCandidate)
      {
        te::gm::Point* pGuessCalculated = calculateGuessPoint(guestPoint, parcelId);

        rootPoint = pGuessCalculated;
      }
      else
      {
        rootPoint = pCandidate;

        m_track->add(objIdCandidate);

        m_deadCount = 0;
      }

      track.push_back(new te::gm::Point(*rootPoint));
    }

    //adjust root point to directional line
    if (newRoot)
      delete newRoot;

    double dist = TePerpendicularDistance(*m_point0, *m_point1, *rootPoint, newRoot);

    rootPoint = newRoot;
  }

  delete newRoot;

  QApplication::restoreOverrideCursor();

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

  return lineBuffer->buffer(distanceTrack / 2., 16, te::gm::CapButtType);
}

void geopx::tools::TrackDeadClassifier::getTrackInfo(te::gm::Point* point0, te::gm::Point* point1)
{
  if (m_distLineEdit->text().isEmpty())
    m_distance = DISTANCE;
  else
    m_distance = m_distLineEdit->text().toDouble();
  
  double bigDistance = point0->distance(point1);

  double big_dx = point1->getX() - point0->getX();
  double big_dy = point1->getY() - point0->getY();

  m_dx = m_distance * big_dx / bigDistance;
  m_dy = m_distance * big_dy / bigDistance;

  m_classify = true;
}

std::unique_ptr<te::gm::Geometry> geopx::tools::TrackDeadClassifier::getParcelGeeom(te::gm::Geometry* root, int& parcelId)
{
  if (!m_parcelLayer.get())
    throw;

  // Bulding the query box
  te::gm::Envelope envelope(*root->getMBR());

  te::gm::Envelope reprojectedEnvelope(envelope);

  if ((root->getSRID() != TE_UNKNOWN_SRS) && (m_parcelLayer->getSRID() != TE_UNKNOWN_SRS) && (root->getSRID() != m_parcelLayer->getSRID()))
    reprojectedEnvelope.transform(root->getSRID(), m_parcelLayer->getSRID());

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
      g->setSRID(m_parcelLayer->getSRID());

    if (g->getSRID() != root->getSRID())
      g->transform(root->getSRID());

    if (g->covers(root))
    {
      parcelId = dataset->getInt32(name);

      break;
    }
  }

  return g;
}

te::gm::Point* geopx::tools::TrackDeadClassifier::createGuessPoint(te::gm::Point* p, double dx, double dy, int srid)
{
  return new te::gm::Point(p->getX() + dx, p->getY() + dy, srid);
}

void geopx::tools::TrackDeadClassifier::getClassDataSets(te::da::DataSetType* dsType, te::mem::DataSet*& liveDataSet, te::mem::DataSet*& intruderDataSet, te::gm::Geometry* buffer)
{
  // Bulding the query box
  te::gm::Envelope envelope(*buffer->getMBR());

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

      if (!buffer->contains(g.get()))
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
        std::string curClass = dataset->getString(4);
        if (curClass == "DEAD")
        {
          item->setString(4, "REMOVED");
        }
        else
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

void geopx::tools::TrackDeadClassifier::createRTree()
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

  QApplication::restoreOverrideCursor();
}

te::gm::Point* geopx::tools::TrackDeadClassifier::getPoint(te::gm::Geometry* g)
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

void geopx::tools::TrackDeadClassifier::getStartIdValue()
{
  if (!m_coordLayer.get())
    throw;

  QApplication::setOverrideCursor(Qt::WaitCursor);

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

  QApplication::restoreOverrideCursor();

  ++m_starterId;
}

bool geopx::tools::TrackDeadClassifier::isDead(te::da::ObjectId* objId, double& area)
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

    if (classValue == "DEAD" || classValue == "REMOVED")
      return true;

    if (classValue == "UNKNOWN")
    {
      //get area attribute and check threshold
      area = ds->getDouble("area");
    }
  }

  return false;
}

te::gm::Point* geopx::tools::TrackDeadClassifier::calculateGuessPoint(te::gm::Point* p, int parcelId)
{
  if (!m_dataSet.get())
  {
    std::unique_ptr<te::da::DataSetType> dsType = m_coordLayer->getSchema();

    //create dataset type
    std::unique_ptr<te::da::DataSetType> dataSetType = createTreeDataSetType();

    m_dataSet.reset(new te::mem::DataSet(dataSetType.get()));
  }

  double toleranceFactor = 0.;

  if (m_distanceToleranceFactorLineEdit->text().isEmpty())
  {
    toleranceFactor = TOLERANCE_FACTOR;
  }
  else
  {
    toleranceFactor = m_distanceToleranceFactorLineEdit->text().toDouble();
  }

  double thresholdValue = NDVI_THRESHOLD;

  if (!m_thresholdLineEdit->text().isEmpty())
  {
    thresholdValue = m_thresholdLineEdit->text().toDouble();
  }

  m_ndviRaster->getGrid()->setSRID(p->getSRID());

  std::string forestType = "UNKNOWN";

  te::gm::Point* pGuess = 0;

  //try guess point
  te::gm::Coord2D coordGuess = m_ndviRaster->getGrid()->geoToGrid(p->getX(), p->getY());

  double valueGuess = 0.;

  m_ndviRaster->getValue(coordGuess.getX(), coordGuess.getY(), valueGuess);

  if (valueGuess > thresholdValue)
  {
    forestType = "LIVE";

    m_deadCount = 0;

    pGuess = p;
  }
  else
  {
    forestType = "DEAD";

    ++m_deadCount;

    pGuess = p;
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
  item->setGeometry(4, new te::gm::Point(*pGuess));

  m_dataSet->add(item);

  ++m_starterId;

  return pGuess;
}

te::gm::Point* geopx::tools::TrackDeadClassifier::getCandidatePoint(te::gm::Point* pRoot, te::gm::Point* pGuess, int srid, std::vector<int>& resultsTree, te::da::ObjectId*& candidateOjbId)
{
  double lowerDistance = std::numeric_limits<double>::max();

  double polyAreaMin = 0.;
  double polyAreaMax = 0.;

  if (m_polyAreaMin->text().isEmpty())
  {
    polyAreaMin = POLY_AREA_MIN;
  }
  else
  {
    polyAreaMin = m_polyAreaMin->text().toDouble();
  }

  if (m_polyAreaMax->text().isEmpty())
  {
    polyAreaMax = POLY_AREA_MAX;
  }
  else
  {
    polyAreaMax = m_polyAreaMax->text().toDouble();
  }

  te::gm::Point* point = 0;

  for (std::size_t t = 0; t < resultsTree.size(); ++t)
  {
    std::map<int, te::gm::Geometry*>::iterator it = m_centroidGeomMap.find(resultsTree[t]);

    std::map<int, te::da::ObjectId*>::iterator itObjId = m_centroidObjIdMap.find(resultsTree[t]);

    double area = 0.;

    if (!isDead(itObjId->second, area))
    {
      if ((area > polyAreaMin && area < polyAreaMax) || area == 0.)
      {
        te::gm::Point* pCandidate = getPoint(it->second);

        pCandidate->setSRID(srid);

        if (pRoot->getX() != pCandidate->getX() || pRoot->getY() != pCandidate->getY())
        {
          //check for lower distance from guest point
          double dist = std::abs(pGuess->distance(pCandidate));

          if (dist < lowerDistance)
          {
            lowerDistance = dist;
            point = pCandidate;
            candidateOjbId = itObjId->second;
          }
        }
      }
    }
  }

  return point;
}

std::unique_ptr<te::da::DataSetType> geopx::tools::TrackDeadClassifier::createTreeDataSetType()
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

  return dataSetType;
}

void geopx::tools::TrackDeadClassifier::processDataSet()
{
  QApplication::setOverrideCursor(Qt::WaitCursor);

  std::unique_ptr<te::da::DataSetType> dsType(m_coordLayer->getSchema());

  te::gm::GeometryProperty* gp = te::da::GetFirstGeomProperty(dsType.get());

  // Let's generate the oid
  std::vector<std::string> pnames;
  te::da::GetOIDPropertyNames(dsType.get(), pnames);

  m_dataSet.reset();

  te::gm::LineString* line = 0;

  std::list<te::gm::Point*> track;

  try
  {
    std::unique_ptr<te::gm::Geometry> buffer(createBuffer(m_point0, m_objId0, m_coordLayer->getSRID(), gp->getName(), line, track));

    if (buffer.get())
    {
      //create dataset type
      te::mem::DataSet* liveDS = 0;
      te::mem::DataSet* intruderDS = 0;

      getClassDataSets(dsType.get(), liveDS, intruderDS, buffer.get());

      //class
      te::map::DataSetLayer* dsLayer = dynamic_cast<te::map::DataSetLayer*>(m_coordLayer.get());

      if (dsLayer)
      {
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

          liveDS->moveBeforeFirst();

          dataSource->update(dsType->getName(), liveDS, properties, ids);
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

          intruderDS->moveBeforeFirst();

          dataSource->update(dsType->getName(), intruderDS, properties, ids);
        }

        //add dead dataset
        if (m_dataSet.get())
        {
          std::map<std::string, std::string> options;

          m_dataSet->moveBeforeFirst();

          dataSource->add(dsType->getName(), m_dataSet.get(), options);

          m_dataSet.reset();
        }
      }
    }
  }
  catch (std::exception& e)
  {
    QApplication::restoreOverrideCursor();

    QMessageBox::critical(m_display, tr("Error"), QString(tr("Error auto classifying track. Details:") + " %1.").arg(e.what()));
  }

  m_classify = true;

  createRTree();

  QApplication::restoreOverrideCursor();
}

bool geopx::tools::TrackDeadClassifier::deadTrackMouseMove(QMouseEvent* e)
{
  if (!m_point0 || m_point1)
    return false;

  QPointF pw = m_display->transform(e->localPos());

  te::gm::Coord2D cFinal = te::gm::Coord2D(pw.x(), pw.y());

  // Clear!
  QPixmap* draft = m_display->getDraftPixmap();
  draft->fill(Qt::transparent);

  const te::gm::Envelope& env = m_display->getExtent();

  // Prepares the canvas
  te::qt::widgets::Canvas canvas(m_display->width(), m_display->height());
  canvas.setDevice(draft, false);
  canvas.setWindow(env.m_llx, env.m_lly, env.m_urx, env.m_ury);
  canvas.setRenderHint(QPainter::Antialiasing, true);

  // Build the geometry
  te::gm::LineString* line = new te::gm::LineString(2, te::gm::LineStringType);

  line->setPoint(0, m_point0->getX(), m_point0->getY());
  line->setPoint(1, cFinal.getX(), cFinal.getY());

  // Setup canvas style
  QPen pen;
  pen.setColor(QColor(100, 177, 216));
  pen.setWidth(3);

  canvas.setLineColor(pen.color().rgba());
  canvas.setLineWidth(pen.width());

  canvas.draw(line);

  delete line;

  m_display->repaint();

  return true;
}

bool geopx::tools::TrackDeadClassifier::panMousePressEvent(QMouseEvent* e)
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

bool geopx::tools::TrackDeadClassifier::panMouseMoveEvent(QMouseEvent* e)
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

bool geopx::tools::TrackDeadClassifier::panMouseReleaseEvent(QMouseEvent* e)
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

double TePerpendicularDistance(const te::gm::Point& first, const te::gm::Point& last, te::gm::Point& pin, te::gm::Point*& pinter)
{
  double d12, xmin, ymin;

  double xi = first.getX();
  double xf = last.getX();
  double yi = first.getY();
  double yf = last.getY();
  double x = pin.getX();
  double y = pin.getY();

  double dx = xf - xi;
  double dy = yf - yi;
  double a2 = (y - yi) * dx - (x - xi)*dy;

  if (dx == 0. && dy == 0.)
  {
    d12 = sqrt(((x - xi) * (x - xi)) + ((y - yi) * (y - yi)));
    d12 *= d12;
  }
  else
    d12 = a2 * a2 / (dx * dx + dy * dy);

  if (dx == 0.)
  {
    xmin = xi;
    ymin = y;
  }
  else if (dy == 0.)
  {
    xmin = x;
    ymin = yi;
  }
  else
  {
    double alfa = dy / dx;
    xmin = (x + alfa * (y - yi) + alfa * alfa * xi) / (1. + alfa * alfa);
    ymin = (x - xmin) / alfa + y;
  }

  pinter = new te::gm::Point(xmin, ymin, pin.getSRID());

  return (sqrt(d12));
}
