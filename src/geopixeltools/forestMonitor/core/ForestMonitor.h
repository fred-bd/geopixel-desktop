/*!
    \file geopx-desktop/src/geopixeltools/core/ForestMonitor.h

    \brief This file contains structures and definitions to monitor forest information.
*/

#ifndef __GEOPXDESKTOP_TOOLS_FORESTMONITOR_FORESTMONITOR_H
#define __GEOPXDESKTOP_TOOLS_FORESTMONITOR_FORESTMONITOR_H

#include "../../Config.h"

// TerraLib
#include <terralib/dataaccess/dataset/DataSet.h>
#include <terralib/memory/DataSet.h>

//STL Includes
#include <memory>
#include <set>

namespace geopx
{
  namespace tools
  {
    /*!
      \class ForestMonitor
          
      \brief This file contains structures and definitions to monitor forest information.

    */
    class ForestMonitor
    {
    public:

      struct TrackPair
      {
        int m_parcelId;
        int m_parcelSRID;
        double m_parcelAngle;
        std::set<int> m_startCentroids;
      };

      public:

        ForestMonitor(double tolAngle, double distance, double distTol, te::mem::DataSet* ds);

        virtual ~ForestMonitor();

      public:

        void execute(std::unique_ptr<te::da::DataSet> parcelDs, int parcelGeomIdx, int parcelIdIdx,
                      std::unique_ptr<te::da::DataSet> angleDs, int angleGeomIdx, int angleIdIdx,
                      std::unique_ptr<te::da::DataSet> centroidDs, int centroidGeomIdx, int centroidIdIdx);

      protected:

        void setParcelDataSet(std::unique_ptr<te::da::DataSet> ds, int geomIdx, int idIdx);

        void setCentroidDataSet(std::unique_ptr<te::da::DataSet> ds, int geomIdx, int idIdx);

        void setAngleDataSet(std::unique_ptr<te::da::DataSet> ds, int geomIdx, int idIdx);

        void createRTree(te::sam::rtree::Index<int> &tree, std::map<int, te::gm::Geometry*> &geomMap, std::unique_ptr<te::da::DataSet> ds, int geomIdx, int idIdx);

        void createParcelLines(te::gm::Geometry* parcelGeom, int parcelId, std::vector<int> centroidsIdx, double angle);

        void createParcelLine(te::gm::Geometry* parcelGeom, int parcelId, std::vector<int> centroidsIdx, double angle, int centroidId);

        std::vector<int> getParcelCentroids(te::gm::Geometry* geom);

        std::vector<int> getCentroidNeighborsCandidates(te::gm::Geometry* parcelGeom, double angle, te::gm::Geometry* centroidGeom);

        double getParcelLineAngle(te::gm::Geometry* geom);

        bool centroidsSameTrack(te::gm::Point* first, te::gm::Point* last, double parcelAngle);

        double getAngle(te::gm::Point* first, te::gm::Point* last);

        te::gm::Envelope createCentroidBox(te::gm::Geometry* geom);

        void saveTrackLines();

        void checkConsistency();

      protected:

        te::sam::rtree::Index<int> m_centroidRtree;
        std::map<int, te::gm::Geometry*> m_centroidGeomMap;

        te::sam::rtree::Index<int> m_angleRtree;
        std::map<int, te::gm::Geometry*> m_angleGeomMap;

        double m_tolAngle;
        double m_distance;
        double m_distTol;

        te::mem::DataSet* m_ds;

        std::map<int, TrackPair> m_trackMap;

        std::set<int> m_ignoredCentroids;

        std::set<int> m_usedCentroids;

        int m_count;
    };

  }  // end namespace tools
}  // end namespace geopx

#endif //__GEOPXDESKTOP_TOOLS_FORESTMONITOR_FORESTMONITOR_H
