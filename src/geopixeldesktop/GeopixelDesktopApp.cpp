/*!
\file geopx-desktop/src/geopixeldesktop/GeopixelDesktopApp.cpp

\brief Qt reimplementation class to treat exceptions.
*/

#include "GeopixelDesktopApp.h"

// STL
#include <exception>
#include <iostream>

// Qt
#include <QMessageBox>

geopx::desktop::GeopixelDesktopApp::GeopixelDesktopApp(int& argc, char** argv) :
  QApplication(argc, argv)
{
}

bool geopx::desktop::GeopixelDesktopApp::notify(QObject* receiver, QEvent* event)
{
  bool done = true;

  try
  {
    done = QApplication::notify(receiver, event);
  }
  catch (const std::exception& ex)
  {
    QMessageBox::warning(0, tr("Geopixel DesktopApp"), QString(ex.what()));

    std::cout << std::endl << "Exception catched: " << ex.what() << std::endl;
  }
  catch (...)
  {
    QMessageBox::warning(0, tr("Geopixel DesktopApp"), "Unknown Error.");

    std::cout << "Unknown exception catched\n";
  }
  
  return done;
}
