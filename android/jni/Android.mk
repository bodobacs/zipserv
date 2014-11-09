LOCAL_PATH := $(call my-dir)

#unzip
include $(CLEAR_VARS)
LOCAL_MODULE := hello-jni
LOCAL_SRC_FILES := \
		$(LOCAL_PATH)/minizip/ioapi.c \
		$(LOCAL_PATH)/minizip/unzip.c \
		$(LOCAL_PATH)/mysrv.cpp

LOCAL_C_INCLUDES := $(LOCAL_PATH)/minizip/
LOCAL_LDLIBS := -lz
include $(BUILD_SHARED_LIBRARY)

