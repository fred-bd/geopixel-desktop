/*!
\file geopx-desktop/src/geopixeltools/tileGenerator/qt/TileGeneratorAction.h

\brief This interface i used to get the input parameters for  tile generation.
*/

#ifndef __GEOPXDESKTOP_TOOLS_TILEGENERATOR_TILEGENERATORDIALOG_H
#define __GEOPXDESKTOP_TOOLS_TILEGENERATOR_TILEGENERATORDIALOG_H

#include "../../Config.h"

// TerraLib
#include <terralib/geometry/Envelope.h>
#include <terralib/maptools/AbstractLayer.h>
#include <terralib/qt/af/connectors/MapDisplay.h>

// STL
#include <memory>

// Qt
#include <QDialog>

namespace Ui { class TileGeneratorDialogForm; }

namespace geopx
{
  namespace tools
  {

    /*!
      \class TileGeneratorDialog

      \brief This interface i used to get the input parameters for  tile generation.
    */
    class TileGeneratorDialog : public QDialog
    {
      Q_OBJECT

      public:

        TileGeneratorDialog(QWidget* parent = 0, Qt::WindowFlags f = 0);

        ~TileGeneratorDialog();

      public:

        void setExtentInfo(te::gm::Envelope env, int srid);

        void setLayerList(std::list<te::map::AbstractLayerPtr> list);

        void setMapDisplay(te::qt::af::MapDisplay* mapDisplay);

      protected slots:

        void onEnvelopeAcquired(te::gm::Envelope env);

        void onToolButtonClicked(bool flag);

        void onDirToolButtonClicked();

        void onValidatePushButtonClicked();

        void onOkPushButtonClicked();

      private:

        std::unique_ptr<Ui::TileGeneratorDialogForm> m_ui;

        std::list<te::map::AbstractLayerPtr> m_layerList;

        te::qt::af::MapDisplay* m_appDisplay;

        te::gm::Envelope m_env;

        int m_srid;

        QAction* m_action;

        bool m_clearTool;
    }; 

  }  // end namespace tools
} // end namespace geopx

#endif  // __GEOPXDESKTOP_TOOLS_TILEGENERATOR_TILEGENERATORDIALOG_H

