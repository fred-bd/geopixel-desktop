/*!
  \file geopx-desktop/src/geopixeltools/qt/tools/Eraser.h

  \brief This class implements a concrete tool to remove points from layer
*/

#ifndef __GEOPXDESKTOP_TOOLS_FORESTMONITOR_ERASER_H
#define __GEOPXDESKTOP_TOOLS_FORESTMONITOR_ERASER_H

// TerraLib
#include <terralib/dataaccess/dataset/ObjectIdSet.h>
#include <terralib/maptools/AbstractLayer.h>
#include <terralib/memory/DataSet.h>
#include <terralib/qt/widgets/tools/AbstractTool.h>
#include "../../../Config.h"

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

    /*!
      \class Info

      \brief This class implements a concrete tool to get informations about a specified feature using pointing operation.

      \ingroup widgets
      */
    class Eraser : public te::qt::widgets::AbstractTool
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
      Eraser(te::qt::widgets::MapDisplay* display, const QCursor& cursor, te::map::AbstractLayerPtr layer, QObject* parent = 0);

      /*! \brief Destructor. */
      ~Eraser();

      //@}

      /** @name AbstractTool Methods
        *  Methods related with tool behavior.
        */
      //@{

      bool eventFilter(QObject* watched, QEvent* e);

      //@}

      public slots:

      void setLayer(te::map::AbstractLayerPtr layer);

    protected:

      void selectObjects(QMouseEvent* e);

      void removeObjects();

      void drawSelecteds();

      void cancelOperation();

      bool panMousePressEvent(QMouseEvent* e);

      bool panMouseMoveEvent(QMouseEvent* e);

      bool panMouseReleaseEvent(QMouseEvent* e);

    private:

      te::map::AbstractLayerPtr m_layer;        //!<The layer that will be queried.

      std::unique_ptr<te::mem::DataSet> m_dataSet;

      //te::da::ObjectIdSet* m_objIdSet;

      //pan attributes
      bool m_panStarted;      //!< Flag that indicates if pan operation was started.
      QPoint m_origin;        //!< Origin point on mouse pressed.
      QPoint m_delta;         //!< Difference between pressed point and destination point on mouse move.
      QCursor m_actionCursor; //!< An optional cursor to be used during the pan user action.
    };

  } // end namespace tools
} // end namespace geopx

#endif  // __GEOPXDESKTOP_TOOLS_FORESTMONITOR_ERASER_H
