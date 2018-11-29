/*!
  \file geopx-desktop/src/geopixeldesktop/ConnectLayersDockWidget.h

  \brief This class provides a rectangle that can indicate a boundary to connect layers.
*/

#ifndef __GEOPXDESKTOP_DESKTOP_CONNECTLAYERSBOX_H
#define __GEOPXDESKTOP_DESKTOP_CONNECTLAYERSBOX_H

// TerraLib
#include <terralib/qt/widgets/canvas/MapDisplay.h>
#include <terralib/qt/widgets/tools/AbstractTool.h>

// Qt
#include <QPen>

namespace geopx
{
  namespace desktop
  {

      /*!
        \class ConnectLayersBox

        \brief This class provides a rectangle that can indicate a boundary to connect layers.

      */
      class ConnectLayersBox : public te::qt::widgets::AbstractTool
      {
        Q_OBJECT

        public:

          /** @name Initializer Methods
           *  Methods related to instantiation and destruction.
           */
          //@{

          /*!
            \brief It constructs a rubber band associated with the given map display and with the specified cursor.

            \param display The map display associated with the tool.
            \param parent The tool's parent.

            \note The tool will NOT take the ownership of the given pointers.
            \note If the given cursor is different of Qt::BlankCursor, it will be setted on map display.
          */
          ConnectLayersBox(te::qt::widgets::MapDisplay* display, QObject* parent = 0);

          /*! \brief Destructor. */
          virtual ~ConnectLayersBox();

          //@}

          /** @name AbstractTool Methods
           *  Methods related with tool behavior.
           */
          //@{

          virtual bool eventFilter(QObject* watched, QEvent* e);

          virtual bool mousePressEvent(QMouseEvent* e);

          virtual bool mouseMoveEvent(QMouseEvent* e);

          virtual bool mouseReleaseEvent(QMouseEvent* e);

          bool keyEvent(QKeyEvent* e);

          //@}

        signals:

          void connectLayerBoxDefined(QRect rect);

          void connectLayerBoxUpLayer();

          void connectLayerBoxDownLayer();

          void connectLayerBoxFlickLayer();

        protected:

          QPoint m_origin;  //!< Origin point on mouse pressed.
          QPoint m_destiny; //!< Destiny point on mouse pressed.
          QRectF m_rect;    //!< The boundary rectangle managed by the rubber band.
          QPen m_pen;       //!< The pen used to draw the rubber band shape.
          QBrush m_brush;   //!< The brush used to draw the rubber band shape.

          bool m_connect;
          bool m_buildRect;
      };

  } // end namespace desktop
} // end namespace geopx

#endif  // __GEOPXDESKTOP_DESKTOP_CONNECTLAYERSBOX_H
