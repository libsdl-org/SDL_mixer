LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := SDL2_mixer

# Enable this if you want to support loading MOD music via mikmod
# The library path should be a relative path to this directory.
SUPPORT_MOD := true
MOD_LIBRARY_PATH := external/libmikmod-3.1.12

# Enable this if you want to support loading MP3 music via SMPEG
# The library path should be a relative path to this directory.
SUPPORT_MP3 := true
MP3_LIBRARY_PATH := external/smpeg2-2.0.0

# Enable this if you want to support loading OGG Vorbis music via Tremor
# The library path should be a relative path to this directory.
SUPPORT_OGG := true
OGG_LIBRARY_PATH := external/libogg-1.3.1
VORBIS_LIBRARY_PATH := external/libvorbisidec-1.2.1


LOCAL_C_INCLUDES := $(NDK_PROJECT_PATH)/jni/SDL/include
LOCAL_CFLAGS := -DWAV_MUSIC

LOCAL_SRC_FILES := $(notdir $(filter-out %/playmus.c %/playwave.c, $(wildcard $(LOCAL_PATH)/*.c)))

LOCAL_LDLIBS :=
LOCAL_STATIC_LIBRARIES :=
LOCAL_SHARED_LIBRARIES := SDL2

ifeq ($(SUPPORT_MOD),true)
    LOCAL_C_INCLUDES += $(LOCAL_PATH)/$(MOD_LIBRARY_PATH)/include
    LOCAL_CFLAGS += -DMOD_MUSIC
    LOCAL_SHARED_LIBRARIES += mikmod
endif

ifeq ($(SUPPORT_MP3),true)
    LOCAL_C_INCLUDES += $(LOCAL_PATH)/$(MP3_LIBRARY_PATH)
    LOCAL_CFLAGS += -DMP3_MUSIC
    LOCAL_SHARED_LIBRARIES += smpeg2
endif

ifeq ($(SUPPORT_OGG),true)
    LOCAL_C_INCLUDES += $(LOCAL_PATH)/$(OGG_LIBRARY_PATH)/include $(LOCAL_PATH)/$(VORBIS_LIBRARY_PATH)
    LOCAL_CFLAGS += -DOGG_MUSIC -DOGG_USE_TREMOR
    LOCAL_SRC_FILES += \
        $(VORBIS_LIBRARY_PATH)/mdct.c \
        $(VORBIS_LIBRARY_PATH)/block.c \
        $(VORBIS_LIBRARY_PATH)/window.c \
        $(VORBIS_LIBRARY_PATH)/synthesis.c \
        $(VORBIS_LIBRARY_PATH)/info.c \
        $(VORBIS_LIBRARY_PATH)/floor1.c \
        $(VORBIS_LIBRARY_PATH)/floor0.c \
        $(VORBIS_LIBRARY_PATH)/vorbisfile.c \
        $(VORBIS_LIBRARY_PATH)/res012.c \
        $(VORBIS_LIBRARY_PATH)/mapping0.c \
        $(VORBIS_LIBRARY_PATH)/registry.c \
        $(VORBIS_LIBRARY_PATH)/codebook.c \
        $(VORBIS_LIBRARY_PATH)/sharedbook.c \
        $(OGG_LIBRARY_PATH)/src/framing.c \
        $(OGG_LIBRARY_PATH)/src/bitwise.c
endif

include $(BUILD_SHARED_LIBRARY)
