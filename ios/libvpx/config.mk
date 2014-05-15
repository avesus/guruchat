## Copyright (c) 2011 The WebM project authors. All Rights Reserved.
## 
## Use of this source code is governed by a BSD-style license
## that can be found in the LICENSE file in the root of the source
## tree. An additional intellectual property rights grant can be found
## in the file PATENTS.  All contributing project authors may
## be found in the AUTHORS file in the root of the source tree.
# This file automatically generated by configure. Do not edit!
TOOLCHAIN := armv7-darwin-gcc
ALL_TARGETS += libs

PREFIX=/usr/local
ifeq ($(MAKECMDGOALS),dist)
DIST_DIR?=vpx-vp8-nodocs-armv7-darwin-v1.3.0-2663-g0d27c74
else
DIST_DIR?=$(DESTDIR)/usr/local
endif
LIBSUBDIR=lib

VERSION_STRING=v1.3.0-2663-g0d27c74

VERSION_MAJOR=1
VERSION_MINOR=3
VERSION_PATCH=0

CONFIGURE_ARGS=--target=armv7-darwin-gcc --disable-examples --disable-vp9 --disable-docs --disable-install-libs --disable-install-bins --enable-postproc --enable-realtime-only --enable-error-concealment --disable-webm-io
CONFIGURE_ARGS?=--target=armv7-darwin-gcc --disable-examples --disable-vp9 --disable-docs --disable-install-libs --disable-install-bins --enable-postproc --enable-realtime-only --enable-error-concealment --disable-webm-io
