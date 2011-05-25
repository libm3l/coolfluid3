// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Signal.hpp"

#include "Common/XML/SignalOptions.hpp"

#include "UI/UICommon/ComponentNames.hpp"

#include "UI/Core/NetworkThread.hpp"
#include "UI/Core/ThreadManager.hpp"

#include "UI/Core/NRemoteFSBrowser.hpp"

#include "Common/XML/FileOperations.hpp"

#include "QFileIconProvider"

using namespace CF::Common::XML;

/////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {
namespace Core {

/////////////////////////////////////////////////////////////////////////////

NRemoteFSBrowser::NRemoteFSBrowser( const std::string & name ) :
  CNode(name, "NRemoteFSBrowser", CNode::DEBUG_NODE),
  m_includeNoExtensions(false),
  m_includeFiles(true)
{
  m_completionModel = new QStringListModel(this);

  m_headers << "Name" << "Size" << "Date Modified";

  FileInfo * info = new FileInfo();

  info->name = "Directory";
  info->type = DIRECTORY;

  m_data.append(info);

  info = new FileInfo();
  info->name = "Blabla.txt";
  info->type = FILE;

  m_iconProvider = new QFileIconProvider();

  m_data.append(info);

  regist_signal("read_dir", "")->signal->
      connect(boost::bind(&NRemoteFSBrowser::signal_read_dir, this, _1));

}

/////////////////////////////////////////////////////////////////////////////

NRemoteFSBrowser::~NRemoteFSBrowser()
{
  delete m_completionModel;
}

/////////////////////////////////////////////////////////////////////////////

void NRemoteFSBrowser::signal_read_dir ( Common::SignalArgs & args )
{
  SignalOptions options( args );
  QStringList completionList;

  std::vector<std::string> dirs;
  std::vector<std::string> files;

  std::vector<std::string>::const_iterator itDirs;
  std::vector<std::string>::const_iterator itFiles;

  m_currentPath = options.option<std::string>("dirPath").c_str();

  // add an ending '/' if the string does not have any
  if( !m_currentPath.endsWith("/") )
    m_currentPath += "/";

  emit currentPathChanged( m_currentPath );

//  if(!m_updatingCompletion)
//    m_editPath->setText(m_currentPath);
//  else
//    m_updatingCompletion = false;

  dirs = options.array<std::string>("dirs");
  files = options.array<std::string>("files");

  // notice the view(s) that the model is about to be completely changed
  emit layoutAboutToBeChanged();

  //clear();
  m_completionModel->setStringList( QStringList() );

  // delete old data
  while( !m_data.empty() )
    delete m_data.takeFirst();

  // add the directories
  for( itDirs = dirs.begin() ; itDirs != dirs.end() ; ++itDirs )
  {
    FileInfo * fileInfo = new FileInfo();
    QString name = itDirs->c_str();

    fileInfo->name = name;
    fileInfo->type = DIRECTORY;

    if(!m_currentPath.isEmpty() && name != "..")
      name.prepend(m_currentPath + (m_currentPath.endsWith("/") ? "" : "/"));

    m_data.append(fileInfo);
    completionList << name;
  }

  m_completionModel->setStringList( completionList );

  // add the files
  for(itFiles = files.begin() ; itFiles != files.end() ; ++itFiles )
  {
    FileInfo * fileInfo = new FileInfo();

    fileInfo->name = itFiles->c_str();
    fileInfo->type = FILE;

    m_data.append(fileInfo);
  }

//  this->updateModel(m_viewModel, "", dirs, files, m_items);
//  this->updateModel(m_completerModel, m_currentPath, dirs,
//                    std::vector<std::string>(), m_itemsCompleter);

  // notice the view(s) that the model has finished changing
  emit layoutChanged();
}

/////////////////////////////////////////////////////////////////////////////

//void NRemoteFSBrowser::updateModel(QAbstractItemModel * model, const QString & path,
//                                   const std::vector<std::string> & dirs,
//                                   const std::vector<std::string> & files,
//                                   QList<FilesListItem *> & modelItems)
//{

//}

/////////////////////////////////////////////////////////////////////////////

QStringListModel * NRemoteFSBrowser::completionModel() const
{
  return m_completionModel;
}

/////////////////////////////////////////////////////////////////////////////

QString NRemoteFSBrowser::currentPath() const
{
  return m_currentPath;
}

/////////////////////////////////////////////////////////////////////////////

void NRemoteFSBrowser::openDir ( const QString & path )
{
  SignalFrame frame("read_dir", full_path(), SERVER_CORE_PATH);
  SignalOptions options( frame );

  std::vector<std::string> vect;
  QStringList::iterator it = m_extensions.begin();

  while(it != m_extensions.end())
  {
    vect.push_back(it->toStdString());
    it++;
  }

  options.add("dirPath", path.toStdString());
  options.add("includeFiles", m_includeFiles);
  options.add("includeNoExtensions", m_includeNoExtensions);
  options.add<std::string>("extensions", vect, ";");

  ThreadManager::instance().network().send(frame);
}

/////////////////////////////////////////////////////////////////////////////

void NRemoteFSBrowser::setExtensions( const QStringList & list )
{
  m_extensions = list;

  m_extensions.removeDuplicates();
}

/////////////////////////////////////////////////////////////////////////////

void NRemoteFSBrowser::addExtension( const QString & extension )
{
  if( !m_extensions.contains( extension ) )
    m_extensions.append( extension );
}

/////////////////////////////////////////////////////////////////////////////

QStringList NRemoteFSBrowser::extensions() const
{
  return m_extensions;
}

/////////////////////////////////////////////////////////////////////////////

void NRemoteFSBrowser::setIncludeNoExtensions ( bool include )
{
  m_includeNoExtensions = include;
}

/////////////////////////////////////////////////////////////////////////////

bool NRemoteFSBrowser::includeNoExtensions () const
{
  return m_includeNoExtensions;
}

/////////////////////////////////////////////////////////////////////////////

void NRemoteFSBrowser::setIncludeFiles ( bool includeFiles )
{
  m_includeFiles = includeFiles;
}

/////////////////////////////////////////////////////////////////////////////

bool NRemoteFSBrowser::includeFiles () const
{
  return m_includeFiles;
}

/////////////////////////////////////////////////////////////////////////////

QVariant NRemoteFSBrowser::data ( const QModelIndex & index, int role ) const
{
  QVariant data;

  if( index.isValid() && index.column() == 0 )
  {
    FileInfo * item = m_data.at(index.row());

    switch(role)
    {
    case Qt::DisplayRole:
      data = item->name;
      break;

    case Qt::UserRole:
      data = item->type;
      break;
    }
  }

  return data;
}

/////////////////////////////////////////////////////////////////////////////

QModelIndex NRemoteFSBrowser::parent ( const QModelIndex & child ) const
{
  return QModelIndex();
}

/////////////////////////////////////////////////////////////////////////////

QModelIndex NRemoteFSBrowser::index ( int row, int column, const QModelIndex & parent ) const
{
  QModelIndex index;

  if( !parent.isValid() && this->hasIndex(row, column, parent) )
  {
    index = createIndex(row, column, m_data.at(row) );
  }

  return index;
}

/////////////////////////////////////////////////////////////////////////////

int NRemoteFSBrowser::rowCount(const QModelIndex &parent) const
{
  return m_data.count();
}

/////////////////////////////////////////////////////////////////////////////

int NRemoteFSBrowser::columnCount(const QModelIndex &parent) const
{
  return m_headers.count();
}

/////////////////////////////////////////////////////////////////////////////

QVariant NRemoteFSBrowser::headerData(int section, Qt::Orientation orientation, int role) const
{
  QVariant header;

  if( role == Qt::DisplayRole && orientation == Qt::Horizontal &&
      section >= 0 && section < m_headers.count() )
      header = m_headers.at( section );

  return header;
}

/////////////////////////////////////////////////////////////////////////////

} // Core
} // UI
} // CF