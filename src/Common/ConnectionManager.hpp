// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_ConnectionManager_hpp
#define CF_Common_ConnectionManager_hpp

#include "Common/CommonAPI.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

/// forward declaradion
class Signal;
class Connection;

/// ConnectionManager executes calls received as string by issuing signals to the slots
/// Slots may be:
///  * its own derived classes that regist  member functions to be called dynamically
///  * other classes that regist themselves to be notified when a signal is issued
///
/// @author Tiago Quintino
class Common_API ConnectionManager {

  typedef std::vector < Connection* >  storage_t;  ///< storage type

  storage_t  m_connections;   ///< the managed signal connections

public: // functions

  /// destructor closes all connections
  ~ConnectionManager();

  /// access a connection
  /// @throws value not found if the connection wasn't found
  Connection* connection( const std::string& name );

  /// manages a connection (creates if necessary)
  Connection* manage_connection( const std::string& name );

}; // ConnectionManager

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_ConnectionManager_hpp
