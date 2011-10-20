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
#include "CreateProfileUI.h"
#include "mwsWebAPI.h"
#include "midasAuthenticator.h"
#include "mdsDatabaseAPI.h"
#include <QString>
#include <QCheckBox>
#include <QMessageBox>
#include <QFileDialog>

CreateProfileUI::CreateProfileUI(QWidget* parent)
  : QDialog(parent)
{
  this->setupUi(this);

  connect(m_ProfileComboBox, SIGNAL(currentIndexChanged(const QString &) ),
          this, SLOT(FillData(const QString &) ) );
  connect(m_AnonymousCheckbox, SIGNAL(stateChanged(int) ),
          this, SLOT(AnonymousChanged(int) ) );
  connect(m_RootDirCheckbox, SIGNAL(stateChanged(int) ),
          this, SLOT(RootDirChecked(int) ) );
  connect(m_DeleteButton, SIGNAL(clicked() ),
          this, SLOT(DeleteProfile() ) );
  connect(m_BrowseButton, SIGNAL(clicked() ),
          this, SLOT(BrowseRootDir() ) );
}

CreateProfileUI::~CreateProfileUI()
{
}

void CreateProfileUI::Init()
{
  m_EmailEdit->setText("");
  m_ProfileNameEdit->setText("");
  m_PasswordEdit->setText("");
  m_RootDirEdit->setText("");

  m_AnonymousCheckbox->setChecked(false);
  m_RootDirCheckbox->setChecked(false);

  m_ProfileComboBox->clear();
  m_ProfileComboBox->addItem("New Profile");

  mds::DatabaseAPI db;
  m_ServerUrlEdit->setText(db.GetSetting(mds::DatabaseAPI::LAST_URL).c_str() );

  std::vector<std::string> profiles = db.GetAuthProfiles();
  for( std::vector<std::string>::iterator i = profiles.begin(); i != profiles.end(); ++i )
    {
    m_ProfileComboBox->addItem(i->c_str() );
    }
}

void CreateProfileUI::FillData(const QString& name)
{
  m_PasswordEdit->setText("");
  if( m_ProfileComboBox->currentIndex() == 0 )
    {
    m_ProfileNameEdit->setText("");
    m_EmailEdit->setText("");
    m_ServerUrlEdit->setText("");
    m_DeleteButton->setEnabled(false);
    }
  else
    {
    mds::DatabaseAPI db;
    midasAuthProfile profile = db.GetAuthProfile(
        name.toStdString() );

    m_ProfileNameEdit->setText(profile.Name.c_str() );
    m_EmailEdit->setText(profile.User.c_str() );
    m_ServerUrlEdit->setText(profile.Url.c_str() );
    m_RootDirEdit->setText(profile.RootDir.c_str() );

    m_RootDirCheckbox->setCheckState(
      profile.HasRootDir() ? Qt::Unchecked : Qt::Checked);
    this->RootDirChecked(m_RootDirCheckbox->checkState() );

    m_AnonymousCheckbox->setCheckState(
      profile.IsAnonymous() ? Qt::Checked : Qt::Unchecked);
    this->AnonymousChanged(m_AnonymousCheckbox->checkState() );
    m_DeleteButton->setEnabled(true);
    }
}

void CreateProfileUI::AnonymousChanged(int state)
{
  bool checked = state == Qt::Checked;

  if( checked )
    {
    m_EmailEdit->setText("");
    m_PasswordEdit->setText("");
    }
  m_EmailEdit->setEnabled(!checked);
  m_PasswordEdit->setEnabled(!checked);
}

void CreateProfileUI::RootDirChecked(int state)
{
  bool checked = state == Qt::Checked;

  if( !checked )
    {
    m_RootDirEdit->setText("");
    }
  m_RootDirEdit->setEnabled(checked);
  m_BrowseButton->setEnabled(checked);
}

int CreateProfileUI::exec()
{
  if( mds::DatabaseInfo::Instance()->GetPath() != "" )
    {
    this->Init();
    return QDialog::exec();
    }
  else
    {
    QMessageBox::critical(this, "Error", "You must select a local database.");
    return 0;
    }
}

void CreateProfileUI::accept()
{
  if( m_ProfileNameEdit->text().trimmed().isEmpty() )
    {
    QMessageBox::critical(this, "Error", "You must enter a profile name.");
    return;
    }
  if( !m_AnonymousCheckbox->isChecked() && m_EmailEdit->text().trimmed().isEmpty() )
    {
    QMessageBox::critical(this, "Error", "You must enter an email or choose anonymous access.");
    }
  if( !m_AnonymousCheckbox->isChecked() && m_PasswordEdit->text().isEmpty() )
    {
    QMessageBox::critical(this, "Error", "You must enter a password or choose anonymous access.");
    return;
    }
  std::string profileName, email, password, serverURL, rootDir;
  profileName = m_ProfileNameEdit->text().trimmed().toStdString();
  email = m_EmailEdit->text().trimmed().toStdString();
  password = m_PasswordEdit->text().toStdString();
  serverURL = m_ServerUrlEdit->text().trimmed().toStdString();
  rootDir = m_RootDirEdit->text() == "" ? "" :
    QDir(m_RootDirEdit->text().trimmed() ).path().toStdString();

  QDialog::accept();

  emit CreatedProfile(profileName, email, std::string("Default"), password, rootDir, serverURL);
}

void CreateProfileUI::DeleteProfile()
{
  std::string profileName = m_ProfileComboBox->currentText().toStdString();

  m_ProfileComboBox->removeItem(m_ProfileComboBox->currentIndex() );

  mds::DatabaseAPI db;
  db.DeleteProfile(profileName);

  std::stringstream text;
  text << "Deleted profile " << profileName;
  db.GetLog()->Message(text.str() );

  this->Init();

  QDialog::accept();
  emit DeletedProfile(profileName);
}

void CreateProfileUI::BrowseRootDir()
{
  QString path = m_RootDirEdit->text().trimmed();

  if( path == "" )
    {
    path = QDir::currentPath();
    }

  QString dir = QFileDialog::getExistingDirectory(
      this, tr("Choose Root Directory"), path, QFileDialog::ShowDirsOnly);

  if( dir != "" )
    {
    m_RootDirEdit->setText(dir);
    }
}
