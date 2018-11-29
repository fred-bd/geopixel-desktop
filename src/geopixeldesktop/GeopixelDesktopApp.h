/*!
\file geopx-desktop/src/geopixeldesktop/GeopixelDesktopApp.h

\brief Qt reimplementation class to treat exceptions.
*/

#ifndef __GEOPXDESKTOP_DESKTOP_GEOPIXELDESKTOPAPP_H
#define __GEOPXDESKTOP_DESKTOP_GEOPIXELDESKTOPAPP_H

// Qt
#include <QApplication>

namespace geopx
{
  namespace desktop
  {
    class GeopixelDesktopApp : public QApplication
    {
      Q_OBJECT
  
      public:
        GeopixelDesktopApp(int &argc, char ** argv);

      private:
        bool notify(QObject *receiver_, QEvent *event_);
    };
  } // end namespace desktop
}   // end namespace geopx

#endif  // __GEOPXDESKTOP_DESKTOP_GEOPIXELDESKTOPAPP_H
