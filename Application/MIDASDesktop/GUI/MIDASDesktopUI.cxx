#include "MIDASDesktopUI.h"

#include <QModelIndex>
#include <QItemSelection>
#include <QContextMenuEvent>
#include <QHeaderView>
#include <QFileDialog>
#include <QTest>
#include <QSettings>
#include <QDesktopServices>

#include <kwsys/SystemTools.hxx>

#include "mwsSettings.h"
#include "mdoCommunity.h"
#include "mwsCommunity.h"
#include "mwsNewResources.h"
#include "mwsSearch.h"
#include "midasAuthenticator.h"
#include "midasLog.h"
#include "midasSynchronizer.h"
#include "midasDatabaseProxy.h"
#include "midasProgressReporter.h"

#include "GUILogger.h"
#include "GUIProgress.h"
#include "Utils.h"

// ------------- Dialogs -------------
#include "CreateMidasCommunityUI.h"
#include "CreateMidasCollectionUI.h"
#include "CreateMidasItemUI.h"
#include "CreateProfileUI.h"
#include "SettingsUI.h"
#include "UploadAgreementUI.h"
#include "SignInUI.h"
#include "AboutUI.h"
#include "AddResourceUI.h"
#include "PullUI.h"
#include "ProcessingStatusUI.h"
// ------------- Dialogs -------------

// ------------- TreeModel / TreeView -------------
#include "MidasTreeItem.h"
#include "MidasTreeView.h"
#include "MidasTreeModel.h"
#include "MidasTreeViewClient.h"
#include "MidasTreeModelClient.h"
#include "MidasCommunityTreeItem.h"
#include "MidasCollectionTreeItem.h"
#include "MidasBitstreamTreeItem.h"
#include "MidasItemTreeItem.h"
#include "MidasItemDelegate.h"
#include "Logger.h"
#include "MidasClientGlobal.h"
#include "mwsWebAPI.h"

// ------------- TreeModel / TreeView -------------
MIDASDesktopUI::MIDASDesktopUI() : currentTransferMode(MIDASDesktopUI::NotSet)
{
  setupUi(this); // this sets up GUI
  int time = static_cast<unsigned int>(kwsys::SystemTools::GetTime() * 1000);
  srand (time); //init random number generator 
  this->setWindowTitle( STR2QSTR( MIDAS_CLIENT_VERSION_STR ) );
  
  // ------------- Instantiate and setup UI dialogs -------------
  dlg_createMidasCommunityUI =    new CreateMidasCommunityUI( this );
  dlg_createMidasSubCommunityUI = new CreateMidasCommunityUI( this, CreateMidasCommunityUI::SubCommunity );
  dlg_addMidasCommunityUI =       new CreateMidasCommunityUI( this, CreateMidasCommunityUI::ClientCommunity);
  dlg_addMidasSubCommunityUI =    new CreateMidasCommunityUI( this, CreateMidasCommunityUI::ClientSubCommunity);
  dlg_createMidasCollectionUI =   new CreateMidasCollectionUI( this );
  dlg_addMidasCollectionUI =      new CreateMidasCollectionUI( this, CreateMidasCollectionUI::ClientCollection);
  dlg_createMidasItemUI =         new CreateMidasItemUI( this );
  dlg_addMidasItemUI =            new CreateMidasItemUI( this, CreateMidasItemUI::ClientItem);

  dlg_settingsUI =         new SettingsUI( this );
  dlg_uploadAgreementUI =  new UploadAgreementUI( this );
  dlg_signInUI =           new SignInUI( this );
  dlg_addResourceUI =      new AddResourceUI( this );
  dlg_createProfileUI =    new CreateProfileUI( this );
  dlg_aboutUI =            new AboutUI( this );
  dlg_pullUI =             new PullUI( this );
  ProcessingStatusUI::init( this );
  
  // ------------- Instantiate and setup UI dialogs -------------

  // ------------- Item info panel -------------
  midasTreeItemInfoTable->horizontalHeader()->setStretchLastSection( true ); 
  midasTreeItemInfoTable->horizontalHeader()->hide();
  midasTreeItemInfoTable->verticalHeader()->hide();
  midasTreeItemInfoTable->setItemDelegate( new EditTextDelegate );
  // ------------- Item info panel -------------

  // ------------- Status bar -------------
  stateLabel    = new QLabel();
  progressBar   = new QProgressBar();
  connectLabel  = new QLabel();
  hostLabel     = new QLabel();

  progressBar->setTextVisible(false);
  //progressBar->setMinimumSize( progressBar->sizeHint() );

  connectLabel->setAlignment( Qt::AlignCenter );
  connectLabel->setFrameShape( QFrame::Panel );
  connectLabel->setFrameShadow( QFrame::Sunken );
  connectLabel->setMinimumSize( connectLabel->sizeHint() );
  connectLabel->clear();

  hostLabel->setAlignment( Qt::AlignCenter );
  hostLabel->setFrameShape( QFrame::Panel );
  hostLabel->setFrameShadow( QFrame::Sunken );
  hostLabel->setMinimumSize( hostLabel->sizeHint() );
  hostLabel->clear();

  statusBar()->addWidget( stateLabel, 1 );
  statusBar()->addWidget( progressBar, 1 );
  statusBar()->addWidget( connectLabel );
  statusBar()->addWidget( hostLabel );
  // ------------- Status bar -------------

  // ------------- setup TreeView signals -------------
  connect(treeView, SIGNAL(midasTreeItemSelected(const MidasTreeItem*)),
    this, SLOT( updateActionState(const MidasTreeItem*) ));
  connect(treeViewClient, SIGNAL(midasTreeItemSelected(const MidasTreeItem*)),
    this, SLOT( updateActionStateClient(const MidasTreeItem*) ));

  connect(treeViewClient, SIGNAL(bitstreamsDropped(const MidasItemTreeItem*, const QStringList&)),
    this, SLOT( addBitstreams(const MidasItemTreeItem*, const QStringList&)));

  connect(treeViewClient, SIGNAL(bitstreamOpenRequest(int)), this, SLOT(openBitstream(int)));

  connect(treeView, SIGNAL(midasCommunityTreeItemSelected(const MidasCommunityTreeItem*)),
    this, SLOT( updateInfoPanel(const MidasCommunityTreeItem*) ));

  connect(treeView, SIGNAL(fetchedMore()), this, SLOT( decorateServerTree() ));

  connect(treeView, SIGNAL(midasCollectionTreeItemSelected(const MidasCollectionTreeItem*)),
    this, SLOT( updateInfoPanel(const MidasCollectionTreeItem*) ));

  connect(treeView, SIGNAL(midasNoTreeItemSelected()),
    this, SLOT( clearInfoPanel() ));

  connect(treeView, SIGNAL(midasItemTreeItemSelected(const MidasItemTreeItem*)),
    this, SLOT( updateInfoPanel(const MidasItemTreeItem*) ));

  connect(treeView, SIGNAL(midasNoTreeItemSelected()),
    dlg_pullUI, SLOT( resetState() ));

  connect(treeView, SIGNAL(midasTreeViewContextMenu(QContextMenuEvent*)),
    this, SLOT( displayServerResourceContextMenu(QContextMenuEvent*) ));

  connect(treeView->model(), SIGNAL(serverPolled()), this, SLOT(storeLastPollTime()));

  connect(treeViewClient, SIGNAL(midasTreeViewContextMenu(QContextMenuEvent*)),
    this, SLOT( displayClientResourceContextMenu(QContextMenuEvent*) ));

  connect(dlg_addResourceUI, SIGNAL(addedResource()), this, SLOT( clientTreeViewUpdated() ) );
  connect(dlg_pullUI, SIGNAL(pulledResources()), this, SLOT( clientTreeViewUpdated() ) );
  // ------------- setup TreeView signals -------------

  // ------------- signal/slot connections -------------
  connect( this->dlg_signInUI, SIGNAL(signedIn()), this, SLOT(signIn()) );

  connect( actionEdit_search_settings,    SIGNAL( triggered() ), this, SLOT( editSearchSettings() ) );
  connect( actionPush_Resources,          SIGNAL( triggered() ), this, SLOT( pushResources() ) );
  connect( actionPull_Resource,           SIGNAL( triggered() ), dlg_pullUI, SLOT( exec() ) );

  connect( actionCreate_Profile, SIGNAL( triggered() ), dlg_createProfileUI, SLOT( exec() ) );
  connect( dlg_createProfileUI,
    SIGNAL( createdProfile(std::string, std::string, std::string, std::string)), this,
    SLOT( createProfile(std::string, std::string, std::string, std::string)));
  connect( dlg_createProfileUI, SIGNAL( serverURLSet(std::string)), this, SLOT( setServerURL(std::string)));
  connect( dlg_signInUI, SIGNAL( createProfileRequest() ), dlg_createProfileUI, SLOT( exec() ) );

  connect( actionChoose_Local_Database, SIGNAL( triggered() ), this, SLOT( chooseLocalDatabase() ) );

  connect( actionSign_In,   SIGNAL( triggered() ), this, SLOT( signInOrOut() ) );
  connect( actionQuit,      SIGNAL( triggered() ), this, SLOT( close() ) );
  connect( actionAbout,     SIGNAL( triggered() ), dlg_aboutUI, SLOT( exec() ) );

  connect( actionAdd_community, SIGNAL(triggered()), this, SLOT(addCommunity()));
  connect( actionAdd_subcommunity, SIGNAL(triggered()), this, SLOT(addSubcommunity()));
  connect( actionAdd_collection, SIGNAL(triggered()), this, SLOT(addCollection()));
  connect( actionAdd_item, SIGNAL(triggered()), this, SLOT(addItem()));
  connect( actionAdd_bitstream, SIGNAL(triggered()), this, SLOT(addBitstream()));

  connect( searchItemsListWidget, SIGNAL( midasListWidgetItemClicked( QListWidgetItemMidasItem * ) ),
    this, SLOT( searchItemClicked( QListWidgetItemMidasItem * ) ) );
  connect( searchItemsListWidget, SIGNAL( midasListWidgetContextMenu( QContextMenuEvent * ) ),
    this, SLOT( searchItemContextMenu( QContextMenuEvent * ) ) );

  connect( addResource_Button, SIGNAL( released() ), dlg_addResourceUI, SLOT( exec() ) );
  connect( push_Button, SIGNAL( released() ), this, SLOT( pushResources() ) );
  connect( pull_Button, SIGNAL( released() ), dlg_pullUI, SLOT( exec() ) );
  connect( refreshButton, SIGNAL( released() ), this, SLOT( serverTreeViewUpdated() ) );
  connect( searchButton, SIGNAL( released() ), this, SLOT( search() ) );

  //connect( log, SIGNAL( textChanged() ), this, SLOT( showLogTab() ) );

  // ------------- signal/slot connections -------------

  // ------------- setup client members and logging ----
  this->m_database = NULL;
  this->m_synch = new midasSynchronizer();
  this->m_auth = new midasAuthenticator();
  this->m_progress = reinterpret_cast<midasProgressReporter*>(new GUIProgress(this->progressBar));
  this->m_logger = new GUILogger(this->log);
  this->m_auth->SetLog(m_logger);
  this->m_synch->SetLog(m_logger);
  this->m_synch->SetProgressReporter(m_progress);
  // ------------- setup client members and logging ----

  // ------------- Handle stored settings -------------
  QSettings settings("Kitware", "MidasDesktop");
  std::string lastDatabase =
    settings.value("lastDatabase", "").toString().toStdString();
  this->setLocalDatabase(lastDatabase);

  // ------------- Handle stored settings -------------
 }

/** Destructor */
MIDASDesktopUI::~MIDASDesktopUI()
  {
  ProcessingStatusUI::finalize();
  delete dlg_aboutUI;
  delete dlg_signInUI;
  delete dlg_settingsUI;
  delete dlg_createProfileUI;
  delete dlg_addResourceUI;
  delete dlg_createMidasCommunityUI;
  delete dlg_createMidasSubCommunityUI;
  delete dlg_createMidasCollectionUI;
  delete dlg_createMidasItemUI;
  delete dlg_uploadAgreementUI;
  delete dlg_pullUI;
  delete stateLabel;
  delete connectLabel;
  delete hostLabel;
  delete m_database;
  delete m_auth;
  delete m_logger;
  delete m_progress;
  delete m_synch;
  }

MidasTreeView * MIDASDesktopUI::getTreeView()
  {
  return this->treeView; 
  }

void MIDASDesktopUI::activateActions(bool value, ActivateActions activateAction)
  {
  if ( activateAction & ACTION_CONNECTED )
    {
    this->searchTab->setEnabled( value );
    this->treeView->setEnabled( value );
    this->midasTreeItemInfoGroupBox->setEnabled( value );
    this->pull_Button->setEnabled( value );
    this->push_Button->setEnabled( value );
    this->actionPush_Resources->setEnabled( value );
    this->searchButton->setEnabled( value );
    this->searchQueryEdit->setEnabled( value );
    this->refreshButton->setEnabled( value );
    actionSign_In->setText( value ? tr("Sign Out") : tr("Sign In") );
    }

  if ( activateAction & ACTION_COMMUNITY  )
    {
    this->actionPull_Resource->setEnabled( value );
    }

  if ( activateAction & ACTION_COLLECTION  )
    {
    this->actionPull_Resource->setEnabled( value );
    }

  if ( activateAction & ACTION_ITEM  )
    {
    this->actionPull_Resource->setEnabled( value );
    }

  if ( activateAction & ACTION_BITSTREAM )
    {
    this->actionPull_Resource->setEnabled( value );
    }

  if ( activateAction & ACTION_CLIENT_COMMUNITY )
    {
    this->actionAdd_subcommunity->setEnabled( value );
    this->actionAdd_collection->setEnabled( value );
    }

  if ( activateAction & ACTION_CLIENT_COLLECTION )
    {
    this->actionAdd_item->setEnabled( value );
    }

  if ( activateAction & ACTION_CLIENT_ITEM )
    {
    this->actionAdd_bitstream->setEnabled( value );
    }

  if ( activateAction & ACTION_CLIENT_BITSTREAM )
    {
    }
  }

void MIDASDesktopUI::updateActionState( const MidasTreeItem* item )
{
  MidasCommunityTreeItem * communityTreeItem = NULL; 
  MidasCollectionTreeItem * collectionTreeItem = NULL; 
  MidasItemTreeItem * itemTreeItem = NULL; 
  MidasBitstreamTreeItem * bitstreamTreeItem = NULL;

  // disable all actions
  this->activateActions( false, ACTION_ALL_CONNECTED ); 

  if ((communityTreeItem = dynamic_cast<MidasCommunityTreeItem*>( const_cast<MidasTreeItem*>( item ) ) ) != NULL )
    {
    this->activateActions( true, ACTION_COMMUNITY );
    this->dlg_pullUI->setPullId(communityTreeItem->getCommunity()->GetId());
    this->dlg_pullUI->setResourceTypeIndex(0);
    }
  else if ((collectionTreeItem = dynamic_cast<MidasCollectionTreeItem*>( const_cast<MidasTreeItem*>( item ) ) ) != NULL )
    {
    this->activateActions( true, ACTION_COLLECTION );
    this->dlg_pullUI->setPullId(collectionTreeItem->getCollection()->GetId());
    this->dlg_pullUI->setResourceTypeIndex(1);
    }
  else if ((itemTreeItem = dynamic_cast<MidasItemTreeItem*>( const_cast<MidasTreeItem*>( item ) ) ) != NULL )
    {
    this->activateActions( true, ACTION_ITEM | ACTION_BITSTREAM );
    this->dlg_pullUI->setPullId(itemTreeItem->getItem()->GetId());
    this->dlg_pullUI->setResourceTypeIndex(2);
    }
  else if ((bitstreamTreeItem = dynamic_cast<MidasBitstreamTreeItem*>( const_cast<MidasTreeItem*>( item ) ) ) != NULL )
    {
    this->activateActions( true, ACTION_BITSTREAM );
    this->dlg_pullUI->setPullId(bitstreamTreeItem->getBitstream()->GetId());
    this->dlg_pullUI->setResourceTypeIndex(3);
    }
}

void MIDASDesktopUI::updateActionStateClient( const MidasTreeItem* item )
{
  MidasCommunityTreeItem * communityTreeItem = NULL;
  MidasCollectionTreeItem * collectionTreeItem = NULL;
  MidasItemTreeItem * itemTreeItem = NULL;
  MidasBitstreamTreeItem * bitstreamTreeItem = NULL;

  // disable all actions
  this->activateActions( false, ACTION_ALL_CONNECTED );
  this->activateActions( false, ACTION_CLIENT_COMMUNITY
                              | ACTION_CLIENT_COLLECTION
                              | ACTION_CLIENT_ITEM
                              | ACTION_CLIENT_BITSTREAM );

  if ((communityTreeItem = dynamic_cast<MidasCommunityTreeItem*>( const_cast<MidasTreeItem*>( item ) ) ) != NULL )
    {
    this->activateActions( true, ACTION_CLIENT_COMMUNITY );
    }
  else if ((collectionTreeItem = dynamic_cast<MidasCollectionTreeItem*>( const_cast<MidasTreeItem*>( item ) ) ) != NULL )
    {
    this->activateActions( true, ACTION_CLIENT_COLLECTION );
    }
  else if ((itemTreeItem = dynamic_cast<MidasItemTreeItem*>( const_cast<MidasTreeItem*>( item ) ) ) != NULL )
    {
    this->activateActions( true, ACTION_CLIENT_ITEM );
    }
  else if ((bitstreamTreeItem = dynamic_cast<MidasBitstreamTreeItem*>( const_cast<MidasTreeItem*>( item ) ) ) != NULL )
    {
    this->activateActions( true, ACTION_CLIENT_BITSTREAM );
    }
}

void MIDASDesktopUI::clientTreeViewUpdated()
{
  this->treeTabContainer->setCurrentIndex(1);
  this->treeViewClient->Update();
  this->treeViewClient->expandAll();
}

void MIDASDesktopUI::serverTreeViewUpdated()
{
  this->treeTabContainer->setCurrentIndex(0);
  this->treeView->Update();
  //this->treeView->expandAll();
}

void MIDASDesktopUI::showLogTab()
{
  this->logAndSearchTabContainer->setCurrentIndex(1);
}

/** Show the community information */
void MIDASDesktopUI::updateInfoPanel( const MidasCommunityTreeItem* communityTreeItem )
{ 
  QTableWidgetDescriptionItem::Options options = QTableWidgetDescriptionItem::Tooltip;
  /*if (!readonly)
    {
    options |= QTableWidgetDescriptionItem::Editable; 
    }*/

  midasTreeItemInfoGroupBox->setTitle(tr(" Community description ")); 

  midasTreeItemInfoTable->disconnect( SIGNAL( itemChanged ( QTableWidgetItem * ) ) );
  midasTreeItemInfoTable->clearSelection();
  midasTreeItemInfoTable->setRowCount( 5 );
  int i = 0; 

  mdo::Community* community = communityTreeItem->getCommunity();
  
  midasTreeItemInfoTable->setRowHeight(i, QTableWidgetDescriptionItem::rowHeight);
  midasTreeItemInfoTable->setItem(i,0,new QTableWidgetDescriptionItem("Name", QTableWidgetDescriptionItem::Bold));
  midasTreeItemInfoTable->setItem(i,1,new QTableWidgetMidasCommunityDescItem(community, community->GetName().c_str(), COMMUNITY_NAME, options));
  i++; 

  midasTreeItemInfoTable->setRowHeight(i, QTableWidgetDescriptionItem::rowHeight);
  midasTreeItemInfoTable->setItem(i,0,new QTableWidgetDescriptionItem("Description", QTableWidgetDescriptionItem::Bold));
  midasTreeItemInfoTable->setItem(i,1,new QTableWidgetMidasCommunityDescItem(community, community->GetUuid().c_str(), COMMUNITY_DESCRIPTION, options));
  i++; 

  midasTreeItemInfoTable->setRowHeight(i, QTableWidgetDescriptionItem::rowHeight);
  midasTreeItemInfoTable->setItem(i,0,new QTableWidgetDescriptionItem("Introductory", QTableWidgetDescriptionItem::Bold));
  midasTreeItemInfoTable->setItem(i,1,new QTableWidgetMidasCommunityDescItem(community, community->GetIntroductoryText().c_str(), COMMUNITY_INTRODUCTORY, options));
  i++;

  midasTreeItemInfoTable->setRowHeight(i, QTableWidgetDescriptionItem::rowHeight);
  midasTreeItemInfoTable->setItem(i,0,new QTableWidgetDescriptionItem("Copyright", QTableWidgetDescriptionItem::Bold));
  midasTreeItemInfoTable->setItem(i,1,new QTableWidgetMidasCommunityDescItem(community, community->GetCopyright().c_str(), COMMUNITY_COPYRIGHT, options));
  i++;

  midasTreeItemInfoTable->setRowHeight(i, QTableWidgetDescriptionItem::rowHeight);
  midasTreeItemInfoTable->setItem(i,0,new QTableWidgetDescriptionItem("Links", QTableWidgetDescriptionItem::Bold));
  midasTreeItemInfoTable->setItem(i,1,new QTableWidgetMidasCommunityDescItem(community, community->GetLinks().c_str(), COMMUNITY_LINKS, options));
  i++;

  midasTreeItemInfoTable->resizeColumnsToContents();

  connect( midasTreeItemInfoTable, SIGNAL( itemChanged ( QTableWidgetItem * ) ),
           this, SLOT( resourceDescriptionChanged(QTableWidgetItem *) ) );
}

void MIDASDesktopUI::updateInfoPanel( const MidasCollectionTreeItem* collectionTreeItem )
{
  QTableWidgetDescriptionItem::Options options = QTableWidgetDescriptionItem::Tooltip; 
/*  if (!readonly)
    {
    options |= QTableWidgetDescriptionItem::Editable; 
    }*/
  mdo::Collection* collection = collectionTreeItem->getCollection();
  midasTreeItemInfoGroupBox->setTitle(tr(" Collection description "));

  midasTreeItemInfoTable->disconnect( SIGNAL( itemChanged ( QTableWidgetItem * ) ) ); 
  midasTreeItemInfoTable->clearSelection();
  midasTreeItemInfoTable->setRowCount( 3 );
  int i = 0;

  midasTreeItemInfoTable->setRowHeight(i, QTableWidgetDescriptionItem::rowHeight);
  midasTreeItemInfoTable->setItem(i,0,new QTableWidgetDescriptionItem("Name", QTableWidgetDescriptionItem::Bold));
  midasTreeItemInfoTable->setItem(i,1,new QTableWidgetMidasCollectionDescItem(collection, collection->GetName().c_str(), COLLECTION_NAME, options));
  i++;

  midasTreeItemInfoTable->setRowHeight(i, QTableWidgetDescriptionItem::rowHeight);
  midasTreeItemInfoTable->setItem(i,0,new QTableWidgetDescriptionItem("Description", QTableWidgetDescriptionItem::Bold));
  midasTreeItemInfoTable->setItem(i,1,new QTableWidgetMidasCollectionDescItem(collection, collection->GetDescription().c_str(), COLLECTION_DESCRIPTION, options));
  i++;

  midasTreeItemInfoTable->setRowHeight(i, QTableWidgetDescriptionItem::rowHeight);
  midasTreeItemInfoTable->setItem(i,0,new QTableWidgetDescriptionItem("Links", QTableWidgetDescriptionItem::Bold));
  midasTreeItemInfoTable->setItem(i,1,new QTableWidgetMidasCollectionDescItem(collection, "", COLLECTION_LINKS, options));
  i++;

  midasTreeItemInfoTable->resizeColumnsToContents();
  connect( midasTreeItemInfoTable, SIGNAL( itemChanged ( QTableWidgetItem * ) ), this, SLOT( resourceDescriptionChanged(QTableWidgetItem *) ) ); 
  }

void MIDASDesktopUI::updateInfoPanel( const MidasItemTreeItem* itemTreeItem )
  {
  QTableWidgetDescriptionItem::Options options = QTableWidgetDescriptionItem::Tooltip; 
/*  if (!readonly)
    {
    options |= QTableWidgetDescriptionItem::Editable;
    }
  */
  mdo::Item* item = itemTreeItem->getItem();
  midasTreeItemInfoGroupBox->setTitle(tr(" Item description "));

  midasTreeItemInfoTable->disconnect( SIGNAL( itemChanged ( QTableWidgetItem * ) ) ); 
  midasTreeItemInfoTable->clearSelection();
  midasTreeItemInfoTable->setRowCount(5);
  int i = 0;

  midasTreeItemInfoTable->setRowHeight(i, QTableWidgetDescriptionItem::rowHeight);
  midasTreeItemInfoTable->setItem(i,0,new QTableWidgetDescriptionItem("Title", QTableWidgetDescriptionItem::Bold));
  midasTreeItemInfoTable->setItem(i,1,new QTableWidgetMidasItemDescItem(item, item->GetTitle().c_str(), ITEM_TITLE, options));
  i++;

  midasTreeItemInfoTable->setRowHeight(i, QTableWidgetDescriptionItem::rowHeight);
  midasTreeItemInfoTable->setItem(i,0,new QTableWidgetDescriptionItem("Authors", QTableWidgetDescriptionItem::Bold));
  midasTreeItemInfoTable->setItem(i,1,new QTableWidgetMidasItemDescItem(item, kwutils::concatenate(item->GetAuthors(), "/ ").c_str(), ITEM_AUTHORS, options));
  i++;

  midasTreeItemInfoTable->setRowHeight(i, QTableWidgetDescriptionItem::rowHeight);
  midasTreeItemInfoTable->setItem(i,0,new QTableWidgetDescriptionItem("Keywords", QTableWidgetDescriptionItem::Bold));
  midasTreeItemInfoTable->setItem(i,1,new QTableWidgetMidasItemDescItem(item, kwutils::concatenate(item->GetKeywords(), "/ ").c_str(), ITEM_KEYWORDS, options));
  i++;

  midasTreeItemInfoTable->setRowHeight(i, QTableWidgetDescriptionItem::rowHeight);
  midasTreeItemInfoTable->setItem(i,0,new QTableWidgetDescriptionItem("Abstract", QTableWidgetDescriptionItem::Bold));
  midasTreeItemInfoTable->setItem(i,1,new QTableWidgetMidasItemDescItem(item, item->GetAbstract().c_str(), ITEM_ABSTRACT, options));
  i++;

  midasTreeItemInfoTable->setRowHeight(i, QTableWidgetDescriptionItem::rowHeight);
  midasTreeItemInfoTable->setItem(i,0,new QTableWidgetDescriptionItem("Description", QTableWidgetDescriptionItem::Bold));
  midasTreeItemInfoTable->setItem(i,1,new QTableWidgetMidasItemDescItem(item, item->GetDescription().c_str(), ITEM_DESCRIPTION, options));
  i++;

  midasTreeItemInfoTable->resizeColumnsToContents();
  connect( midasTreeItemInfoTable, SIGNAL( itemChanged ( QTableWidgetItem * ) ), this, SLOT( resourceDescriptionChanged(QTableWidgetItem *) ) ); 
  }

void MIDASDesktopUI::clearInfoPanel()
  {
  midasTreeItemInfoTable->clear();
  midasTreeItemInfoTable->setRowCount( 0 );
  }

void MIDASDesktopUI::resourceDescriptionChanged(QTableWidgetItem * item )
  { 
  QTableWidgetMidasCommunityDescItem * communityItem = NULL; 
  QTableWidgetMidasCollectionDescItem * collectionItem = NULL; 
  QTableWidgetMidasItemDescItem * itemItem = NULL; 

  const QModelIndex index = this->treeView->getSelectedModelIndex(); 
/*
  if ((communityItem = dynamic_cast<QTableWidgetMidasCommunityDescItem*>( const_cast<QTableWidgetItem*>( item ) ) ) != NULL )
    {
    this->getTreeView()->model()->updateCommunityProperty( index, communityItem->getPropertyName(), QSTR2STR(item->text()) ); 
    }
  else if ((collectionItem = dynamic_cast<QTableWidgetMidasCollectionDescItem*>( const_cast<QTableWidgetItem*>( item ) ) ) != NULL )
    {
    this->getTreeView()->model()->updateCollectionProperty( index, collectionItem->getPropertyName(), QSTR2STR(item->text()) ); 
    }
  else if ((itemItem = dynamic_cast<QTableWidgetMidasItemDescItem*>( const_cast<QTableWidgetItem*>( item ) ) ) != NULL )
    {
    this->getTreeView()->model()->updateItemProperty( index, itemItem->getPropertyName(), QSTR2STR(item->text()) ); 
    }
    */
  }

void MIDASDesktopUI::displayClientResourceContextMenu( QContextMenuEvent* e )
{
  QMenu menu( this );
  MidasCommunityTreeItem * communityTreeItem = NULL;
  MidasCollectionTreeItem * collectionTreeItem = NULL;
  MidasItemTreeItem * itemTreeItem = NULL;
  MidasBitstreamTreeItem * bitstreamTreeItem = NULL;

  QModelIndex index = treeViewClient->indexAt( e->pos() );
  MidasTreeItem * item = const_cast<MidasTreeItem*>( 
                         reinterpret_cast<MidasTreeModelClient*>(treeViewClient->model())->midasTreeItem( index ) );
  
  if ( index.isValid() )
    {
    treeViewClient->selectionModel()->select( index, QItemSelectionModel::SelectCurrent ); 

    if ( ( communityTreeItem = dynamic_cast<MidasCommunityTreeItem*>( item ) ) != NULL)
      {
      menu.addAction( this->actionAdd_subcommunity );
      menu.addAction( this->actionAdd_collection );
      }
    else if ( ( collectionTreeItem = dynamic_cast<MidasCollectionTreeItem*>( item ) ) != NULL )
      {
      menu.addAction( this->actionAdd_item );
      }
    else if ( ( itemTreeItem = dynamic_cast<MidasItemTreeItem*>( item ) ) != NULL )
      {
      menu.addAction( this->actionAdd_bitstream );
      }
    else if ( ( bitstreamTreeItem = dynamic_cast<MidasBitstreamTreeItem*>( item ) ) != NULL )
      {
      //add any bitstream contextmenu actions here.
      }
    }
  else 
    {
    treeView->selectionModel()->clearSelection(); 
    menu.addAction( this->actionAdd_community );
    menu.addAction( this->actionPush_Resources );
    }
  menu.exec( e->globalPos() );
}

void MIDASDesktopUI::displayServerResourceContextMenu( QContextMenuEvent* e )
{
  QMenu menu( this );
  QModelIndex index = treeView->indexAt( e->pos() );

  if ( index.isValid() )
    {
    treeView->selectionModel()->select( index, QItemSelectionModel::SelectCurrent ); 
    }
  else 
    {
    treeView->selectionModel()->clearSelection(); 
    }
  menu.addAction( this->actionPull_Resource );
  menu.exec( e->globalPos() );
}

void MIDASDesktopUI::addCommunity()
{
  this->dlg_addMidasCommunityUI->exec();
}

void MIDASDesktopUI::addSubcommunity()
{
  this->dlg_addMidasSubCommunityUI->exec();
}

void MIDASDesktopUI::addCollection()
{
  this->dlg_addMidasCollectionUI->exec();
}

void MIDASDesktopUI::addItem()
{
  this->dlg_addMidasItemUI->exec();
}

void MIDASDesktopUI::addBitstream()
{
  QStringList files = QFileDialog::getOpenFileNames(
                                    this, tr("Load Files"),
                                    QDir::current().absolutePath(),
                                    tr("All Files (*)"));

  if(files.size())
    {
    addBitstreams(reinterpret_cast<MidasItemTreeItem*>(
      const_cast<MidasTreeItem*>(
      treeViewClient->getSelectedMidasTreeItem())), files);
    }
}

void MIDASDesktopUI::addBitstreams(const MidasItemTreeItem* parentItem,
                               const QStringList & files)
{
  for(QStringList::const_iterator i = files.begin(); i != files.end(); ++i)
    {
    std::string path = i->toStdString();
    std::string name = kwsys::SystemTools::GetFilenameName(path.c_str());
    std::string uuid = midasUtils::GenerateUUID();
    this->m_database->Open();
    std::string parentUuid = this->m_database->GetUuid(
      midasResourceType::ITEM, parentItem->getItem()->GetId());
    int id = this->m_database->AddResource(midasResourceType::BITSTREAM, uuid, 
      path, name, parentUuid, 0);
    this->m_database->MarkDirtyResource(uuid, midasDirtyAction::ADDED);
    this->m_database->Close();

    if(id)
      {
      std::stringstream text;
      text << "Added bitstream " << name << " under item " << 
        parentItem->getItem()->GetTitle() << ".";
      this->m_logger->Message(text.str());
      }
    }
  this->clientTreeViewUpdated();
}

void MIDASDesktopUI::displayStatus(const QString& message)
  {
  stateLabel->setText(message); 
  }

void MIDASDesktopUI::resetStatus()
  {
  stateLabel->setText(""); 
  }

void MIDASDesktopUI::signInOrOut()
  {
  if ( !this->m_signIn )
    {
    this->dlg_signInUI->exec(); 
    }
  else
    {
    this->signOut(); 
    }
  }

void MIDASDesktopUI::signIn()
{
  connectLabel->hide();
  hostLabel->hide();
  this->displayStatus(tr("Connecting to server..."));
  this->getTreeView()->SetWebAPI(mws::WebAPI::Instance());
  this->getTreeView()->Initialize();
  this->activateActions( true, ACTION_CONNECTED );

  // Satus bar
  std::string connect = "  Connected to " + std::string(mws::WebAPI::Instance()->GetServerUrl()) + "  "; 
  connectLabel->setText( connect.c_str() );
  connectLabel->show();
  this->treeTabContainer->setCurrentIndex(0);
  
  std::string host = "  " + std::string(mws::WebAPI::Instance()->GetServerUrl()) + "  ";
  hostLabel->setText( host.c_str() );
  hostLabel->show();
  m_signIn = true; 
  this->displayStatus(tr(""));
}

void MIDASDesktopUI::chooseLocalDatabase()
{
  // ------------- display FileDialog -------------
  QString file = QFileDialog::getOpenFileName(
                                    this, tr("Load Files"),
                                    QDir::current().absolutePath(),
                                    tr("All Files (*)"));

  setLocalDatabase(file.toStdString());
}

void MIDASDesktopUI::setLocalDatabase(std::string file)
{
  if(file == "")
    {
    return;
    }

  if(midasUtils::IsDatabaseValid(file))
    {
    delete m_database;
    this->m_auth->SetDatabase(file);
    this->m_synch->SetDatabase(file);
    this->m_database = new midasDatabaseProxy(file);
    QSettings settings("Kitware", "MidasDesktop");
    settings.setValue("lastDatabase", file.c_str());
    settings.sync();
    this->displayStatus(tr("Opened database successfully."));
    this->treeViewClient->SetDatabaseProxy(m_database);
    this->treeViewClient->setEnabled(true);
    this->clientCollapseAllButton->setEnabled(true);
    this->clientExpandAllButton->setEnabled(true);
    this->addResource_Button->setEnabled(true);
    this->actionAdd_community->setEnabled(true);
    this->treeViewClient->SetLog(m_logger);
    this->clientTreeViewUpdated();
    this->treeViewClient->collapseAll();
    }
  else
    {
    std::stringstream text;
    text << "The path " << file << " does not refer to a valid MidasDesktop "
      "database.";
    m_logger->Error(text.str());
    }
}

void MIDASDesktopUI::setServerURL(std::string url)
{
  this->m_synch->SetServerURL(url);
  this->m_auth->SetServerURL(url);
  mws::WebAPI::Instance()->SetServerUrl(url.c_str());
  this->m_url = url;
}

void MIDASDesktopUI::createProfile(std::string name, std::string email,
                               std::string apiName, std::string apiKey)
{
  if(!m_database)
    {
    this->displayStatus(tr("Please choose a local database first."));
    return;
    }

  std::string msg;
  m_auth->SetServerURL(m_url);
  if(m_auth->AddAuthProfile(email, apiName, apiKey, name))
    {
    msg = "Successfully created profile \"" + name + "\".";
    this->dlg_signInUI->profileCreated(name);
    }
  else
    {
    msg = "Failed to create authentication profile.";
    }
  this->displayStatus(tr(msg.c_str()));
}

/** Signing out */
void MIDASDesktopUI::signOut()
{
  this->activateActions(false, ACTION_ALL); 
  treeView->Clear(); 
  this->clearInfoPanel();
  connectLabel->hide();
  hostLabel->hide();
  this->displayStatus(tr("Logout")); 
  m_signIn = false;
}

void MIDASDesktopUI::editServerSettings()
  {
  this->dlg_settingsUI->exec(0); 
  }

void MIDASDesktopUI::editSearchSettings()
  {
  this->dlg_settingsUI->exec(1);
  }

void MIDASDesktopUI::checkDatabaseSettings()
  {
  this->dlg_settingsUI->exec(2);
  }

void MIDASDesktopUI::pushResources()
{
  this->displayStatus(tr("Beginning to push locally added resources..."));
  this->m_synch->SetOperation(midasSynchronizer::OPERATION_PUSH);
  if(this->m_synch->Perform() == 0)
    {
    this->clientTreeViewUpdated();
    this->serverTreeViewUpdated();
    }
  this->progressBar->setValue(0);
  this->displayStatus(tr("Finished pushing locally added resources."));
}

void MIDASDesktopUI::search()
{
  this->displayStatus(tr("Searching...")); 
  searchItemsListWidget->clear();
  std::vector<std::string> words;
  kwutils::tokenize(searchQueryEdit->text().toStdString(), words);
  std::vector<mdo::Object*> results = mws::Search::SearchServer(words);

  for(std::vector<mdo::Object*>::iterator i = results.begin();
      i != results.end(); ++i)
    {
    new QListWidgetItemMidasItem(this->searchItemsListWidget, *i);
    }
  this->resetStatus();
}

void MIDASDesktopUI::searchItemClicked(QListWidgetItemMidasItem * listItem)
{
  this->treeView->selectByObject(listItem->getObject());
}

void MIDASDesktopUI::searchItemContextMenu(QContextMenuEvent* e)
{
  QMenu menu( this );
  QModelIndex index = searchItemsListWidget->indexAt( e->pos() );

  if ( index.isValid() )
    {
    menu.addAction( this->actionPull_Resource );
    menu.exec( e->globalPos() );
    }
}

void MIDASDesktopUI::openBitstream(int id)
{
  if(!this->m_database)
    {
    return;
    }
  m_database->Open();
  std::string path = m_database->GetRecordByUuid(
    m_database->GetUuid(midasResourceType::BITSTREAM, id)).Path;
  m_database->Close();

  if(path != "")
    {
    path = "file:" + path;
    QUrl url(path.c_str());
    if(!QDesktopServices::openUrl(url))
      {
      std::stringstream text;
      text << "The operating system does not know how to open "
        << path << std::endl;
      m_logger->Error(text.str());
      }
    }
}

void MIDASDesktopUI::storeLastPollTime()
{
  mws::NewResources newResources;
  m_database->Open();
  newResources.SetSince(m_database->GetSetting(midasDatabaseProxy::LAST_FETCH_TIME));
  newResources.Fetch();
  this->m_dirtyUuids = newResources.GetUuids();
  m_database->SetSetting(midasDatabaseProxy::LAST_FETCH_TIME, newResources.GetTimestamp());
  m_database->Close();

  this->decorateServerTree();
}

void MIDASDesktopUI::decorateServerTree()
{
  for(std::vector<std::string>::iterator i = m_dirtyUuids.begin();
      i != m_dirtyUuids.end(); ++i)
    {
    this->treeView->decorateByUuid(*i);
    }
}