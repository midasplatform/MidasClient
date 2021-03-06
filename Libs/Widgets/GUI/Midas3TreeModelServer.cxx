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
#include "Midas3TreeModelServer.h"

#include <QPixmap>
#include <QApplication>

#include "Midas3TreeItem.h"
#include "Midas3FolderTreeItem.h"
#include "Midas3ItemTreeItem.h"
#include "Midas3BitstreamTreeItem.h"

#include "m3doFolder.h"
#include "m3wsFolder.h"
#include "m3doItem.h"
#include "m3wsItem.h"
#include "m3doBitstream.h"
#include "m3wsBitstream.h"

#include <iostream>
#include "midasSynchronizer.h"

Midas3TreeModelServer::Midas3TreeModelServer(QObject *parent)
  : Midas3TreeModel(parent)
{
}

Midas3TreeModelServer::~Midas3TreeModelServer()
{
}

/** Populate the tree */
void Midas3TreeModelServer::Populate()
{
  m3ws::Folder  remote;
  m3do::Folder* folder = new m3do::Folder;

  // 0 Id means fetch all top level folders
  folder->SetId(0);
  remote.SetAuthenticator(m_Synch->GetAuthenticator() );
  remote.SetObject(folder);

  if( !remote.FetchTree() )
    {
    return;
    }

  std::vector<m3do::Folder *> folders = folder->GetFolders();

  int row = 0;
  for( std::vector<m3do::Folder *>::const_iterator itF = folders.begin();
       itF != folders.end(); ++itF )
    {
    // Set the name of the folder or community
    QList<QVariant> columnData;
    columnData << (*itF)->GetName().c_str();

    // Add the folder
    Midas3FolderTreeItem* folderItem = new Midas3FolderTreeItem(columnData, this, NULL);
    folderItem->SetFolder(*itF);
    folderItem->SetDynamicFetch(true);
    folderItem->SetFetchedChildren(false);
    m_TopLevelFolders.append(folderItem);

    QModelIndex index = this->index(row, 0);
    this->RegisterResource( (*itF)->GetUuid(), index);

    folderItem->Populate(index);
    folderItem->SetTopLevelFolders(&m_TopLevelFolders);

    row++;
    }
  emit layoutChanged();
  emit ServerPolled();
}

/** Fetch more data */
void Midas3TreeModelServer::fetchMore(const QModelIndex& parent)
{
  if( !parent.isValid() || !canFetchMore(parent) )
    {
    return;
    }
  Midas3TreeItem*       item = const_cast<Midas3TreeItem *>(
    this->GetMidasTreeItem(parent) );
  Midas3FolderTreeItem* folderTreeItem = NULL;
  Midas3ItemTreeItem*   itemTreeItem = NULL;

  if( (folderTreeItem = dynamic_cast<Midas3FolderTreeItem *>(const_cast<Midas3TreeItem *>(item) ) ) != NULL )
    {
    this->FetchFolder(folderTreeItem);
    }
  else if( (itemTreeItem = dynamic_cast<Midas3ItemTreeItem *>(const_cast<Midas3TreeItem *>(item) ) ) != NULL )
    {
    this->FetchItem(itemTreeItem);
    }
  item->SetFetchedChildren(true);
  emit layoutChanged();
  emit FetchedMore();
}

// -------------------------------------------------------------------------
void Midas3TreeModelServer::FetchFolder(Midas3FolderTreeItem* parent)
{
  m3ws::Folder  remote;
  m3do::Folder* folder = parent->GetFolder();

  remote.SetAuthenticator(m_Synch->GetAuthenticator() );
  remote.SetObject(folder);

  if( !remote.FetchTree() )
    {
    // emit fetchError();
    return;
    }

  QModelIndex parentIndex = this->GetIndexByUuid(parent->GetUuid() );
  int         row = 0;
  for( std::vector<m3do::Folder *>::const_iterator i = folder->GetFolders().begin();
       i != folder->GetFolders().end(); ++i )
    {
    QList<QVariant> name;
    name << (*i)->GetName().c_str();
    Midas3FolderTreeItem* subfolder = new Midas3FolderTreeItem(name, this, parent);
    subfolder->SetFolder(*i);
    subfolder->SetDynamicFetch(true);
    subfolder->SetFetchedChildren(false);
    parent->AppendChild(subfolder);

    QModelIndex index = this->index(row, 0, parentIndex);
    this->RegisterResource( (*i)->GetUuid(), index);
    row++;
    }
  for( std::vector<m3do::Item *>::const_iterator i = folder->GetItems().begin();
       i != folder->GetItems().end(); ++i )
    {
    QList<QVariant> name;
    name << (*i)->GetName().c_str();
    Midas3ItemTreeItem* item = new Midas3ItemTreeItem(name, this, parent);
    item->SetItem(*i);
    item->SetDynamicFetch(true);
    item->SetFetchedChildren(false);
    parent->AppendChild(item);

    QModelIndex index = this->index(row, 0, parentIndex);
    this->RegisterResource( (*i)->GetUuid(), index);
    row++;
    }
  emit layoutChanged();
}

// -------------------------------------------------------------------------
void Midas3TreeModelServer::FetchItem(Midas3ItemTreeItem* parent)
{
  m3ws::Item  remote;
  m3do::Item* item = parent->GetItem();

  remote.SetAuthenticator(m_Synch->GetAuthenticator() );
  remote.SetObject(item);

  if( !remote.Fetch() )
    {
    // emit fetchError();
    return;
    }

  QModelIndex parentIndex = this->GetIndexByUuid(parent->GetUuid() );
  int         row = 0;
  for( std::vector<m3do::Bitstream *>::const_iterator i = item->GetBitstreams().begin();
       i != item->GetBitstreams().end(); ++i )
    {
    QList<QVariant> name;
    name << (*i)->GetName().c_str();
    Midas3BitstreamTreeItem* bitstream = new Midas3BitstreamTreeItem(name, this, parent);
    bitstream->SetBitstream(*i);
    parent->AppendChild(bitstream);
    QModelIndex       index = this->index(row, 0, parentIndex);
    std::stringstream uuid;
    uuid << (*i)->GetChecksum() << (*i)->GetId(); // bitstreams have no uuid, so
                                                  // we fudge one
    this->RegisterResource(uuid.str(), index);
    row++;
    }
  emit layoutChanged();
}

// -------------------------------------------------------------------------
void Midas3TreeModelServer::ItemExpanded(const QModelIndex& index)
{
  Midas3TreeItem* item = const_cast<Midas3TreeItem *>(this->GetMidasTreeItem(index) );

  item->SetDecorationRole(Midas3TreeItem::Expanded);

  if( m_AlterList && item->GetType() == midas3ResourceType::COMMUNITY )
    {
    m_ExpandedList.insert(item->GetUuid() );
    }
}

