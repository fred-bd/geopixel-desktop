/*!
  \file geopx-desktop/src/geopixeltools/ForestMonitorClassAction.h

  \brief This file defines the Forest Monitor Class Action class
*/

#ifndef __GEOPXDESKTOP_TOOLS_FORESTMONITORCLASSACTION_H
#define __GEOPXDESKTOP_TOOLS_FORESTMONITORCLASSACTION_H

#include "../Config.h"
#include "../AbstractAction.h"

namespace geopx
{
  namespace tools
  {
    /*!
      \class ForestMonitorClassAction
          
      \brief This file defines the Forest Monitor Class Action class

    */
    class GEOPXTOOLSEXPORT ForestMonitorClassAction : public geopx::tools::AbstractAction
    {
      Q_OBJECT

      public:

        ForestMonitorClassAction(QMenu* menu);

        virtual ~ForestMonitorClassAction();

      protected slots:

        virtual void onActionActivated(bool checked);
    };

  }  // end namespace tools
}  // end namespace geopx

#endif //__GEOPXDESKTOP_TOOLS_FORESTMONITORCLASSACTION_H
