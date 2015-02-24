LOCAL_PATH := $(call my-dir)

#unzip
include $(CLEAR_VARS)
LOCAL_MODULE := jnizsrv
LOCAL_SRC_FILES := \
		$(LOCAL_PATH)/minizip/ioapi.c \
		$(LOCAL_PATH)/minizip/unzip.c \
		$(LOCAL_PATH)/zsrv.cpp \
		$(LOCAL_PATH)/jnizsrv.cpp

LOCAL_C_FLAGS := -g
LOCAL_C_INCLUDES := $(LOCAL_PATH) $(LOCAL_PATH)/minizip/
LOCAL_LDLIBS := -lz -llog
include $(BUILD_SHARED_LIBRARY)

