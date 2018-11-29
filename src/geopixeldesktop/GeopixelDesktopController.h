/*!
  \file geopx-desktop/src/geopixeldesktop/GeopixelDesktopController.h

  \brief The API for controller of GeopixelDesktop application.
*/

#ifndef __GEOPXDESKTOP_DESKTOP_GEOPIXELDESKTOPCONTROLLER_H
#define __GEOPXDESKTOP_DESKTOP_GEOPIXELDESKTOPCONTROLLER_H

// Terralib
#include <terralib/qt/af/ApplicationController.h>
#include "../Config.h"

// STL
#include <map>
#include <set>
#include <vector>


// Qt
#include <QObject>
#include <QStringList>
#include <QSettings>
#include <QColor>

// Forward declarations
class QAction;
class QActionGroup;
class QMenu;
class QMenuBar;
class QToolBar;
class QWidget;

/*!
  \class GeopixelDesktopController

  \brief The API for controller of GeopixelDesktop application.

*/
namespace geopx
{
  namespace desktop
  {
	class GeopixelDesktopController : public QObject
	{
	  Q_OBJECT

	  public:

		/*!
		  \brief Constructor.

		  \param parent The parent object.
		*/
		GeopixelDesktopController(te::qt::af::ApplicationController* app, std::string appConfigFile);

		/*! 
		  \brief Destructor. 
		*/
		virtual ~GeopixelDesktopController();

		
		/*!
		  \brief Initializes the menus for the most recent open projects.
		*/
		virtual void initializeProjectMenus();

		/*!
		  \brief Update the list of recent projects. This is commonly used when there's a new most recent project.

		  \param prj_file Complete file name for the project file.

		  \param prj_title Title of the project.
		*/
		void updateRecentProjects(const QString& prjFile, const QString& prjTitle);

		/*!
		  \brief Returns the application project extension.

		  \return Application project extension.
		*/
		const QString& getAppProjectExtension() const;

		/*!
		\brief Returns the most recent project.

		\return Application most recent opened project.
		*/
		QString getMostRecentProject() const;

		/*!
		\brief Returns the project extension filter .

		\return Filter string.
		*/
		QString getExtensionFilter();

	  protected:

		te::qt::af::ApplicationController* m_app;

		QString m_appProjectExtension;              //!< Application project extension.
		QStringList m_recentProjs;                  //!< List of the recent projects.
		QStringList m_recentProjsTitles;            //!< List of the titles of the recent projects.
	};
  } // end namespace desktop
}   // end namespace geopx

#endif // __GEOPXDESKTOP_DESKTOP_GEOPIXELDESKTOPCONTROLLER_H

