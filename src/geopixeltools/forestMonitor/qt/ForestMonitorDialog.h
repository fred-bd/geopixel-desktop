/*!
  \file geopx-desktop/src/geopixeltools/qt/ForestMonitorDialog.h

  \brief This interface is used to get the input parameters for forest monitor information.
*/

#ifndef __GEOPXDESKTOP_TOOLS_FORESTMONITOR_FORESTMONITORDIALOG_H
#define __GEOPXDESKTOP_TOOLS_FORESTMONITOR_FORESTMONITORDIALOG_H

#include "../../Config.h"

// TerraLib
#include <terralib/dataaccess/datasource/DataSourceInfo.h>
#include <terralib/maptools/AbstractLayer.h>

// STL
#include <memory>

// Qt
#include <QDialog>

namespace Ui { class ForestMonitorDialogForm; }

namespace geopx
{
  namespace tools
  {
    /*!
      \class ForestMonitorDialog

      \brief This interface is used to get the input parameters for forest monitor information.
    */
    class ForestMonitorDialog : public QDialog
    {
      Q_OBJECT

      public:

        ForestMonitorDialog(QWidget* parent = 0, Qt::WindowFlags f = 0);

        ~ForestMonitorDialog();

      public:

        void setLayerList(std::list<te::map::AbstractLayerPtr> list);

        te::map::AbstractLayerPtr getOutputLayer();

      protected slots:

        void onOkPushButtonClicked();

        void onTargetDatasourceToolButtonPressed();

        void onTargetFileToolButtonPressed();

      private:

        std::unique_ptr<Ui::ForestMonitorDialogForm> m_ui;

        te::da::DataSourceInfoPtr m_outputDatasource;

        te::map::AbstractLayerPtr m_outputLayer;                                          //!< Generated Layer.

        bool m_toFile;
    }; 

  } // end namespace tools
} // end namespace geopx

#endif  // __GEOPXDESKTOP_TOOLS_FORESTMONITOR_FORESTMONITORDIALOG_H

