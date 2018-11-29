/*!
  \file geopx-desktop/src/geopixeltools/core/ForestMonitor.cpp

  \brief This file contains structures and definitions to monitor forest information.
*/

#include "ForestMonitor.h"

//TerraLib Includes
#include <terralib/common/progress/TaskProgress.h>
#include <terralib/common/STLUtils.h>
#include <terralib/geometry/MultiLineString.h>
#include <terralib/geometry/MultiPoint.h>
#include <terralib/memory/DataSetItem.h>

//STL Includes
#include <cassert>

geopx::tools::ForestMonitor::ForestMonitor(double tolAngle, double distance, double distTol, te::mem::DataSet* ds) :
  m_tolAngle(tolAngle), m_distance(distance), m_distTol(distTol), m_ds(ds)
{
  m_count = 0;
}

geopx::tools::ForestMonitor::~ForestMonitor()
{
  m_ignoredCentroids.clear();

  m_usedCentroids.clear();

  m_trackMap.clear();

  m_centroidRtree.clear();
  te::common::FreeContents(m_centroidGeomMap);

  m_angleRtree.clear();
  te::common::FreeContents(m_angleGeomMap);
}

void geopx::tools::ForestMonitor::execute(std::unique_ptr<te::da::DataSet> parcelDs, int parcelGeomIdx, int parcelIdIdx,
                                                         std::unique_ptr<te::da::DataSet> angleDs, int angleGeomIdx, int angleIdIdx,
                                                         std::unique_ptr<te::da::DataSet> centroidDs, int centroidGeomIdx, int centroidIdIdx)
{
  //set centroid info
  setCentroidDataSet(std::move(centroidDs), centroidGeomIdx, centroidIdIdx);
  
  //set angle info
  setAngleDataSet(std::move(angleDs), angleGeomIdx, angleIdIdx);

  //set parcel info and create the track information
  setParcelDataSet(std::move(parcelDs), parcelGeomIdx, parcelIdIdx);
}

void geopx::tools::ForestMonitor::setParcelDataSet(std::unique_ptr<te::da::DataSet> ds, int geomIdx, int idIdx)
{
  assert(ds.get());

  std::size_t size = ds->size();

  te::common::TaskProgress task("Creating Tracks");
  task.setTotalSteps(size);

  //move over the data set
  ds->moveBeforeFirst();

  while(ds->moveNext())
  {
    //clear memory data
    m_trackMap.clear();

    m_ignoredCentroids.clear();

    m_usedCentroids.clear();

    if(!task.isActive())
      break;

    //get parcel (polygon)
    std::unique_ptr<te::gm::Geometry> g = ds->getGeometry(geomIdx);

    std::string strId = ds->getAsString(idIdx);

    int id = atoi(strId.c_str());

    //get parcel angle
    double angle = getParcelLineAngle(g.get());

    //get centroids
    std::vector<int> results = getParcelCentroids(g.get());

    //create parcel lines
    createParcelLines(g.get(), id, results, angle);

    saveTrackLines();

    task.pulse();

    break;
  }
}

void geopx::tools::ForestMonitor::setCentroidDataSet(std::unique_ptr<te::da::DataSet> ds, int geomIdx, int idIdx)
{
  //create tree
  createRTree(m_centroidRtree, m_centroidGeomMap, std::move(ds), geomIdx, idIdx);
}

void geopx::tools::ForestMonitor::setAngleDataSet(std::unique_ptr<te::da::DataSet> ds, int geomIdx, int idIdx)
{
  //create tree
  createRTree(m_angleRtree, m_angleGeomMap, std::move(ds), geomIdx, idIdx);
}

void geopx::tools::ForestMonitor::createRTree(te::sam::rtree::Index<int> &tree, std::map<int, te::gm::Geometry*> &geomMap, std::unique_ptr<te::da::DataSet> ds, int geomIdx, int idIdx)
{
  assert(ds.get());

  //create tree
  tree.clear();
  te::common::FreeContents(geomMap);

  ds->moveBeforeFirst();

  while(ds->moveNext())
  {
    std::string strId = ds->getAsString(idIdx);

    int id = atoi(strId.c_str());

    te::gm::Geometry* g = ds->getGeometry(geomIdx).release();
    const te::gm::Envelope* box = g->getMBR();

    tree.insert(*box, id);

    geomMap.insert(std::map<int, te::gm::Geometry*>::value_type(id, g));
  }
}

void geopx::tools::ForestMonitor::createParcelLines(te::gm::Geometry* parcelGeom, int parcelId, std::vector<int> centroidsIdx, double angle)
{
  assert(parcelGeom);

  for(std::size_t t = 0; t < centroidsIdx.size(); ++t)
  {
    int centroidId = centroidsIdx[t];

    std::set<int>::iterator it = m_usedCentroids.find(centroidId);

    if(it != m_usedCentroids.end())
      continue;

    //add as used centroid
    m_usedCentroids.insert(centroidId);

    createParcelLine(parcelGeom, parcelId, centroidsIdx, angle, centroidId);
  }
}

void geopx::tools::ForestMonitor::createParcelLine(te::gm::Geometry* parcelGeom, int parcelId, std::vector<int> centroidsIdx, double angle, int centroidId)
{
  std::set<int>::iterator itIgnored = m_ignoredCentroids.find(centroidId);

  if(itIgnored != m_ignoredCentroids.end())
    return;

  bool newSeg = false;
  int newId;

  std::map<int, te::gm::Geometry*>::iterator it = m_centroidGeomMap.find(centroidId);

  if(it != m_centroidGeomMap.end())
  {
    te::gm::Geometry* centroid = it->second;

    te::gm::MultiPoint* mFirst = dynamic_cast<te::gm::MultiPoint*>(centroid);
    te::gm::Point* first = dynamic_cast<te::gm::Point*>(mFirst->getGeometryN(0));

    std::vector<int> centroids = getCentroidNeighborsCandidates(parcelGeom, angle, centroid);

    std::map<double, std::pair<int, int> > anglesDiffs;

    for(std::size_t p = 0; p < centroids.size(); ++p)
    {
      std::map<int, te::gm::Geometry*>::iterator itP = m_centroidGeomMap.find(centroids[p]);

      te::gm::Geometry* c = itP->second;

      if(centroid == c)
        continue;

      te::gm::MultiPoint* mLast = dynamic_cast<te::gm::MultiPoint*>(c);
      te::gm::Point* last = dynamic_cast<te::gm::Point*>(mLast->getGeometryN(0));

      double a = getAngle(first, last);
      double angleDiff = abs(angle - a);

      std::pair<int, int> pair(centroidId, centroids[p]);

      anglesDiffs.insert(std::map<double, std::pair<int, int> >::value_type(angleDiff, pair));
    }

    //get line with minimum distance
    double minDist = std::numeric_limits<double>::max();

    std::pair<int, int> pairTrack;

    std::map<double, std::pair<int, int> >::iterator itAngles = anglesDiffs.begin();

    while(itAngles != anglesDiffs.end())
    {
      if(itAngles->first != 0. && itAngles->first < minDist)
      {
        minDist = itAngles->first;
        pairTrack = itAngles->second;
      }

      ++itAngles;
    }

    //add to track map
    if(minDist != std::numeric_limits<double>::max())
    {
      std::map<int, TrackPair>::iterator itTrackMap = m_trackMap.find(pairTrack.second);

      if(itTrackMap != m_trackMap.end())
      {
        itTrackMap->second.m_startCentroids.insert(pairTrack.first);
      }
      else
      {
        TrackPair tp;
        tp.m_parcelId = parcelId;
        tp.m_parcelAngle = angle;
        tp.m_parcelSRID = parcelGeom->getSRID();
        tp.m_startCentroids.insert(pairTrack.first);

        m_trackMap.insert(std::map<int, TrackPair>::value_type(pairTrack.second, tp));

        m_usedCentroids.insert(pairTrack.second);

        newSeg = true;
        newId = pairTrack.second;
      }
    }

    //ignore others
    itAngles = anglesDiffs.begin();

    while(itAngles != anglesDiffs.end())
    {
      if(itAngles->second.second != newId)
      {
        m_ignoredCentroids.insert(itAngles->second.second);
      }
     
      ++itAngles;
    }

    anglesDiffs.clear();
  }

  //recursive... used to continue the line
  if(newSeg)
    createParcelLine(parcelGeom, parcelId, centroidsIdx, angle, newId);
}

std::vector<int> geopx::tools::ForestMonitor::getParcelCentroids(te::gm::Geometry* geom)
{
  assert(geom);

  te::gm::Envelope ext(*geom->getMBR());

  std::vector<int> resultsTree;

  std::vector<int> resultsContains;

  m_centroidRtree.search(ext, resultsTree);

  for(size_t t = 0; t < resultsTree.size(); ++t)
  {
    std::map<int, te::gm::Geometry*>::iterator it = m_centroidGeomMap.find(resultsTree[t]);

    if(it != m_centroidGeomMap.end())
    {
      if(geom->contains(it->second))
      {
        resultsContains.push_back(resultsTree[t]);
      }
    }
  }

  return resultsContains;
}

std::vector<int> geopx::tools::ForestMonitor::getCentroidNeighborsCandidates(te::gm::Geometry* parcelGeom, double angle, te::gm::Geometry* centroidGeom)
{
  assert(parcelGeom && centroidGeom);

  std::vector<int> resultsTree;

  std::vector<int> resultsContains;

  te::gm::Envelope ext = createCentroidBox(centroidGeom);

  m_centroidRtree.search(ext, resultsTree);

  for(size_t t = 0; t < resultsTree.size(); ++t)
  {
    std::map<int, te::gm::Geometry*>::iterator it = m_centroidGeomMap.find(resultsTree[t]);

    if(it != m_centroidGeomMap.end())
    {
      //check if centroid is inside parcel
      if(parcelGeom->contains(it->second))
      {
        //check distance
        double minDist = m_distance - m_distTol;
        double maxDist = m_distance + m_distTol;

        double dist = centroidGeom->distance(it->second);

        if(dist > minDist && dist < maxDist)
        {
          te::gm::MultiPoint* mFirst = dynamic_cast<te::gm::MultiPoint*>(centroidGeom);
          te::gm::Point* first = dynamic_cast<te::gm::Point*>(mFirst->getGeometryN(0));

          te::gm::MultiPoint* mLast = dynamic_cast<te::gm::MultiPoint*>(it->second);
          te::gm::Point* last = dynamic_cast<te::gm::Point*>(mLast->getGeometryN(0));

          //check angle
          if(centroidsSameTrack(first, last, angle))
          {
            //check if is not ignored or used
            std::set<int>::iterator itIgnored = m_ignoredCentroids.find(resultsTree[t]);
            std::set<int>::iterator itUsed = m_usedCentroids.find(resultsTree[t]);

            if(itIgnored == m_ignoredCentroids.end() && itUsed == m_usedCentroids.end())
            {
              resultsContains.push_back(resultsTree[t]);
            }
          }
          else
          {
            //check inverted angle
            double a = getAngle(first, last);

            bool ignore = true;

            //case 1
            double minus180a = angle - 180 - m_tolAngle;
            double minus180b = angle - 180 + m_tolAngle;

            if(a > minus180a && a < minus180b)
              ignore = false;

            //case 2
            double plus180a = angle + 180 - m_tolAngle;
            double plus180b = angle + 180 + m_tolAngle;

            if(a > plus180a && a < plus180b)
              ignore = false;
             

            if(ignore)
              m_ignoredCentroids.insert(resultsTree[t]);
          }
        }
        else if(dist < minDist && dist != 0.)
        {
          m_ignoredCentroids.insert(resultsTree[t]);
        }
      }
      else
      {
        m_ignoredCentroids.insert(resultsTree[t]);
      }
    }
  }

  return resultsContains;
}

double geopx::tools::ForestMonitor::getParcelLineAngle(te::gm::Geometry* geom)
{
  assert(geom);

  te::gm::Envelope ext(*geom->getMBR());

  std::vector<int> results;

  m_angleRtree.search(ext, results);

  for(size_t t = 0; t < results.size(); ++t)
  {
    std::map<int, te::gm::Geometry*>::iterator it = m_angleGeomMap.find(results[t]);

    if(it != m_angleGeomMap.end())
    {
      if(geom->contains(it->second))
      {
        te::gm::MultiLineString* mLine = dynamic_cast<te::gm::MultiLineString*>(it->second);

        if(mLine && mLine->getNumGeometries() != 0)
        {
          te::gm::LineString* line = dynamic_cast<te::gm::LineString*>(mLine->getGeometryN(0));

          assert(line && line->size() == 2);

          std::unique_ptr<te::gm::Point> first(line->getPointN(0));
          std::unique_ptr<te::gm::Point> last(line->getPointN(1));

          return getAngle(first.get(), last.get());
        }
      }
    }
  }

  return 0.;
}

bool geopx::tools::ForestMonitor::centroidsSameTrack(te::gm::Point* first, te::gm::Point* last, double parcelAngle)
{
  assert(first && last);

  double angle = getAngle(first, last);

  //check tolerance
  double absDiff = abs(parcelAngle - angle);

  if(absDiff > m_tolAngle)
    return false;
  else
    return true;
}

double geopx::tools::ForestMonitor::getAngle(te::gm::Point* first, te::gm::Point* last)
{
  double dx = last->getX() - first->getX();
  double ax = fabs(dx);
  double dy = last->getY() - first->getY();
  double ay = fabs(dy);

  double t = 0.0;

  if((dx == 0.0) && (dy == 0.0))
    t = 0.0;
  else
    t = dy / (ax + ay);

  if(dx < 0.0)
    t = 2 - t;
  else if(dy < 0.0)
    t = 4.0 + t;

  double angle = t * 90.0;

  return angle;
}

te::gm::Envelope geopx::tools::ForestMonitor::createCentroidBox(te::gm::Geometry* geom)
{
  assert(geom);

  te::gm::Envelope ext(*geom->getMBR());

  ext.m_llx -= m_distance - m_distTol;
  ext.m_lly -= m_distance - m_distTol;
  ext.m_urx += m_distance + m_distTol;
  ext.m_ury += m_distance + m_distTol;

  return ext;
}

void geopx::tools::ForestMonitor::saveTrackLines()
{
  checkConsistency();

  std::map<int, TrackPair>::iterator it =  m_trackMap.begin();

  while(it != m_trackMap.end())
  {
    int parcelId = it->second.m_parcelId;

    //get centroid start
    std::map<int, te::gm::Geometry*>::iterator itCentroid = m_centroidGeomMap.find(*it->second.m_startCentroids.begin());
    te::gm::MultiPoint* mFirst = dynamic_cast<te::gm::MultiPoint*>(itCentroid->second);
    te::gm::Point* first = dynamic_cast<te::gm::Point*>(mFirst->getGeometryN(0));

    //get centroid last
    itCentroid = m_centroidGeomMap.find(it->first);
    te::gm::MultiPoint* mLast = dynamic_cast<te::gm::MultiPoint*>(itCentroid->second);
    te::gm::Point* last = dynamic_cast<te::gm::Point*>(mLast->getGeometryN(0));

    //create line
    te::gm::LineString* line = new te::gm::LineString(2, te::gm::LineStringType, it->second.m_parcelSRID);
    line->setPoint(0, first->getX(), first->getY());
    line->setPoint(1, last->getX(), last->getY());

    //create dataset item
    te::mem::DataSetItem* item = new te::mem::DataSetItem(m_ds);

    //set id
    item->setInt32("trackId", m_count);

    //set parcel id
    item->setInt32("parcelId", parcelId);

    //set geometry
    item->setGeometry("geom", line);

    m_ds->add(item);

    ++m_count;

    ++it;
  }
}

void geopx::tools::ForestMonitor::checkConsistency()
{
  std::map<int, TrackPair>::iterator it =  m_trackMap.begin();

  while(it != m_trackMap.end())
  {
    if(it->second.m_startCentroids.size() == 2)
    {
      //get last centroid
      std::map<int, te::gm::Geometry*>::iterator itCentroid = m_centroidGeomMap.find(it->first);
      te::gm::MultiPoint* mLast = dynamic_cast<te::gm::MultiPoint*>(itCentroid->second);
      te::gm::Point* last = dynamic_cast<te::gm::Point*>(mLast->getGeometryN(0));

      //vector with angle diffs
      double minDiff = std::numeric_limits<double>::max();

      int minCentroidId;

      //get best first centroid
      std::set<int>::iterator itSet = it->second.m_startCentroids.begin();
      while(itSet != it->second.m_startCentroids.end())
      {
        itCentroid = m_centroidGeomMap.find(*itSet);
        te::gm::MultiPoint* mFirst = dynamic_cast<te::gm::MultiPoint*>(itCentroid->second);
        te::gm::Point* first = dynamic_cast<te::gm::Point*>(mFirst->getGeometryN(0));

        double angle = getAngle(first, last);

        //check tolerance
        double absDiff = abs(it->second.m_parcelAngle - angle);

        if(absDiff < minDiff)
        {
          minDiff = absDiff;

          minCentroidId = *itSet;
        }

        ++itSet;
      }

      if(minDiff != std::numeric_limits<double>::max())
      {
        it->second.m_startCentroids.clear();
        it->second.m_startCentroids.insert(minCentroidId);
      }

    }

    ++it;
  }
}
