## Copyright (c) 2011 The WebM project authors. All Rights Reserved.
## 
## Use of this source code is governed by a BSD-style license
## that can be found in the LICENSE file in the root of the source
## tree. An additional intellectual property rights grant can be found
## in the file PATENTS.  All contributing project authors may
## be found in the AUTHORS file in the root of the source tree.
# This file automatically generated by configure. Do not edit!
TOOLCHAIN := x86-win32-vs9
ALL_TARGETS += libs
ALL_TARGETS += solution

PREFIX=/usr/local
ifeq ($(MAKECMDGOALS),dist)
DIST_DIR?=vpx-vp8-nodocs-x86-win32md-vs9-v1.3.0-2663-g0d27c74
else
DIST_DIR?=$(DESTDIR)/usr/local
endif
LIBSUBDIR=lib

VERSION_STRING=v1.3.0-2663-g0d27c74

VERSION_MAJOR=1
VERSION_MINOR=3
VERSION_PATCH=0

CONFIGURE_ARGS=--target=x86-win32-vs9 --disable-examples --disable-vp9 --disable-docs --disable-install-libs --disable-install-bins --enable-postproc --enable-realtime-only --enable-error-concealment --disable-webm-io --enable-runtime-cpu-detect
CONFIGURE_ARGS?=--target=x86-win32-vs9 --disable-examples --disable-vp9 --disable-docs --disable-install-libs --disable-install-bins --enable-postproc --enable-realtime-only --enable-error-concealment --disable-webm-io --enable-runtime-cpu-detect
