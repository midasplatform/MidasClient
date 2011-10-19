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
#include "PreferencesUI.h"

#include "mdsDatabaseAPI.h"
#include "UnifyTreeThread.h"
#include "PollFilesystemThread.h"
#include <QString>
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>

PreferencesUI::PreferencesUI(QWidget* parent)
  : QDialog(parent)
{
  this->setupUi(this);

  m_UnifyTreeThread = NULL;

  connect(m_SettingComboBox, SIGNAL(currentIndexChanged(int) ), this,
          SLOT( EnableActions(int) ) );
  connect(m_WorkingDirBrowseButton, SIGNAL(released() ), this,
          SLOT( SelectWorkingDir() ) );
  connect(m_CopyNowButton, SIGNAL(released() ), this,
          SLOT( UnifyTree() ) );
}

PreferencesUI::~PreferencesUI()
{
  delete m_UnifyTreeThread;
}

void PreferencesUI::UnifyTree()
{
  bool copy = false;

  switch( QMessageBox::question(this, "Relocate Files",
                                "Do you want to <b>move</b> the files, or <b>copy</b> them?",
                                "Move", "Copy", "Cancel") )
    {
    case 0:
      copy = false;
      break;
    case 1:
      copy = true;
      break;
    default:
      return;
    }

  this->accept();

  if( m_UnifyTreeThread )
    {
    if( m_UnifyTreeThread->isRunning() )
      {
      return;
      }
    disconnect(m_UnifyTreeThread);
    }
  delete m_UnifyTreeThread;

  m_UnifyTreeThread = new UnifyTreeThread;
  m_UnifyTreeThread->setCopy(copy);

  connect(m_UnifyTreeThread, SIGNAL( finished() ), this, SLOT(unifyTreeDone() ) );

  emit UnifyingTree();

  m_UnifyTreeThread->start();
}

void PreferencesUI::Reset()
{
  mds::DatabaseAPI db;
  std::string      interval = db.GetSetting(
      mds::DatabaseAPI::AUTO_REFRESH_INTERVAL);
  std::string index = db.GetSetting(
      mds::DatabaseAPI::AUTO_REFRESH_SETTING);
  std::string path = db.GetSetting(
      mds::DatabaseAPI::ROOT_DIR);

  m_UnifiedTree = db.GetSettingBool(
      mds::DatabaseAPI::UNIFIED_TREE);

  if( index != "" )
    {
    m_SettingComboBox->setCurrentIndex(atoi(index.c_str() ) );
    }
  if( interval != "" )
    {
    m_TimeSpinBox->setValue(atoi(interval.c_str() ) );
    }
  this->EnableActions(m_SettingComboBox->currentIndex() );

  if( path == "" )
    {
    QFileInfo dbLocation(mds::DatabaseInfo::Instance()->GetPath().c_str() );
    path = dbLocation.path().toStdString();
    }

  m_CopyResourcesCheckbox->setChecked(m_UnifiedTree);
  m_WorkingDirEdit->setText(path.c_str() );
}

void PreferencesUI::EnableActions(int index)
{
  bool val = index < 2;

  m_RefreshLabel1->setEnabled( val );
  m_RefreshLabel2->setEnabled( val );
  m_TimeSpinBox->setEnabled( val );
}

void PreferencesUI::SelectWorkingDir()
{
  mds::DatabaseAPI db;
  std::string      path = db.GetSetting(mds::DatabaseAPI::ROOT_DIR);

  if( path == "" )
    {
    QFileInfo dbLocation(mds::DatabaseInfo::Instance()->GetPath().c_str() );
    path = dbLocation.path().toStdString();
    }

  QString dir = QFileDialog::getExistingDirectory(
      this, tr("Choose Root Directory"),
      m_WorkingDirEdit->text(),
      QFileDialog::ShowDirsOnly);
  if( dir != "" )
    {
    m_WorkingDirEdit->setText(dir);
    }
}

int PreferencesUI::exec()
{
  this->Reset();
  return QDialog::exec();
}

void PreferencesUI::accept()
{
  // Save the preferences
  mds::DatabaseAPI db;

  db.SetSetting(mds::DatabaseAPI::AUTO_REFRESH_INTERVAL, m_TimeSpinBox->value() );
  db.SetSetting(mds::DatabaseAPI::AUTO_REFRESH_SETTING, m_SettingComboBox->currentIndex() );
  db.SetSetting(mds::DatabaseAPI::UNIFIED_TREE, m_CopyResourcesCheckbox->isChecked() );

  QFileInfo fileInfo(m_WorkingDirEdit->text() );

  if( fileInfo.isDir() )
    {
    db.SetSetting(mds::DatabaseAPI::ROOT_DIR, fileInfo.absoluteFilePath().toStdString() );
    }

  emit IntervalChanged();
  emit SettingChanged();

  QDialog::accept();
}

void PreferencesUI::UnifyTreeDone()
{
  emit TreeUnified();
}

