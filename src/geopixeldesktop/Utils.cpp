/*!
\file geopx-desktop/src/geopixeldesktop/Utils.cpp

\brief Utility routines for the Geopixel Desktop Application.
*/

#include "Utils.h"

// GeopixelDesktop
#include "../Config.h"

// TerraLib
#include <terralib/core/filesystem/FileSystem.h>
#include <terralib/core/utils/Platform.h>
#include <terralib/Version.h>

// Boost
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>

// STL
#include <cassert>
#include <fstream>
#include <memory>

// Qt
#include <QApplication>
#include <QSettings>

std::string geopx::desktop::FindInPath(const std::string& path)
{
  //check install dir
  boost::filesystem::path tl_path = te::core::FileSystem::executableDirectory();

  tl_path /= "..";

  boost::filesystem::path eval_path = tl_path / path;

  if (boost::filesystem::exists(eval_path))
    return eval_path.string();

  //check develop dir
  tl_path = GEOPIXELDESKTOP_SOURCE_PATH;

  eval_path = tl_path / path;

  if (boost::filesystem::exists(eval_path))
    return eval_path.string();

  //check terralib dirs
  return te::core::FindInTerraLibPath(path);
}


void geopx::desktop::GetProjectInformationsFromSettings(QString& defaultAuthor, int& maxSaved)
{
  QSettings sett(QSettings::IniFormat, QSettings::UserScope, qApp->organizationName(), qApp->applicationName());

  sett.beginGroup("projects");
  defaultAuthor = sett.value("author_name").toString();
  maxSaved = sett.value("recents_history_size").toInt();
  sett.endGroup();
}

void geopx::desktop::SaveProjectInformationsOnSettings(const QString& defaultAuthor, const int& maxSaved)
{
  QSettings sett(QSettings::IniFormat, QSettings::UserScope, qApp->organizationName(), qApp->applicationName());

  sett.beginGroup("projects");
  sett.setValue("author_name", defaultAuthor);
  sett.setValue("recents_history_size", maxSaved);
  sett.endGroup();
}

void geopx::desktop::SaveOpenLastProjectOnSettings(bool openLast)
{
  QSettings sett(QSettings::IniFormat, QSettings::UserScope, qApp->organizationName(), qApp->applicationName());

  sett.setValue("projects/openLastDataSource", openLast);
}

bool geopx::desktop::GetOpenLastProjectFromSettings()
{
  QSettings sett(QSettings::IniFormat, QSettings::UserScope, qApp->organizationName(), qApp->applicationName());

  QVariant variant = sett.value("projects/openLastDataSource");

  // If the option was never edited
  if (variant.isNull() || !variant.isValid())
    return true;

  return variant.toBool();
}

void geopx::desktop::WriteDefaultProjectFile(const QString& fileName)
{
  boost::property_tree::ptree p;

  std::string schema_location = te::core::FindInTerraLibPath("share/terralib/schemas/terralib/qt/af/project.xsd");

  //Header
  p.add("Project.<xmlattr>.xmlns:xsd", "http://www.w3.org/2001/XMLSchema-instance");
  p.add("Project.<xmlattr>.xmlns:te_map", "http://www.terralib.org/schemas/maptools");
  p.add("Project.<xmlattr>.xmlns:te_qt_af", "http://www.terralib.org/schemas/qt/af");
  p.add("Project.<xmlattr>.xmlns", "http://www.terralib.org/schemas/qt/af");
  p.add("Project.<xmlattr>.xsd:schemaLocation", "http://www.terralib.org/schemas/qt/af " + schema_location);
  p.add("Project.<xmlattr>.version", TERRALIB_VERSION_STRING);

  //Contents
  p.add("Project.Title", "Default project");
  p.add("Project.Author", "");
  p.add("Project.ComponentList", "");
  p.add("Project.te_map:LayerList", "");

  //Store file
  boost::property_tree::xml_writer_settings<std::string> settings('\t', 1);
  boost::property_tree::write_xml(fileName.toUtf8().data(), p, std::locale(), settings);
}
