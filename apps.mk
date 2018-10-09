define useprebuiltapk

include $$(CLEAR_VARS)
LOCAL_MODULE := $(2)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := APPS
LOCAL_CERTIFICATE := PRESIGNED
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_SRC_FILES := $(1)
include $(BUILD_SYSTEM)/base_rules.mk
$$(LOCAL_BUILT_MODULE) : $$(LOCAL_SRC_FILES) | $$(ACP)
	$$(transform-prebuilt-to-target)

endef

LOCAL_PATH := $(call my-dir)

$(eval $(call useprebuiltapk,\
	$(LOCAL_PATH)/XRFAndroid.apk, \
	XRFAndroid \
	))

$(eval $(call useprebuiltapk,\
	$(LOCAL_PATH)/LIBZAlloyMatch.apk, \
	LIBZAlloyMatch \
	))

$(eval $(call useprebuiltapk,\
	$(LOCAL_PATH)/Updater.apk, \
	Updater \
))

$(eval $(call useprebuiltapk,\
	$(LOCAL_PATH)/SciapsLIBZHome.apk, \
	SciapsLIBZHome \
	))

$(eval $(call useprebuiltapk,\
	$(LOCAL_PATH)/SciapsHome-XRF.apk, \
	SciapsXRFHome \
	))

$(eval $(call useprebuiltapk,\
	$(LOCAL_PATH)/LIBZFactoryMode.apk, \
	LIBZFactoryMode \
))

$(eval $(call useprebuiltapk,\
	$(LOCAL_PATH)/ZebraPrint.apk, \
	ZebraPrint \
	))

$(eval $(call useprebuiltapk,\
	$(LOCAL_PATH)/aLogCat.apk, \
	aLogCat \
	))

$(eval $(call useprebuiltapk,\
	$(LOCAL_PATH)/LIBSSettings.apk, \
	LIBSSettings.apk \
	))
