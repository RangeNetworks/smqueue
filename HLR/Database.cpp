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

#include <Logger.h>
#include "Database.h"


void DatabaseArgList::set(const char* key, unsigned v)
{
	char buf[40];
	sprintf(buf,"%u",v);
	set(key,buf);
}



void DatabaseArgList::setHex(const char* key, uint64_t v)
{
	char buf[40];
	sprintf(buf,"%llx",v);
	set(key,buf);
}



Database::Database(const char* server, const char* user, const char* password, const char* database)
{
	try {
		mDriver = sql::mysql::MySQL_Driver::Instance();
		mConnection = mDriver->connect(server,user,password);
	
		if (!mConnection) {
			LOG(ERROR) << "cannot connect to database " << server << " as " << user;
			return;
		}
		sql::Statement *stmt = mConnection->createStatement();
		char buffer[100];
		sprintf(buffer,"USE %s",database);
		stmt->execute(buffer);
		delete stmt;
	}

	catch (sql::SQLException exp) {
		LOG(ERROR) << "SQL exception: " << exp.what();
	}

}



bool Database::commit(const char* tableName, const DatabaseArgList& values)
{
	if (values.size()==0) return true;

	char cols[1024];
	char vals[1024];
	char *cp = cols;
	char *vp = vals;
	DatabaseArgList::Map::const_iterator iter = values.begin();
	while (iter != values.end()) {
		cp += sprintf(cp,"%s,",iter->first.c_str());
		vp += sprintf(vp,"\'%s\',",iter->second.c_str());
		++iter;
	}
	// Replace final "," with termination.
	cp--;
	*cp = '\0';
	vp--;
	*vp = '\0';

	try {
		// Form the final command.
		sql::Statement *stmt = mConnection->createStatement();
		char buffer[2048];
		sprintf(buffer,"INSERT INTO %s(%s) VALUES (%s)", tableName,cols,vals);
		bool retVal = stmt->execute(buffer);
		delete stmt;
		return retVal;
	}

	catch (sql::SQLException exp) {
		LOG(ERROR) << "database write failed, SQL exception: " << exp.what();
		return false;
	}
}


sql::ResultSet *Database::query(const char *queryString) const
{
	try {
		// Form the final command.
		sql::Statement *stmt = mConnection->createStatement();
		sql::ResultSet* retVal = stmt->executeQuery(queryString);
		delete stmt;
		return retVal;
	}

	catch (sql::SQLException exp) {
		LOG(ERROR) << "database write failed, SQL exception: " << exp.what();
		return NULL;
	}
}


// vim: ts=4 sw=4
