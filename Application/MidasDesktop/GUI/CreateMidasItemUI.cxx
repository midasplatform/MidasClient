#include "CreateMidasItemUI.h"

#include <QtGui> 
#include <Qt>
#include <QDate>

#include "MIDASDesktopUI.h"
#include "mwsSettings.h"
#include "midasSynchronizer.h"
#include "Utils.h"

#include "MidasTreeItem.h"
#include "MidasTreeModel.h"
#include "MidasTreeModelClient.h"
#include "MidasCollectionTreeItem.h"
#include "MidasClientGlobal.h"

CreateMidasItemUI::CreateMidasItemUI(DionysusUI *parent, CreateMidasItemUI::Types type):
  QDialog(parent), parent(parent)
{
  this->setType(type);
  setupUi(this); // this sets up GUI

  connect( this->addAuthorButton, SIGNAL( released() ), this, SLOT( addAuthor() ) ); 
  connect( this->addKeywordButton, SIGNAL( released() ), this, SLOT( addKeyword() ) ); 
}

void CreateMidasItemUI::setType(Types type)
{
  this->type = type;
  if (type == CreateMidasItemUI::ClientItem)
    {
    QWidget::setWindowTitle(tr("Add item")); 
    }
}

void CreateMidasItemUI::reset()
{
  this->authorsEdit->clear();
  this->titleEdit->clear();
  this->keywordsEdit->clear();
  this->issueDateEdit->setDate(QDate::currentDate());
  this->descriptionEdit->clear();
  this->abstractEdit->clear();
  this->sponsorEdit->clear();
}

int CreateMidasItemUI::exec()
{
  this->reset();
  return QDialog::exec();
}

void CreateMidasItemUI::accept()
{
  if(this->titleEdit->text() == "")
    {
    parent->getLog()->Error("You must supply a title for the item.");
    QDialog::accept();
    return;
    }
  
  QModelIndex selected;
  if(this->type == CreateMidasItemUI::ClientItem)
    {
    std::string path = kwsys::SystemTools::GetCurrentWorkingDirectory();
    selected = this->parent->getTreeViewClient()->getSelectedModelIndex();
    MidasCollectionTreeItem* parentColl =
      reinterpret_cast<MidasCollectionTreeItem*>(
      const_cast<MidasTreeItem*>(
      reinterpret_cast<MidasTreeModelClient*>(
      this->parent->getTreeViewClient()->model())->midasTreeItem(selected)));

    parent->getDatabaseProxy()->Open();
    path = parent->getDatabaseProxy()->GetRecordByUuid(
      parent->getDatabaseProxy()->GetUuid(
      midasResourceType::COLLECTION,
      parentColl->getCollection()->GetId())).Path;
    parent->getDatabaseProxy()->Close();
    
    path += "/" + titleEdit->text().toStdString();
    kwsys::SystemTools::MakeDirectory(path.c_str());

    parent->getSynchronizer()->SetOperation(midasSynchronizer::OPERATION_ADD);
    parent->getSynchronizer()->SetResourceType(midasResourceType::ITEM);
    parent->getSynchronizer()->SetResourceHandle(path);
    
    std::stringstream text;
    if(parent->getSynchronizer()->Perform() == 0)
      {    
      text << "Successfully added item " << path << ".";
      parent->getLog()->Message(text.str());
      parent->clientTreeViewUpdated();
      }
    else
      {
      text << "Failed to add item at " << path << ".";
      parent->getLog()->Error(text.str());
      }
    }
  else
    {
    selected = this->parent->getTreeView()->getSelectedModelIndex();

    std::vector<std::string> keywords;
    kwutils::tokenize( QSTR2STR(this->keywordsEdit->text()), keywords, MIDAS_KEYWORDS_DELIMITER);

    std::vector<std::string> authors;
    kwutils::tokenize( QSTR2STR(this->authorsEdit->text()), authors, MIDAS_AUTHORS_DELIMITER);

    reinterpret_cast<MidasTreeModel*>(this->parent->getTreeView()->model())
      ->insertMidasItem(
      selected, 
      QSTR2STR(this->titleEdit->text()), 
      authors, 
      keywords,
      QSTR2STR(this->abstractEdit->toPlainText()), 
      QSTR2STR(this->descriptionEdit->toPlainText()), 
      QSTR2STR(this->issueDateEdit->date().toString("yyyy-MM-dd")) + " 00:00:00",
      QSTR2STR(this->linksEdit->text()));
    }
  QDialog::accept();
}

void CreateMidasItemUI::addAuthor()
{
  QString value = QInputDialog::getText( this, tr("Add author"), tr("Separate First and Last name using a comma") ).simplified(); 
  if ( !value.isEmpty() )
    {
    QString current = this->authorsEdit->text(); 
    this->authorsEdit->setText(current.isEmpty() ? value : current + MIDAS_AUTHORS_DELIMITER + " " + value); 
    }
}

void CreateMidasItemUI::addKeyword()
{
  QString value = QInputDialog::getText( this, tr("Add keyword"), tr("Keyword") ).simplified(); 
  if ( !value.isEmpty() )
    {
    QString current = this->keywordsEdit->text(); 
    this->keywordsEdit->setText(current.isEmpty() ? value : current + " " + MIDAS_KEYWORDS_DELIMITER + " " + value); 
    }
}
