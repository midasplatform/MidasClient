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
#include "PollFilesystemThread.h"
#include "mdsDatabaseAPI.h"

PollFilesystemThread::PollFilesystemThread()
{
  m_Run = true;
  m_DatabaseLocked = false;
  m_Terminate = false;
}

PollFilesystemThread::~PollFilesystemThread()
{
}

void PollFilesystemThread::Pause()
{
  m_Run = false;
  while( m_DatabaseLocked )
    {
    }

  emit Paused();
}

void PollFilesystemThread::Resume()
{
  m_Run = true;
}

void PollFilesystemThread::Terminate()
{
  m_Terminate = true;
}

void PollFilesystemThread::run()
{
  while( !m_Terminate )
    {
    if( m_Run )
      {
      m_DatabaseLocked = true;
      mds::DatabaseAPI db;
      if( m_Run && db.CheckModifiedBitstreams() )
        {
        emit NeedToRefresh();
        }
      m_DatabaseLocked = false;
      }
    this->sleep(5);
    }
}
