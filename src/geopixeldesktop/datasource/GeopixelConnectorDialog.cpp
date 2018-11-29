/*!
  \file geopx-desktop/src/geopixeldesktop/datasource/GeopixelConnectorDialog.cpp

  \brief  A dialog window for showing the Geopixel connector widget.
*/

#include "../Config.h"
#include "../Utils.h"
#include "GeopixelConnectorDialog.h"
#include "ui_GeopixelConnectorDialogForm.h"

// TerraLib
#include <terralib/dataaccess/datasource/DataSourceFactory.h>

// Boost
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

// STL
#include <bitset>

// Qt
#include <QFileDialog>
#include <QMessageBox>
#include <QCryptographicHash>


geopx::desktop::GeopixelConnectorDialog::GeopixelConnectorDialog(QWidget* parent, Qt::WindowFlags f)
  : QDialog(parent, f)
  , m_ui(new Ui::GeopixelConnectorDialogForm)
  , m_validConfigFile(false)
{
  // add controls
  m_ui->setupUi(this);

  m_ui->m_logo->setPixmap(QIcon(geopx::desktop::FindInPath("share/geopixeldesktop/images/png/geopixeldesktop-icon-transp-2.png").c_str()).pixmap(48, 48));

  connect(m_ui->m_openPushButton, SIGNAL(pressed()), this, SLOT(onOpenPushButtonClicked()));
  connect(m_ui->m_exportPushButton, SIGNAL(pressed()), this, SLOT(onExportPushButtonClicked()));
  connect(m_ui->m_loginLineEdit, SIGNAL(editingFinished()), this, SLOT(passwordLineEditEditingFinished()));
  connect(m_ui->m_passwordLineEdit, SIGNAL(editingFinished()), this, SLOT(passwordLineEditEditingFinished()));

  std::string internalBuildFlag = GEOPIXELDESKTOP_INTERNAL_BUILD;

  readConfigFile();

  if (internalBuildFlag == "ON")
  {
    m_ui->m_connectionGroupBox->setVisible(true);
    m_ui->m_exportPushButton->setVisible(true);
    m_ui->m_aliasLabel->setText("GEOPIXEL");

    connect(m_ui->m_userLineEdit, SIGNAL(editingFinished()), this, SLOT(passwdLineEditEditingFinished()));
    connect(m_ui->m_passwdLineEdit, SIGNAL(editingFinished()), this, SLOT(passwdLineEditEditingFinished()));
  }
  else
  {
    m_ui->m_connectionGroupBox->setVisible(false);
    m_ui->m_exportPushButton->setVisible(false);
  }
}

geopx::desktop::GeopixelConnectorDialog::~GeopixelConnectorDialog() =  default;

const te::da::DataSourceInfoPtr& geopx::desktop::GeopixelConnectorDialog::getDataSource() const
{
  return m_datasource;
}

const te::da::DataSourcePtr& geopx::desktop::GeopixelConnectorDialog::getDriver() const
{
  return m_driver;
}

std::string geopx::desktop::GeopixelConnectorDialog::getProfile()
{
  std::string profile = m_ui->m_profileComboBox->currentText().toUtf8().data();

  return profile;
}

std::string geopx::desktop::GeopixelConnectorDialog::getUser()
{
  std::string user = m_ui->m_loginLineEdit->text().toUtf8().data();

  return user;
}

void geopx::desktop::GeopixelConnectorDialog::onOpenPushButtonClicked() 
{
  std::string internalBuildFlag = GEOPIXELDESKTOP_INTERNAL_BUILD;

  if (internalBuildFlag == "OFF" && !m_validConfigFile)
  {
    QMessageBox::warning(this, tr("Warning"), tr("Config file not found."));
    return;
  }

  if (m_ui->m_profileComboBox->currentText().isEmpty())
  {
    QMessageBox::warning(this, tr("Warning"), tr("Missing information to connect to Geopixel Data Source."));

    return;
  }

  if (m_ui->m_passwdLineEdit->text().isEmpty() || m_ui->m_userLineEdit->text().isEmpty())
  {
    QMessageBox::warning(this, tr("Warning"), tr("Missing information to connect to Geopixel Data Source."));

    return;
  }

  // Perform connection
  std::string url = m_ui->m_ipLineEdit->text().toStdString();
  std::string databaseName = m_ui->m_databaseComboBox->currentText().toStdString();
  std::string port = m_ui->m_portLineEdit->text().toStdString();

  std::unique_ptr<te::da::DataSource> ds = getConnection(url, databaseName, port);
  ds->open();
  m_driver.reset(ds.release());

  if (m_driver.get() == nullptr)
    throw;

  // Create a new data source based on the form data
  m_datasource.reset(new te::da::DataSourceInfo);

  m_datasource->setConnInfo(getConnectionURI(url, databaseName, port));

  boost::uuids::basic_random_generator<boost::mt19937> gen;
  boost::uuids::uuid u = gen();
  std::string dsId = boost::uuids::to_string(u);

  m_datasource->setId(dsId);
  m_driver->setId(dsId);
  m_datasource->setTitle(databaseName);
  m_datasource->setDescription("Geopixel Data Source");
  m_datasource->setAccessDriver("POSTGIS");
  m_datasource->setType("GEOPIXEL");

  accept();
}

void geopx::desktop::GeopixelConnectorDialog::passwdLineEditEditingFinished()
{
  if (!m_ui->m_passwdLineEdit->text().isEmpty() && !m_ui->m_userLineEdit->text().isEmpty())
  {
    try
    {
      std::string url = "localhost";

      if (!m_ui->m_ipLineEdit->text().isEmpty())
        url = m_ui->m_ipLineEdit->text().toStdString();

      std::string port = "5432";

      if (!m_ui->m_portLineEdit->text().isEmpty())
        port = m_ui->m_portLineEdit->text().toStdString();

      std::string strURI = getConnectionURI(url, "template1", port);

      // Get DataSources
      std::vector<std::string> dbNames = te::da::DataSource::getDataSourceNames("POSTGIS", strURI);

      setDatabasesNames(dbNames);
    }
    catch (...)
    {
    }
  }
}

void geopx::desktop::GeopixelConnectorDialog::setDatabasesNames(std::vector<std::string> names)
{
  m_ui->m_databaseComboBox->clear();

  std::sort(names.begin(), names.end());

  for (std::size_t i = 0; i < names.size(); ++i)
  {
    m_ui->m_databaseComboBox->addItem(names[i].c_str());
  }
}

void geopx::desktop::GeopixelConnectorDialog::passwordLineEditEditingFinished()
{
  std::string internalBuildFlag = GEOPIXELDESKTOP_INTERNAL_BUILD;

  if (internalBuildFlag == "OFF" && !m_validConfigFile)
  {
    QMessageBox::warning(this, tr("Warning"), tr("Config file not found."));
    return;
  }

  if (!m_ui->m_passwordLineEdit->text().isEmpty() &&  !m_ui->m_loginLineEdit->text().isEmpty()) 
  {
    try 
    {
      std::string url = m_ui->m_ipLineEdit->text().toStdString();
      std::string databaseName = m_ui->m_databaseComboBox->currentText().toStdString();
      std::string port = m_ui->m_portLineEdit->text().toStdString();

      std::unique_ptr<te::da::DataSource> ds = getConnection(url, databaseName, port);
      ds->open();

      QByteArray password = m_ui->m_passwordLineEdit->text().toUtf8();
      QByteArray login = m_ui->m_loginLineEdit->text().toUtf8();

      //pass will be used in login hash
      QByteArray pass = QCryptographicHash::hash(password, QCryptographicHash::Md5).toHex();
      QString passHash = QString("%1").arg(QString(pass));

      QString loginHash = QString(QCryptographicHash::hash((login + pass), QCryptographicHash::Md5).toHex());

      std::string query = "SELECT * FROM app_acesso WHERE login_hash = '";
      query += loginHash.toStdString() + "' AND pass_hash = '" + passHash.toStdString() + "'";

      std::unique_ptr<te::da::DataSet> datasetUserId = ds->query(query);

      if (!datasetUserId->isEmpty())
      {
        datasetUserId->moveBeforeFirst();
        datasetUserId->moveNext();

        std::string usrID = datasetUserId->getAsString(5);

        query = "SELECT p.perfil FROM app_usuarioxperfil as u ";
        query += "INNER JOIN app_perfil AS p on p.prf_id = u.prf_id WHERE usr_id =" + usrID;

        std::unique_ptr<te::da::DataSet> datasetProfile = ds->query(query);
        datasetProfile->moveBeforeFirst();

        while (datasetProfile->moveNext())
        {
          QString prfl = datasetProfile->getAsString(0).c_str();
          m_ui->m_profileComboBox->addItem(prfl);
        }
      }
    }
    catch (...)
    {
    }
  }
}

void geopx::desktop::GeopixelConnectorDialog::onExportPushButtonClicked()
{
  writeConfigFile();
}

std::string geopx::desktop::GeopixelConnectorDialog::getConnectionURI(std::string url, std::string databaseName, std::string port)
{
  std::string user = m_ui->m_userLineEdit->text().toStdString();;
  std::string passwd = m_ui->m_passwdLineEdit->text().toStdString();;

  std::string strURI = "pgsql://";
  strURI += user;
  strURI += ":";
  strURI += passwd;
  strURI += "@";
  strURI += url + ":";
  strURI += port + "/";
  strURI += databaseName;
  strURI += "?";
  //ssl mode
  strURI += "&PG_SSL_MODE=disable";
  //connection time out
  strURI += "&PG_CONNECT_TIMEOUT=4";
  // get MaxPoolSize
  strURI += "&PG_MAX_POOL_SIZE=1000";
  // get MinPoolSize
  strURI += "&PG_MIN_POOL_SIZE=100";

  return strURI;
}

std::unique_ptr<te::da::DataSource> geopx::desktop::GeopixelConnectorDialog::getConnection(std::string url, std::string databaseName, std::string port)
{
  std::string strURI = getConnectionURI(url, databaseName, port);
  
  std::unique_ptr<te::da::DataSource> ds = te::da::DataSourceFactory::make("POSTGIS", strURI);

  return ds;
}

void geopx::desktop::GeopixelConnectorDialog::readConfigFile()
{
  try
  {
    boost::property_tree::ptree pt;
    boost::property_tree::json_parser::read_json("config.json", pt);

    boost::property_tree::ptree connInfo = pt.get_child("CONNECTION_INFO");

    std::string alias = connInfo.get<std::string>("ALIAS");
    std::string url = connInfo.get<std::string>("URL");
    std::string port = connInfo.get<std::string>("PORT");
    std::string user = connInfo.get<std::string>("USER");
    std::string passwd = connInfo.get<std::string>("PASSWORD");
    std::string database = connInfo.get<std::string>("DATABASE");

    m_ui->m_aliasLineEdit->setText(alias.c_str());
    m_ui->m_ipLineEdit->setText(url.c_str());
    m_ui->m_portLineEdit->setText(port.c_str());
    m_ui->m_userLineEdit->setText(user.c_str());
    m_ui->m_passwdLineEdit->setText(passwd.c_str());
    m_ui->m_databaseComboBox->addItem(database.c_str());

    m_ui->m_aliasLabel->setText(alias.c_str());
  }
  catch (...)
  {
    m_validConfigFile = false;

    return;
  }

  m_validConfigFile = true;
}

void geopx::desktop::GeopixelConnectorDialog::writeConfigFile()
{
  if (m_ui->m_aliasLineEdit->text().isEmpty() ||
      m_ui->m_ipLineEdit->text().isEmpty() ||
      m_ui->m_portLineEdit->text().isEmpty() ||
      m_ui->m_userLineEdit->text().isEmpty() ||
      m_ui->m_passwdLineEdit->text().isEmpty() ||
      m_ui->m_databaseComboBox->currentText().isEmpty() )
  {
    QMessageBox::warning(this, tr("Warning"), tr("Missing information to export connection information."));

    return;
  }

  boost::property_tree::ptree pt;

  boost::property_tree::ptree connInfo;

  connInfo.put("ALIAS", m_ui->m_aliasLineEdit->text().toUtf8().data());
  connInfo.put("URL", m_ui->m_ipLineEdit->text().toUtf8().data());
  connInfo.put("PORT", m_ui->m_portLineEdit->text().toUtf8().data());
  connInfo.put("USER", m_ui->m_userLineEdit->text().toUtf8().data());
  connInfo.put("PASSWORD", m_ui->m_passwdLineEdit->text().toUtf8().data());
  connInfo.put("DATABASE", m_ui->m_databaseComboBox->currentText().toUtf8().data());

  pt.add_child("CONNECTION_INFO", connInfo);

  std::string file = QFileDialog::getSaveFileName(this, tr("Save Connection Info"), "", "JSON File (*.json)").toUtf8().data();

  if(!file.empty())
    boost::property_tree::json_parser::write_json(file, pt);
}