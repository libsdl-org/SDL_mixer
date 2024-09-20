# Save the local path
SDL_MIXER_LOCAL_PATH := $(call my-dir)

# Enable this if you want to support loading WAV music
SUPPORT_WAV ?= true

# Enable this if you want to support loading FLAC music via dr_flac
SUPPORT_FLAC_DRFLAC ?= true

# Enable this if you want to support loading FLAC music with libFLAC
SUPPORT_FLAC_LIBFLAC ?= false
FLAC_LIBRARY_PATH := external/flac

# Enable this if you want to support loading OGG Vorbis music via stb_vorbis
SUPPORT_OGG_STB ?= true

# Enable this if you want to support loading OGG Vorbis music via Tremor
SUPPORT_OGG ?= false
OGG_LIBRARY_PATH := external/ogg
VORBIS_LIBRARY_PATH := external/tremor

# Enable this if you want to support loading MP3 music via MINIMP3
SUPPORT_MP3_MINIMP3 ?= true

# Enable this if you want to support loading MP3 music via MPG123
SUPPORT_MP3_MPG123 ?= false
MPG123_LIBRARY_PATH := external/mpg123

# Enable this if you want to support loading WavPack music via libwavpack
SUPPORT_WAVPACK ?= true
WAVPACK_LIBRARY_PATH := external/wavpack

# Enable this if you want to support loading music via libgme
SUPPORT_GME ?= true
GME_LIBRARY_PATH := external/libgme

# Enable this if you want to support loading MOD music via XMP-lite
SUPPORT_MOD_XMP ?= false
XMP_LIBRARY_PATH := external/libxmp

# Enable this if you want to support TiMidity
SUPPORT_MID_TIMIDITY ?= false
TIMIDITY_LIBRARY_PATH := src/codecs/timidity


# Build the library
ifeq ($(SUPPORT_FLAC_LIBFLAC),true)
    include $(SDL_MIXER_LOCAL_PATH)/$(FLAC_LIBRARY_PATH)/Android.mk
endif

# Build the library
ifeq ($(SUPPORT_OGG),true)
    include $(SDL_MIXER_LOCAL_PATH)/$(OGG_LIBRARY_PATH)/Android.mk
    include $(SDL_MIXER_LOCAL_PATH)/$(VORBIS_LIBRARY_PATH)/Android.mk
endif

# Build the library
ifeq ($(SUPPORT_MP3_MPG123),true)
    include $(SDL_MIXER_LOCAL_PATH)/$(MPG123_LIBRARY_PATH)/Android.mk
endif

# Build the library
ifeq ($(SUPPORT_WAVPACK),true)
    include $(SDL_MIXER_LOCAL_PATH)/$(WAVPACK_LIBRARY_PATH)/Android.mk
endif

# Build the library
ifeq ($(SUPPORT_GME),true)
    include $(SDL_MIXER_LOCAL_PATH)/$(GME_LIBRARY_PATH)/Android.mk
endif

# Build the library
ifeq ($(SUPPORT_MOD_XMP),true)
    include $(SDL_MIXER_LOCAL_PATH)/$(XMP_LIBRARY_PATH)/Android.mk
endif

# Build the library
ifeq ($(SUPPORT_MID_TIMIDITY),true)
    include $(SDL_MIXER_LOCAL_PATH)/$(TIMIDITY_LIBRARY_PATH)/Android.mk
endif

# Restore local path
LOCAL_PATH := $(SDL_MIXER_LOCAL_PATH)

include $(CLEAR_VARS)

LOCAL_MODULE := SDL3_mixer

LOCAL_C_INCLUDES :=                                     \
    $(LOCAL_PATH)/include                               \
    $(LOCAL_PATH)/src/                                  \
    $(LOCAL_PATH)/src/codecs                            \


LOCAL_SRC_FILES :=                                      \
    $(subst $(LOCAL_PATH)/,,                            \
    $(wildcard $(LOCAL_PATH)/src/*.c)                   \
    $(wildcard $(LOCAL_PATH)/src/codecs/*.c)            \
    )

LOCAL_CFLAGS :=
LOCAL_LDLIBS :=
LOCAL_LDFLAGS := -Wl,--no-undefined -Wl,--version-script=$(LOCAL_PATH)/src/SDL_mixer.sym
LOCAL_STATIC_LIBRARIES :=
LOCAL_SHARED_LIBRARIES := SDL3

ifeq ($(SUPPORT_WAV),true)
    LOCAL_CFLAGS += -DMUSIC_WAV
endif

ifeq ($(SUPPORT_FLAC_DRFLAC),true)
    LOCAL_CFLAGS += -DMUSIC_FLAC_DRFLAC
endif

ifeq ($(SUPPORT_FLAC_LIBFLAC),true)
    LOCAL_C_INCLUDES += $(LOCAL_PATH)/$(FLAC_LIBRARY_PATH)/include
    LOCAL_CFLAGS += -DMUSIC_FLAC_LIBFLAC
    LOCAL_STATIC_LIBRARIES += libFLAC
endif

ifeq ($(SUPPORT_OGG_STB),true)
    LOCAL_CFLAGS += -DMUSIC_OGG -DOGG_USE_STB
endif

ifeq ($(SUPPORT_OGG),true)
    LOCAL_C_INCLUDES += $(LOCAL_PATH)/$(OGG_LIBRARY_PATH)/include
    LOCAL_C_INCLUDES += $(LOCAL_PATH)/$(VORBIS_LIBRARY_PATH)
    LOCAL_CFLAGS += -DMUSIC_OGG -DOGG_USE_TREMOR -DOGG_HEADER="<ivorbisfile.h>"
    LOCAL_STATIC_LIBRARIES += ogg vorbisidec
endif

ifeq ($(SUPPORT_MP3_MINIMP3),true)
    LOCAL_CFLAGS += -DMUSIC_MP3_MINIMP3
endif

# This needs to be a shared library to comply with the LGPL license
ifeq ($(SUPPORT_MP3_MPG123),true)
    LOCAL_C_INCLUDES += $(LOCAL_PATH)/$(MPG123_LIBRARY_PATH)/android
    LOCAL_CFLAGS += -DMUSIC_MP3_MPG123
    LOCAL_SHARED_LIBRARIES += mpg123
endif

ifeq ($(SUPPORT_WAVPACK),true)
    LOCAL_CFLAGS += -DMUSIC_WAVPACK -DMUSIC_WAVPACK_DSD -DWAVPACK_HEADER=\"../external/wavpack/include/wavpack.h\"
    LOCAL_STATIC_LIBRARIES += wavpack
endif

ifeq ($(SUPPORT_GME),true)
    LOCAL_CFLAGS += -DMUSIC_GME
    LOCAL_C_INCLUDES += $(LOCAL_PATH)/$(GME_LIBRARY_PATH)
    LOCAL_STATIC_LIBRARIES += libgme
endif

ifeq ($(SUPPORT_MOD_XMP),true)
    LOCAL_CFLAGS += -DMUSIC_MOD_XMP -DLIBXMP_HEADER=\"../external/libxmp/include/xmp.h\"
    LOCAL_STATIC_LIBRARIES += xmp
endif

ifeq ($(SUPPORT_MID_TIMIDITY),true)
    LOCAL_C_INCLUDES += $(LOCAL_PATH)/$(TIMIDITY_LIBRARY_PATH)
    LOCAL_CFLAGS += -DMUSIC_MID_TIMIDITY
    LOCAL_STATIC_LIBRARIES += timidity
endif

LOCAL_EXPORT_C_INCLUDES += $(LOCAL_PATH)/include

include $(BUILD_SHARED_LIBRARY)

###########################
#
# SDL3_mixer static library
#
###########################

LOCAL_MODULE := SDL3_mixer_static

LOCAL_MODULE_FILENAME := libSDL3_mixer

LOCAL_LDLIBS :=
LOCAL_EXPORT_LDLIBS :=

include $(BUILD_STATIC_LIBRARY)

