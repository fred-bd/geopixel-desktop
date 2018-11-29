/*!
\file geopx-desktop/src/geopixeldesktop/Project.cpp

\brief This class models the concept of a project for the GeopixelDesktop.
*/

#include "Project.h"

// TerraLib
#include <terralib/common/BoostUtils.h>
#include <terralib/core/utils/Platform.h>
#include <terralib/core/uri/Utils.h>
#include <terralib/dataaccess/datasource/DataSourceInfoManager.h>
#include <terralib/dataaccess/serialization/xml/Serializer.h>
#include <terralib/maptools/serialization/xml/Layer.h>
#include <terralib/xml/AbstractWriter.h>
#include <terralib/xml/AbstractWriterFactory.h>
#include <terralib/xml/Reader.h>
#include <terralib/xml/ReaderFactory.h>
#include <terralib/Version.h>

// Boost
#include <boost/algorithm/string/replace.hpp>

// Qt
#include <QFileInfo>

void UpdateLayerDataSource(const te::map::AbstractLayerPtr& layer, const std::map<std::string, std::string>& invalidToValidIds)
{
  size_t count = layer->getChildrenCount();

  for (size_t i = 0; i < count; ++i)
  {
    te::map::AbstractLayerPtr child(boost::dynamic_pointer_cast<te::map::AbstractLayer>(layer->getChild(i)));
    if (child.get())
      UpdateLayerDataSource(child, invalidToValidIds);
  }

  std::map<std::string, std::string>::const_iterator end = invalidToValidIds.end();

  if (invalidToValidIds.find(layer->getDataSourceId()) != end)
    layer->setDataSourceId(invalidToValidIds.find(layer->getDataSourceId())->second);
}

std::vector<std::string> GetDataSourceIds(const std::list<te::map::AbstractLayerPtr>& layers)
{
  std::vector<std::string> dsIds;

  for (std::list<te::map::AbstractLayerPtr>::const_iterator itL = layers.begin(); itL != layers.end(); ++itL)
  {
    te::map::AbstractLayerPtr layer = (*itL);

    if (layer->getType() != "FOLDERLAYER")
      dsIds.push_back(layer->getDataSourceId());
    else
    {
      std::list<te::map::AbstractLayerPtr> converted;

      std::list<te::common::TreeItemPtr> children = layer->getChildren();
      std::list<te::common::TreeItemPtr>::iterator childrenB = children.begin();
      std::list<te::common::TreeItemPtr>::iterator childrenE = children.end();

      while (childrenB != childrenE)
      {
        te::map::AbstractLayerPtr currentLayer = dynamic_cast<te::map::AbstractLayer*>((*childrenB).get());
        converted.push_back(currentLayer);
        childrenB++;
      }

      std::vector<std::string> childrenDsIds = GetDataSourceIds(converted);
      dsIds.insert(dsIds.end(), childrenDsIds.begin(), childrenDsIds.end());
    }
  }

  return dsIds;
}

void geopx::desktop::SaveProject(const ProjectMetadata& proj, const std::list<te::map::AbstractLayerPtr>& layers)
{
  std::unique_ptr<te::xml::AbstractWriter> writer(te::xml::AbstractWriterFactory::make());

  writer->setURI(proj.m_fileName.toUtf8().data());

  std::string schema_loc = te::core::FindInTerraLibPath("share/terralib/schemas/terralib/qt/af/project.xsd");

  writer->writeStartDocument("UTF-8", "no");

  writer->writeStartElement("Project");

  boost::replace_all(schema_loc, " ", "%20");

  writer->writeAttribute("xmlns:xsd", "http://www.w3.org/2001/XMLSchema-instance");
  writer->writeAttribute("xmlns:te_da", "http://www.terralib.org/schemas/dataaccess");
  writer->writeAttribute("xmlns:te_map", "http://www.terralib.org/schemas/maptools");
  writer->writeAttribute("xmlns:te_qt_af", "http://www.terralib.org/schemas/common/af");

  writer->writeAttribute("xmlns:se", "http://www.opengis.net/se");
  writer->writeAttribute("xmlns:ogc", "http://www.opengis.net/ogc");
  writer->writeAttribute("xmlns:xlink", "http://www.w3.org/1999/xlink");

  writer->writeAttribute("xmlns", "http://www.terralib.org/schemas/qt/af");
  writer->writeAttribute("xsd:schemaLocation", "http://www.terralib.org/schemas/qt/af " + schema_loc);
  writer->writeAttribute("version", TERRALIB_VERSION_STRING);

  writer->writeElement("Title", proj.m_title.toUtf8().data());
  writer->writeElement("Author", proj.m_author.toUtf8().data());

  //write data source list
  writer->writeStartElement("te_da:DataSourceList");

  writer->writeAttribute("xmlns:te_common", "http://www.terralib.org/schemas/common");

  te::da::DataSourceInfoManager::iterator itBegin = te::da::DataSourceInfoManager::getInstance().begin();
  te::da::DataSourceInfoManager::iterator itEnd = te::da::DataSourceInfoManager::getInstance().end();
  te::da::DataSourceInfoManager::iterator it;

  std::vector<std::string> dsIdVec = GetDataSourceIds(layers);

  for(it=itBegin; it!=itEnd; ++it)
  {

    if (std::find(dsIdVec.begin(), dsIdVec.end(), it->second->getId()) == dsIdVec.end())
      continue;

    writer->writeStartElement("te_da:DataSource");

    writer->writeAttribute("id", it->second->getId());
    writer->writeAttribute("type", it->second->getType());
    writer->writeAttribute("access_driver", it->second->getAccessDriver());

    writer->writeStartElement("te_da:Title");
    writer->writeValue(it->second->getTitle());
    writer->writeEndElement("te_da:Title");

    writer->writeStartElement("te_da:Description");
    writer->writeValue(it->second->getDescription());
    writer->writeEndElement("te_da:Description");

    writer->writeStartElement("te_da:URI");
    writer->writeValue(te::core::URIEncode(it->second->getConnInfoAsString()));
    writer->writeEndElement("te_da:URI");

    writer->writeEndElement("te_da:DataSource");
  }

  writer->writeEndElement("te_da:DataSourceList");
  //end write

  writer->writeStartElement("ComponentList");
  writer->writeEndElement("ComponentList");

  writer->writeStartElement("te_map:LayerList");

  const te::map::serialize::Layer& lserial = te::map::serialize::Layer::getInstance();

  for(std::list<te::map::AbstractLayerPtr>::const_iterator itL = layers.begin(); itL != layers.end(); ++itL)
    lserial.write((*itL).get(), *writer.get());

  writer->writeEndElement("te_map:LayerList");

  writer->writeEndElement("Project");

  writer->writeToFile();
}

void geopx::desktop::LoadProject(const QString& projFile, ProjectMetadata& proj, std::list<te::map::AbstractLayerPtr>& layers)
{
  QFileInfo info(projFile);
  std::string fName = projFile.toUtf8().data();

  if(!info.exists() || !info.isFile())
  {
    QString msg = QObject::tr("Could not read project file: ") + projFile;
    throw te::common::Exception(msg.toUtf8().data());
  }

  std::unique_ptr<te::xml::Reader> xmlReader(te::xml::ReaderFactory::make());

  xmlReader->read(fName);

  if(!xmlReader->next())
  {
    QString msg = QObject::tr("Could not read project information in the file: ") + projFile + ".";
    throw te::common::Exception(msg.toUtf8().data());
  }

  if(xmlReader->getNodeType() != te::xml::START_ELEMENT)
  {
    QString msg = QObject::tr("Error reading the document ") + projFile + QObject::tr(", the start element wasn't found.");
    throw te::common::Exception(msg.toUtf8().data());
  }

  if(xmlReader->getElementLocalName() != "Project")
  {
    QString msg = QObject::tr("The first tag in the document ") + projFile + QObject::tr(" is not 'Project'.");
    throw te::common::Exception(msg.toUtf8().data());
  }

  xmlReader->next();
  assert(xmlReader->getNodeType() == te::xml::START_ELEMENT);
  assert(xmlReader->getElementLocalName() == "Title");

  xmlReader->next();
  assert(xmlReader->getNodeType() == te::xml::VALUE);
  proj.m_title = xmlReader->getElementValue().c_str();

  xmlReader->next(); // End element

  xmlReader->next();
  assert(xmlReader->getNodeType() == te::xml::START_ELEMENT);
  assert(xmlReader->getElementLocalName() == "Author");

  xmlReader->next();

  if(xmlReader->getNodeType() == te::xml::VALUE)
  {
    proj.m_author = xmlReader->getElementValue().c_str();
    xmlReader->next(); // End element
  }

  //  //read data source list from this project
  xmlReader->next();

  assert(xmlReader->getNodeType() == te::xml::START_ELEMENT);
  assert(xmlReader->getElementLocalName() == "DataSourceList");

  xmlReader->next();

  // DataSourceList contract form
  if(xmlReader->getNodeType() == te::xml::END_ELEMENT &&
     xmlReader->getElementLocalName() == "DataSourceList")
  {
    xmlReader->next();
  }

  //Datasources that may need to be updated
  std::map<std::string, std::string> invalidDsConnInfo;

  while((xmlReader->getNodeType() == te::xml::START_ELEMENT) &&
        (xmlReader->getElementLocalName() == "DataSource"))
  {
    te::da::DataSourceInfoPtr ds(te::serialize::xml::ReadDataSourceInfo(*(xmlReader.get())));
    std::string dsId = ds->getId();
    if (!te::da::DataSourceInfoManager::getInstance().add(ds))
      invalidDsConnInfo.insert(std::make_pair(dsId, ds->getId()));
  }

  //  //end read data source list

  assert(xmlReader->getNodeType() == te::xml::START_ELEMENT);
  assert(xmlReader->getElementLocalName() == "ComponentList");
  xmlReader->next(); // End element
  xmlReader->next(); // next after </ComponentList>

  assert(xmlReader->getNodeType() == te::xml::START_ELEMENT);
  assert(xmlReader->getElementLocalName() == "LayerList");

  xmlReader->next();

  const te::map::serialize::Layer& lserial = te::map::serialize::Layer::getInstance();

  // Read the layers
  while((xmlReader->getNodeType() != te::xml::END_ELEMENT) &&
        (xmlReader->getElementLocalName() != "LayerList"))
  {
    te::map::AbstractLayerPtr layer(lserial.read(*(xmlReader.get())));

    assert(layer.get());

    layers.push_back(layer);
  }

  assert(xmlReader->getNodeType() == te::xml::END_ELEMENT);
  assert(xmlReader->getElementLocalName() == "LayerList");

  xmlReader->next();
  assert((xmlReader->getNodeType() == te::xml::END_ELEMENT) || (xmlReader->getNodeType() == te::xml::END_DOCUMENT));
  assert(xmlReader->getElementLocalName() == "Project");

  proj.m_fileName = projFile;

  //updating the datasources of any layers with outdated ids, whenever possible
  for (te::map::AbstractLayerPtr layer : layers) {
    UpdateLayerDataSource(layer, invalidDsConnInfo);
  }
}
