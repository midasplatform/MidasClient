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
#include "DeleteResourceUI.h"

/** Constructor */
DeleteResourceUI::DeleteResourceUI(QWidget* parent, bool server)
  : QDialog(parent), m_Server(server)
{
  this->setupUi(this);
}

DeleteResourceUI::~DeleteResourceUI()
{
}

void DeleteResourceUI::Init()
{
  if( this->m_Server )
    {
    m_DeleteLabel->setText("<b>Are you sure you want to delete the selected resource on the server?</b>");
    m_DeleteFileCheckbox->hide();
    }
  else
    {
    m_DeleteLabel->setText("<b>Are you sure you want to delete the selected resource on the client?</b>");
    m_DeleteFileCheckbox->show();
    }
}

int DeleteResourceUI::exec()
{
  this->Init();
  return QDialog::exec();
}

void DeleteResourceUI::accept()
{
  emit DeleteResource(m_DeleteFileCheckbox->isChecked() );

  QDialog::accept();
}

