## Copyright (c) 2011 The WebM project authors. All Rights Reserved.
## 
## Use of this source code is governed by a BSD-style license
## that can be found in the LICENSE file in the root of the source
## tree. An additional intellectual property rights grant can be found
## in the file PATENTS.  All contributing project authors may
## be found in the AUTHORS file in the root of the source tree.
# This file automatically generated by configure. Do not edit!
SRC_PATH="./libvpx"
SRC_PATH_BARE=./libvpx
BUILD_PFX=
TOOLCHAIN=armv5te-android-gcc
ASM_CONVERSION=./libvpx/build/make/ads2gas.pl
GEN_VCPROJ=
MSVS_ARCH_DIR=

CC=/Users/avesus/Development/android-ndk-r9d/toolchains/arm-linux-androideabi-4.6/prebuilt/darwin-x86_64/bin/arm-linux-androideabi-gcc
CXX=/Users/avesus/Development/android-ndk-r9d/toolchains/arm-linux-androideabi-4.6/prebuilt/darwin-x86_64/bin/arm-linux-androideabi-g++
AR=/Users/avesus/Development/android-ndk-r9d/toolchains/arm-linux-androideabi-4.6/prebuilt/darwin-x86_64/bin/arm-linux-androideabi-ar
LD=/Users/avesus/Development/android-ndk-r9d/toolchains/arm-linux-androideabi-4.6/prebuilt/darwin-x86_64/bin/arm-linux-androideabi-gcc
AS=/Users/avesus/Development/android-ndk-r9d/toolchains/arm-linux-androideabi-4.6/prebuilt/darwin-x86_64/bin/arm-linux-androideabi-as
STRIP=/Users/avesus/Development/android-ndk-r9d/toolchains/arm-linux-androideabi-4.6/prebuilt/darwin-x86_64/bin/arm-linux-androideabi-strip
NM=/Users/avesus/Development/android-ndk-r9d/toolchains/arm-linux-androideabi-4.6/prebuilt/darwin-x86_64/bin/arm-linux-androideabi-nm

CFLAGS  =  --sysroot=/Users/avesus/Development/android-ndk-r9d/platforms/android-19/arch-arm -O3 -fPIC -Wall -Wdeclaration-after-statement -Wdisabled-optimization -Wpointer-arith -Wtype-limits -Wcast-qual -Wvla -Wimplicit-function-declaration -Wuninitialized -Wunused-variable -Wunused-but-set-variable -Wno-unused-function
CXXFLAGS  =  --sysroot=/Users/avesus/Development/android-ndk-r9d/platforms/android-19/arch-arm -O3 -fPIC -Wall -Wdisabled-optimization -Wpointer-arith -Wtype-limits -Wcast-qual -Wvla -Wuninitialized -Wunused-variable -Wunused-but-set-variable -Wno-unused-function
ARFLAGS = -rus$(if $(quiet),c,v)
LDFLAGS =  --sysroot=/Users/avesus/Development/android-ndk-r9d/platforms/android-19/arch-arm -Wl,--fix-cortex-a8
ASFLAGS =  --defsym ARCHITECTURE=5 -march=armv5te
extralibs = 
AS_SFX    = .s
EXE_SFX   = 
VCPROJ_SFX = 
RTCD_OPTIONS = 
fmt_deps = sed -e 's;^\([a-zA-Z0-9_]*\)\.o;${@:.d=.o} $@;'
ARCH_ARM=yes
HAVE_EDSP=yes
HAVE_VPX_PORTS=yes
HAVE_STDINT_H=yes
HAVE_PTHREAD_H=yes
HAVE_SYS_MMAN_H=yes
HAVE_UNISTD_H=yes
CONFIG_USE_X86INC=yes
CONFIG_GCC=yes
CONFIG_PIC=yes
CONFIG_POSTPROC=yes
CONFIG_MULTITHREAD=yes
CONFIG_VP8_ENCODER=yes
CONFIG_VP8_DECODER=yes
CONFIG_VP8=yes
CONFIG_ENCODERS=yes
CONFIG_DECODERS=yes
CONFIG_SPATIAL_RESAMPLING=yes
CONFIG_REALTIME_ONLY=yes
CONFIG_ERROR_CONCEALMENT=yes
CONFIG_STATIC=yes
CONFIG_OS_SUPPORT=yes
CONFIG_TEMPORAL_DENOISING=yes
HAVE_GNU_STRIP=yes