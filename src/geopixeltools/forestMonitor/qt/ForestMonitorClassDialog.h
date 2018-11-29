/*!
  \file geopx-desktop/src/geopixeltools/qt/ForestMonitorClassDialog.h

  \brief This interface is used to get the input parameters for forest monitor classification information.
*/

#ifndef __GEOPXDESKTOP_TOOLS_FORESTMONITOR_FORESTMONITORCLASSDIALOG_H
#define __GEOPXDESKTOP_TOOLS_FORESTMONITOR_FORESTMONITORCLASSDIALOG_H

#include "../../Config.h"

// TerraLib
#include <terralib/dataaccess/datasource/DataSource.h>
#include <terralib/dataaccess/datasource/DataSourceInfo.h>
#include <terralib/geometry/Envelope.h>
#include <terralib/maptools/AbstractLayer.h>
#include <terralib/qt/widgets/canvas/MapDisplay.h>
#include <terralib/qt/widgets/progress/ProgressViewerDialog.h>
#include <terralib/raster/Raster.h>
#include <terralib/se/Style.h>

// STL
#include <memory>

// Qt
#include <QDialog>

namespace Ui { class ForestMonitorClassDialogForm; }

namespace geopx
{
  namespace tools
  {
    /*!
      \class ForestMonitorClassDialog

      \brief This interface is used to get the input parameters for forest monitor classification information.
    */
    class ForestMonitorClassDialog : public QDialog
    {
      Q_OBJECT

      public:

        ForestMonitorClassDialog(QWidget* parent = 0, Qt::WindowFlags f = 0);

        ~ForestMonitorClassDialog();

      public:

        void setExtentInfo(te::gm::Envelope env, int srid);

        void setLayerList(std::list<te::map::AbstractLayerPtr> list);

        te::map::AbstractLayerPtr getOutputLayer();

      protected slots:

        void onGenerateNDVISampleClicked();

        void onThresholdSliderReleased();

        void onGenerateErosionSampleClicked();

        void onDilationPushButtonClicked();

        void onErosionPushButtonClicked();

        void onTargetFileToolButtonPressed();

        void onOkPushButtonClicked();

      protected:

        void drawRaster(te::rst::Raster* raster, te::qt::widgets::MapDisplay* mapDisplay, te::se::Style* style = 0);

        te::da::DataSourcePtr createDataSource(std::string repository);

      private:

        std::unique_ptr<Ui::ForestMonitorClassDialogForm> m_ui;

        std::unique_ptr<te::qt::widgets::MapDisplay> m_thresholdDisplay;

        std::unique_ptr<te::qt::widgets::MapDisplay> m_erosionDisplay;

        std::unique_ptr<te::rst::Raster> m_thresholdRaster;

        std::unique_ptr<te::rst::Raster> m_filterRaster;

        std::unique_ptr<te::rst::Raster> m_filterDilRaster;
            
        std::unique_ptr<te::se::Style> m_styleThresholdRaster;

        te::map::AbstractLayerPtr m_outputLayer;                                          //!< Generated Layer.

        te::qt::widgets::ProgressViewerDialog* m_progressDlg;

        int m_progressId;

        te::gm::Envelope m_env;

        int m_srid;
    }; 

  }  // end namespace tools
}  // end namespace geopx

#endif  // __GEOPXDESKTOP_TOOLS_FORESTMONITOR_FORESTMONITORCLASSDIALOG_H

