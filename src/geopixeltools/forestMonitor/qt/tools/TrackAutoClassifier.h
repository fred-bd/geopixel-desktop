/*!
  \file geopx-desktop/src/geopixeltools/qt/tools/TrackAutoClassifier.h

  \brief This class implements a concrete tool to track classifier
*/

#ifndef __GEOPXDESKTOP_TOOLS_FORESTMONITOR_TRACKAUTOCLASSIFIER_H
#define __GEOPXDESKTOP_TOOLS_FORESTMONITOR_TRACKAUTOCLASSIFIER_H

#include "../../../Config.h"

// TerraLib
#include <terralib/dataaccess/dataset/ObjectIdSet.h>
#include <terralib/maptools/AbstractLayer.h>
#include <terralib/memory/DataSet.h>
#include <terralib/qt/widgets/tools/AbstractTool.h>

// STL
#include <list>
#include <string>

// QT
#include <QLineEdit>

namespace te
{
  namespace da { class Where; }

  namespace gm { class Geometry; }

  namespace rst { class Raster; }

  namespace qt
  {
    namespace widgets
    {
      class MapDisplay;
    }
  }
}

namespace geopx
{
  namespace tools
  {

    /*!
      \class TrackClassifier

      \brief This class implements a concrete tool to track classifier

      \ingroup widgets
      */
    class TrackAutoClassifier : public te::qt::widgets::AbstractTool
    {
      Q_OBJECT

    public:

      /** @name Initializer Methods
        *  Methods related to instantiation and destruction.
        */
      //@{

      /*!
        \brief It constructs a info tool associated with the given map display.

        \param display The map display associated with the tool.
        \param cursor The tool cursor.
        \param layers  The layer list that will be queried.
        \param parent  The tool's parent.

        \note The tool will NOT take the ownership of the given pointers.
        */
      TrackAutoClassifier(te::qt::widgets::MapDisplay* display, const QCursor& cursor, te::map::AbstractLayerPtr coordLayer, te::map::AbstractLayerPtr parcelLayer, 
        te::map::AbstractLayerPtr rasterLayer, te::map::AbstractLayerPtr dirLayer, QObject* parent = 0);

      /*! \brief Destructor. */
      ~TrackAutoClassifier();

      void setLineEditComponents(QLineEdit* distLineEdit, QLineEdit* distanceTrackLineEdit, QLineEdit* distanceToleranceFactorLineEdit, 
        QLineEdit* distanceTrackToleranceFactorLineEdit, QLineEdit* polyAreaMin, QLineEdit* polyAreaMax, QLineEdit* maxDead, QLineEdit* deadTol, QLineEdit* threshold);

      void setAdjustTrack(bool adjust, int nSteps);

      //@}

      /** @name AbstractTool Methods
        *  Methods related with tool behavior.
        */
      //@{

      bool eventFilter(QObject* watched, QEvent* e);

      //@}

    protected:

      void selectObjects(QMouseEvent* e);

      void classifyObjects();
          
      void autoClassifyObjects();

      void cancelOperation(bool restart = false);

      void drawSelecteds();

      te::gm::Geometry* createBuffer(te::gm::Point* rootPoint, te::da::ObjectId* objIdRoot, int srid, std::string gpName, te::gm::LineString*& lineBuffer, std::list<te::gm::Point*>& track);

      void getTrackInfo(te::gm::Point* point0, te::gm::Point* point1);

      std::unique_ptr<te::gm::Geometry> getParcelGeeom(te::gm::Geometry* root, int& parcelId);

      te::gm::Point* createGuessPoint(te::gm::Point* p, double dx, double dy, int srid);

      void getClassDataSets(te::da::DataSetType* dsType, te::mem::DataSet*& liveDataSet, te::mem::DataSet*& intruderDataSet, te::gm::Geometry* buffer);

      void createRTree();

      te::gm::Point* getPoint(te::gm::Geometry* g);

      void getStartIdValue();

      bool isClassified(te::da::ObjectId* objId, double& area, std::string& classValue);

      te::gm::Point* calculateGuessPoint(te::gm::Point* p, int parcelId);

      te::gm::Point* getCandidatePoint(te::gm::Point* pRoot, te::gm::Point* pGuess, int srid, std::vector<int>& resultsTree, te::da::ObjectId*& candidateOjbId, bool& abort);

      void adjustTrack(te::gm::Point* point, double& dx, double& dy);

      std::unique_ptr<te::da::DataSetType> createTreeDataSetType();

      te::da::Where* getRestriction();

      void processDataSet(te::da::DataSet* ds);

      bool panMousePressEvent(QMouseEvent* e);

      bool panMouseMoveEvent(QMouseEvent* e);

      bool panMouseReleaseEvent(QMouseEvent* e);

    private:

      te::map::AbstractLayerPtr m_coordLayer;         //!<The layer that will be classified.
      te::map::AbstractLayerPtr m_parcelLayer;        //!<The layer with geometry restriction.
      te::map::AbstractLayerPtr m_dirLayer;           //!<The layer with direction information.

      te::sam::rtree::Index<int> m_centroidRtree;
      std::map<int, te::gm::Geometry*> m_centroidGeomMap;
      std::map<int, te::da::ObjectId*> m_centroidObjIdMap;

      te::gm::Point* m_point0;
      te::da::ObjectId* m_objId0;

      te::gm::Point* m_point1;
      te::da::ObjectId* m_objId1;

      te::da::ObjectIdSet* m_track;

      te::da::ObjectIdSet* m_roots;

      std::unique_ptr<te::mem::DataSet> m_dataSet;

      int m_starterId;

      QLineEdit* m_distLineEdit;
      QLineEdit* m_distanceTrackLineEdit;
      QLineEdit* m_distanceToleranceFactorLineEdit;
      QLineEdit* m_distanceTrackToleranceFactorLineEdit;
      QLineEdit* m_polyAreaMin;
      QLineEdit* m_polyAreaMax;
      QLineEdit* m_maxDeadLineEdit;
      QLineEdit* m_deadTolLineEdit;
      QLineEdit* m_thresholdLineEdit;

      double m_dx;
      double m_dy;
      double m_distance;

      bool m_adjustTrack;
      int m_adjustTrackSteps;
      std::list<te::gm::Point*> m_adjustTrackPoints;

      bool m_classify;

      unsigned int m_maxDead;
      unsigned int m_deadCount;
      double m_deltaTol;

      te::rst::Raster* m_ndviRaster;

      te::sam::rtree::Index<int> m_angleRtree;
      std::map<int, te::gm::Geometry*> m_angleGeomMap;

      //pan attributes
      bool m_panStarted;      //!< Flag that indicates if pan operation was started.
      QPoint m_origin;        //!< Origin point on mouse pressed.
      QPoint m_delta;         //!< Difference between pressed point and destination point on mouse move.
      QCursor m_actionCursor; //!< An optional cursor to be used during the pan user action.
    };

  } // end namespace tools
} // end namespace geopx

#endif  // __GEOPXDESKTOP_TOOLS_FORESTMONITOR_TRACKAUTOCLASSIFIER_H
