SET GYP_DEFINES=target_arch=ia32
call python gyp_libyuv -fninja -G msvs_version=2008 --depth=. libyuv.gyp
ninja -j7 -C out\Release
ninja -j7 -C out\Debug