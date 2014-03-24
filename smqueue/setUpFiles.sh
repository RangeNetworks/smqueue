#!/bin/sh

# Install smqueue and associated configuration files.

if [ $# -eq 1 ]; then
    PREFIX=$1
else
    if [ "$USER" != "root" ]; then
            echo This script must be run as super-user.
            exit 1
    fi
    PREFIX=""
fi

INSTALL_DIR=$PREFIX/OpenBTS
UPSTART_DIR=$PREFIX/etc/init
CONFIG_DIR=$PREFIX/etc/OpenBTS



if [ ! -e $INSTALL_DIR ]; then
	mkdir -p $INSTALL_DIR
fi

if [ ! -e $UPSTART_DIR ]; then
	mkdir -p $UPSTART_DIR
fi

if [ ! -e $CONFIG_DIR ]; then
	mkdir -p $CONFIG_DIR
fi

cp smqueue $INSTALL_DIR
cp smqueue.conf $UPSTART_DIR

if [ $# -ne 1 ]; then
	cp smqueue.example.sql $CONFIG_DIR
	sqlite3 $CONFIG_DIR/smqueue.db ".read smqueue.example.sql"
else
	cp smqueue.example.sql $CONFIG_DIR/
fi

chown -R openbts $CONFIG_DIR
chown -R openbts $INSTALL_DIR
chmod -R a-w $INSTALL_DIR
