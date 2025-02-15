LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := test
LOCAL_CPPFLAGS := -w -std=c++17

LOCAL_C_INCLUDES +=$(LOCAL_PATH)/include


LOCAL_SRC_FILES :=  $(LOCAL_PATH)/src/main.cpp


LOCAL_LDFLAGS += -lEGL -lGLESv2 -lGLESv3 -landroid -llog
include $(BUILD_EXECUTABLE)
