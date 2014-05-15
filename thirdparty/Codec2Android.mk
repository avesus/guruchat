LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    codec2/src/dump.c \
    codec2/src/lpc.c \
    codec2/src/nlp.c \
    codec2/src/postfilter.c \
    codec2/src/sine.c \
    codec2/src/codec2.c \
    codec2/src/fifo.c \
    codec2/src/fdmdv.c \
    codec2/src/kiss_fft.c \
    codec2/src/interp.c \
    codec2/src/lsp.c \
    codec2/src/phase.c \
    codec2/src/quantise.c \
    codec2/src/pack.c \
    codec2/src/codebook.c \
    codec2/src/codebookd.c \
    codec2/src/codebookvq.c \
    codec2/src/codebookjnd.c \
    codec2/src/codebookjvm.c \
    codec2/src/codebookvqanssi.c \
    codec2/src/codebookdt.c \
    codec2/src/codebookge.c \
    codec2/src/golay23.c

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/codec2/src
LOCAL_C_INCLUDES += $(LOCAL_PATH)/codec2/src

LOCAL_MODULE := codec2
LOCAL_MODULE_TAGS := optional

include $(BUILD_STATIC_LIBRARY)
