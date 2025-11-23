#!/bin/sh

set -e
. ./headers.sh

for PROJECT in $PROJECTS; do
    (cd $PROJECT && $MAKE install)
done