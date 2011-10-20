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
#include "FileOverwriteUI.h"
#include "GUIFileOverwriteHandler.h"

FileOverwriteUI::FileOverwriteUI(QWidget* parent)
  : QDialog(parent), m_Overwrite(true)
{
  this->setupUi(this);
  this->setModal(true);

  connect(m_OverwriteButton, SIGNAL( released() ), this, SLOT( Overwrite() ) );
  connect(m_UseExistingButton, SIGNAL( released() ), this, SLOT( UseExisting() ) );
}

FileOverwriteUI::~FileOverwriteUI()
{
}

bool FileOverwriteUI::ShouldOverwrite()
{
  return m_Overwrite;
}

bool FileOverwriteUI::ShouldApplyToAll()
{
  return m_ApplyToAllCheckbox->isChecked();
}

void FileOverwriteUI::SetPath(const std::string& path)
{
  m_Path = path;
}

void FileOverwriteUI::Overwrite()
{
  m_Overwrite = true;
  QDialog::accept();
}

void FileOverwriteUI::UseExisting()
{
  m_Overwrite = false;
  QDialog::accept();
}

void FileOverwriteUI::exec()
{
  QString text = "<b>";
  text.append(m_Path.c_str() );
  text.append("</b>");
  m_PathLabel->setText(text);

  QDialog::exec();
}

