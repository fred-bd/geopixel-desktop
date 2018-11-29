/*!
  \file geopx-desktop/src/geopixeltools/photoindex/core/PhotoIndex.h

  \brief This file contains structures and definitions to photo index information.
*/

#ifndef __GEOPXDESKTOP_TOOLS_PHOTOINDEX_PHOTOINDEX_H
#define __GEOPXDESKTOP_TOOLS_PHOTOINDEX_PHOTOINDEX_H

#include "../../Config.h"

// TerraLib
#include <terralib/dataaccess/datasource/DataSource.h>
#include <terralib/dataaccess/dataset/DataSet.h>
#include <terralib/memory/DataSet.h>

//STL Includes
#include <memory>

namespace geopx
{
  namespace tools
  {
    /*!
      \class PhotoIndex
	  
      \brief This file contains structures and definitions to monitor forest information.

    */
    class GEOPXTOOLSEXPORT PhotoIndex
    {
      public:

        PhotoIndex();

        virtual ~PhotoIndex();

      public:

        void execute(te::da::DataSourcePtr dataSource, te::mem::DataSet* dataSet);
    };

  } // end namespace tools
} // end namespace geopx

#endif //__GEOPXDESKTOP_TOOLS_PHOTOINDEX_PHOTOINDEX_H
