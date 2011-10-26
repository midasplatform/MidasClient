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


#include "GUIProgress.h"

GUIProgress::GUIProgress(QProgressBar* progressBar)
{
  m_ProgressBar = progressBar;
  m_ProgressBar->setMinimum(0);
  m_ProgressBar->setMaximum(100);
  m_ProgressBar->setTextVisible(false);
  this->ResetProgress();
  m_LastAmount = 0;
  m_StartTime = 0;
  m_StartAmount = 0;

  connect(this, SIGNAL(UpdateProgressMin(int) ),
          m_ProgressBar, SLOT( setMinimum(int) ) );
  connect(this, SIGNAL(UpdateProgressMax(int) ),
          m_ProgressBar, SLOT( setMaximum(int) ) );
  connect(this, SIGNAL(UpdateProgressValue(int) ),
          m_ProgressBar, SLOT( setValue(int) ) );
}

GUIProgress::~GUIProgress()
{
}

void GUIProgress::UpdateProgress(double current, double max)
{
  m_Total += (current - m_LastAmount); // record difference
  m_LastAmount = current;
  emit CurrentProgress(current, max);
  emit OverallProgressTotal(m_Total, m_MaxTotal);

  if( current == max )
    {
    m_LastAmount = 0;
    m_StartTime = 0;
    m_StartAmount = 0;
    }

  double currentTime = midasUtils::CurrentTime();
  if( m_StartTime == 0 )
    {
    m_StartTime = currentTime;
    m_StartAmount = current;
    }
  else if( currentTime - m_StartTime > SPEED_CALC_INTERVAL )
    {
    double speed = (current - m_StartAmount) / (currentTime - m_StartTime);
    emit   Speed(speed);

    double estimatedTimeLeft = (m_MaxTotal - m_Total) / speed;
    emit   EstimatedTime(estimatedTimeLeft);

    m_StartTime = currentTime;
    m_StartAmount = current;
    }
}

void GUIProgress::UpdateOverallCount(int value)
{
  emit OverallProgressCount(value, m_MaxCount);
}

void GUIProgress::UpdateTotalProgress(double value)
{
  m_Total += value;
  emit OverallProgressTotal(m_Total, m_MaxTotal);
}

void GUIProgress::SetMessage(std::string message)
{
  emit ProgressMessage(message.c_str() );
}

void GUIProgress::SetIndeterminate()
{
  emit UpdateProgressMin(0);
  emit UpdateProgressMax(0);
}

void GUIProgress::ResetProgress()
{
  m_StartAmount = 0;
  m_StartTime = 0;
  m_LastAmount = 0;
  m_Done = false;
  emit UpdateProgressMax(100);
  emit UpdateProgressValue(0);
}

void GUIProgress::ResetOverall()
{
  m_Total = 0;
  m_MaxTotal = 0;
  emit OverallProgressTotal(0, 0);
}

