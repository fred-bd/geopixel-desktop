/*!
  \file geopx-desktop/src/geopixeldesktop/ConnectLayersTracking.h

  \brief This class implements a concrete tool send events for connect layers operation.
*/

#ifndef __GEOPXDESKTOP_DESKTOP_CONNECTLAYERSTRACKING_H
#define __GEOPXDESKTOP_DESKTOP_CONNECTLAYERSTRACKING_H


#include "Config.h"

// TerraLib
#include <terralib/qt/widgets/tools/AbstractTool.h>
#include <terralib/qt/widgets/canvas/MapDisplay.h>

namespace geopx
{
  namespace desktop
  {

    /*!
      \class CoonectLayersTracking

      \brief This class implements a concrete tool send events for connect layers operation.

    */
    class ConnectLayersTracking : public te::qt::widgets::AbstractTool
    {
      Q_OBJECT

      public:

        /** @name Initializer Methods
          *  Methods related to instantiation and destruction.
          */
        //@{

        /*!
          \brief It constructs a coordinate tracking tool associated with the given map display.

          \param display The map display associated with the tool.
          \param parent The tool's parent.

          \note The tool will NOT take the ownership of the given pointers.
        */
        ConnectLayersTracking(te::qt::widgets::MapDisplay* display, QObject* parent = 0);

        /*! \brief Destructor. */
        ~ConnectLayersTracking();

      public:

        void enableMouseEvents();

        void disableMouseEvents();

        //@}

        /** @name AbstractTool Methods
          *  Methods related with tool behavior.
          */
        //@{

        bool eventFilter(QObject* watched, QEvent* e);

        bool keyEvent(QKeyEvent* e);

        bool mouseReleaseEvent(QMouseEvent* e);

        //@}

      signals:

        void keyUpTracked();

        void keyCtrlUpTracked();

        void keyDownTracked();

        void keyCtrlDownTracked();

        void keyLeftTracked();

        void keyCtrlLeftTracked();

        void keyRightTracked();

        void keyCtrlRightTracked();

        void keySpaceTracked();

      protected:

        bool m_mouseEventsEnabled;
    };


  } // end namespace desktop
} // end namespace geopx

#endif  // __GEOPXDESKTOP_DESKTOP_CONNECTLAYERSTRACKING_H
