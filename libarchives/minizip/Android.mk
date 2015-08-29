LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_C_INCLUDES :=	$(LOCAL_PATH)
LOCAL_MODULE := unzip
LOCAL_SRC_FILES := ioapi.c unzip.c
 

include $(BUILD_STATIC_LIBRARY)


#$(warning $(LOCAL_MODULE_FILENAME)) 
