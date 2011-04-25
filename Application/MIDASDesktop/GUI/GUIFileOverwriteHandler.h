/*=========================================================================

  Copyright (c) Kitware Inc. All rights reserved.
  See Copyright.txt or http://www.Kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef __GUIFileOverwriteHandler_H
#define __GUIFileOverwriteHandler_H

#include "midasFileOverwriteHandler.h"
#include <QObject>

class FileOverwriteUI;

class GUIFileOverwriteHandler : public QObject, public midasFileOverwriteHandler
{
  Q_OBJECT
public:
  GUIFileOverwriteHandler(FileOverwriteUI* dialog);
  ~GUIFileOverwriteHandler();

  Action HandleConflict(std::string path);

signals:
  void displayDialog();

private:
  FileOverwriteUI* m_Dialog;
  bool             m_ApplyToAll;
};

#endif
