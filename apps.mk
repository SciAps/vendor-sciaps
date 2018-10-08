define downloadapk

.PHONY: $(2)

$(2):
	wget -O $(2) $(1)

include $$(CLEAR_VARS)
LOCAL_MODULE := $(3)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := APPS
LOCAL_CERTIFICATE := PRESIGNED
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_SRC_FILES := $(2)
include $(BUILD_SYSTEM)/base_rules.mk
$$(LOCAL_BUILT_MODULE) : $$(LOCAL_SRC_FILES) | $$(ACP)
	$$(transform-prebuilt-to-target)

endef

LOCAL_PATH := $(call my-dir)

$(eval $(call downloadapk,\
	https://s3.us-east-2.amazonaws.com/sciaps-firmware-dependencies/xrf_v3.2.apk, \
	$(LOCAL_PATH)/XRFAndroid.apk, \
	XRFAndroid \
	))

$(eval $(call downloadapk,\
	https://s3.us-east-2.amazonaws.com/sciaps-firmware-dependencies/SciapsLIBS-release.apk, \
	$(LOCAL_PATH)/LIBZAlloyMatch.apk, \
	LIBZAlloyMatch \
	))

$(eval $(call downloadapk,\
	https://s3.us-east-2.amazonaws.com/sciaps-firmware-dependencies/updater-release.apk, \
	$(LOCAL_PATH)/Updater.apk, \
	Updater \
))

$(eval $(call downloadapk,\
	https://s3.us-east-2.amazonaws.com/sciaps-firmware-dependencies/libz-home-release.apk, \
	$(LOCAL_PATH)/SciapsLIBZHome.apk, \
	SciapsLIBZHome \
	))

$(eval $(call downloadapk,\
	https://s3.us-east-2.amazonaws.com/sciaps-firmware-dependencies/xrf-home-release.apk, \
	$(LOCAL_PATH)/SciapsHome-XRF.apk, \
	SciapsXRFHome \
	))

$(eval $(call downloadapk,\
	https://s3.us-east-2.amazonaws.com/sciaps-firmware-dependencies/libz-factory-mode-release.apk, \
	$(LOCAL_PATH)/LIBZFactoryMode.apk, \
	LIBZFactoryMode \
))

$(eval $(call downloadapk,\
	https://s3.us-east-2.amazonaws.com/sciaps-firmware-dependencies/ZebraPrint-release.apk, \
	$(LOCAL_PATH)/ZebraPrint.apk, \
	ZebraPrint \
	))

$(eval $(call downloadapk,\
	https://s3.us-east-2.amazonaws.com/sciaps-firmware-dependencies/sciaps_alogcat_v1.0.apk, \
	$(LOCAL_PATH)/aLogCat.apk, \
	aLogCat \
	))
