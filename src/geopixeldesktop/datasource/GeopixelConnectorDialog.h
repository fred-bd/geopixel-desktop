/*!
  \file geopx-desktop/src/geopixeldesktop/datasource/GeopixelConnectorDialog.h

  \brief  A dialog window for showing the Geopixel connector widget.
*/

#ifndef __GEOPXDESKTOP_DESKTOP_DATASOURCE_GEOPIXELCONNECTORDIALOG_H
#define __GEOPXDESKTOP_DESKTOP_DATASOURCE_GEOPIXELCONNECTORDIALOG_H

// TerraLib
#ifndef Q_MOC_RUN
#include <terralib/dataaccess/datasource/DataSource.h>
#include <terralib/dataaccess/datasource/DataSourceInfo.h>
#endif

// STL
#include <memory>

// Qt
#include <QDialog>

namespace Ui { class GeopixelConnectorDialogForm; }

namespace geopx
{
  namespace desktop
  {
    /*!
      \class GeopixelConnectorDialog

      \brief A dialog window for showing the Geopixel connector widget.
    */
    class GeopixelConnectorDialog : public QDialog
    {
      Q_OBJECT

      public:

        GeopixelConnectorDialog(QWidget* parent = 0, Qt::WindowFlags f = 0);

        ~GeopixelConnectorDialog();

      public:

        const te::da::DataSourceInfoPtr& getDataSource() const;

        const te::da::DataSourcePtr& getDriver() const;

        std::string getProfile();

        std::string getUser();

      public slots:

        void onOpenPushButtonClicked();
        void passwdLineEditEditingFinished();
        void passwordLineEditEditingFinished();
        void onExportPushButtonClicked();
  
      protected:

        void setDatabasesNames(std::vector<std::string> names);

        std::string getConnectionURI(std::string url, std::string databaseName, std::string port);

        std::unique_ptr<te::da::DataSource> getConnection(std::string url, std::string databaseName, std::string port);

        void readConfigFile();

        void writeConfigFile();

      protected:

        std::unique_ptr<Ui::GeopixelConnectorDialogForm> m_ui;

        te::da::DataSourceInfoPtr m_datasource;
        te::da::DataSourcePtr m_driver;

        bool m_validConfigFile;
      };

  } // end namespace desktop
} // end namespace geopx

#endif  // __GEOPXDESKTOP_DESKTOP_DATASOURCE_GEOPIXELCONNECTORDIALOG_H

