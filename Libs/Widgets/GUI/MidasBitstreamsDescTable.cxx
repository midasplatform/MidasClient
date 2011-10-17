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
#include "MidasBitstreamsDescTable.h"

#include <QDragEnterEvent>
#include <QUrl>
#include <QFileInfo>

#include "Logger.h"

QTableWidgetBitstreamItem::QTableWidgetBitstreamItem(STRUCT_BITSTREAM * modeldata, STRUCT_ITEM * itemModelData,
                                                     const char* text,
                                                     QTableWidgetBitstreamItem::Options options) :
  QTableWidgetDescriptionItem(text, options),
  itemModelData(itemModelData), modeldata(modeldata)
{
}

void MidasBitstreamsDescTable::contextMenuEvent( QContextMenuEvent * e )
{
  emit midasTableWidgetContextMenu( e );
}

void MidasBitstreamsDescTable::dragEnterEvent( QDragEnterEvent * event )
{
  if( event && event->mimeData() )
    {
    const QMimeData * md = event->mimeData();
    QStringList       mimetypes = md->formats();
    foreach(QString mimetype, mimetypes)
      {
      Logger::debug(STR("\t") + QSTR2STR(mimetype) );
      }
    if( md->hasUrls() )
      {
      Logger::debug("acceptProposedAction");
      event->acceptProposedAction();
      }
    }
}

void MidasBitstreamsDescTable::dragMoveEvent( QDragMoveEvent * event )
{
}

void MidasBitstreamsDescTable::dropEvent( QDropEvent * event )
{
  if( event && event->mimeData() )
    {
    const QMimeData * md = event->mimeData();
    if( md->hasUrls() )
      {
      QStringList files;
      foreach(QUrl url, md->urls() )
        {
        QFileInfo info(url.toLocalFile() );

        if( info.exists() && info.isFile() )
          {
          Logger::debug("name:" + QSTR2STR(url.toLocalFile() ) );
          files << url.toLocalFile();
          // emit localFileDropped( url.toLocalFile() );
          }
        }
      emit localFilesDropped( files );
      }
    }
}

