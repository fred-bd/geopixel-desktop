/*!
 \file geopx-desktop/src/geopixeltools/photoindex/qt/PhotoIndexDialog.h

 \brief This interface is used to get the input parameters for photo index information.
*/

#ifndef __GEOPXDESKTOP_TOOLS_PHOTOINDEX_PHOTOINDEXDIALOG_H
#define __GEOPXDESKTOP_TOOLS_PHOTOINDEX_PHOTOINDEXDIALOG_H

#include "../../Config.h"

// TerraLib
#include <terralib/dataaccess/datasource/DataSourceInfo.h>
#include <terralib/maptools/AbstractLayer.h>

// STL
#include <memory>

// Qt
#include <QDialog>

namespace Ui { class PhotoIndexDialogForm; }

namespace geopx
{
  namespace tools
  {

    /*!
      \class PhotoIndexDialog

      \brief This interface is used to get the input parameters for photo index information.
    */
    class GEOPXTOOLSEXPORT PhotoIndexDialog : public QDialog
    {
      Q_OBJECT

      public:

        PhotoIndexDialog(QWidget* parent = 0, Qt::WindowFlags f = 0);

        ~PhotoIndexDialog();

      public:

        te::map::AbstractLayerPtr getOutputLayer();

      protected slots:

        void onDirToolButtonClicked();

        void onOkPushButtonClicked();

        void onTargetDatasourceToolButtonPressed();

        void onTargetFileToolButtonPressed();

      private:

        std::unique_ptr<Ui::PhotoIndexDialogForm> m_ui;

        te::da::DataSourceInfoPtr m_outputDatasource;

        te::map::AbstractLayerPtr m_outputLayer;         //!< Generated Layer.

        bool m_toFile;
    }; 

  } // end namespace qt
} // end namespace te

#endif  // __GEOPXDESKTOP_TOOLS_PHOTOINDEX_PHOTOINDEXDIALOG_H

