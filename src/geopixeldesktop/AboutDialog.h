/*!
\file geopx-desktop/src/geopixeldesktop/AboutDialog.h

\brief A Qt dialog showing GeopixelDesktop about window.
*/

#ifndef __GEOPXDESKTOP_DESKTOP_ABOUTDIALOG_H
#define __GEOPXDESKTOP_DESKTOP_ABOUTDIALOG_H

// STL
#include <memory>

// Qt
#include <QDialog>

// Forward declaration
namespace Ui { class AboutDialogForm; }

namespace geopx
{
  namespace desktop
  {
    class AboutDialog : public QDialog
    {
      Q_OBJECT
  
      public:

        AboutDialog(QWidget* parent = 0, Qt::WindowFlags f = 0);

        ~AboutDialog();

      private:

        std::unique_ptr<Ui::AboutDialogForm> m_ui;
    };
  } // end namespace desktop
}   // end namespace geopx

#endif  // __GEOPXDESKTOP_DESKTOP_ABOUTDIALOG_H
