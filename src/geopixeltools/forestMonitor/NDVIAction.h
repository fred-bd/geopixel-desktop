/*!
    \file geopx-desktop/src/geopixeltools/NDVIAction.h

    \brief This file defines the NDVI Class Action class
*/

#ifndef __GEOPXDESKTOP_TOOLS_NDVIACTION_H
#define __GEOPXDESKTOP_TOOLS_NDVIACTION_H

#include "../Config.h"
#include "../AbstractAction.h"

namespace geopx
{
  namespace tools
  {
    /*!
      \class NDVIAction
          
      \brief This file defines the NDVI Action class

    */
    class GEOPXTOOLSEXPORT NDVIAction : public geopx::tools::AbstractAction
    {
      Q_OBJECT

      public:

        NDVIAction(QMenu* menu);

        virtual ~NDVIAction();

      protected slots:

        virtual void onActionActivated(bool checked);
    };

  }  // end namespace tools
}  // end namespace geopx

#endif //__GEOPXDESKTOP_TOOLS_NDVIACTION_H
