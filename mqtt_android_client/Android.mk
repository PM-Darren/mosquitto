SKY_BUILD_PATH := $(call my-dir)

include $(call all-subdir-makefiles)
LOCAL_PATH := $(SKY_BUILD_PATH)
$(warning $(LOCAL_PATH))

#=========================
include $(CLEAR_VARS)
OBJS := src/transport.c \
        src/MQTTConnectClient.c \
        src/MQTTSerializePublish.c \
        src/MQTTPacket.c \
        src/MQTTSubscribeClient.c \
        src/MQTTDeserializePublish.c \
        src/MQTTConnectServer.c \
        src/MQTTSubscribeServer.c \
        src/MQTTUnsubscribeServer.c \
        src/MQTTUnsubscribeClient.c \
        src/tools.c \
        

INCLUDES := $(KERNEL_HEADERS) \
                $(LOCAL_PATH)/src 

LOCAL_SHARED_LIBRARIES := \
                libutils \
                libnetutils \
                libhardware_legacy \
                libcutils \
                libsocket_ipc \
                libdevconf \
                libdev_thread \
                libwifi \
                libmedia \
                libcrypto \
                libsky \
                libstlport
############################
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := eng
LOCAL_MODULE := pub
LOCAL_SRC_FILES := $(OBJS) \
                pub.c
LOCAL_C_INCLUDES := $(INCLUDES)
include $(BUILD_EXECUTABLE)
############################


############################
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := eng
LOCAL_MODULE := sub
LOCAL_SRC_FILES := $(OBJS) \
                sub.c

LOCAL_C_INCLUDES := $(INCLUDES)
include $(BUILD_EXECUTABLE)
############################




