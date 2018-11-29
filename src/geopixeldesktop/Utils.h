/*!
  \file geopx-desktop/src/geopixeldesktop/Utils.h

  \brief Utility routines for the Geopixel Desktop Application.
*/

#ifndef __GEOPXDESKTOP_DESKTOP_UTILS_H
#define __GEOPXDESKTOP_DESKTOP_UTILS_H

// Geopixel Desktop
#include "../Config.h"

// STL
#include <string>

// Forward declarations
class QString;

namespace geopx
{
  namespace desktop
  {
    /*!
      \brief

      \return
    */
    std::string FindInPath(const std::string& path);

    /*
    \brief

    \param[out]

    \param[out]
    */
    void GetProjectInformationsFromSettings(QString& defaultAuthor, int& maxSaved);

    /*
    \brief

    \param

    \param
    */
    void SaveProjectInformationsOnSettings(const QString& defaultAuthor, const int& maxSaved);

    /*!
    \brief

    \param
    */
    void SaveOpenLastProjectOnSettings(bool openLast);

    /*!
    \brief

    \return
    */
    bool GetOpenLastProjectFromSettings();

    /*!
    \brief Writes the default project file.
    */
    void WriteDefaultProjectFile(const QString& fileName);

  } // end namespace desktop
}   // end namespace geopx

#endif  // __GEOPXDESKTOP_DESKTOP_UTILS_H

