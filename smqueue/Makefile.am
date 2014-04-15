#
# Copyright 2008 Free Software Foundation, Inc.
#
# This software is distributed under the terms of the GNU Affero Public License.
# See the COPYING file in the main directory for details.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

include $(top_srcdir)/Makefile.common

DESTDIR := 

AM_CPPFLAGS = $(STD_DEFINES_AND_INCLUDES)
AM_CXXFLAGS = -lrt -O3 -g -lpthread

EXTRA_DIST = \
	README.smqueue \
	smqueue.conf

noinst_PROGRAMS = \
	smqueue

noinst_HEADERS = \
	poll.h \
	smnet.h \
	smqueue.h \
	QueuedMsgHdrs.h \
	SmqGlobals.h \
	SmqMessageHandler.h \
	SmqReader.h \
	SmqWriter.h \
	SmqTest.h
	smsc.h

smqueue_SOURCES = \
	poll.c \
	smcommands.cpp \
	smnet.cpp \
	smqueue.cpp \
	QueuedMsgHdrs.cpp \
	SmqGlobals.cpp \
	SmqMessageHandler.cpp \
	SmqReader.cpp \
	SmqWriter.cpp \
	SmqTest.cpp \
	smsc.cpp \
	../SR/SubscriberRegistry.cpp

smqueue_LDADD = \
	$(GLOBALS_LA) \
	$(SMS_LA) \
	$(GSM_LA) \
	$(COMMON_LA) \
	$(SQLITE_LA) \
	$(SIP_LA) \
	$(NODEMANAGER_LA) \
	$(OSIP_LIBS)

install: smqueue
	mkdir -p "$(DESTDIR)/OpenBTS/"
	install smqueue "$(DESTDIR)/OpenBTS/"
	mkdir -p "$(DESTDIR)/etc/supervisor/conf.d/"
	install smqueue.conf "$(DESTDIR)/etc/supervisor/conf.d/"
	mkdir -p "$(DESTDIR)/etc/OpenBTS/"
	install smqueue.example.sql "$(DESTDIR)/etc/OpenBTS/"

