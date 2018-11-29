/*!
\file geopx-desktop/src/geopixeldesktop/AboutDialog.cpp

\brief A Qt dialog showing GeopixelDesktop about window.
*/

// Geopixel Desktop
#include "../Config.h"
#include "AboutDialog.h"
#include "Utils.h"
#include "Version.h"
#include "ui_AboutDialogForm.h"

// TerraLib
#include <terralib/common/Version.h>
#include <terralib/qt/af/ApplicationController.h>
#include <terralib/Version.h>

// Qt
#include <QtGui/QPixmap>

geopx::desktop::AboutDialog::AboutDialog(QWidget* parent, Qt::WindowFlags f)
  : QDialog(parent, f),
    m_ui(new Ui::AboutDialogForm)
{
  m_ui->setupUi(this);

  std::string copyrightStr = tr("<p>Copyright &copy; 2018 Geopixel<BR>").toUtf8().data();
  m_ui->m_copyrightLabel->setText(copyrightStr.c_str());

  std::string logoGeopixLargeFileName = geopx::desktop::FindInPath("share/geopixeldesktop/images/png/geopixeldesktop-horizontal-transp.png");
  QPixmap pixmapGeopixLarge(logoGeopixLargeFileName.c_str());
  m_ui->m_applicationLargeLogo->setPixmap(pixmapGeopixLarge.scaled(QSize(256, 64), Qt::KeepAspectRatio, Qt::SmoothTransformation));
 

  //TerraLib Info

  std::string logoTEFileName = geopx::desktop::FindInPath("share/geopixeldesktop/images/png/terralib-globe.png");
  QPixmap pixmapTE(logoTEFileName.c_str());

  pixmapTE = pixmapTE.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation);

  m_ui->m_terralibLogo->setPixmap(pixmapTE);

  std::string terralibVersionStr = tr("TerraLib Version: ").toUtf8().data() + te::common::Version::asString();
  m_ui->m_terralibVersionLabel->setText(terralibVersionStr.c_str());

  std::string buildDateStr = tr("Build Date: ").toUtf8().data() + te::common::Version::buildDate();
  m_ui->m_buildDateLabel->setText(buildDateStr.c_str());

  //Geopixel Desktop info

  std::string logoFileName = geopx::desktop::FindInPath("share/geopixeldesktop/images/png/geopixeldesktop-icon.png");
  QPixmap pixmapGeopix(logoFileName.c_str());

  pixmapGeopix = pixmapGeopix.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation);

  m_ui->m_geopixelDesktopLogo->setPixmap(pixmapGeopix);

  std::string geopixelVersionStr = tr("Geopixel Desktop Version: ").toUtf8().data() + geopx::desktop::Version::asString();
  m_ui->m_geopixelDesktopVersionLabel->setText(geopixelVersionStr.c_str());

  std::string geopixelbuildDateStr = tr("Build Date: ").toUtf8().data() + geopx::desktop::Version::buildDate();
  m_ui->m_geopixelDesktopBuildDateLabel->setText(geopixelbuildDateStr.c_str());
}

geopx::desktop::AboutDialog::~AboutDialog() = default;
