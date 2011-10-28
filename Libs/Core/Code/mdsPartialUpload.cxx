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
#include "mdsPartialUpload.h"

#include "mdsDatabaseAPI.h"
#include "mdoVersion.h"

namespace mds
{

PartialUpload::PartialUpload()
{
  m_Id = 0;
  m_BitstreamId = 0;
  m_UserId = 0;
  m_ParentItem = 0;
}

PartialUpload::~PartialUpload()
{
}

bool PartialUpload::Commit()
{
  if( m_BitstreamId <= 0 || m_Token == "" || m_ParentItem <= 0 )
    {
    return false;
    }
  if( !DB_IS_MIDAS3 && m_UserId <= 0 )
    {
    return false;
    }
  DatabaseAPI db;
  db.Open();

  std::stringstream query;
  query << "INSERT INTO partial_upload (bitstream_id, uploadtoken, user_id, "
  "item_id) VALUES ('" << m_BitstreamId << "', '"
        << m_Token << "', '"
        << m_UserId << "', '"
        << m_ParentItem << "')";

  if( !db.Database->ExecuteQuery(query.str().c_str() ) )
    {
    db.Close();
    return false;
    }
  db.Close();
  return true;
}

bool PartialUpload::Remove()
{
  if( m_BitstreamId <= 0 )
    {
    return false;
    }
  DatabaseAPI db;
  db.Open();

  std::stringstream query;
  query << "DELETE FROM partial_upload WHERE bitstream_id='"
        << m_BitstreamId << "'";

  if( !db.Database->ExecuteQuery(query.str().c_str() ) )
    {
    db.Close();
    return false;
    }
  db.Close();
  return true;
}

bool PartialUpload::FetchAll(std::vector<PartialUpload *>& list)
{
  DatabaseAPI db;
  db.Open();

  if( !db.Database->ExecuteQuery("SELECT id, uploadtoken, bitstream_id, "
                                 "user_id, item_id FROM partial_upload") )
    {
    db.Close();
    return false;
    }
  while( db.Database->GetNextRow() )
    {
    PartialUpload* ul = new PartialUpload;
    ul->SetId(db.Database->GetValueAsInt(0) );
    ul->SetToken(db.Database->GetValueAsString(1) );
    ul->SetBitstreamId(db.Database->GetValueAsInt(2) );
    ul->SetUserId(db.Database->GetValueAsInt(3) );
    ul->SetParentItem(db.Database->GetValueAsInt(4) );
    list.push_back(ul);
    }

  db.Close();
  return true;
}

PartialUpload* PartialUpload::FetchByBitstreamId(int id)
{
  DatabaseAPI db;
  db.Open();

  std::stringstream query;
  query << "SELECT id, uploadtoken, user_id, item_id "
    "FROM partial_upload WHERE bitstream_id='" << id << "'";

  if( !db.Database->ExecuteQuery(query.str().c_str() ) )
    {
    db.Close();
    return NULL;
    }
  while( db.Database->GetNextRow() )
    {
    PartialUpload* ul = new PartialUpload;
    ul->SetId(db.Database->GetValueAsInt(0) );
    ul->SetToken(db.Database->GetValueAsString(1) );
    ul->SetUserId(db.Database->GetValueAsInt(2) );
    ul->SetParentItem(db.Database->GetValueAsInt(3) );
    ul->SetBitstreamId(id);
    db.Close();
    return ul;
    }

  db.Close();
  return NULL;
}

bool PartialUpload::RemoveAll()
{
  DatabaseAPI db;
  db.Open();

  if( !db.Database->ExecuteQuery("DELETE FROM partial_upload") )
    {
    db.Close();
    return false;
    }

  db.Close();
  return true;
}

void PartialUpload::SetId(int id)
{
  m_Id = id;
}

int PartialUpload::GetId()
{
  return m_Id;
}

void PartialUpload::SetBitstreamId(int id)
{
  m_BitstreamId = id;
}

int PartialUpload::GetBitstreamId()
{
  return m_BitstreamId;
}

void PartialUpload::SetToken(const std::string& token)
{
  m_Token = token;
}

std::string PartialUpload::GetToken()
{
  return m_Token;
}

void PartialUpload::SetUserId(int id)
{
  m_UserId = id;
}

int PartialUpload::GetUserId()
{
  return m_UserId;
}

void PartialUpload::SetParentItem(int id)
{
  m_ParentItem = id;
}

int PartialUpload::GetParentItem()
{
  return m_ParentItem;
}

}
