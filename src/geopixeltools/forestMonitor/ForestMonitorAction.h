 /*!
   \file geopx-desktop/src/geopixeltools/ForestMonitorAction.h

   \brief This file defines the Forest Monitor Action class
 */

#ifndef __GEOPXDESKTOP_TOOLS_FORESTMONITORACTION_H
#define __GEOPXDESKTOP_TOOLS_FORESTMONITORACTION_H

// TerraLib
#include "../Config.h"
#include "../AbstractAction.h"

namespace geopx
{
  namespace tools
  {
    /*!
      \class ForestMonitorAction
          
      \brief This file defines the Forest Monitor Action class

    */
    class GEOPXTOOLSEXPORT ForestMonitorAction : public geopx::tools::AbstractAction
    {
      Q_OBJECT

      public:

        ForestMonitorAction(QMenu* menu);

        virtual ~ForestMonitorAction();

      protected slots:

        virtual void onActionActivated(bool checked);
    };
  } // end namespace tools
}  // end namespace geopx

#endif //__GEOPXDESKTOP_TOOLS_FORESTMONITORACTION_H
