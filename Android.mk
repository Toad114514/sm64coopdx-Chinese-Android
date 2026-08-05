LOCAL_PATH := $(realpath $(call my-dir))

# Check Arch
VALID_ARCH := 0
ifeq ($(TARGET_ARCH_ABI),x86_64)
  VALID_ARCH := 1
endif
ifeq ($(TARGET_ARCH_ABI),arm64-v8a)
  VALID_ARCH := 1
endif

ifeq ($(VALID_ARCH),0)
  $(error $(TARGET_ARCH_ABI) is not supported)
endif

# Lua
include $(CLEAR_VARS)
LOCAL_MODULE := lua5.3.5
LOCAL_SRC_FILES := $(LOCAL_PATH)/lib/lua/android/$(TARGET_ARCH_ABI)/liblua.a
include $(PREBUILT_STATIC_LIBRARY)

# Coopnet
include $(CLEAR_VARS)
LOCAL_MODULE := coopnet
LOCAL_SRC_FILES := $(LOCAL_PATH)/lib/coopnet/android/$(TARGET_ARCH_ABI)/libcoopnet.a
include $(PREBUILT_STATIC_LIBRARY)

# Libjuice
include $(CLEAR_VARS)
LOCAL_MODULE := libjuice
LOCAL_SRC_FILES := $(LOCAL_PATH)/lib/coopnet/android/$(TARGET_ARCH_ABI)/libjuice.a
include $(PREBUILT_STATIC_LIBRARY)

# SDL2
include $(CLEAR_VARS)
LOCAL_MODULE := sdl2
LOCAL_SRC_FILES := $(LOCAL_PATH)/lib/sdl2/android/$(TARGET_ARCH_ABI)/libSDL2.so
include $(PREBUILT_SHARED_LIBRARY)

# Curl
include $(CLEAR_VARS)
LOCAL_MODULE := curl
LOCAL_SRC_FILES := $(LOCAL_PATH)/lib/curl/android/$(TARGET_ARCH_ABI)/libcurl.so
include $(PREBUILT_SHARED_LIBRARY)

# --- MAIN MODULE ---
include $(CLEAR_VARS)
LOCAL_MODULE := main

VERSION ?= us
TOUCH_CONTROLS ?= 1

ifeq ($(VERSION),jp)
  VERSION_DEF := VERSION_JP
else ifeq ($(VERSION),us)
  VERSION_DEF := VERSION_US
else ifeq ($(VERSION),eu)
  VERSION_DEF := VERSION_EU
else ifeq ($(VERSION),sh)
  $(warning Building SH is experimental and is prone to breaking. Try at your own risk.)
  VERSION_DEF := VERSION_SH
else
  $(error unknown version "$(VERSION)")
endif

PC_BUILD_DIR := $(LOCAL_PATH)/build/$(VERSION)_pc

# 1. 源码目录声明
SRC_DIRS := src src/engine src/game src/audio src/menu src/buffers actors levels bin bin/$(VERSION) data assets sound \
            src/pc src/pc/gfx src/pc/audio src/pc/controller src/pc/fs src/pc/mods src/pc/dev \
            src/pc/network src/pc/network/packets src/pc/network/socket src/pc/network/coopnet \
            src/pc/utils src/pc/utils/miniz src/pc/djui src/pc/lua src/pc/lua/utils src/pc/mumble \
            include/android_execinfo src/pc/ui \
            src/pc/cimgui src/pc/cimgui/imgui src/pc/cimgui/imgui/backends src/pc/derect

SRC_DIRS_ABS := $(addprefix $(LOCAL_PATH)/,$(SRC_DIRS))

GODDARD_SRC_DIRS := src/goddard src/goddard/dynlists
GODDARD_SRC_DIRS_ABS := $(addprefix $(LOCAL_PATH)/,$(GODDARD_SRC_DIRS))

# 2. 搜寻 C 和 C++ 源文件
LEVEL_C_FILES := $(wildcard $(LOCAL_PATH)/levels/*/leveldata.c) $(wildcard $(LOCAL_PATH)/levels/*/script.c) $(wildcard $(LOCAL_PATH)/levels/*/geo.c)
C_FILES       := $(foreach dir,$(SRC_DIRS_ABS),$(wildcard $(dir)/*.c)) $(LEVEL_C_FILES)
CXX_FILES     := $(foreach dir,$(SRC_DIRS_ABS),$(wildcard $(dir)/*.cpp))

ULTRA_C_FILES := \
  alBnkfNew.c \
  guLookAtRef.c \
  guMtxF2L.c \
  guNormalize.c \
  guOrthoF.c \
  guPerspectiveF.c \
  guRotateF.c \
  guScaleF.c \
  guTranslateF.c

C_FILES := $(filter-out $(LOCAL_PATH)/src/game/main.c,$(C_FILES))
ULTRA_C_FILES := $(addprefix $(LOCAL_PATH)/lib/src/,$(ULTRA_C_FILES))
GODDARD_C_FILES := $(foreach dir,$(GODDARD_SRC_DIRS_ABS),$(wildcard $(dir)/*.c))

GENERATED_C_FILES := $(PC_BUILD_DIR)/assets/mario_anim_data.c $(PC_BUILD_DIR)/assets/demo_data.c \
  $(addprefix $(LOCAL_PATH)/bin/,$(addsuffix _skybox.c,$(notdir $(basename $(wildcard $(LOCAL_PATH)/textures/skyboxes/*.png)))))

# 3. 将绝对路径统一转换为 NDK 需要的相对路径
ALL_SRC_FILES := $(C_FILES) $(CXX_FILES) $(GENERATED_C_FILES) $(ULTRA_C_FILES) $(GODDARD_C_FILES)
LOCAL_SRC_FILES := $(ALL_SRC_FILES:$(LOCAL_PATH)/%=%)

# 4. 依赖库与编译参数设置
LOCAL_SHORT_COMMANDS := true
LOCAL_SHARED_LIBRARIES := sdl2 curl
LOCAL_STATIC_LIBRARIES := lua5.3.5 coopnet libjuice
LOCAL_LDLIBS := -lEGL -lGLESv3 -llog -lz -pthread -rdynamic -ldl -landroid

# 补全 ImGui 和 cimgui 的头文件包含路径
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include \
                    $(LOCAL_PATH)/include/android_execinfo \
                    $(LOCAL_PATH)/src \
                    $(LOCAL_PATH)/sound \
                    $(LOCAL_PATH)/lib/lua/include \
                    $(LOCAL_PATH)/lib/coopnet/include \
                    $(LOCAL_PATH)/lib/sdl2/include \
                    $(LOCAL_PATH)/lib/curl/include \
                    $(PC_BUILD_DIR) \
                    $(PC_BUILD_DIR)/include \
                    $(JNI_H_INCLUDE) \
                    $(LOCAL_PATH)/src/pc/cimgui \
                    $(LOCAL_PATH)/src/pc/cimgui/imgui \
                    $(LOCAL_PATH)/src/pc/cimgui/imgui/backends \
                    $(LOCAL_PATH)/src/pc/ui

LOCAL_CFLAGS := -DNON_MATCHING -DAVOID_UB -DTARGET_LINUX -DTARGET_ANDROID -DENABLE_OPENGL -DWIDESCREEN \
                -DF3DEX_GBI_2E -D_LANGUAGE_C -DNO_SEGMENTED_MEMORY -D$(VERSION_DEF) -DSTDC_HEADERS \
                -DDYNOS -DCOOP -DCOOPNET -DUSE_GLES -DTEXTURE_FIX -DBETTERCAMERA -DEXT_OPTIONS_MENU \
                -DIMMEDIATELOAD -DRAPI_GL=1 -DWAPI_SDL2=1 -DAAPI_SDL2=1 -DCAPI_SDL2 -DHAVE_SDL2=1 \
                -DIMGUI_IMPL_OPENGL_ES3 -DCIMGUI_DEFINE_ENUMS_AND_STRUCTS -O3 -w

LOCAL_CPPFLAGS += -std=c++17 -DIMGUI_IMPL_OPENGL_ES3 -DCIMGUI_DEFINE_ENUMS_AND_STRUCTS

ifeq ($(TOUCH_CONTROLS),1)
  LOCAL_CFLAGS += -DTOUCH_CONTROLS
endif

include $(BUILD_SHARED_LIBRARY)