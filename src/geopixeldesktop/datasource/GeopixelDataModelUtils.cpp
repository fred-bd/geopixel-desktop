/*!
  \file geopx-desktop/src/geopixeldesktop/datasourec/GeopixelDataModelUtils.cpp

  \brief Utility routines for the Geopixel Database Model access.
*/

#include "GeopixelDataModelUtils.h"

// GeopixelDesktop
#include "../Config.h"

std::string geopx::desktop::datasource::GetQueryLayersXYZ(const std::string& profile)
{
  std::string query = "select * from app_param where tma_id IN (";
  query += " select tma_id from app_tema where prf_id = (";
  query += " select prf_id from app_perfil where perfil = '";
  query += profile;
  query += "' ) )";
  query += " and tipo = 'XYZ' order by nome";

  return query;
}

std::string geopx::desktop::datasource::GetQueryLayersVec(const std::string& profile)
{
  std::string query = "select * from app_camada where cmd_id IN (";
  query += " select cmd_id from app_param where tma_id IN (";
  query += " select tma_id from app_tema where prf_id = (";
  query += " select prf_id from app_perfil where perfil = '";
  query += profile;
  query += "' ) )";
  query += " and tipo = 'WMS' order by nome )";

  return query;
}

int geopx::desktop::datasource::GetThemeId(te::da::DataSource* ds, const std::string& profile, const int& cmdId)
{
  std::string query = " select tma_id from app_tema where cmd_id = ";
  query += std::to_string(cmdId);
  query += " and prf_id = (";
  query += " select prf_id from app_perfil where perfil = '";
  query += profile;
  query += "' )";

  std::unique_ptr<te::da::DataSet> dataset = ds->query(query);
  
  if (dataset->moveFirst())
  {
    return dataset->getInt32("tma_id");
  }

  return -1;
}

bool geopx::desktop::datasource::IsLayerEditionEnabled(te::da::DataSource* ds, const int& themeId)
{
  std::string query = "select editavel from app_json where tma_id = ";
  query += std::to_string(themeId);

  std::unique_ptr<te::da::DataSet> dataset = ds->query(query);

  if (dataset->moveFirst())
  {
    return dataset->getBool("editavel");
  }

  return false;
}
