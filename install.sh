#!/bin/sh

set -x

cp libquadtree.so.1.0.1 $LOCAL_VAR/lib/
ln -s $LOCAL_VAR/lib/libquadtree.so.1.0.1 $LOCAL_VAR/lib/libquadtree.so
ln -s $LOCAL_VAR/lib/libquadtree.so.1.0.1 $LOCAL_VAR/lib/libquadtree.so.1
cp *.h $LOCAL_VAR/include/