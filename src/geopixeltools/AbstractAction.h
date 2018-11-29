/*!
  \file geopx-desktop/src/geopixeltools/AbstractAction.h

  \brief This file defines the abstract class AbstractAction
*/

#ifndef __GEOPXDESKTOP_TOOLS_ABSTRACTACTION_H
#define __GEOPXDESKTOP_TOOLS_ABSTRACTACTION_H

#include "Config.h"

// TerraLib
#include <terralib/maptools/AbstractLayer.h>
#include <terralib/qt/af/events/Event.h>

// Qt
#include <QObject>
#include <QMenu>
#include <QAction>

namespace geopx
{
  namespace tools
  {
    /*!
    \class AbstractAction

    \brief This is an abstract class used to register actions.

    */
    class GEOPXTOOLSEXPORT AbstractAction : public QObject
    {
      Q_OBJECT

      public:

        /*!
          \brief Constructor.

          \param menu The parent menu object.
        */
        AbstractAction(QMenu* menu);

        /*! 
  
        */
        virtual ~AbstractAction();

        protected slots:

        /*!
          \brief Slot function used when a action was selected.

          \param checked Flag used in case a toggle action.
        */
        virtual void onActionActivated(bool checked) = 0;

      protected:

        /*!
          \brief Create and set the actions parameters.

          \param name The action name.

          \param pixmap The action pixmap name.
        */
        void createAction(std::string name, std::string pixmap = "");

        /*!
          \brief Add a new layer into layer explorer widget

          \param layer The layer auto pointer

        */
        void addNewLayer(te::map::AbstractLayerPtr layer);

        /*!
          \brief Get the list of layers from app

          \return The list pf layer auto pointers

        */
        std::list<te::map::AbstractLayerPtr> getLayers();

      Q_SIGNALS:

        void triggered(te::qt::af::evt::Event* e);

      protected:

        QMenu* m_menu;          //!< Parent Menu.
        QAction* m_action;      //!< Action used to call the process.
    };
  } // end namespace tools
} // end namespace geopx

#endif //__GEOPXDESKTOP_TOOLS_ABSTRACTACTION_H