/*!
  \file geopx-desktop/src/geopixeltools/photoindex/PhotoIndexAction.h

  \brief This file defines the Photo Index Action class
*/

#ifndef __GEOPXDESKTOP_TOOLS_PHOTOINDEX_PHOTOINDEXACTION_H
#define __GEOPXDESKTOP_TOOLS_PHOTOINDEX_PHOTOINDEXACTION_H

// Geopx
#include "../Config.h"
#include "../AbstractAction.h"

namespace geopx
{
  namespace tools
  {
    /*!
      \class PhotoIndexAction

      \brief This file defines the Photo Index Action class

    */
    class GEOPXTOOLSEXPORT PhotoIndexAction : public geopx::tools::AbstractAction
    {
      Q_OBJECT

      public:

        PhotoIndexAction(QMenu* menu);

        virtual ~PhotoIndexAction();

      protected slots:

        virtual void onActionActivated(bool checked);
    };

  }     // end namespace tools
}       // end namespace geopx

#endif //__GEOPXDESKTOP_TOOLS_PHOTOINDEX_PHOTOINDEXACTION_H
