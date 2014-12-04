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

# use this script to build Debian or RPM packages using fpm

set -e

BUILDVERSION='5.1-master'
RPMBUILDVERSION='5.1_master'
BUILDITERATION=1

function usage() {
        echo "# usage: $0 [destination]"
        echo "  where destination = folder where the package will be saved"
        exit 1
}

function get_linux_flavor() {
    if [ -f /etc/os-release ] && grep "ID=ubuntu" /etc/os-release &> /dev/null ;  then
        OS_FLAVOR='ubuntu'
                OS_VERSION=`lsb_release -r -s`
    fi
    if [ -f /etc/centos-release ]; then
        OS_FLAVOR='centos'
                OS_VERSION=`cat /etc/centos-release | sed 's/^.* release //g' | sed 's/(.*$//g'`
    fi
        OS_MAJOR=`echo $OS_VERSION | cut -d '.' -f 1`
        OS_ARCH=`arch`
}


echo == Building SMQueue package
srcrepodir='COMMERCIAL-smqueue'
builddir='package/tmp'
here=`pwd`
dirname=`basename ${here}`

# place the resulting package in $packagedir (or simply one folder up if not specified)
if [ -z "$1" ]; then
	packagedir='..'
elif [ "$1" == "--help" ]; then
        usage
else
	packagedir=$1
	if [ ! -d "$packagedir" ]; then
		echo "  $packagedir is not a valid directory. Exiting..."
		exit 1
	fi
fi

if [ x$dirname != x$srcrepodir ]; then
	echo "cd to $srcrepodir and try again. Exiting..."
	exit 1
fi

echo " # checking your OS flavor:"
get_linux_flavor
case "${OS_FLAVOR}" in
ubuntu) echo " * It is Ubuntu ${OS_VERSION}, ${OS_ARCH}, we are building a Debian package"
    ;;
centos) echo " * It is CentOS ${OS_VERSION}, ${OS_ARCH}, we are building an RPM package"
    ;;
*) echo "!! Unsupported OS, exiting..."
    exit 1
    ;;
esac
echo

echo " # making a home for this build in ${here}/${builddir}:"
rm -rf ${builddir}
mkdir -p ${builddir}/build
mkdir -p ${builddir}/package
cp package/*.sh ${builddir}/package/
cp package/inputs ${builddir}/package/

case "${OS_FLAVOR}" in
ubuntu) mkdir ${builddir}/deb
    ;;
centos) mkdir ${builddir}/rpm
    ;;
*) exit 1
	;;
esac
echo

echo " # building the component:"
./autogen.sh
./configure
make clean
make
# make check
make install DESTDIR=${here}/${builddir}/build
cd ${builddir}/build
mkdir -p OpenBTS
cd OpenBTS
ln -s ../usr/local/sbin/smqueue
cd ${here}
echo

echo " # creating a package:"
BUILDREPOREV=`git rev-parse --short=10 HEAD`
case "${OS_FLAVOR}" in
ubuntu)	fpm -s dir -t deb -n smqueue -C ${builddir}/build \
	-p ${packagedir}/smqueue_VERSION-sha1.${BUILDREPOREV}_ARCH.deb \
	--description 'Range Networks - SMQueue RFC-3428 Store and Forward Server' \
	--version ${BUILDVERSION} \
	--iteration ${BUILDITERATION} \
	--deb-ignore-iteration-in-dependencies \
	--category 'comm' \
	--license 'AGPLv3' \
	--vendor 'Range Packager <buildmeister@rangenetworks.com>' \
	--maintainer 'Range Packager <buildmeister@rangenetworks.com>' \
	--url 'http://www.rangenetworks.com' \
	-d 'sqlite3' -d 'libosip2-4 | libosip2-10' -d 'libzmq3' \
	--deb-build-depends 'build-essential, debhelper (>= 7), libtool, autoconf, pkg-config, libsqlite3-dev, libosip2-dev, libzmq3-dev' \
	--after-install package/after-install.sh \
	--pre-uninstall package/pre-uninstall.sh \
	--directories 'OpenBTS' \
	--deb-upstart ${here}/debian/smqueue.upstart \
	--deb-priority 'optional' \
	--workdir ${here}/${builddir}/deb \
	--inputs package/inputs
#	--verbose \
    ;;
centos)
	# place the upstart script manually
	mkdir -p ${builddir}/build/etc/init
	cp ${here}/debian/smqueue.upstart ${builddir}/build/etc/init/smqueue.conf

	# Bump up the iteration if necessary
	while [ -f "${packagedir}/smqueue_${RPMBUILDVERSION}-sha1.${BUILDREPOREV}-${BUILDITERATION}.${OS_ARCH}.rpm" ]; do
		BUILDITERATION=$((BUILDITERATION + 1))
	done
	echo "New build iteration: ${BUILDITERATION}"

	fpm -s dir -t rpm -n smqueue -C ${builddir}/build \
	-p ${packagedir}/smqueue_VERSION-sha1.${BUILDREPOREV}-${BUILDITERATION}.ARCH.rpm \
	--description 'Range Networks - SMQueue RFC-3428 Store and Forward Server' \
	--version ${BUILDVERSION} \
	--iteration ${BUILDITERATION} \
	--rpm-ignore-iteration-in-dependencies \
	--category 'comm' \
	--license 'AGPLv3' \
	--vendor 'Range Packager <buildmeister@rangenetworks.com>' \
	--maintainer 'Range Packager <buildmeister@rangenetworks.com>' \
	--url 'http://www.rangenetworks.com' \
	-d 'sqlite >= 3' -d 'libosip2' -d 'zeromq' \
	-d 'rpm-build' -d 'redhat-rpm-config' -d 'libtool' -d 'autoconf' -d 'pkgconfig' -d 'sqlite-devel >= 3' -d 'libosip2-devel' -d 'zeromq-devel' \
	--after-install package/after-install.sh \
	--pre-uninstall package/pre-uninstall.sh \
	--directories 'OpenBTS' \
	--no-rpm-sign \
	--workdir ${here}/${builddir}/rpm \
	--inputs package/inputs \
	etc/init/smqueue.conf
#	--verbose \
    ;;
*)
	;;
esac
echo

echo " # cleaning up:"
make clean
echo

echo "== All done."
