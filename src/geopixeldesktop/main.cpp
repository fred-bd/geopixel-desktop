/*!
  \file geopx-desktop/src/geopixeldesktop/main.cpp

  \brief It contains the main routine of Geopixel Desktop.
*/

// Geopixel Desktop
#include "../Config.h"
#include "GeopixelDesktop.h"
#include "GeopixelDesktopApp.h"
#include "Utils.h"
#include "Version.h"

// TerraLib
#include <terralib/Exception.h>
#include <terralib/common/PlatformUtils.h>
#include <terralib/qt/af/Utils.h>
#include <terralib/qt/af/SplashScreenManager.h>

// STL
#include <cstdlib>
#include <exception>
#include <locale>

// Qt
#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QLibraryInfo>
#include <QLocale>
#include <QMessageBox>
#include <QSplashScreen>
#include <QTextCodec>
#include <QTranslator>

int main(int argc, char** argv)
{
  geopx::desktop::GeopixelDesktopApp app(argc, argv);
  app.setApplicationVersion(QString::fromStdString(geopx::desktop::Version::asString()));

  //load translations
  QDir dir(QLibraryInfo::location(QLibraryInfo::TranslationsPath));

  QStringList filters;
  filters << "*" + QLocale::system().name().toLower() + ".qm";

  QFileInfoList lst = dir.entryInfoList(filters, QDir::Files);

  for (int i = 0; i<lst.size(); ++i)
  {
    QTranslator* trans = new QTranslator;
    trans->load(lst.at(i).baseName(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    app.installTranslator(trans);
  }

  //set locale info
  setlocale(LC_ALL, "C"); // This force to use "." as decimal separator.

  QTextCodec::setCodecForLocale(QTextCodec::codecForLocale());
  
  int waitVal = EXIT_FAILURE;

  const int RESTART_CODE = 1000;

  try
  {
    do
    {
	  //start splash screen
      std::string splash_pix = geopx::desktop::FindInPath(GEOPIXELDESKTOP_SPLASH_SCREEN_PIXMAP);

      QPixmap pixmap(splash_pix.c_str());

      pixmap = pixmap.scaled(720, 540, Qt::KeepAspectRatio, Qt::SmoothTransformation);

      QSplashScreen splash(pixmap/*, Qt::WindowStaysOnTopHint*/);

      splash.setStyleSheet("QWidget { font-size: 12px; font-weight: bold }");

      te::qt::af::SplashScreenManager::getInstance().set(&splash, Qt::AlignBottom | Qt::AlignHCenter, Qt::white);

      splash.show();

	  //start geopixel desktop
      geopx::desktop::GeopixelDesktop geopxDesktop;

      std::string cfg = geopx::desktop::FindInPath("share/geopixeldesktop/config/config.xml");

      geopxDesktop.init(QString::fromUtf8(cfg.c_str()));

      splash.finish(&geopxDesktop);

      geopxDesktop.showMaximized();

      waitVal = app.exec();

    } while (waitVal == RESTART_CODE);
  }
  catch (const boost::exception& e)
  {
    if(const std::string* d = boost::get_error_info<te::ErrorDescription>(e))
      QMessageBox::warning(nullptr, "Geopixel Desktop", d->c_str());
    else
      QMessageBox::warning(nullptr,"Geopixel Desktop", "An unknown error has occurred");
  
    return EXIT_FAILURE;
  }
  catch (const std::exception& e)
  {
    QMessageBox::warning(nullptr, "Geopixel Desktop", e.what());
    return EXIT_FAILURE;
  }
  catch (...)
  {
    return EXIT_FAILURE;
  }

  return waitVal;
}
