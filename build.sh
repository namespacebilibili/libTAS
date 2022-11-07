#!/bin/sh

set -e
cd "$(dirname "$0")"

aclocal
autoconf
autoheader
automake --add-missing
CXXFLAGS="-std=c++14 -O2 -g -Wall -pedantic -fpermissive" ./configure "$@"
make -j4
