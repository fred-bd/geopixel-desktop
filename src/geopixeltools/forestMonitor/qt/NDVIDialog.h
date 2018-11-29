/*!
  \file geopx-desktop/src/geopixeltools/qt/ForestMonitorClassDialog.h

  \brief This interface is used to get the input parameters for NDVI operation.
*/

#ifndef __GEOPXDESKTOP_TOOLS_FORESTMONITOR_NDVIDIALOG_H
#define __GEOPXDESKTOP_TOOLS_FORESTMONITOR_NDVIDIALOG_H

#include "../../Config.h"

// TerraLib
#include <terralib/dataaccess/datasource/DataSourceInfo.h>
#include <terralib/maptools/AbstractLayer.h>

// STL
#include <memory>

// Qt
#include <QDialog>

namespace Ui { class NDVIDialogForm; }

namespace geopx
{
  namespace tools
  {
    /*!
      \class NDVIDialog

      \brief This interface is used to get the input parameters for NDVI operation.
    */
    class NDVIDialog : public QDialog
    {
      Q_OBJECT

      public:

        NDVIDialog(QWidget* parent = 0, Qt::WindowFlags f = 0);

        ~NDVIDialog();

      public:

        void setLayerList(std::list<te::map::AbstractLayerPtr> list);

        te::map::AbstractLayerPtr getOutputLayer();

      protected slots:

        void onNIRLayerCmbActivated(int idx);

        void onVISLayerCmbActivated(int idx);

        void onOkPushButtonClicked();

        void onTargetFileToolButtonPressed();

      private:

        std::unique_ptr<Ui::NDVIDialogForm> m_ui;

        te::map::AbstractLayerPtr m_outputLayer;                                          //!< Generated Layer.
    }; 
  }  // end namespace tools
}  // end namespace geopx

#endif  // __GEOPXDESKTOP_TOOLS_FORESTMONITOR_NDVIDIALOG_H

