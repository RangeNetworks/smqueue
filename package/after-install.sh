#!/bin/bash

# Copyright 2014 Range Networks, Inc.
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU Affero General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.

#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU Affero General Public License for more details.
#
#    You should have received a copy of the GNU Affero General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.

set -e
CONFIG_DB='/etc/OpenBTS/smqueue.db'
DATE=$(date +'%Y-%m-%dT%H-%M')

backup()
{
	CONFIG_BACKUP="/etc/OpenBTS/smqueue.sql-${DATE}"
	if [ -f ${CONFIG_DB} ] && [ ! -e ${CONFIG_BACKUP} ]; then
		echo "backing up SMQueue configuration DB..."
		sqlite3 ${CONFIG_DB} ".dump" > ${CONFIG_BACKUP}
		echo "done"
	fi
}

config() {
	# setup configuration db
	sqlite3 ${CONFIG_DB} ".read /etc/OpenBTS/smqueue.example.sql" > /dev/null 2>&1
}

backup
config
