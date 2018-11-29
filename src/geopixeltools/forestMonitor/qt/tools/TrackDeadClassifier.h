/*!
  \file geopx-desktop/src/geopixeltools/qt/tools/TrackDeadClassifier.h

  \brief This class implements a concrete tool to track classifier
*/

#ifndef __GEOPXDESKTOP_TOOLS_FORESTMONITOR_TRACKDEADCLASSIFIER_H
#define __GEOPXDESKTOP_TOOLS_FORESTMONITOR_TRACKDEADCLASSIFIER_H

// TerraLib
#include <terralib/dataaccess/dataset/ObjectIdSet.h>
#include <terralib/maptools/AbstractLayer.h>
#include <terralib/memory/DataSet.h>
#include <terralib/qt/widgets/tools/AbstractTool.h>
#include "../../../Config.h"

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
    class TrackDeadClassifier : public te::qt::widgets::AbstractTool
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
      TrackDeadClassifier(te::qt::widgets::MapDisplay* display, const QCursor& cursor, te::map::AbstractLayerPtr coordLayer, te::map::AbstractLayerPtr parcelLayer, 
        te::map::AbstractLayerPtr rasterLayer, QObject* parent = 0);

      /*! \brief Destructor. */
      ~TrackDeadClassifier();

      void setLineEditComponents(QLineEdit* distLineEdit, QLineEdit* distanceTrackLineEdit, QLineEdit* distanceToleranceFactorLineEdit, 
        QLineEdit* distanceTrackToleranceFactorLineEdit, QLineEdit* polyAreaMin, QLineEdit* polyAreaMax, QLineEdit* deadTol, QLineEdit* threshold);

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
          
      void cancelOperation();

      void deleteOperation();

      void removeObjects();

      void drawSelecteds();

      te::gm::Geometry* createBuffer(te::gm::Point* rootPoint, te::da::ObjectId* objIdRoot, int srid, std::string gpName, te::gm::LineString*& lineBuffer, std::list<te::gm::Point*>& track);

      void getTrackInfo(te::gm::Point* point0, te::gm::Point* point1);

      std::unique_ptr<te::gm::Geometry> getParcelGeeom(te::gm::Geometry* root, int& parcelId);

      te::gm::Point* createGuessPoint(te::gm::Point* p, double dx, double dy, int srid);

      void getClassDataSets(te::da::DataSetType* dsType, te::mem::DataSet*& liveDataSet, te::mem::DataSet*& intruderDataSet, te::gm::Geometry* buffer);

      void createRTree();

      te::gm::Point* getPoint(te::gm::Geometry* g);

      void getStartIdValue();

      bool isDead(te::da::ObjectId* objId, double& area);

      te::gm::Point* calculateGuessPoint(te::gm::Point* p, int parcelId);

      te::gm::Point* getCandidatePoint(te::gm::Point* pRoot, te::gm::Point* pGuess, int srid, std::vector<int>& resultsTree, te::da::ObjectId*& candidateOjbId);

      std::unique_ptr<te::da::DataSetType> createTreeDataSetType();

      void processDataSet();

      bool deadTrackMouseMove(QMouseEvent* e);

      bool panMousePressEvent(QMouseEvent* e);

      bool panMouseMoveEvent(QMouseEvent* e);

      bool panMouseReleaseEvent(QMouseEvent* e);

    private:

      te::map::AbstractLayerPtr m_coordLayer;         //!<The layer that will be classified.
      te::map::AbstractLayerPtr m_parcelLayer;        //!<The layer with geometry restriction.

      te::sam::rtree::Index<int> m_centroidRtree;
      std::map<int, te::gm::Geometry*> m_centroidGeomMap;
      std::map<int, te::da::ObjectId*> m_centroidObjIdMap;

      te::gm::Point* m_point0;
      te::da::ObjectId* m_objId0;

      te::gm::Point* m_point1;

      te::da::ObjectIdSet* m_track;

      std::unique_ptr<te::mem::DataSet> m_dataSet;

      int m_starterId;

      QLineEdit* m_distLineEdit;
      QLineEdit* m_distanceTrackLineEdit;
      QLineEdit* m_distanceToleranceFactorLineEdit;
      QLineEdit* m_distanceTrackToleranceFactorLineEdit;
      QLineEdit* m_polyAreaMin;
      QLineEdit* m_polyAreaMax;
      QLineEdit* m_deadTolLineEdit;
      QLineEdit* m_thresholdLineEdit;

      double m_dx;
      double m_dy;
      double m_distance;

      bool m_classify;

      unsigned int m_deadCount;
      double m_deltaTol;

      double m_totalDistance;

      te::rst::Raster* m_ndviRaster;

      //pan attributes
      bool m_panStarted;      //!< Flag that indicates if pan operation was started.
      QPoint m_origin;        //!< Origin point on mouse pressed.
      QPoint m_delta;         //!< Difference between pressed point and destination point on mouse move.
      QCursor m_actionCursor; //!< An optional cursor to be used during the pan user action.
    };

  } // end namespace tools
} // end namespace geopx

#endif  // __GEOPXDESKTOP_TOOLS_FORESTMONITOR_TRACKDEADCLASSIFIER_H
