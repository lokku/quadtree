#!/bin/sh

set -x

mkdir -p $PLAYPEN_ROOT/ext/lib/
cp libquadtree.so.1.0.1 $PLAYPEN_ROOT/ext/lib/
ln -s $PLAYPEN_ROOT/ext/lib/libquadtree.so.1.0.1 $PLAYPEN_ROOT/ext/lib/libquadtree.so
ln -s $PLAYPEN_ROOT/ext/lib/libquadtree.so.1.0.1 $PLAYPEN_ROOT/ext/lib/libquadtree.so.1
cp *.h $PLAYPEN_ROOT/ext/include/