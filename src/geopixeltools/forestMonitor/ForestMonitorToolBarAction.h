/*!
  \file geopx-desktop/src/geopixeltools/ForestMonitorToolBarAction.h

  \brief This file defines the Forest Monitor Tool Bar Action class
*/

#ifndef __GEOPXDESKTOP_TOOLS_FORESTMONITORTOOLBARACTION_H
#define __GEOPXDESKTOP_TOOLS_FORESTMONITORTOOLBARACTION_H

// TerraLib
#include "qt/ForestMonitorToolBarDialog.h"
#include "../AbstractAction.h"
#include "../Config.h"

namespace geopx
{
  namespace tools
  {
    /*!
      \class ForestMonitorToolBarAction
          
      \brief This file defines the Forest Monitor Tool Bar Action class

    */
    class GEOPXTOOLSEXPORT ForestMonitorToolBarAction : public geopx::tools::AbstractAction
    {
      Q_OBJECT

      public:

        ForestMonitorToolBarAction(QMenu* menu);

        virtual ~ForestMonitorToolBarAction();

      protected slots:

        virtual void onActionActivated(bool checked);

      protected:
          
        geopx::tools::ForestMonitorToolBarDialog*  m_dlg;
    };

  }  // end namespace tools
}  // end namespace geopx

#endif //__GEOPXDESKTOP_TOOLS_FORESTMONITORTOOLBARACTION_H
