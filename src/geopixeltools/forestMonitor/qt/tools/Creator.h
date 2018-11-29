/*!
  \file geopx-desktop/src/geopixeltools/qt/tools/Creator.h

  \brief This class implements a concrete tool to create points from layer
*/

#ifndef __GEOPXDESKTOP_TOOLS_FORESTMONITOR_CREATOR_H
#define __GEOPXDESKTOP_TOOLS_FORESTMONITOR_CREATOR_H

#include "../../../Config.h"

// TerraLib
#include <terralib/dataaccess/dataset/DataSetType.h>
#include <terralib/maptools/AbstractLayer.h>
#include <terralib/memory/DataSet.h>
#include <terralib/qt/widgets/tools/AbstractTool.h>

// STL
#include <list>
#include <string>

namespace te
{
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

    enum CreatorType
    {
      CREATED_TYPE,
      LIVE_TYPE,
      DEAD_TYPE
    };

    /*!
      \class Info

      \brief This class implements a concrete tool to create points from layer

      \ingroup widgets
      */
    class Creator : public te::qt::widgets::AbstractTool
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
      Creator(te::qt::widgets::MapDisplay* display, const QCursor& cursor, te::map::AbstractLayerPtr coordLayer, te::map::AbstractLayerPtr parcelLayer, geopx::tools::CreatorType type, QObject* parent = 0);

      /*! \brief Destructor. */
      ~Creator();

      //@}

      /** @name AbstractTool Methods
        *  Methods related with tool behavior.
        */
      //@{

      bool eventFilter(QObject* watched, QEvent* e);

      //@}

    protected:

      void selectObjects(QMouseEvent* e);

      void saveObjects();

      void drawSelecteds();

      bool getParcelParentId(te::gm::Point* point, int& id);

      void getStartIdValue();

      void cancelOperation();

      bool panMousePressEvent(QMouseEvent* e);

      bool panMouseMoveEvent(QMouseEvent* e);

      bool panMouseReleaseEvent(QMouseEvent* e);

    private:

      te::map::AbstractLayerPtr m_coordLayer;         //!<The layer that will be classified.
      te::map::AbstractLayerPtr m_parcelLayer;        //!<The layer with geometry restriction.

      std::unique_ptr<te::mem::DataSet> m_dataSet;

      int m_starterId;

      geopx::tools::CreatorType m_type;

      //pan attributes
      bool m_panStarted;      //!< Flag that indicates if pan operation was started.
      QPoint m_origin;        //!< Origin point on mouse pressed.
      QPoint m_delta;         //!< Difference between pressed point and destination point on mouse move.
      QCursor m_actionCursor; //!< An optional cursor to be used during the pan user action.
    };

  }     // end namespace qt
}       // end namespace te

#endif  // __GEOPXDESKTOP_TOOLS_FORESTMONITOR_CREATOR_H
