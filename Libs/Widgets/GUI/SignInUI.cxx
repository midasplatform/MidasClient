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
#include "SignInUI.h"
#include "SignInThread.h"

#include "mwsWebAPI.h"
#include "midasAuthenticator.h"
#include "mdsDatabaseAPI.h"
#include "midasSynchronizer.h"

#include <QMessageBox>

SignInUI::SignInUI(QWidget* parent, midasSynchronizer* synch)
  : QDialog(parent), m_Synch(synch)
{
  this->setupUi(this);
  m_SignInThread = NULL;
  connect( m_CreateProfileButton, SIGNAL( released() ), this, SLOT( ShowCreateProfileDialog() ) );
}

SignInUI::~SignInUI()
{
  delete m_SignInThread;
}

void SignInUI::Init()
{
  m_ProfileComboBox->clear();

  mds::DatabaseAPI         db;
  std::vector<std::string> profiles = db.GetAuthProfiles();
  for( std::vector<std::string>::iterator i = profiles.begin(); i != profiles.end(); ++i )
    {
    m_ProfileComboBox->addItem(i->c_str() );
    }
}

int SignInUI::exec()
{
  if( mds::DatabaseInfo::Instance()->GetPath() != "" )
    {
    this->Init();
    return QDialog::exec();
    }
  else
    {
    QMessageBox::critical(this, "Error", "You must select a local database first.");
    return 0;
    }
}

void SignInUI::accept()
{
  if( m_ProfileComboBox->currentText().toStdString() == "" )
    {
    QMessageBox::critical(this, "Error", "Please select a profile or create a new one");
    return;
    }

  if( m_SignInThread )
    {
    disconnect(m_SignInThread);
    }
  delete m_SignInThread;
  m_SignInThread = new SignInThread(m_Synch);
  m_SignInThread->SetProfile(m_ProfileComboBox->currentText() );

  connect(m_SignInThread, SIGNAL( Initialized(bool) ), this, SLOT( SignIn(bool) ) );

  emit SigningIn();

  m_SignInThread->start();
  QDialog::accept();
}

void SignInUI::SignIn(bool ok)
{
  emit SignedIn(ok);
}

void SignInUI::ProfileCreated(const std::string& name)
{
  this->Init();
  m_ProfileComboBox->setCurrentIndex(m_ProfileComboBox->findText(name.c_str() ) );
}

void SignInUI::ShowCreateProfileDialog()
{
  emit CreateProfileRequest();
}

void SignInUI::RemoveProfile(const std::string& name)
{
  (void)name;
  this->Init();
}

