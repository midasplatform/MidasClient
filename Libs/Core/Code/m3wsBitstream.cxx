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

#include "m3wsBitstream.h"
#include "m3doBitstream.h"
#include "m3doItem.h"
#include "m3wsItem.h"
#include "mdsPartialDownload.h"
#include "mdsPartialUpload.h"
#include "mwsRestResponseParser.h"

#include <QDir>
#include <QFileInfo>
#include <QScriptEngine>
#include <QScriptValueIterator>

namespace m3ws
{

// ------------------------------------------------------------------

/** Constructor */
Bitstream::Bitstream()
{
  m_Bitstream = NULL;
}

/** Destructor */
Bitstream::~Bitstream()
{
}

// Set the object
void Bitstream::SetObject(mdo::Object* object)
{
  m_Bitstream = static_cast<m3do::Bitstream *>(object);
}

/** Fetch */
bool Bitstream::Fetch()
{
  // this get method also fetches the subtree, so the behavior is the same
  return this->FetchTree();
}

bool Bitstream::FetchTree()
{
  if( !m_Bitstream )
    {
    std::cerr << "Bitstream::Fetch() : Bitstream not set" << std::endl;
    return false;
    }

  if( m_Bitstream->IsFetched() )
    {
    return true;
    }

  if( !m_Bitstream->GetId() )
    {
    std::cerr << "Bitstream::Fetch() : Bitstream id not set" << std::endl;
    return false;
    }

  mws::RestResponseParser parser;
  m_Bitstream->Clear();
  parser.AddTag("name", m_Bitstream->GetName() );
  parser.AddTag("checksum", m_Bitstream->GetChecksum() );
  // parser.AddTag("uuid", m_Bitstream->GetUuid());
  parser.AddTag("item_id", m_Bitstream->GetParentStr() );
  parser.AddTag("size", m_Bitstream->GetSize() );
  // parser.AddTag("hasAgreement", m_Item->RefAgreement());

  std::stringstream url;
  url << "midas.bitstream.get&id=" << m_Bitstream->GetId();
  if( !mws::WebAPI::Instance()->Execute(url.str().c_str(), &parser) )
    {
    return false;
    }
  m_Bitstream->SetFetched(true);
  return true;
}

bool Bitstream::FetchParent()
{
  int id = m_Bitstream->GetParentId();

  if( id )
    {
    m3do::Item* parent = new m3do::Item;
    m_Bitstream->SetParentItem(parent);
    parent->SetId(id);

    m3ws::Item remote;
    remote.SetObject(parent);
    return remote.Fetch();
    }
  return true;
}

bool Bitstream::Delete()
{
  if( !m_Bitstream )
    {
    std::cerr << "Bitstream::Delete() : Bitstream not set" << std::endl;
    return false;
    }

  if( !m_Bitstream->GetId() )
    {
    std::cerr << "Bitstream::Delete() : Bitstream id not set" << std::endl;
    return false;
    }

  std::stringstream url;
  url << "midas.bitstream.delete&id=" << m_Bitstream->GetId();
  if( !mws::WebAPI::Instance()->Execute(url.str().c_str() ) )
    {
    return false;
    }
  return true;
}

bool Bitstream::Commit()
{
  return true; // NOP
}

bool Bitstream::Download()
{
  qint64 offset = 0;
  mds::PartialDownload* partial =
    mds::PartialDownload::FetchByUuid(m_Bitstream->GetChecksum() );
  QString path;
  if( partial )
    {
    path = partial->GetPath().c_str();
    QFileInfo fileInfo(path);
    offset = fileInfo.exists() ? fileInfo.size() : 0;
    }
  else
    {
    path = m_Bitstream->GetPath().c_str();
    }

  mds::PartialDownload thisPartial;
  thisPartial.SetPath(m_Bitstream->GetPath() );
  thisPartial.SetUuid(m_Bitstream->GetChecksum() );
  thisPartial.SetParentItem(m_Bitstream->GetParentItem()->GetId() );
  if( !partial || (partial && partial->GetPath() != m_Bitstream->GetPath() ) )
    {
    thisPartial.Commit();
    }

  std::stringstream fields;

  fields << "midas.bitstream.download";
  if( m_Bitstream->GetChecksum() != "")
    {
    fields << "&checksum=" << m_Bitstream->GetChecksum();
    }
  else if( m_Bitstream->GetId() > 0 )
    {
    fields << "&id=" << m_Bitstream->GetId();
    }
  else
    {
    delete partial;
    return false;
    }
  bool ok = mws::WebAPI::Instance()->DownloadFile(fields.str().c_str(),
    path.toStdString().c_str(), offset);

  if( partial && ok )
    {
    if( path.toStdString() != m_Bitstream->GetPath() )
      {
      if( !QFile::copy(path, m_Bitstream->GetPath().c_str() ) )
        {
        ok = false;
        }
      }
    else
      {
      partial->Remove();
      }
    }
  delete partial;

  if( ok )
    {
    thisPartial.Remove();
    }

  return ok;
}

bool Bitstream::Upload()
{
  qint64 offset = 0;
  std::string uploadToken;
  std::stringstream fields;

  mds::PartialUpload* partial =
    mds::PartialUpload::FetchByBitstreamId(m_Bitstream->GetId() );
  mds::PartialUpload thisPartial;

  if( partial )
    {
    uploadToken = partial->GetToken();
    std::string offsetStr;
    mws::RestResponseParser parser;
    parser.AddTag("offset", offsetStr);
    fields << "midas.upload.getoffset&uploadtoken=" << uploadToken;
    if( !mws::WebAPI::Instance()->Execute(fields.str().c_str(), &parser) )
      {
      return false;
      }
    QString qOffsetStr(offsetStr.c_str() );
    offset = qOffsetStr.toLongLong();
    }
  else
    {
    mws::RestResponseParser parser;
    parser.AddTag("token", uploadToken);
    fields << "midas.upload.generatetoken&itemid="
           << m_Bitstream->GetParentId()
           << "&checksum=" << m_Bitstream->GetChecksum()
           << "&filename=" << m_Bitstream->GetName();
    if( !mws::WebAPI::Instance()->Execute(fields.str().c_str(), &parser) )
      {
      return false;
      }
    if( uploadToken == "" ) // server already has this file, no need to upload
      {
      return true;
      }

    thisPartial.SetToken(uploadToken);
    thisPartial.SetBitstreamId(m_Bitstream->GetId() );
    thisPartial.SetParentItem(m_Bitstream->GetParentId() );
    thisPartial.Commit();
    }
  
  // Perform the actual upload
  fields.str(std::string() );
  fields << "midas.upload.perform&mode=stream&revision=head"
  "&itemid=" << m_Bitstream->GetParentId()
         << "&filename=" << m_Bitstream->GetName()
         << "&length=" << m_Bitstream->GetSize()
         << "&uploadtoken=" << uploadToken;

  if( !mws::WebAPI::Instance()->UploadFile(fields.str().c_str(),
                                           m_Bitstream->GetPath().c_str(),
                                           offset) )
    {
    return false;
    }

  if( partial )
    {
    partial->Remove();
    }
  else
    {
    thisPartial.Remove();
    }
  return true;
}

} // end namespace
