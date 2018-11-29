/*!
  \file geopx-desktop/src/geopixeltools/qt/tools/TrackAutoClassifier.h

  \brief This class implements a concrete tool to track classifier
*/

#include "TrackAutoClassifier.h"

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
#define MAX_DEAD 6
#define DELTA_TOL 0.1
#define NDVI_THRESHOLD 120.0

geopx::tools::TrackAutoClassifier::TrackAutoClassifier(te::qt::widgets::MapDisplay* display, const QCursor& cursor, te::map::AbstractLayerPtr coordLayer, te::map::AbstractLayerPtr parcelLayer, te::map::AbstractLayerPtr rasterLayer, te::map::AbstractLayerPtr dirLayer, QObject* parent)
  : AbstractTool(display, parent),
  m_coordLayer(coordLayer),
  m_parcelLayer(parcelLayer),
  m_dirLayer(dirLayer),
  m_track(0),
  m_point0(0),
  m_objId0(0),
  m_point1(0),
  m_objId1(0),
  m_starterId(0),
  m_roots(0),
  m_panStarted(false)
{
  m_distLineEdit = 0;
  m_distanceTrackLineEdit = 0;
  m_distanceToleranceFactorLineEdit = 0;

  m_dx = 0.;
  m_dy = 0.;
  m_distance = DISTANCE;

  m_maxDead = MAX_DEAD;
  m_deadCount = 0;
  m_deltaTol = DELTA_TOL;

  m_adjustTrack = false;

  m_classify = false;

  setCursor(cursor);
  
  display->setFocus();
  
  createRTree();

  getStartIdValue();

  //get raster
  std::unique_ptr<te::da::DataSet> ds = rasterLayer->getData();

  m_ndviRaster = ds->getRaster(0).release();
}

geopx::tools::TrackAutoClassifier::~TrackAutoClassifier()
{
  QPixmap* draft = m_display->getDraftPixmap();
  draft->fill(Qt::transparent);

  m_adjustTrackPoints.clear();

  m_centroidRtree.clear();
  te::common::FreeContents(m_centroidGeomMap);
  te::common::FreeContents(m_centroidObjIdMap);

  m_angleRtree.clear();
  te::common::FreeContents(m_angleGeomMap);

  delete m_point0;
  delete m_point1;

  delete m_roots;

  delete m_ndviRaster;
}

void geopx::tools::TrackAutoClassifier::setLineEditComponents(QLineEdit* distLineEdit, QLineEdit* distanceTrackLineEdit, QLineEdit* distanceToleranceFactorLineEdit, QLineEdit* distanceTrackToleranceFactorLineEdit, QLineEdit* polyAreaMin, QLineEdit* polyAreaMax, QLineEdit* maxDead, QLineEdit* deadTol, QLineEdit* threshold)
{
  m_distLineEdit = distLineEdit;
  m_distanceTrackLineEdit = distanceTrackLineEdit;
  m_distanceToleranceFactorLineEdit = distanceToleranceFactorLineEdit;
  m_distanceTrackToleranceFactorLineEdit = distanceTrackToleranceFactorLineEdit;
  m_polyAreaMin = polyAreaMin;
  m_polyAreaMax = polyAreaMax;
  m_maxDeadLineEdit = maxDead;
  m_deadTolLineEdit = deadTol;
  m_thresholdLineEdit = threshold;
}

void geopx::tools::TrackAutoClassifier::setAdjustTrack(bool adjust, int nSteps)
{
  m_adjustTrack = adjust;
  m_adjustTrackSteps = nSteps;
}

bool geopx::tools::TrackAutoClassifier::eventFilter(QObject* watched, QEvent* e)
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

    return panMouseMoveEvent(event);
  }
  else if (e->type() == QEvent::KeyPress)
  {
    QKeyEvent* event = static_cast<QKeyEvent*>(e);

    if ((event->key() == Qt::Key_Return || event->key() == Qt::Key_Control) && m_point1 && m_point0)
      classifyObjects();

    if ((event->key() == Qt::Key_Alt || event->key() == Qt::Key_AltGr))
      autoClassifyObjects();

    if (event->key() == Qt::Key_Escape)
      cancelOperation(false);

    if (event->key() == Qt::Key_Backspace)
      cancelOperation(true);

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

void geopx::tools::TrackAutoClassifier::selectObjects(QMouseEvent* e)
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

      te::da::ObjectIdSet* oids = 0;
      te::da::GetEmptyOIDSet(schema.get(), oids);

      oids->add(te::da::GenerateOID(dataset.get(), pnames));

      if (!m_roots)
      {
        te::da::GetEmptyOIDSet(schema.get(), m_roots);
      }

      m_roots->symDifference(oids);
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

void geopx::tools::TrackAutoClassifier::classifyObjects()
{
  if (!m_roots || m_roots->size() == 0)
    return;

  std::unique_ptr<te::da::DataSet> dsRoots = m_coordLayer->getData(m_roots);

  processDataSet(dsRoots.get());

  delete m_roots;
  m_roots = 0;

  //repaint the layer
  te::qt::widgets::MultiThreadMapDisplay* mtmp = dynamic_cast<te::qt::widgets::MultiThreadMapDisplay*>(m_display);
  if (mtmp)
    mtmp->updateLayer(m_coordLayer);
  else
    m_display->repaint();
}

void geopx::tools::TrackAutoClassifier::autoClassifyObjects()
{
  //get restriction
  te::da::Where* whereClause = getRestriction();

  std::unique_ptr<te::da::DataSet> dsResult = m_coordLayer->getData(whereClause->getExp());

  processDataSet(dsResult.get());

  //repaint the layer
  te::qt::widgets::MultiThreadMapDisplay* mtmp = dynamic_cast<te::qt::widgets::MultiThreadMapDisplay*>(m_display);
  if (mtmp)
    mtmp->updateLayer(m_coordLayer);
  else
    m_display->repaint();
}

void geopx::tools::TrackAutoClassifier::cancelOperation(bool restart)
{
  // Clear draft!
  QPixmap* draft = m_display->getDraftPixmap();
  draft->fill(Qt::transparent);

  if (restart)
  {
    delete m_point0;
    m_point0 = 0;
    delete m_objId0;
    m_objId0 = 0;

    delete m_point1;
    m_point1 = 0;
    delete m_objId1;
    m_objId1 = 0;

    m_classify = false;
  }

  delete m_roots;
  m_roots = 0;
  
  m_dataSet.reset();

  std::unique_ptr<te::da::DataSetType> schema = m_coordLayer->getSchema();

  te::da::GetEmptyOIDSet(schema.get(), m_track);

  //repaint the layer
  m_display->repaint();
}

void geopx::tools::TrackAutoClassifier::drawSelecteds()
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
    {
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
    }

    // Gets the dataset
    if (m_point0)
      canvas.draw(m_point0);

    if (m_point1)
      canvas.draw(m_point1);

    {
      //configure for points
      std::size_t size = 24;

      te::se::Stroke* stroke = te::se::CreateStroke("#00FF00", "2", "0.5");
      te::se::Fill* fill = te::se::CreateFill("#FFFFFF", "0.5");
      te::se::Mark* mark = te::se::CreateMark("square", stroke, fill);

      te::color::RGBAColor** rgba = te::map::MarkRendererManager::getInstance().render(mark, size);

      canvas.setPointColor(te::color::RGBAColor(0, 0, 0, TE_TRANSPARENT));
      canvas.setPointPattern(rgba, size, size);

      te::common::Free(rgba, size);
      delete mark;
    }

    if (m_roots)
    {
      std::unique_ptr<te::da::DataSet> dsCoords = m_coordLayer->getData(m_roots);

      while (dsCoords->moveNext())
      {
        std::unique_ptr<te::gm::Geometry> g(dsCoords->getGeometry(gp->getName()));

        if (g->getSRID() == TE_UNKNOWN_SRS)
          g->setSRID(m_coordLayer->getSRID());

        canvas.draw(g.get());
      }
    }
  }
  catch (std::exception& e)
  {
    QMessageBox::critical(m_display, tr("Error"), QString(tr("Error creating buffer. Details:") + " %1.").arg(e.what()));
    return;
  }
}

te::gm::Geometry* geopx::tools::TrackAutoClassifier::createBuffer(te::gm::Point* rootPoint, te::da::ObjectId* objIdRoot, int srid, std::string gpName, te::gm::LineString*& lineBuffer, std::list<te::gm::Point*>& track)
{
  std::unique_ptr<te::da::DataSetType> schema = m_coordLayer->getSchema();

  std::vector<std::string> pnames;
  te::da::GetOIDPropertyNames(schema.get(), pnames);

  //get max dead info
  if (m_maxDeadLineEdit->text().isEmpty())
  {
    m_maxDead = MAX_DEAD;
  }
  else
  {
    m_maxDead = m_maxDeadLineEdit->text().toInt();
  }

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

  te::gm::Point* starter = new te::gm::Point(*rootPoint);

  m_track->add(objIdRoot);

  bool invert = false;

  QApplication::setOverrideCursor(Qt::WaitCursor);

  //calculate track info angle
  if (m_point0 && m_point1)
  {
    getTrackInfo(m_point0, m_point1);
  }
  else
  {
    if (parcelGeom->getSRID() != m_dirLayer->getSRID())
      parcelGeom->transform(m_dirLayer->getSRID());

    te::gm::Envelope ext(*parcelGeom->getMBR());

    std::vector<int> results;

    m_angleRtree.search(ext, results);

    for (size_t t = 0; t < results.size(); ++t)
    {
      std::map<int, te::gm::Geometry*>::iterator it = m_angleGeomMap.find(results[t]);

      if (it != m_angleGeomMap.end())
      {
        if (parcelGeom->contains(it->second))
        {
          te::gm::MultiLineString* mLine = dynamic_cast<te::gm::MultiLineString*>(it->second);

          if (mLine && mLine->getNumGeometries() != 0)
          {
            te::gm::LineString* line = dynamic_cast<te::gm::LineString*>(mLine->getGeometryN(0));

            if (line && line->size() >= 2)
            {
              std::unique_ptr<te::gm::Point> first(line->getPointN(0));
              std::unique_ptr<te::gm::Point> last(line->getPointN(1));

              if (first->getSRID() != m_coordLayer->getSRID())
                first->transform(m_coordLayer->getSRID());

              if (last->getSRID() != m_coordLayer->getSRID())
                last->transform(m_coordLayer->getSRID());

              getTrackInfo(first.get(), last.get());

              break;
            }
          }
        }
      }
    }
  }
  

  double dx = m_dx;
  double dy = m_dy;

  m_deadCount = 0;

  if (parcelGeom->getSRID() != rootPoint->getSRID())
    parcelGeom->transform(rootPoint->getSRID());

  bool insideParcel = true;

  m_adjustTrackPoints.clear();

  while (insideParcel)
  {
    //adjust track
    if (m_adjustTrack)
      adjustTrack(rootPoint, dx, dy);

    te::gm::Point* guestPoint = createGuessPoint(rootPoint, dx, dy, srid);

    //create envelope to find if guest point exist
    //te::gm::Envelope ext(guestPoint->getX(), guestPoint->getY(), guestPoint->getX(), guestPoint->getY());

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

    //ext.m_llx -= (m_distance * toleranceFactor);
    //ext.m_lly -= (m_distance * toleranceFactor);
    //ext.m_urx += (m_distance * toleranceFactor);
    //ext.m_ury += (m_distance * toleranceFactor);

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

      insideParcel = parcelGeom->covers(rootPoint);

      if (insideParcel)
      {
        te::gm::Point* pGuessCalculated = calculateGuessPoint(rootPoint, parcelId);

        if (pGuessCalculated)
        {
          rootPoint = pGuessCalculated;

          if (!invert)
          {
            track.push_back(new te::gm::Point(*rootPoint));
          }
          else
          {
            track.push_front(new te::gm::Point(*rootPoint));
          }
        }
      }
      else
      {
        if (!invert)
        {
          invert = true;
          insideParcel = true;
          dx = m_dx * -1;
          dy = m_dy * -1;
          m_adjustTrackPoints.clear();
          rootPoint = starter;
          m_deadCount = 0;
        }
      }

    }
    else
    {
      //live point
      te::da::ObjectId* objIdCandidate = 0;
      bool abort = false;
      te::gm::Point* pCandidate = getCandidatePoint(rootPoint, guestPoint, srid, resultsTree, objIdCandidate, abort);

      if (abort)
      {
        if (!invert)
        {
          invert = true;
          insideParcel = true;
          dx = m_dx * -1;
          dy = m_dy * -1;
          m_adjustTrackPoints.clear();
          rootPoint = starter;
          m_deadCount = 0;
        }
        else
        {
          break;
        }
      }
      else
      {
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
            dx = m_dx * -1;
            dy = m_dy * -1;
            m_adjustTrackPoints.clear();
            rootPoint = starter;
            m_deadCount = 0;
          }
        }
      }
    }

    if (m_deadCount >= m_maxDead)
    {
      if (!invert)
      {
        invert = true;
        insideParcel = true;
        dx = m_dx * -1;
        dy = m_dy * -1;
        m_adjustTrackPoints.clear();
        rootPoint = starter;
        m_deadCount = 0;
      }
      else
      {
        break;
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

  return lineBuffer->buffer(distanceTrack / 2., 16, te::gm::CapButtType);
}

void geopx::tools::TrackAutoClassifier::getTrackInfo(te::gm::Point* point0, te::gm::Point* point1)
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

std::unique_ptr<te::gm::Geometry> geopx::tools::TrackAutoClassifier::getParcelGeeom(te::gm::Geometry* root, int& parcelId)
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

te::gm::Point* geopx::tools::TrackAutoClassifier::createGuessPoint(te::gm::Point* p, double dx, double dy, int srid)
{
  return new te::gm::Point(p->getX() + dx, p->getY() + dy, srid);
}

void geopx::tools::TrackAutoClassifier::getClassDataSets(te::da::DataSetType* dsType, te::mem::DataSet*& liveDataSet, te::mem::DataSet*& intruderDataSet, te::gm::Geometry* buffer)
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

void geopx::tools::TrackAutoClassifier::createRTree()
{
  QApplication::setOverrideCursor(Qt::WaitCursor);

  te::common::FreeContents(m_centroidGeomMap);
  te::common::FreeContents(m_centroidObjIdMap);
  te::common::FreeContents(m_angleGeomMap);

  m_centroidRtree.clear();
  m_centroidGeomMap.clear();
  m_centroidObjIdMap.clear();
  m_angleRtree.clear();
  m_angleGeomMap.clear();

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

  //get direction geometries
  std::unique_ptr<const te::map::LayerSchema> schemaDir(m_dirLayer->getSchema());
  std::unique_ptr<te::da::DataSet> dsDir(m_dirLayer->getData());

  te::gm::GeometryProperty* gmPropDir = te::da::GetFirstGeomProperty(schemaDir.get());
  int geomDirIdx = te::da::GetPropertyPos(schemaDir.get(), gmPropDir->getName());

  te::da::PrimaryKey* pkDir = schemaDir->getPrimaryKey();
  int idDirIdx = te::da::GetPropertyPos(schemaDir.get(), pkDir->getProperties()[0]->getName());

  dsDir->moveBeforeFirst();

  while (dsDir->moveNext())
  {
    std::string strId = dsDir->getAsString(idDirIdx);

    int id = atoi(strId.c_str());

    te::gm::Geometry* g = dsDir->getGeometry(geomDirIdx).release();
    const te::gm::Envelope* box = g->getMBR();

    m_angleRtree.insert(*box, id);

    m_angleGeomMap.insert(std::map<int, te::gm::Geometry*>::value_type(id, g));
  }

  QApplication::restoreOverrideCursor();
}

te::gm::Point* geopx::tools::TrackAutoClassifier::getPoint(te::gm::Geometry* g)
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

void geopx::tools::TrackAutoClassifier::getStartIdValue()
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

bool geopx::tools::TrackAutoClassifier::isClassified(te::da::ObjectId* objId, double& area, std::string& classValue)
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

    classValue = ds->getString("type");

    if (classValue == "CREATED")
      return false;

    if (classValue == "UNKNOWN")
    {
      //get area attribute and check threshold
      area = ds->getDouble("area");

      return false;
    }
  }

  return true;
}

te::gm::Point* geopx::tools::TrackAutoClassifier::calculateGuessPoint(te::gm::Point* p, int parcelId)
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

  ////try ll guess point
  //te::gm::Coord2D coordGuessLL = m_ndviRaster->getGrid()->geoToGrid(p->getX() - (m_dx * toleranceFactor), p->getY() - (m_dy * toleranceFactor));

  //double valueGuessLL = 0.;

  //m_ndviRaster->getValue(coordGuessLL.getX(), coordGuessLL.getY(), valueGuessLL);

  ////try lr guess point
  //te::gm::Coord2D coordGuessLR = m_ndviRaster->getGrid()->geoToGrid(p->getX() + (m_dx * toleranceFactor), p->getY() + (m_dy * toleranceFactor));

  //double valueGuessLR = 0.;

  //m_ndviRaster->getValue(coordGuessLR.getX(), coordGuessLR.getY(), valueGuessLR);

  ////try ul guess point
  //te::gm::Coord2D coordGuessUL = m_ndviRaster->getGrid()->geoToGrid(p->getX() - (m_dy * toleranceFactor), p->getY() + (m_dx * toleranceFactor));

  //double valueGuessUL = 0.;

  //m_ndviRaster->getValue(coordGuessUL.getX(), coordGuessUL.getY(), valueGuessUL);

  ////try ur guess point
  //te::gm::Coord2D coordGuessUR = m_ndviRaster->getGrid()->geoToGrid(p->getX() + (m_dy * toleranceFactor), p->getY() - (m_dx * toleranceFactor));

  //double valueGuessUR = 0.;

  //m_ndviRaster->getValue(coordGuessUR.getX(), coordGuessUR.getY(), valueGuessUR);


  if (valueGuess > thresholdValue)
  {
    forestType = "LIVE";

    m_deadCount = 0;

    pGuess = p;
  }
  /*else if (valueGuessLL > thresholdValue)
  {
    forestType = "LIVE";

    m_deadCount = 0;

    pGuess = new te::gm::Point(p->getX() - (m_dx * toleranceFactor), p->getY() - (m_dy * toleranceFactor), p->getSRID());
  }
  else if (valueGuessLR > thresholdValue)
  {
    forestType = "LIVE";

    m_deadCount = 0;

    pGuess = new te::gm::Point(p->getX() + (m_dx * toleranceFactor), p->getY() + (m_dy * toleranceFactor), p->getSRID());
  }
  else if (valueGuessUL > thresholdValue)
  {
    forestType = "LIVE";

    m_deadCount = 0;

    pGuess = new te::gm::Point(p->getX() - (m_dy * toleranceFactor), p->getY() + (m_dx * toleranceFactor), p->getSRID());
  }
  else if (valueGuessUR > thresholdValue)
  {
    forestType = "LIVE";

    m_deadCount = 0;

    pGuess = new te::gm::Point(p->getX() + (m_dy * toleranceFactor), p->getY() - (m_dx * toleranceFactor), p->getSRID());
  }*/
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

te::gm::Point* geopx::tools::TrackAutoClassifier::getCandidatePoint(te::gm::Point* pRoot, te::gm::Point* pGuess, int srid, std::vector<int>& resultsTree, te::da::ObjectId*& candidateOjbId, bool& abort)
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

    std::string classValue = "";

    if (!isClassified(itObjId->second, area, classValue))
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
    else
    {
      abort = true;

      return 0;
    }
  }

  abort = false;

  return point;
}

void geopx::tools::TrackAutoClassifier::adjustTrack(te::gm::Point* point, double& dx, double& dy)
{
  m_adjustTrackPoints.push_back(point);

  if (m_adjustTrackPoints.size() == m_adjustTrackSteps)
  {
    te::gm::Point* p0 = m_adjustTrackPoints.front();

    double bigDistance = p0->distance(point);

    double big_dx = point->getX() - p0->getX();
    double big_dy = point->getY() - p0->getY();

    dx = m_distance * big_dx / bigDistance;
    dy = m_distance * big_dy / bigDistance;

    m_adjustTrackPoints.pop_front();
  }
}

std::unique_ptr<te::da::DataSetType> geopx::tools::TrackAutoClassifier::createTreeDataSetType()
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

te::da::Where* geopx::tools::TrackAutoClassifier::getRestriction()
{
  // select * from pontos where (originId = X E ( tipo = CREATED OR (tipo = UNKNOWN E (area > 0.8 E area < 1.0))) order by tipo

  ////area1
  //te::da::PropertyName* propArea1 = new te::da::PropertyName("area");

  ////0.8
  //te::da::Literal* minorAreaValue = new te::da::LiteralDouble(0.6);

  ////area > 0.8
  //te::da::GreaterThan* greaterThanArea = new te::da::GreaterThan(propArea1, minorAreaValue);

  ////area2
  //te::da::PropertyName* propArea2 = new te::da::PropertyName("area");

  ////1.0
  //te::da::Literal* majorAreaValue = new te::da::LiteralDouble(1.3);

  ////area < 1.0
  //te::da::LessThan* lessThanArea = new te::da::LessThan(propArea2, majorAreaValue);

  ////area > 0.8 E area < 1.0
  //te::da::And* andArea = new te::da::And(greaterThanArea, lessThanArea);

  ////tipo = UNKNOWN
  //te::da::PropertyName* propTypeUnknown = new te::da::PropertyName("type");

  ////UNKNOWN
  //te::da::Literal* typeUnknownValue = new te::da::LiteralString("UNKNOWN");

  ////type equal
  //te::da::EqualTo* typeUnknownEqual = new te::da::EqualTo(propTypeUnknown, typeUnknownValue);

  ////tipo = UNKNOWN E (area > 0.8 E area < 1.0)
  //te::da::And* andType = new te::da::And(typeUnknownEqual, andArea);

  //tipo = CREATED
  te::da::PropertyName* propTypeCreated = new te::da::PropertyName("type");

  //UNKNOWN
  te::da::Literal* typeCreatedValue = new te::da::LiteralString("CREATED");

  //type equal
  te::da::EqualTo* typeCreatedEqual = new te::da::EqualTo(propTypeCreated, typeCreatedValue);

  ////tipo = CREATED OR (tipo = UNKNOWN E (area > 0.8 E area < 1.0))
  //te::da::Or* orType = new te::da::Or(typeCreatedEqual, andType);

  ////originId = X
  //te::da::PropertyName* propOriginId = new te::da::PropertyName("originId");

  ////UNKNOWN
  //te::da::Literal* originIdValue = new te::da::LiteralInt32(originId);

  ////type equal
  //te::da::EqualTo* originEqual = new te::da::EqualTo(propOriginId, originIdValue);

  ////originId = X E ( tipo = CREATED OR (tipo = UNKNOWN E (area > 0.8 E area < 1.0)))
  //te::da::And* andOrigin = new te::da::And(originEqual, typeCreatedEqual);

  //create where
  te::da::Where* where = new te::da::Where(typeCreatedEqual);

  return where;
}

void geopx::tools::TrackAutoClassifier::processDataSet(te::da::DataSet* ds)
{
  QApplication::setOverrideCursor(Qt::WaitCursor);

  std::unique_ptr<te::da::DataSetType> dsType(m_coordLayer->getSchema());

  te::gm::GeometryProperty* gp = te::da::GetFirstGeomProperty(dsType.get());

  // Let's generate the oid
  std::vector<std::string> pnames;
  te::da::GetOIDPropertyNames(dsType.get(), pnames);

  te::common::TaskProgress task("Auto Classifier");
  task.setTotalSteps(ds->size());

  ds->moveBeforeFirst();

  while (ds->moveNext())
  {
    if (!task.isActive())
    {
      break;
    }

    m_dataSet.reset();

    std::unique_ptr<te::gm::Geometry> g(ds->getGeometry(gp->getName()));

    te::gm::Point* rootPoint = getPoint(g.get());

    te::da::ObjectId* objIdRoot = te::da::GenerateOID(ds, pnames);

    te::gm::LineString* line = 0;

    std::list<te::gm::Point*> track;

    try
    {
      std::unique_ptr<te::gm::Geometry> buffer(createBuffer(rootPoint, objIdRoot, m_coordLayer->getSRID(), gp->getName(), line, track));

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
      break;
    }
    

    task.pulse();
  }

  m_classify = true;

  createRTree();

  QApplication::restoreOverrideCursor();
}

bool geopx::tools::TrackAutoClassifier::panMousePressEvent(QMouseEvent* e)
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

bool geopx::tools::TrackAutoClassifier::panMouseMoveEvent(QMouseEvent* e)
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

bool geopx::tools::TrackAutoClassifier::panMouseReleaseEvent(QMouseEvent* e)
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
