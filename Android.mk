LOCAL_PATH := $(call my-dir)

include $(LOCAL_PATH)/mkfs.mk
include $(LOCAL_PATH)/apps.mk

include $(call all-makefiles-under,$(LOCAL_PATH))
