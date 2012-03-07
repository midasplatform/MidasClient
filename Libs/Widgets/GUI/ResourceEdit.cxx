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
#include "ResourceEdit.h"
#include "MidasResourceDescTable.h"
#include "midasUtils.h"
#include "mdsDatabaseAPI.h"
#include "mdoCommunity.h"
#include "mdsCommunity.h"
#include "mdoCollection.h"
#include "mdsCollection.h"
#include "mdoItem.h"
#include "mdsItem.h"
#include "mdoBitstream.h"
#include "mdsBitstream.h"
#include "mdoVersion.h"
#include "m3doFolder.h"
#include "m3dsFolder.h"
#include "m3doItem.h"
#include "m3dsItem.h"

ResourceEdit::ResourceEdit()
{
}

ResourceEdit::~ResourceEdit()
{
}

void ResourceEdit::Save(QTableWidgetItem* row)
{
  std::string data = row->data(Qt::DisplayRole).toString().toStdString();
  midasUtils::StringTrim(data);

  if( DB_IS_MIDAS3 )
    {
    QTableWidgetMidas3FolderDescItem* folderRow =
      dynamic_cast<QTableWidgetMidas3FolderDescItem *>(row);
    QTableWidgetMidas3ItemDescItem*   itemRow =
      dynamic_cast<QTableWidgetMidas3ItemDescItem *>(row);

    if( folderRow != NULL )
      {
      this->SaveFolder3(folderRow->getModelData(), folderRow->getField(), data);
      }
    else if( itemRow != NULL )
      {
      this->SaveItem3(itemRow->getModelData(), itemRow->getField(), data, itemRow->getFieldName());
      }
    }
  else
    {
    QTableWidgetMidasCommunityDescItem*  commRow = NULL;
    QTableWidgetMidasCollectionDescItem* collRow = NULL;
    QTableWidgetMidasItemDescItem*       itemRow = NULL;
    QTableWidgetMidasBitstreamDescItem*  bitstreamRow = NULL;

    if( (commRow = dynamic_cast<QTableWidgetMidasCommunityDescItem *>(row) ) != NULL )
      {
      this->SaveCommunity(commRow->getModelData(), commRow->getField(), data);
      }
    else if( (collRow = dynamic_cast<QTableWidgetMidasCollectionDescItem *>(row) ) != NULL )
      {
      this->SaveCollection(collRow->getModelData(), collRow->getField(), data);
      }
    else if( (itemRow = dynamic_cast<QTableWidgetMidasItemDescItem *>(row) ) != NULL )
      {
      this->SaveItem(itemRow->getModelData(), itemRow->getField(), data);
      }
    else if( (bitstreamRow = dynamic_cast<QTableWidgetMidasBitstreamDescItem *>(row) ) != NULL )
      {
      this->SaveBitstream(bitstreamRow->getModelData(), itemRow->getField(), data);
      }
    }
}

void ResourceEdit::SaveCommunity(mdo::Community* comm, MIDASFields field,
                                 std::string data)
{
  bool changed = false;

  switch( field )
    {
    case COMMUNITY_NAME:
      if( comm->GetName() != data )
        {
        comm->SetName(data.c_str() );
        changed = true;
        }
      break;
    case COMMUNITY_DESCRIPTION:
      if( comm->GetDescription() != data )
        {
        comm->SetDescription(data.c_str() );
        changed = true;
        }
      break;
    case COMMUNITY_INTRODUCTORY:
      if( comm->GetIntroductoryText() != data )
        {
        comm->SetIntroductoryText(data.c_str() );
        changed = true;
        }
      break;
    case COMMUNITY_COPYRIGHT:
      if( comm->GetCopyright() != data )
        {
        comm->SetCopyright(data.c_str() );
        changed = true;
        }
      break;
    case COMMUNITY_LINKS:
      if( comm->GetLinks() != data )
        {
        comm->SetLinks(data.c_str() );
        changed = true;
        }
      break;
    default:
      return;
    }

  if( changed )
    {
    mds::Community mdsComm;
    mdsComm.SetObject(comm);
    mdsComm.MarkAsDirty();
    mdsComm.Commit();

    this->Log->Status("Community saved successfully");
    this->Log->Message("Community saved successfully");

    emit DataModified(comm->GetUuid() );
    }
}

void ResourceEdit::SaveCollection(mdo::Collection* coll, MIDASFields field,
                                  std::string data)
{
  bool changed = false;

  switch( field )
    {
    case COLLECTION_NAME:
      if( coll->GetName() != data )
        {
        coll->SetName(data.c_str() );
        changed = true;
        }
      break;
    case COLLECTION_DESCRIPTION:
      if( coll->GetDescription() != data )
        {
        coll->SetDescription(data.c_str() );
        changed = true;
        }
      break;
    case COLLECTION_COPYRIGHT:
      if( coll->GetCopyright() != data )
        {
        coll->SetCopyright(data.c_str() );
        changed = true;
        }
      break;
    case COLLECTION_INTRODUCTORY:
      if( coll->GetIntroductoryText() != data )
        {
        coll->SetIntroductoryText(data.c_str() );
        changed = true;
        }
      break;
    default:
      return;
    }

  if( changed )
    {
    mds::Collection mdsColl;
    mdsColl.SetObject(coll);
    mdsColl.MarkAsDirty();
    mdsColl.Commit();

    this->Log->Status("Collection saved successfully");
    this->Log->Message("Collection saved successfully");

    emit DataModified(coll->GetUuid() );
    }
}

void ResourceEdit::SaveItem(mdo::Item* item, MIDASFields field,
                            std::string data)
{
  bool changed = false;

  std::vector<std::string> tokens;

  switch( field )
    {
    case ITEM_TITLE:
      if( item->GetTitle() != data )
        {
        item->SetTitle(data.c_str() );
        changed = true;
        }
      break;
    case ITEM_ABSTRACT:
      if( item->GetAbstract() != data )
        {
        item->SetAbstract(data.c_str() );
        changed = true;
        }
      break;
    case ITEM_DESCRIPTION:
      if( item->GetDescription() != data )
        {
        item->SetDescription(data.c_str() );
        changed = true;
        }
      break;
    case ITEM_KEYWORDS:
      midasUtils::Tokenize(data, tokens, "/", true);
      item->SetKeywords(tokens);
      changed = true;
      break;
    case ITEM_AUTHORS:
      midasUtils::Tokenize(data, tokens, "/", true);
      item->SetAuthors(tokens);
      changed = true;
      break;
    default:
      return;
    }

  if( changed )
    {
    mds::Item mdsItem;
    mdsItem.SetObject(item);
    mdsItem.MarkAsDirty();
    mdsItem.Commit();

    this->Log->Status("Item saved successfully");
    this->Log->Message("Item saved successfully");

    emit DataModified(item->GetUuid() );
    }
}

void ResourceEdit::SaveBitstream(mdo::Bitstream* bitstream, MIDASFields field,
                                 std::string data)
{
  (void)bitstream;
  (void)field;
  (void)data;
  // nothing to do at this time
}

void ResourceEdit::SaveFolder3(m3do::Folder * folder, MIDASFields field, const std::string &data)
{
  bool changed = false;
  switch( field )
    {
    case FOLDER3_NAME:
      if( folder->GetName() != data )
        {
        folder->SetName(data.c_str() );
        changed = true;
        }
      break;
    case FOLDER3_DESCRIPTION:
      if( folder->GetDescription() != data )
        {
        folder->SetDescription(data.c_str() );
        changed = true;
        }
      break;
    default:
      return;
    }

  if( changed )
    {
    m3ds::Folder mdsFolder;
    mdsFolder.SetObject(folder);
    mdsFolder.MarkAsDirty();
    mdsFolder.Commit();

    this->Log->Status("Folder saved successfully");
    this->Log->Message("Folder saved successfully");

    emit DataModified(folder->GetUuid() );
    }
}

void ResourceEdit::SaveItem3(m3do::Item * item, MIDASFields field, const std::string &data,
                             const std::string& fieldName)
{
  bool changed = false;

  switch( field )
    {
    case ITEM3_NAME:
      if( item->GetName() != data )
        {
        item->SetName(data.c_str() );
        changed = true;
        }
      break;
    case ITEM3_DESCRIPTION:
      if( item->GetDescription() != data )
        {
        item->SetDescription(data.c_str() );
        changed = true;
        }
      break;
    case ITEM3_EXTRAFIELD:
      if( fieldName != "" && item->GetExtraField(fieldName) != data )
        {
        item->SetExtraField(fieldName, data);
        changed = true;
        }
      break;
    default:
      return;
    }

  if( changed )
    {
    m3ds::Item mdsItem;
    mdsItem.SetObject(item);
    mdsItem.MarkAsDirty();
    mdsItem.Commit();

    this->Log->Status("Item saved successfully");
    this->Log->Message("Item saved successfully");

    emit DataModified(item->GetUuid() );
    }
}

