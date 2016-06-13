ifeq ($(MTK_C2K_SUPPORT), yes)

ifeq ($(SU_CW_SUPPORT),yes)

LOCAL_PATH:= $(call my-dir)

#cwpd
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_MULTILIB := 32
LOCAL_PREBUILT_EXECUTABLES := cwpd
include $(BUILD_MULTI_PREBUILT)

#cwservice.apk
include $(CLEAR_VARS)
LOCAL_MODULE := cwservice
LOCAL_MULTILIB := 32
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(LOCAL_MODULE).apk
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_MODULE_CLASS := APPS
LOCAL_CERTIFICATE := platform
include $(BUILD_PREBUILT)

#ip-up-cwpd
include $(CLEAR_VARS)
LOCAL_MULTILIB := 32
LOCAL_MODULE := ip-up-cwpd
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/ppp
include $(BUILD_PREBUILT)

#su2_cw_license.txt
include $(CLEAR_VARS)
LOCAL_MULTILIB := 32
LOCAL_MODULE := su2_cw_license.txt
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)
include $(BUILD_PREBUILT)

PRODUCT_PROPERTY_OVERRIDES += \
	ro.config.su2.cw = true
endif

endif