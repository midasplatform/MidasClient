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
#ifndef __CreateProfileUI_H
#define __CreateProfileUI_H

#include "ui_CreateProfileUI.h"

class CreateProfileUI :  public QDialog, private Ui::CreateProfileDialog
{
  Q_OBJECT
public:

  CreateProfileUI(QWidget* parent);
  ~CreateProfileUI();

  void Init();

signals:
  void CreatedProfile(std::string name, std::string email, std::string apiName, std::string password,
                      std::string rootDir,
                      std::string serverURL);

  void DeletedProfile(std::string name);

public slots:

  int exec();

  virtual void accept();

  void FillData(const QString& profileName);

  void AnonymousChanged(int state);

  void RootDirChecked(int state);

  void DeleteProfile();

  void BrowseRootDir();

};

#endif // __CreateProfileUI_H
