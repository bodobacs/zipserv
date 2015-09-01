LOCAL_PATH := $(call my-dir)
MYLIBSPATH := $(call my-dir)/../../libarchives

#unzip
include $(CLEAR_VARS)

LOCAL_MODULE := unzip
LOCAL_SRC_FILES := \
		$(MYLIBSPATH)/minizip/ioapi.c \
		$(MYLIBSPATH)/minizip/unzip.c

LOCAL_C_FLAGS := -g
LOCAL_C_INCLUDES := $(MYLIBSPATH)/minizip/
LOCAL_LDLIBS := -lz -llog
include $(BUILD_SHARED_LIBRARY)

#modchmlib
include $(CLEAR_VARS)

LOCAL_MODULE := modchmlib
LOCAL_SRC_FILES := \
		$(MYLIBSPATH)/modchmlib/chm_lib.c \
		$(MYLIBSPATH)/modchmlib/lzx.c

LOCAL_C_FLAGS := -g
LOCAL_C_INCLUDES := $(MYLIBSPATH)/modchmlib/
LOCAL_LDLIBS := -llog
include $(BUILD_SHARED_LIBRARY)

#archives
include $(CLEAR_VARS)

LOCAL_MODULE := jnizsrv
LOCAL_SRC_FILES := \
		$(MYLIBSPATH)/archives.cpp \
		$(LOCAL_PATH)/../../src/zsrv.cpp \
		$(LOCAL_PATH)/jnizsrv.cpp

LOCAL_C_FLAGS := -g
LOCAL_C_INCLUDES := $(MYLIBSPATH)/ \
					$(LOCAL_PATH)/../../src
LOCAL_LDLIBS := -llog
LOCAL_SHARED_LIBRARIES := modchmlib unzip
include $(BUILD_SHARED_LIBRARY)

