LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := lights.msm7k
LOCAL_PRELINK_MODULE := false

LOCAL_SRC_FILES += \
    vogue_lights.c

include $(BUILD_SHARED_LIBRARY)
