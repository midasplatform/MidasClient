/******************************************************************************
 * Copyright 2011 Kitware Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************/


#include "mwsSearch.h"
#include "mdoObject.h"
#include "mdoCommunity.h"
#include "mdoCollection.h"
#include "mdoItem.h"
#include "mdoBitstream.h"
#include "m3doFolder.h"
#include "m3doCommunity.h"
#include "m3doItem.h"
#include "midasUtils.h"
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <iostream>
#include "mwsWebAPI.h"
#include "mwsRestResponseParser.h"

#include <QScriptEngine>
#include <QScriptValueIterator>

namespace mws
{
/** Custom JSON parser */
class SearchResponseParser : public RestResponseParser
{
public:

  SearchResponseParser()
  {
    m_Results = NULL;
  };

  ~SearchResponseParser()
  {
  };

  virtual bool Parse(const QString& response)
  {
    if( !RestResponseParser::Parse(response) )
      {
      return false;
      }

    QScriptEngine engine;
    QScriptValue  data = engine.evaluate(response).property("data");
    if( SERVER_IS_MIDAS3 )
	  {
      if( data.property("results").isArray() )
        {
        QScriptValueIterator rslts(data.property("results") );
        while( rslts.hasNext() )
          {
          rslts.next();
		  std::string rsltType = rslts.value().property("resultType").toString().toStdString();
		  if( rsltType.compare("item") == 0 )
		    {
			m3do::Item* item = new m3do::Item();
            item->SetId(rslts.value().property("item_id").toInt32() );
            item->SetUuid(rslts.value().property("uuid").toString().toStdString().c_str() );
            item->SetName(rslts.value().property("name").toString().toStdString().c_str() );
            m_Results->push_back(item);
		    }
		  else if( rsltType.compare("folder") == 0 )
		    {
            m3do::Folder* fldr = new m3do::Folder();
			fldr->SetId(rslts.value().property("folder_id").toInt32() );
            fldr->SetUuid(rslts.value().property("uuid").toString().toStdString().c_str() );
            fldr->SetName(rslts.value().property("name").toString().toStdString().c_str() );
            m_Results->push_back(fldr);
		    }
		  else if( rsltType.compare("community") == 0 )
		    {
		    m3do::Community* comm = new m3do::Community();
			comm->SetFolderId(rslts.value().property("folder_id").toInt32() );
            comm->SetUuid(rslts.value().property("uuid").toString().toStdString().c_str() );
            comm->SetName(rslts.value().property("name").toString().toStdString().c_str() );
            m_Results->push_back(comm);
		    }  
		  // TODO:  else if( rsltType.compare("user") == 0 )
          }
        }
	  }
	else
	  {
       if( data.property("communities").isArray() )
        {
        QScriptValueIterator comms(data.property("communities") );
        while( comms.hasNext() )
          {
          comms.next();
          mdo::Community* comm = new mdo::Community();
          comm->SetId(comms.value().property("community_id").toInt32() );
          comm->SetUuid(comms.value().property("uuid").toString().toStdString().c_str() );
          comm->SetName(comms.value().property("community_name").toString().toStdString().c_str() );
          m_Results->push_back(comm);
          }
        }
      if( data.property("collections").isArray() )
        {
        QScriptValueIterator colls(data.property("collections") );
        while( colls.hasNext() )
          {
          colls.next();
          mdo::Collection* coll = new mdo::Collection();
          coll->SetId(colls.value().property("collection_id").toInt32() );
          coll->SetUuid(colls.value().property("uuid").toString().toStdString().c_str() );
          coll->SetName(colls.value().property("collection_name").toString().toStdString().c_str() );
          m_Results->push_back(coll);
          }
        }
      if( data.property("items").isArray() )
        {
        QScriptValueIterator items(data.property("items") );
        while( items.hasNext() )
          {
          items.next();
          mdo::Item* item = new mdo::Item();
          item->SetId(items.value().property("item_id").toInt32() );
          item->SetUuid(items.value().property("uuid").toString().toStdString().c_str() );
          item->SetTitle(items.value().property("item_name").toString().toStdString().c_str() );
          m_Results->push_back(item);
          }
        }
      if( data.property("bitstreams").isArray() )
        {
        QScriptValueIterator bitstreams(data.property("bitstreams") );
        while( bitstreams.hasNext() )
          {
          bitstreams.next();
          mdo::Bitstream* bitstream = new mdo::Bitstream();
          bitstream->SetId(bitstreams.value().property("bitstream_id").toInt32() );
          bitstream->SetUuid(bitstreams.value().property("uuid").toString().toStdString().c_str() );
          bitstream->SetName(bitstreams.value().property("bitstream_name").toString().toStdString().c_str() );
          m_Results->push_back(bitstream);
          }
        }
	  }

    
    return true;
  }

  /** Set the results object */
  void SetObject(std::vector<mdo::Object *>* results)
  {
    m_Results = results;
  }

protected:
  std::vector<mdo::Object *>* m_Results;
};

std::vector<mdo::Object *> Search::SearchServer(std::vector<std::string> tokens)
{
  std::vector<mdo::Object *> results;
  SearchResponseParser       parser;
  parser.SetObject(&results);

  std::string fields = "midas.resource.search&search=";

  bool space = false;
  for( std::vector<std::string>::iterator i = tokens.begin();
       i != tokens.end(); ++i )
    {
    fields += *i;
    if( space )
      {
      fields += " ";
      }
    space = true;
    }
  mws::WebAPI::Instance()->Execute(fields.c_str(), &parser);
  return results;
}

} // end namespace
