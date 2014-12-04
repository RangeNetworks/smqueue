#
# Copyright 2008 Free Software Foundation, Inc.
# Copyright 2014 Range Networks, Inc.
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

# EXTRA_DIST = \
#	README.smqueue \
#	smqueue.conf

ourlibs = \
	$(GLOBALS_LA) \
	$(SMS_LA) \
	$(GSM_LA) \
	$(COMMON_LA) \
	$(NODEMANAGER_LA)

sbin_PROGRAMS = smqueue
confdir = /etc/OpenBTS
conf_DATA = smqueue.example.sql

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

smqueue_LDADD = $(ourlibs) $(OSIP_LIBS)

smqueue.example.sql: smqueue
	( ./smqueue --gensql > smqueue.example.sql || true )

