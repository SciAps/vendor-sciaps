LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    screencast.cpp

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    libutils \
		libstagefright \
		libstagefright_foundation \
		libmedia \
    libbinder \
    libskia \
    libui \
    libgui

LOCAL_MODULE:= screencast

LOCAL_MODULE_TAGS := optional

LOCAL_CFLAGS += -Wall -Wunused -Wunreachable-code

LOCAL_C_INCLUDES += \
	external/skia/include/core \
	external/skia/include/effects \
	external/skia/include/images \
	external/skia/src/ports \
	external/skia/include/utils \
	frameworks/av/media/libstagefright \
	frameworks/av/media/libstagefright/include \
	$(TOP)/frameworks/native/include/media/openmax \
	external/jpeg

include $(BUILD_EXECUTABLE)
