#include "module.h"
#include "config.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define KEYBIND_CONFIG_PATH "/storage/emulated/0/com.toad1145.derectcoopdxcn/config.ini"

static void parse_shortcut(const char* shortcut, int* out_key, int* out_mods);

Module g_modules[MAX_MODULES];
int g_module_count = 0;

static const char* g_category_names[] = {
    "Core", "Mario", "Render", "Control", "Coopnet/Web", "Misc", "Demo"
};

const char* Module_GetCategoryName(ModuleCategory cat) {
    if (cat >= 0 && cat < CAT_COUNT) {
        return g_category_names[cat];
    }
    return "Unknown";
}

// 注册新模块的公共方法
void Module_Register(const char* name, ModuleCategory category, bool default_enabled, const char* shortcut, ImVec4 color, ModuleCallBack on_enable, ModuleCallBack on_disable, ModuleCallBack on_loop) {
    if (g_module_count >= MAX_MODULES) return;

    Module* mod = &g_modules[g_module_count];
    memset(mod, 0, sizeof(*mod));
    mod->name = name;
    mod->category = category;
    mod->enabled = default_enabled;
    mod->color = color;
    mod->on_enable = on_enable;
    mod->on_disable = on_disable;
    mod->on_loop = on_loop;

    if (shortcut) {
        snprintf(mod->shortcut, sizeof(mod->shortcut), "%s", shortcut);
    }
    parse_shortcut(shortcut, &mod->bind_key, &mod->bind_mods);

    g_module_count++;
    
    printf("[Derect] Register Module %s \n", name);
}

// 设置/更新模块的快捷键文本并重新解析绑定 (供配置加载与运行时绑定使用)
void Module_SetShortcut(Module* mod, const char* shortcut) {
    if (!mod) return;

    if (shortcut) {
        snprintf(mod->shortcut, sizeof(mod->shortcut), "%s", shortcut);
    } else {
        mod->shortcut[0] = '\0';
    }
    parse_shortcut(shortcut, &mod->bind_key, &mod->bind_mods);
}

void Module_HookRender(const char* name, ModuleCallBack on_render) {
    int count = sizeof(g_modules) / sizeof(g_modules[0]);
    for (int i = 0; i < count; i++) {
        if (name == g_modules[i].name) {
            g_modules[i].on_render = on_render;
            printf("[Derect] Register Module %s on_render hook\n", name);
        }
    }
}

void Module_HookConfig(const char* name, ModuleCallBack config) {
    int count = sizeof(g_modules) / sizeof(g_modules[0]);
    for (int i = 0; i < count; i++) {
        if (name == g_modules[i].name) {
            g_modules[i].config = config;
            printf("[Derect] Register Module %s config hook\n", name);
        }
    }
}


// Module text
static void printf_test_on_enable(){
    printf("[Test] test enable");
}

static void printf_test_on_disable(){
    printf("[Test] Bye");
}

static void printf_test_good_work(){
    printf("[Test] HolyMoly");
}

static bool demo_close_btn = true;

static void demo_loop(){
    igShowDemoWindow(&demo_close_btn);
}

static void demo_close(){
    demo_close_btn = false;
}

static void demo_show() {
    demo_close_btn = true;
}

/// extern Init
extern void module_mario_state(void);
extern void module_esp(void);
extern void module_bgblur(void);
extern void module_network(void);
extern void module_autobowser(void);

// 统一在此处注册所有游戏/应用模块
void Module_InitRegistry(void) {
    g_module_count = 0;

    // 预设常用颜色
    ImVec4 green = (ImVec4){0.10f, 0.85f, 0.30f, 1.0f};
    ImVec4 cyan  = (ImVec4){0.00f, 0.75f, 0.70f, 1.0f};
    
    module_mario_state();
    module_esp();
    module_bgblur();
    module_network();
    module_autobowser();

    // ==================== 1. Render 分类 ====================
    Module_Register("Arraylist",       CAT_RENDER, true,  NULL,       green, NULL, NULL, NULL);
    Module_Register("AudioVisualizer", CAT_RENDER, true,  NULL,       green, NULL, NULL, NULL);
    Module_Register("BlackCapture",    CAT_RENDER, false, "LCtrl+NO", green, NULL, NULL, NULL);
    Module_Register("Keystrokes",      CAT_RENDER, false, "LAlt+K",   green, NULL, NULL, NULL);

    // ==================== 2. Web 分类 ====================
    Module_Register("AntiRickroll",    CAT_WEB,    true,  NULL,       MOD_COLOR_RED, NULL, NULL, NULL);

    // ==================== 3. Misc 分类 ====================
    Module_Register("AutoSpeak",       CAT_MISC,   true,  NULL,       MOD_COLOR_BLUE, NULL, NULL, NULL);
    Module_Register("MemeTrigger",     CAT_MISC,   true,  NULL,       green, NULL, NULL, NULL);
    Module_Register("Volume",          CAT_MISC,   true,  "RAlt+Del", green, NULL, NULL, NULL);
    
    // Demo Sections
    Module_Register("Notification",    CAT_DEMO,   false, NULL,       green, NULL, NULL, NULL);
    Module_Register("Function Printf", CAT_DEMO,   false, NULL,       green, printf_test_on_enable, printf_test_on_disable, printf_test_good_work);
    Module_Register("这是中文合成效果",   CAT_DEMO,   false, NULL,       cyan,  NULL, NULL, NULL);
    Module_Register("ImGUI Demo",      CAT_DEMO,   false, NULL,       green, demo_show, demo_close, demo_loop);
    Module_HookRender("ImGUI Demo",    demo_loop);
}
    

// 获取已注册模块列表指针
Module* Module_GetAll(int* out_count) {
    if (out_count) *out_count = g_module_count;
    return g_modules;
}

// mods
Module* Module_Find(const char* name) {
    if (!name) return NULL;
    
    for (int i = 0; i < g_module_count; i++) {
        if (g_modules[i].name && strcmp(g_modules[i].name, name) == 0) {
            return &g_modules[i];
        }
    }

    return NULL;
}

// =========================================================================
// 快捷键解析与绑定
// =========================================================================

static bool ieq(const char* a, const char* b) {
    while (*a && *b) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) return false;
        a++; b++;
    }
    return *a == *b;
}

// 修饰键名 -> ImGuiMod_* (左右不分，统一为通用修饰位)
static int parse_modifier(const char* s) {
    if (ieq(s, "Ctrl")   || ieq(s, "LCtrl") || ieq(s, "RCtrl")
        || ieq(s, "LControl") || ieq(s, "RControl")) return ImGuiMod_Ctrl;
    if (ieq(s, "Shift")  || ieq(s, "LShift") || ieq(s, "RShift")) return ImGuiMod_Shift;
    if (ieq(s, "Alt")    || ieq(s, "LAlt")   || ieq(s, "RAlt"))   return ImGuiMod_Alt;
    if (ieq(s, "Super")  || ieq(s, "LSuper") || ieq(s, "RSuper")
        || ieq(s, "Win") || ieq(s, "LWin")   || ieq(s, "RWin")
        || ieq(s, "Cmd") || ieq(s, "LCmd")   || ieq(s, "RCmd"))   return ImGuiMod_Super;
    return 0;
}

// 按键名 -> ImGuiKey (识别失败返回 ImGuiKey_None)
static int parse_key(const char* s) {
    if (!s || !*s) return ImGuiKey_None;

    if (strlen(s) == 1) {
        char c = toupper((unsigned char)s[0]);
        if (c >= 'A' && c <= 'Z') return ImGuiKey_A + (c - 'A');
        if (c >= '0' && c <= '9') return ImGuiKey_0 + (c - '0');
    }

    if ((s[0] == 'F' || s[0] == 'f') && isdigit((unsigned char)s[1])) {
        int n = atoi(s + 1);
        if (n >= 1 && n <= 24) return ImGuiKey_F1 + (n - 1);
    }

    if (ieq(s, "Tab"))          return ImGuiKey_Tab;
    if (ieq(s, "Space"))        return ImGuiKey_Space;
    if (ieq(s, "Enter"))        return ImGuiKey_Enter;
    if (ieq(s, "Esc") || ieq(s, "Escape")) return ImGuiKey_Escape;
    if (ieq(s, "Backspace"))    return ImGuiKey_Backspace;
    if (ieq(s, "Del") || ieq(s, "Delete")) return ImGuiKey_Delete;
    if (ieq(s, "Insert"))       return ImGuiKey_Insert;
    if (ieq(s, "Home"))         return ImGuiKey_Home;
    if (ieq(s, "End"))          return ImGuiKey_End;
    if (ieq(s, "PgUp") || ieq(s, "PageUp"))     return ImGuiKey_PageUp;
    if (ieq(s, "PgDn") || ieq(s, "PageDown"))   return ImGuiKey_PageDown;
    if (ieq(s, "Up")    || ieq(s, "ArrowUp"))    return ImGuiKey_UpArrow;
    if (ieq(s, "Down")  || ieq(s, "ArrowDown"))  return ImGuiKey_DownArrow;
    if (ieq(s, "Left")  || ieq(s, "ArrowLeft"))  return ImGuiKey_LeftArrow;
    if (ieq(s, "Right") || ieq(s, "ArrowRight")) return ImGuiKey_RightArrow;

    return ImGuiKey_None;
}

// 解析 "Modifier+Key" 字符串 -> bind_key / bind_mods
// 修饰键无效或触发键无法识别时整个绑定视为无效
static void parse_shortcut(const char* shortcut, int* out_key, int* out_mods) {
    int key = ImGuiKey_None;
    int mods = 0;

    if (shortcut && *shortcut) {
        char buf[64];
        snprintf(buf, sizeof(buf), "%s", shortcut);

        char* last = NULL;
        char* tok = strtok(buf, "+");
        while (tok) {
            while (*tok == ' ') tok++;
            if (last) {
                int m = parse_modifier(last);
                if (!m) { mods = 0; key = ImGuiKey_None; break; }
                mods |= m;
            }
            last = tok;
            tok = strtok(NULL, "+");
        }
        if (last && parse_key(last) != ImGuiKey_None) {
            key = parse_key(last);
        } else {
            key = ImGuiKey_None;
            mods = 0;
        }
    }

    if (out_key)  *out_key  = key;
    if (out_mods) *out_mods = mods;
}

// ImGuiKey -> 显示名 (简单按键)
static const char* key_display_name(int key) {
    switch (key) {
        case ImGuiKey_Tab:          return "Tab";
        case ImGuiKey_Space:        return "Space";
        case ImGuiKey_Enter:        return "Enter";
        case ImGuiKey_Escape:       return "Esc";
        case ImGuiKey_Backspace:    return "Backspace";
        case ImGuiKey_Delete:       return "Del";
        case ImGuiKey_Insert:       return "Insert";
        case ImGuiKey_Home:         return "Home";
        case ImGuiKey_End:          return "End";
        case ImGuiKey_PageUp:       return "PgUp";
        case ImGuiKey_PageDown:     return "PgDn";
        case ImGuiKey_UpArrow:      return "Up";
        case ImGuiKey_DownArrow:    return "Down";
        case ImGuiKey_LeftArrow:    return "Left";
        case ImGuiKey_RightArrow:   return "Right";
        default:                    return NULL;
    }
}

// 把 bind_key/bind_mods 格式化为 "Ctrl+Alt+K" 形式的文本
void Module_FormatShortcut(const Module* mod, char* buf, size_t buf_size) {
    if (!buf || buf_size == 0) return;
    buf[0] = '\0';

    if (!mod || mod->bind_key == ImGuiKey_None) {
        snprintf(buf, buf_size, "None");
        return;
    }

    const char* mod_names[4] = { "Ctrl", "Shift", "Alt", "Win" };
    int mod_bits[4] = { ImGuiMod_Ctrl, ImGuiMod_Shift, ImGuiMod_Alt, ImGuiMod_Super };

    size_t used = 0;
    for (int i = 0; i < 4; i++) {
        if (mod->bind_mods & mod_bits[i]) {
            int n = snprintf(buf + used, buf_size - used, "%s%s", used ? "+" : "", mod_names[i]);
            if (n > 0) used += (size_t)n;
            if (used >= buf_size) return;
        }
    }

    char kbuf[16];
    const char* kname = key_display_name(mod->bind_key);
    if (!kname) {
        if (mod->bind_key >= ImGuiKey_F1 && mod->bind_key <= ImGuiKey_F24)
            snprintf(kbuf, sizeof(kbuf), "F%d", mod->bind_key - ImGuiKey_F1 + 1);
        else if (mod->bind_key >= ImGuiKey_0 && mod->bind_key <= ImGuiKey_9)
            snprintf(kbuf, sizeof(kbuf), "%d", mod->bind_key - ImGuiKey_0);
        else if (mod->bind_key >= ImGuiKey_A && mod->bind_key <= ImGuiKey_Z)
            snprintf(kbuf, sizeof(kbuf), "%c", 'A' + (mod->bind_key - ImGuiKey_A));
        else
            snprintf(kbuf, sizeof(kbuf), "?");
        kname = kbuf;
    }
    snprintf(buf + used, buf_size - used, "%s%s", used ? "+" : "", kname);
}

// 切换模块开关 (点击/快捷键共用)
void Module_Toggle(Module* mod) {
    if (!mod) return;

    mod->enabled = !mod->enabled;
    if (mod->enabled) {
        if (mod->on_enable) mod->on_enable();
    } else {
        if (mod->on_disable) mod->on_disable();
    }

    Config_Save(KEYBIND_CONFIG_PATH);
}

// =========================================================================
// 快捷键触发 & 录制
// =========================================================================

static Module* s_binding_module = NULL;

// 上一帧(游戏逻辑帧)的按键状态，用于自己检测"按下"上升沿。
// 不能依赖 igIsKeyPressed()：它只在 igNewFrame 内部计算 (DownDuration==0 的窗口极短)，
// 而本项目的 Module_Update 在每帧的 igNewFrame 之前执行，中间还可能有多个插值渲染帧，
// 导致按下事件经常被漏检。
static bool s_prev_key_down[ImGuiKey_NamedKey_END] = { false };

// 每次游戏逻辑帧结束后保存当前按键状态
static void Module_SaveKeyStates(void) {
    for (int k = ImGuiKey_Tab; k < ImGuiKey_NamedKey_END; k++) {
        s_prev_key_down[k] = igIsKeyDown_Nil(k);
    }
}

void Module_BeginKeybind(Module* mod) {
    s_binding_module = mod;
}

void Module_CancelKeybind(void) {
    s_binding_module = NULL;
}

// 当前正在录制绑定的模块 (无则为 NULL)
Module* Module_GetBindingModule(void) {
    return s_binding_module;
}

// 录制过程中忽略纯修饰键 / 保留键
static bool is_modifier_key(int key) {
    switch (key) {
        case ImGuiKey_LeftCtrl: case ImGuiKey_RightCtrl:
        case ImGuiKey_LeftShift: case ImGuiKey_RightShift:
        case ImGuiKey_LeftAlt: case ImGuiKey_RightAlt:
        case ImGuiKey_LeftSuper: case ImGuiKey_RightSuper:
        case ImGuiKey_Menu:
        case ImGuiKey_CapsLock: case ImGuiKey_ScrollLock: case ImGuiKey_NumLock:
        case ImGuiKey_PrintScreen: case ImGuiKey_Pause:
            return true;
        default:
            return false;
    }
}

// 录制状态：等待下一个非修饰键按下 (基于自身上升沿检测)
static void Module_CaptureKeybind(void) {
    if (!s_binding_module) return;
    ImGuiIO* io = igGetIO();
    if (!io) return;

    for (int k = ImGuiKey_Tab; k < ImGuiKey_NamedKey_END; k++) {
        if (is_modifier_key(k)) continue;
        if (k >= ImGuiKey_GamepadStart) break; // 手柄/鼠标键不参与绑定
        if (igIsKeyDown_Nil(k) && !s_prev_key_down[k]) {
            if (k == ImGuiKey_Escape) {
                s_binding_module = NULL; // 取消录制
                return;
            }
            Module* mod = s_binding_module;
            mod->bind_key = k;
            mod->bind_mods = (io->KeyCtrl  ? ImGuiMod_Ctrl  : 0)
                           | (io->KeyShift ? ImGuiMod_Shift : 0)
                           | (io->KeyAlt   ? ImGuiMod_Alt   : 0)
                           | (io->KeySuper ? ImGuiMod_Super : 0);
            Module_FormatShortcut(mod, mod->shortcut, sizeof(mod->shortcut));
            s_binding_module = NULL;
            Config_Save(KEYBIND_CONFIG_PATH);
            printf("[Derect] Keybind %s -> %s\n", mod->name, mod->shortcut);
            return;
        }
    }
}

void Module_HandleShortcuts(void) {
    ImGuiIO* io = igGetIO();
    if (!io) return;

    // 录制模式优先，且不触发其他模块开关
    if (s_binding_module) {
        Module_CaptureKeybind();
        return;
    }

    // 有文本输入框聚焦时跳过，避免打字误触
    if (io->WantCaptureKeyboard) return;

    for (int i = 0; i < g_module_count; i++) {
        Module* mod = &g_modules[i];
        if (mod->bind_key == ImGuiKey_None) continue;

        if ((mod->bind_mods & ImGuiMod_Ctrl)  && !io->KeyCtrl)  continue;
        if ((mod->bind_mods & ImGuiMod_Shift) && !io->KeyShift) continue;
        if ((mod->bind_mods & ImGuiMod_Alt)   && !io->KeyAlt)   continue;
        if ((mod->bind_mods & ImGuiMod_Super) && !io->KeySuper) continue;

        // 自检测上升沿：上一逻辑帧未按下、当前已按下
        if (igIsKeyDown_Nil(mod->bind_key) && !s_prev_key_down[mod->bind_key]) {
            Module_Toggle(mod);
        }
    }
}

// will call on game/game_init.c
void Module_Update(void) {
    Module_HandleShortcuts();
    Module_SaveKeyStates();

    int count = sizeof(g_modules) / sizeof(g_modules[0]);
    for (int i = 0; i < count; i++) {
        if (g_modules[i].enabled && g_modules[i].on_loop) {
            g_modules[i].on_loop();
        }
    }
}

// will call on derect/ui Render
void Module_Render(void) {
    int count = sizeof(g_modules) / sizeof(g_modules[0]);
    for (int i = 0; i < count; i++) {
        // blacklist
        if (g_modules[i].name == "BackgroundBlur") continue;
        
        if (g_modules[i].enabled && g_modules[i].on_render) {
            g_modules[i].on_render();
        }
    }
}
