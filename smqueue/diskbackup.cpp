/*
* Copyright 2012 Range Networks, Inc.
*
* Written by Kurtis Heimerl, November 2012
*
* This software is distributed under the terms of the GNU Affero Public License.
* See the COPYING file in the main directory for details.
*
* This use of this software may be subject to additional restrictions.
* See the LEGAL file in the main directory for details.

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU Affero General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Affero General Public License for more details.

	You should have received a copy of the GNU Affero General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "diskbackup.h"
#include <sys/stat.h>
#include "sqlite3.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <Configuration.h>

extern ConfigurationTable gConfig;

using namespace std;

//basically a copy of the datastructure but in a table
static const char* createMessageTable = {
    "CREATE TABLE IF NOT EXISTS MESSAGES ("
		"timestamp	integer UNIQUE not null, "
		"message        text not null"
    ")"
};

int SQLiteBackup::init()
{
    string ldb = gConfig.getStr("Backup.db");
    size_t p = ldb.find_last_of('/');
    if (p == string::npos) {
	LOG(EMERG) << "Backup.db not in a directory?";
	mDB = NULL;
	return FAILURE;
    }
    string dir = ldb.substr(0, p);
    struct stat buf;
    if (stat(dir.c_str(), &buf)) {
	LOG(EMERG) << dir << " does not exist";
	mDB = NULL;
	return FAILURE;
    } 
    int rc = sqlite3_open(ldb.c_str(),&mDB);
    if (rc) {
	LOG(EMERG) << "Cannot open SQLiteBackup database: " << sqlite3_errmsg(mDB);
	sqlite3_close(mDB);
	mDB = NULL;
	return FAILURE;
    }
    if (!sqlite3_command(mDB,createMessageTable)) {
        LOG(EMERG) << "Cannot create MESSAGES table";
        return FAILURE;
    }
    return SUCCESS;
}

SQLiteBackup::~SQLiteBackup()
{
    if (mDB) sqlite3_close(mDB);
}

backup_msg_list* SQLiteBackup::get_stored_messages(){
    // Prepare the statement.
    string cmd = "SELECT timestamp,message FROM MESSAGES";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_statement(mDB,&stmt,cmd.c_str())) return NULL;
    // Read the result.
    backup_msg_list* list = new backup_msg_list(0);
    int src = sqlite3_run_query(mDB,stmt);
    while (src==SQLITE_ROW) {
	long long timestamp = sqlite3_column_int(stmt,0);
	string text = (char *)sqlite3_column_text(stmt,1);
	backup_msg msg;
	msg.timestamp = timestamp;
	msg.text = text;
	list->push_back(msg);
	src = sqlite3_run_query(mDB,stmt);
    }
    sqlite3_finalize(stmt);
    return list;
}

int SQLiteBackup::insert(long long timestamp, char* text)
{
    ostringstream os;
    os << "INSERT into MESSAGES (timestamp, message) values (";
    os << "\"" << timestamp << "\"";
    os << ",";
    os << "\"" << text << "\"";
    os << ")";
    LOG(INFO) << "Inserting " << timestamp << " into backup db";
    LOG(DEBUG) << "Inserting " << timestamp << ":" << text;
    return sqlite3_command(db(), os.str().c_str());
}

int SQLiteBackup::remove(long long timestamp)
{
    ostringstream os;
    os << "DELETE from MESSAGES WHERE timestamp=";
    os << timestamp;
    LOG(INFO) << "Removing " << timestamp << " from backup db";
    return sqlite3_command(db(), os.str().c_str());
}
