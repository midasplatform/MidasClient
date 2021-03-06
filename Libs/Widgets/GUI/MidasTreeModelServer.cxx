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
#include "MidasTreeModelServer.h"

#include <QPixmap>
#include <QApplication>

#include "MidasTreeItem.h"
#include "MidasCommunityTreeItem.h"
#include "MidasCollectionTreeItem.h"
#include "MidasItemTreeItem.h"
#include "MidasBitstreamTreeItem.h"

#include "mdoCollection.h"
#include "mwsCollection.h"
#include "mdoItem.h"
#include "mwsItem.h"
#include "mdoBitstream.h"

#include <iostream>
#include <mdoCommunity.h>
#include <mwsCommunity.h>
#include "midasSynchronizer.h"

MidasTreeModelServer::MidasTreeModelServer(QObject *parent)
  : MidasTreeModel(parent)
{
}

MidasTreeModelServer::~MidasTreeModelServer()
{
}

/** Populate the tree */
void MidasTreeModelServer::Populate()
{
  mws::Community  remote;
  mdo::Community* community = new mdo::Community;

  community->SetId(0);
  remote.SetAuthenticator(m_Synch->GetAuthenticator() );
  remote.SetObject(community);

  if( !remote.FetchTree() )
    {
    return;
    }

  std::vector<mdo::Community *> communities = community->GetCommunities();

  int row = 0;
  for( std::vector<mdo::Community *>::const_iterator itCom = communities.begin();
       itCom != communities.end(); ++itCom )
    {
    // Set the name of the community
    QList<QVariant> columnData;
    columnData << (*itCom)->GetName().c_str();

    // Add the community
    MidasCommunityTreeItem* communityItem = new MidasCommunityTreeItem(columnData, this, NULL);
    communityItem->SetCommunity(*itCom);
    communityItem->SetDynamicFetch(true);
    m_TopLevelCommunities.append(communityItem);

    QModelIndex index = this->index(row, 0);
    this->RegisterResource( (*itCom)->GetUuid(), index);

    communityItem->Populate(index);
    communityItem->SetTopLevelCommunities(&m_TopLevelCommunities);

    row++;
    }
  emit layoutChanged();
  emit ServerPolled();
}

/** Fetch more data */
void MidasTreeModelServer::fetchMore( const QModelIndex & parent )
{
  if( !parent.isValid() || !canFetchMore(parent) )
    {
    return;
    }
  MidasTreeItem *           item = const_cast<MidasTreeItem *>(this->GetMidasTreeItem(parent) );
  MidasCollectionTreeItem * collectionTreeItem = NULL;
  MidasItemTreeItem *       itemTreeItem = NULL;

  if( (collectionTreeItem = dynamic_cast<MidasCollectionTreeItem *>( const_cast<MidasTreeItem *>( item ) ) ) != NULL )
    {
    this->FetchCollection(collectionTreeItem);
    }
  else if( (itemTreeItem = dynamic_cast<MidasItemTreeItem *>( const_cast<MidasTreeItem *>( item ) ) ) != NULL )
    {
    this->FetchItem(itemTreeItem);
    }
  item->SetFetchedChildren(true);
  emit layoutChanged();
  emit FetchedMore();
}

// -------------------------------------------------------------------------
void MidasTreeModelServer::FetchCollection(MidasCollectionTreeItem* parent)
{
  mws::Collection  remote;
  mdo::Collection* collection = parent->GetCollection();

  remote.SetAuthenticator(m_Synch->GetAuthenticator() );
  remote.SetObject(collection);

  if( !remote.Fetch() )
    {
    // emit fetchError();
    return;
    }

  int row = 0;
  for( std::vector<mdo::Item *>::const_iterator i = collection->GetItems().begin();
       i != collection->GetItems().end(); ++i )
    {
    QList<QVariant> name;
    name << (*i)->GetTitle().c_str();
    MidasItemTreeItem* item = new MidasItemTreeItem(name, this, parent);
    item->SetItem(*i);
    item->SetDynamicFetch(true);
    item->SetFetchedChildren(false);
    parent->AppendChild(item);

    QModelIndex index = this->index(row, 0, this->GetIndexByUuid(parent->GetUuid() ) );
    this->RegisterResource( (*i)->GetUuid(), index);
    row++;
    }
  emit layoutChanged();
}

// -------------------------------------------------------------------------
void MidasTreeModelServer::FetchItem(MidasItemTreeItem* parent)
{
  mws::Item  remote;
  mdo::Item* item = parent->GetItem();

  remote.SetAuthenticator(m_Synch->GetAuthenticator() );
  remote.SetObject(item);

  if( !remote.Fetch() )
    {
    // emit fetchError();
    return;
    }

  int row = 0;
  for( std::vector<mdo::Bitstream *>::const_iterator i = item->GetBitstreams().begin();
       i != item->GetBitstreams().end(); ++i )
    {
    QList<QVariant> name;
    name << (*i)->GetName().c_str();
    MidasBitstreamTreeItem* bitstream = new MidasBitstreamTreeItem(name, this, parent);
    bitstream->SetBitstream(*i);
    parent->AppendChild(bitstream);
    QModelIndex index = this->index(row, 0, this->GetIndexByUuid(parent->GetUuid() ) );
    this->RegisterResource( (*i)->GetUuid(), index);
    row++;
    }
  emit layoutChanged();
}

// -------------------------------------------------------------------------
void MidasTreeModelServer::ItemExpanded( const QModelIndex & index )
{
  MidasTreeItem* item = const_cast<MidasTreeItem *>(this->GetMidasTreeItem(index) );

  item->SetDecorationRole(MidasTreeItem::Expanded);

  if( m_AlterList && item->GetType() == midasResourceType::COMMUNITY )
    {
    m_ExpandedList.insert(item->GetUuid() );
    }
}
