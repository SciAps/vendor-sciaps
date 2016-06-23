
define gradlebuild

$(1): JAVA_HOME = $(JAVA_HOME_JDK8)
$(1): PATH = $(JAVA_HOME)/bin:$(PATH)
$(1): $(call all-java-files-under,$(2))
	cd $(2) && \
	git submodule init && \
	git submodule foreach 'git remote prune origin' && \
	git submodule foreach 'git fetch' && \
	git submodule foreach 'git clean -d -x -f' && \
	git submodule update && \
	echo "Using jdk: $JAVA_HOME" && \
	./gradlew clean assemble

include $$(CLEAR_VARS)
LOCAL_MODULE := $(3)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := APPS
LOCAL_CERTIFICATE := PRESIGNED
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_SRC_FILES := $(1)
include $(BUILD_SYSTEM)/base_rules.mk
$$(LOCAL_BUILT_MODULE) : $$(LOCAL_SRC_FILES) | $$(ACP)
	$$(transform-prebuilt-to-target)

endef

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
	http://jenkins2.sciaps.local/jenkins/job/XRFAndroid/lastStableBuild/artifact/app/build/outputs/apk/app-release.apk, \
	$(LOCAL_PATH)/XRFAndroid.apk, \
	XRFAndroid \
	))

$(eval $(call gradlebuild,\
	$(LOCAL_PATH)/LIBZAlloyMatch/SciapsLIBS/build/outputs/apk/SciapsLIBS-libz500-hardware-release.apk, \
	$(LOCAL_PATH)/LIBZAlloyMatch, \
	LIBZAlloyMatch \
	))

$(eval $(call gradlebuild,\
	$(LOCAL_PATH)/Updater/app/build/outputs/apk/app-release.apk, \
	$(LOCAL_PATH)/Updater, \
	LIBZUpdater \
	))

$(eval $(call gradlebuild,\
	$(LOCAL_PATH)/LIBZHome/app/build/outputs/apk/app-libzArgon-release.apk, \
	$(LOCAL_PATH)/LIBZHome, \
	SciapsLIBZHomeZ500 \
	))

$(eval $(call downloadapk,\
	http://jenkins2.sciaps.local/jenkins/job/SciapsHome/lastStableBuild/artifact/app/build/outputs/apk/app-xrf-release.apk, \
	$(LOCAL_PATH)/SciapsHome-XRF.apk, \
	SciapsXRFHome \
	))

$(eval $(call gradlebuild,\
	$(LOCAL_PATH)/LIBZAlloySpec/spec/build/outputs/apk/spec-release.apk, \
	$(LOCAL_PATH)/LIBZAlloySpec, \
	LIBZAlloySpec \
	))

$(eval $(call gradlebuild,\
	$(LOCAL_PATH)/LIBZFactoryMode/app/build/outputs/apk/app-libz500-release.apk, \
	$(LOCAL_PATH)/LIBZFactoryMode, \
	LIBZFactoryMode \
	))
