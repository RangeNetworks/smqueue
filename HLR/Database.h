/*
* Copyright 2009 Kestrel Signal Processing, Inc.
*
* This software is distributed under multiple licenses; see the COPYING file in the main directory for licensing information for this specific distribuion.
*
* This use of this software may be subject to additional restrictions.
* See the LEGAL file in the main directory for details.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

*/



#ifndef DATABASE_H
#define DATABASE_H

#include <mysql_connection.h>
#include <mysql_driver.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

#include <map>

class DatabaseArgList {

	public:

	typedef std::map<std::string,std::string> Map;

	private:

	Map mTable;

	public:

	unsigned size() const { return mTable.size(); }
	void set(const char* key, const char* v) { mTable[key]=v; }
	void set(const char* key, unsigned v);
	void setHex(const char* key, uint64_t v);
	Map::const_iterator begin() const { return mTable.begin(); }
	Map::const_iterator end() const { return mTable.end(); }

};




class Database {

	private:

	sql::mysql::MySQL_Driver *mDriver;
	sql::Connection *mConnection;

	public:

	/**
		Create a connection to a MySQL server on a specific database.
		@param server server URI, "tcp://127.0.0.1:3306", for example.
		@param user user name on the server
		@param password the user password on the server
	*/
	Database(const char* server, const char* user, const char* password, const char* database);

	/**
		Commit (INSERT) a record to the database.
		@param tableName the database table name
		@param argList a set of key/value pairs to insert into the table.
		@return true on success
	*/
	bool commit(const char* tableName, const DatabaseArgList&);

	/**
		Query the database.
		@param query A general MySQL query.
		@return  A result set, to be deleted by the caller, NULL on failure.
	*/
	sql::ResultSet* query(const char* queryString) const;

};


extern Database gDatabase;

#endif
// vim: ts=4 sw=4
