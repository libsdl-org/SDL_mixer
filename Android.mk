LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := SDL_mixer

LOCAL_CFLAGS := -I$(LOCAL_PATH)/.. -I$(LOCAL_PATH)/../SDL/include \
	-DWAV_MUSIC -DOGG_MUSIC -DOGG_USE_TREMOR #-DMOD_MUSIC

LOCAL_SRC_FILES := $(notdir $(filter-out %/playmus.c %/playwave.c, $(wildcard $(LOCAL_PATH)/*.c)))

LOCAL_SHARED_LIBRARIES := SDL
LOCAL_STATIC_LIBRARIES := tremor

include $(BUILD_SHARED_LIBRARY)
