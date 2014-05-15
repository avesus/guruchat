# The ARMv7 is significanly faster due to the use of the hardware FPU
APP_ABI := armeabi           # armeabi-v7a
APP_PLATFORM := android-10
APP_OPTIM := debug

APP_STL := gnustl_static
APP_CPPFLAGS += -frtti
APP_CPPFLAGS += -fexceptions
APP_USE_CPP0X := true