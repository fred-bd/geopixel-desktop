/*!
  \file geopx-desktop/src/geopixeltools/photoindex/core/PhotoIndexService.h

  \brief This file implements the service to photo index information.
*/

#ifndef __GEOPXDESKTOP_TOOLS_PHOTOINDEX_PHOTOINDEXSERVICE_H
#define __GEOPXDESKTOP_TOOLS_PHOTOINDEX_PHOTOINDEXSERVICE_H

#include "../../Config.h"

//TerraLib Includes
#include <terralib/dataaccess/datasource/DataSource.h>
#include <terralib/maptools/AbstractLayer.h>
#include <terralib/memory/DataSet.h>

// STL
#include <memory>

namespace geopx
{
  namespace tools
  {
    //forward declarations
    class PhotoIndex;

    class GEOPXTOOLSEXPORT PhotoIndexService
    {
      public:

        PhotoIndexService();

        ~PhotoIndexService();

      public:

        void setInputParameters(std::string path);

        void setOutputParameters(te::da::DataSourcePtr ds, std::string outputDataSetName);

        void runService();

      protected:

        void checkParameters();

        /*! Function used to create the output dataset type */
        std::unique_ptr<te::da::DataSetType> createDataSetType(int srid);

        /*! Function used to save the output dataset */
        void saveDataSet(te::mem::DataSet* dataSet, te::da::DataSetType* dsType);

        /*! Function used to get the SRID information */
        int getSRID(te::da::DataSourcePtr ds);

      protected:

        std::string m_path;                               //!< Path from input rasters

        te::da::DataSourcePtr m_ds;                       //!< Pointer to the output datasource.

        std::string m_outputDataSetName;                  //!< Attribute that defines the output dataset name
    };
  }     // end namespace qt
}       // end namespace te

#endif //__GEOPXDESKTOP_TOOLS_PHOTOINDEX_PHOTOINDEXSERVICE_H
