/*!
  \file geopx-desktop/src/geopixeldesktop/GeopixelDesktopController.cpp

  \brief The API for controller of GeopixelDesktop application.
*/

// TerraLib
#include <terralib_buildconfig.h>
#include <terralib/common/SystemApplicationSettings.h>
#include <terralib/qt/af/SplashScreenManager.h>
#include <terralib/qt/widgets/utils/ScopedCursor.h>

#include "GeopixelDesktopController.h"
#include "Utils.h"

// Qt
#include <QApplication>
#include <QDesktopServices>
#include <QDir>
#include <QIcon>
#include <QMenu>
#include <QMessageBox>
#include <QResource>
#include <QStandardPaths>
#include <QWidget>

// Boost
#include <boost/filesystem.hpp>

//STL
#include <algorithm>

geopx::desktop::GeopixelDesktopController::GeopixelDesktopController(te::qt::af::ApplicationController* app, std::string appConfigFile)
    : m_app(app)
{
  te::common::SystemApplicationSettings::getInstance().load(appConfigFile);

  m_appProjectExtension = QString::fromUtf8(te::common::SystemApplicationSettings::getInstance().getValue("Application.ProjectExtension").c_str());
}

geopx::desktop::GeopixelDesktopController::~GeopixelDesktopController()
{
  m_appProjectExtension.clear();

  m_recentProjs.clear();

  m_recentProjsTitles.clear();
}

void geopx::desktop::GeopixelDesktopController::initializeProjectMenus()
{
  te::qt::af::SplashScreenManager::getInstance().showMessage("Loading recent projects...");

  try
  {
    QSettings user_settings(QSettings::IniFormat,
                            QSettings::UserScope,
                            QApplication::instance()->organizationName(),
                            QApplication::instance()->applicationName());

    QVariant projPath = user_settings.value("projects/most_recent/path", "");
    QVariant projTitle = user_settings.value("projects/most_recent/title", "");

    QMenu* mnu = m_app->getMenu("File.Recent Projects");

    if(!projPath.toString().isEmpty())
    {
      QAction* act = mnu->addAction(projPath.toString());
      act->setData(projPath);

      mnu->addSeparator();

      m_recentProjs.append(projPath.toString());
      m_recentProjsTitles.append(projTitle.toString());
    }
    
    user_settings.beginGroup("projects");
    
    int nrc = user_settings.beginReadArray("recents");
    
    for(int i = 0; i != nrc; ++i)
    {
      user_settings.setArrayIndex(i);
      QString npath = user_settings.value("project/path").toString();
      QString ntitle = user_settings.value("project/title").toString();
      
      
      QAction* act = mnu->addAction(npath);
      act->setData(npath);
      m_recentProjs.append(npath);
      m_recentProjsTitles.append(ntitle);
    }

    mnu->setEnabled(true);

    te::qt::af::SplashScreenManager::getInstance().showMessage("Recent projects loaded!");
  }
  catch(const std::exception& e)
  {
    te::qt::widgets::ScopedCursor acursor(Qt::ArrowCursor);

    QString msgErr(tr("Error loading the registered projects: %1"));

    msgErr = msgErr.arg(e.what());

    QMessageBox::warning(m_app->getMainWindow(), m_app->getAppTitle(), msgErr);
  }
}

void geopx::desktop::GeopixelDesktopController::updateRecentProjects(const QString& prjFile, const QString& prjTitle)
{
  int pos = m_recentProjs.indexOf(prjFile);

  QString author;
  int maxSaved;

  GetProjectInformationsFromSettings(author, maxSaved);

  if(pos != 0)
  {
    if(pos < 0)
    {
      if(m_recentProjs.size() > maxSaved) // TODO: Size of the list must be configurable.
      {
        m_recentProjs.removeLast();
        m_recentProjsTitles.removeLast();
      }

      m_recentProjs.prepend(prjFile);
      m_recentProjsTitles.prepend(prjTitle);
    }
    else
    {
      m_recentProjs.move(pos, 0);
      m_recentProjsTitles.move(pos, 0);
    }

    if(m_recentProjs.isEmpty())
      return;

    QMenu* mnu = m_app->getMenu("File.Recent Projects");

    mnu->clear();

    mnu->setEnabled(true);

    QString recPrj = m_recentProjs.at(0);
    QAction* act = mnu->addAction(recPrj);
    act->setData(recPrj);

    mnu->addSeparator();

    if(m_recentProjs.size() > 1)
      for(int i=1; i<m_recentProjs.size(); i++)
      {
        recPrj = m_recentProjs.at(i);
        act = mnu->addAction(recPrj);
        act->setData(recPrj);
      }
  }

  QAction* act = m_app->findAction("File.Save Project As");

  if(act != nullptr)
    act->setEnabled(true);
}



const QString& geopx::desktop::GeopixelDesktopController::getAppProjectExtension() const
{
  return m_appProjectExtension;
}


QString geopx::desktop::GeopixelDesktopController::getMostRecentProject() const
{
  return m_recentProjs.isEmpty() ? QString("") : m_recentProjs.front();
}

QString geopx::desktop::GeopixelDesktopController::getExtensionFilter()
{
  QString appName = m_app->getAppName();
  QString appProjectExtension = m_appProjectExtension;
  QString extensionFilter = appName;
  extensionFilter += QString(" (*.");
  extensionFilter += appProjectExtension + ")";

  return extensionFilter;
}

