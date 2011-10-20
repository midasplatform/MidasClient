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
#include "PushUI.h"
#include "SynchronizerThread.h"
#include "midasSynchronizer.h"
#include "mdoObject.h"

/** Constructor */
PushUI::PushUI(QWidget* parent, midasSynchronizer* synch)
  : QDialog(parent), m_Delete(false), m_Object(NULL), m_Synch(synch), m_SynchThread(NULL)
{
  this->setupUi(this);
  this->ResetState();

  connect( m_PushAllRadioButton, SIGNAL( released() ), this, SLOT( RadioButtonChanged() ) );
  connect( m_SelectedRadioButton, SIGNAL( released() ), this, SLOT( RadioButtonChanged() ) );
}

PushUI::~PushUI()
{
  delete m_SynchThread;
}

SynchronizerThread * PushUI::GetSynchronizerThread()
{
  return m_SynchThread;
}

void PushUI::SetObject(mdo::Object* object)
{
  m_Object = object;
}

void PushUI::SetDelete(bool val)
{
  m_Delete = val;
}

void PushUI::ResetState()
{
  m_SelectedRadioButton->setChecked(true);
  this->RadioButtonChanged();
}

void PushUI::RadioButtonChanged()
{
  bool val = m_SelectedRadioButton->isChecked();

  m_RecursiveCheckbox->setEnabled(val);
}

void PushUI::Init()
{
  if( m_Object )
    {
    m_SelectedRadioButton->setChecked(true);
    }
  else
    {
    m_PushAllRadioButton->setChecked(true);
    }
  this->RadioButtonChanged();
}

int PushUI::exec()
{
  this->Init();
  return QDialog::exec();
}

void PushUI::accept()
{
  delete m_SynchThread;
  m_SynchThread = new SynchronizerThread;
  m_SynchThread->SetSynchronizer(m_Synch);

  m_Synch->SetOperation(midasSynchronizer::OPERATION_PUSH);

  if( m_Object && m_SelectedRadioButton->isChecked() )
    {
    std::stringstream text;
    text << m_Object->GetId();

    m_Synch->SetClientHandle(text.str() );
    m_Synch->SetServerHandle(m_Object->GetUuid() );
    m_Synch->SetResourceType(m_Object->GetResourceType() );
    m_Synch->SetResourceType3(m_Object->GetResourceType() );
    m_Synch->SetRecursive(m_RecursiveCheckbox->isChecked() );
    }
  else
    {
    m_Synch->SetResourceType(midasResourceType::NONE);
    m_Synch->SetResourceType3(midas3ResourceType::NONE);
    }

  connect(m_SynchThread, SIGNAL( PerformReturned(int) ),
          this, SIGNAL( PushedResources(int) ) );
  connect(m_SynchThread, SIGNAL( EnableActions(bool) ),
          this, SIGNAL( EnableActions(bool) ) );

  if( m_Delete )
    {
    connect(m_SynchThread, SIGNAL( finished() ),
            this, SLOT( DeleteObject() ) );
    }

  m_SynchThread->start();

  m_Delete = false;
  QDialog::accept();
}

void PushUI::reject()
{
  if( m_Delete )
    {
    this->DeleteObject();
    }

  m_Delete = false;
  QDialog::reject();
}

void PushUI::DeleteObject()
{
  delete m_Object;
}
