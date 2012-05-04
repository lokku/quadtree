# Copyright (C) 2011-2012 Lokku ltd. and contributors
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

SUBDIRS = tests

##
## CFLAGS
##

# Don't change this! Adding anything but warning flags may break CFLAGS
# customization for people not using gcc.
AM_CFLAGS = -Wall -W


##
## The quadtree library
##

lib_LTLIBRARIES = libquadtree.la
libquadtree_la_SOURCES = quadtree.c quadtree_private.h
libquadtree_la_SOURCES += quadtree_portable.h
libquadtree_ladir = ${libdir}
libquadtree_la_LDFLAGS = -version-info ${QUADTREE_LIB_VERSION}

# Headers that should be installed in ${prefix}/include/
include_HEADERS = quadtree.h

##
## Help 
##

help:
	@cat $(top_srcdir)/docs/make-help.txt
