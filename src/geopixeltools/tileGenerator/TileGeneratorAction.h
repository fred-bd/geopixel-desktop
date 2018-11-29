/*!
\file geopx-desktop/src/geopixeltools/tileGenerator/TileGeneratorAction.h

\brief This file defines the Tile Generator Action class
*/

#ifndef __GEOPXDESKTOP_TOOLS_TILEGENERATOR_TILEGENERATORACTION_H
#define __GEOPXDESKTOP_TOOLS_TILEGENERATOR_TILEGENERATORACTION_H

// Geopixel
#include "../AbstractAction.h"
#include "../Config.h"
#include "qt/TileGeneratorDialog.h"

namespace geopx
{
  namespace tools
  {
    /*!
      \class TileGeneratorAction
          
      \brief This file defines the Tile Generator Action class

    */
    class GEOPXTOOLSEXPORT TileGeneratorAction : public geopx::tools::AbstractAction
    {
      Q_OBJECT

      public:

        TileGeneratorAction(QMenu* menu);

        virtual ~TileGeneratorAction();

      protected slots:

        virtual void onActionActivated(bool checked);

      protected:

        geopx::tools::TileGeneratorDialog*  m_dlg;
    };

  }     // end namespace tools
}       // end namespace geopx

#endif //__GEOPXDESKTOP_TOOLS_TILEGENERATOR_TILEGENERATORACTION_H
