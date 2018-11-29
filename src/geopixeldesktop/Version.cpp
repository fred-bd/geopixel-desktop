/*!
  \file geopx-desktop/src/geopixeldesktop/Version.cpp

  \brief Utility class for system versioning.
*/

// GeopixelDesktop
#include "../Version.h"
#include "Version.h"

// STL
#include <cassert>

int geopx::desktop::Version::majorNumber()
{
  return GEOPIXELDESKTOP_VERSION_MAJOR;
}

int geopx::desktop::Version::minorNumber()
{
  return GEOPIXELDESKTOP_VERSION_MINOR;
}

int geopx::desktop::Version::patchNumber()
{
  return GEOPIXELDESKTOP_VERSION_PATCH;
}

std::string geopx::desktop::Version::releaseStatus()
{
  assert(GEOPIXELDESKTOP_VERSION_STATUS);
  return std::string(GEOPIXELDESKTOP_VERSION_STATUS);
}

std::string geopx::desktop::Version::buildDate()
{
  assert(__DATE__ " " __TIME__);
  return std::string(__DATE__ " " __TIME__);
}

std::string geopx::desktop::Version::asString()
{
  assert(GEOPIXELDESKTOP_VERSION_STRING);
  return std::string(GEOPIXELDESKTOP_VERSION_STRING);
}

int geopx::desktop::Version::asInt()
{
  return GEOPIXELDESKTOP_VERSION;
}
