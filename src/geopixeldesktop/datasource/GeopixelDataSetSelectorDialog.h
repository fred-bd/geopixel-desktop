/*!
  \file geopx-desktop/src/geopixeldesktop/datasource/GeopixelDataSetSelectorDialog.h

  \brief  A dialog window for showing the data sets from a geopixel data source.
*/

#ifndef __GEOPXDESKTOP_DESKTOP_DATASOURCE_GEOPIXELDATASETSELECTORDIALOG_H
#define __GEOPXDESKTOP_DESKTOP_DATASOURCE_GEOPIXELDATASETSELECTORDIALOG_H

// TerraLib
#ifndef Q_MOC_RUN
#include <terralib/dataaccess/datasource/DataSource.h>
#endif

// STL
#include <memory>

// Qt
#include <QDialog>

namespace Ui { class GeopixelDataSetSelectorDialogForm; }

namespace geopx
{
  namespace desktop
  {
    /*!
      \class GeopixelDataSetSelectorDialog

      \brief  A dialog window for showing the data sets from a geopixel data source.
    */
    class GeopixelDataSetSelectorDialog : public QDialog
    {
      Q_OBJECT

      public:

        GeopixelDataSetSelectorDialog(QWidget* parent = 0, Qt::WindowFlags f = 0);

        ~GeopixelDataSetSelectorDialog();

      public:

        void setConnection(te::da::DataSource* ds, std::string user, std::string profile, bool XYZDataSet);

      protected slots:

        void onAddPushButtonClicked();

      protected:

        void listLayersXYZ();

        void listLayers();

      protected:

        std::unique_ptr<Ui::GeopixelDataSetSelectorDialogForm> m_ui;

        te::da::DataSource* m_dataSource;
        std::string m_profile;
        std::string m_user;
        bool m_xyzDataSet;
      };

  } // end namespace desktop
} // end namespace geopx

#endif  // __GEOPXDESKTOP_DESKTOP_DATASOURCE_GEOPIXELDATASETSELECTORDIALOG_H

