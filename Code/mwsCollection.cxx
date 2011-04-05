/*=========================================================================

  Copyright (c) Kitware Inc. All rights reserved.
  See Copyright.txt or http://www.Kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "mwsCollection.h"
#include <sstream>
#include <iostream>
#include "mwsRestXMLParser.h"
#include "mwsCommunity.h"
#include "mdoCollection.h"
#include "mdoCommunity.h"
#include "mdoItem.h"

namespace mws{

/** Custom XML parser */
class CollectionXMLParser : public RestXMLParser
{
public:
   
  CollectionXMLParser()
    {
    m_Collection = NULL;
    m_Item = NULL;
    };
    
  ~CollectionXMLParser() 
    {
    };

  /// Callback function -- called from XML parser with start-of-element
  /// information.
  virtual void StartElement(const char * name,const char **atts)
    {
    RestXMLParser::StartElement(name,atts);
    if(!m_CurrentTag.compare("/rsp/items/data"))
      {
      m_Item = new mdo::Item();
      }
    m_CurrentValue = "";  
    }

  /// Callback function -- called from XML parser when ending tag
  /// encountered
  virtual void EndElement(const char *name)
    {
    if(!m_CurrentTag.compare("/rsp/items/data/id"))
      {
      m_Item->SetId(atoi(m_CurrentValue.c_str()));
      }
    else if(!m_CurrentTag.compare("/rsp/items/data/title"))
      {
      m_Item->SetTitle(m_CurrentValue.c_str());
      }
    else if(!m_CurrentTag.compare("/rsp/items/data/uuid"))
      {
      m_Item->SetUuid(m_CurrentValue.c_str());
      }
    else if(!m_CurrentTag.compare("/rsp/items/data"))
      {
      m_Collection->AddItem(m_Item);
      }
    RestXMLParser::EndElement(name);
    }
    
  /// Callback function -- called from XML parser with the character data
  /// for an XML element
  virtual void CharacterDataHandler(const char *inData, int inLength)
    {
    RestXMLParser::CharacterDataHandler(inData,inLength);
    m_CurrentValue.append(inData,inLength);
    }
  
  /** Set the collection object */
  void SetCollection(mdo::Collection* collection) {m_Collection = collection;}
  
protected:

  mdo::Collection*   m_Collection;
  mdo::Item*         m_Item;
  std::string        m_CurrentValue;
};


/** Constructor */
Collection::Collection()
{
  m_Collection=0;
}
  
/** Destructor */
Collection::~Collection()
{
}

/** Add the object */
void Collection::SetObject(mdo::Object* object)
{  
  m_Collection = static_cast<mdo::Collection*>(object);
}

/** Fetch */
bool Collection::Fetch()
{
  if(!m_Collection)
    {
    std::cerr << "Collection::Fetch() : Collection not set" << std::endl;
    return false;
    }

  if(m_Collection->IsFetched())
    {
    return true;
    }
    
  if(!m_Collection->GetId())
    {
    std::cerr << "Collection::Fetch() : Collection id not set" << std::endl;
    return false;
    }
       
  CollectionXMLParser parser;
  parser.SetCollection(m_Collection);
  m_Collection->Clear();
  parser.AddTag("/rsp/name",m_Collection->GetName());
  parser.AddTag("/rsp/description",m_Collection->GetDescription());
  parser.AddTag("/rsp/copyright",m_Collection->GetCopyright());
  parser.AddTag("/rsp/introductory",m_Collection->GetIntroductoryText());
  parser.AddTag("/rsp/uuid",m_Collection->GetUuid());
  parser.AddTag("/rsp/parent",m_Collection->GetParent());
  parser.AddTag("/rsp/hasAgreement",m_Collection->RefAgreement());
  parser.AddTag("/rsp/size",m_Collection->GetSize());

  mws::WebAPI::Instance()->GetRestAPI()->SetXMLParser(&parser);

  std::stringstream url;
  url << "midas.collection.get?id=" << m_Collection->GetId();
  if(!mws::WebAPI::Instance()->Execute(url.str().c_str()))
    {
    std::cout << mws::WebAPI::Instance()->GetErrorMessage() << std::endl;
    return false;
    }
  m_Collection->SetFetched(true);
  return true;
}

/** Commit */
bool Collection::Commit()
{
  return true;
}

bool Collection::FetchParent()
{
  mdo::Community* parent = new mdo::Community;
  m_Collection->SetParentCommunity(parent);
  parent->SetId(m_Collection->GetParentId());

  mws::Community remote;
  remote.SetObject(parent);
  return remote.Fetch();
}

bool Collection::Delete()
{
  if(!m_Collection)
    {
    std::cerr << "Collection::Delete() : Collection not set" << std::endl;
    return false;
    }

  if(!m_Collection->GetId())
    {
    std::cerr << "Collection::Delete() : Collection id not set" << std::endl;
    return false;
    }
       
  RestXMLParser parser;
  mws::WebAPI::Instance()->GetRestAPI()->SetXMLParser(&parser);

  std::stringstream url;
  url << "midas.collection.delete?id=" << m_Collection->GetId();
  if(!mws::WebAPI::Instance()->Execute(url.str().c_str()))
    {
    std::cout << mws::WebAPI::Instance()->GetErrorMessage() << std::endl;
    return false;
    }
  return true;
}

bool Collection::Create()
{
  std::stringstream postData;
  postData << "uuid=" << m_Collection->GetUuid()
    << "&parentid=" << m_Collection->GetParentId()
    << "&name=" << midasUtils::EscapeForURL(m_Collection->GetName())
    << "&description=" <<
    midasUtils::EscapeForURL(m_Collection->GetDescription())
    << "&introductorytext=" <<
    midasUtils::EscapeForURL(m_Collection->GetIntroductoryText())
    << "&copyright=" <<
    midasUtils::EscapeForURL(m_Collection->GetCopyright());

  mws::RestXMLParser parser;
  mws::WebAPI::Instance()->SetPostData(postData.str().c_str());
  mws::WebAPI::Instance()->GetRestAPI()->SetXMLParser(&parser);
  return mws::WebAPI::Instance()->Execute("midas.collection.create");
}

} // end namespace
