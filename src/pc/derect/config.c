#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "config.h"
#include "module.h"

#include "../cimgui/cimgui.h"

typedef enum {
    CFG_TYPE_BOOL,
    CFG_TYPE_INT,
    CFG_TYPE_FLOAT,
    CFG_TYPE_STR
} ConfigType;

typedef struct {
    char mod_name[32];
    char var_name[32];
    ConfigType type;
    void* ptr;
    int size;   // OPT_STR 字符串缓冲区大小 (含结尾 '\0')
} ConfigEntry;

#define MAX_CONFIG_ENTRIES 128
static ConfigEntry s_entries[MAX_CONFIG_ENTRIES];
static int s_entry_count = 0;

// 去除字符串前后空格
static char* trim_whitespace(char* str) {
    char* end;
    while (isspace((unsigned char)*str)) str++;
    if (*str == 0) return str;
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return str;
}

static void AddEntry(const char* mod_name, const char* var_name, ConfigType type, void* ptr, int size) {
    if (s_entry_count >= MAX_CONFIG_ENTRIES) return;
    snprintf(s_entries[s_entry_count].mod_name, 32, "%s", mod_name);
    snprintf(s_entries[s_entry_count].var_name, 32, "%s", var_name);
    s_entries[s_entry_count].type = type;
    s_entries[s_entry_count].ptr = ptr;
    s_entries[s_entry_count].size = size;
    s_entry_count++;
}

void Config_RegisterBool(const char* mod_name, const char* var_name, bool* ptr) {
    AddEntry(mod_name, var_name, CFG_TYPE_BOOL, ptr, 0);
}

void Config_RegisterInt(const char* mod_name, const char* var_name, int* ptr) {
    AddEntry(mod_name, var_name, CFG_TYPE_INT, ptr, 0);
}

void Config_RegisterFloat(const char* mod_name, const char* var_name, float* ptr) {
    AddEntry(mod_name, var_name, CFG_TYPE_FLOAT, ptr, 0);
}

void Config_RegisterStr(const char* mod_name, const char* var_name, char* ptr, int str_max) {
    AddEntry(mod_name, var_name, CFG_TYPE_STR, ptr, str_max);
}

// 写入配置到文件
void Config_Save(const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) return;
    
    printf("[derect_config] Saving data conf to %s\n", filename);

    for (int i = 0; i < g_module_count; i++) {
        Module* mod = &g_modules[i];
        fprintf(file, "[%s]\n", mod->name);
        fprintf(file, "enabled=%d\n", mod->enabled ? 1 : 0);
        if (mod->shortcut[0] != '\0') {
            fprintf(file, "shortcut=%s\n", mod->shortcut);
        }

        // 写入该模块注册的所有自定义变量
        for (int j = 0; j < s_entry_count; j++) {
            if (strcmp(s_entries[j].mod_name, mod->name) == 0) {
                switch (s_entries[j].type) {
                    case CFG_TYPE_BOOL:
                        fprintf(file, "%s=%d\n",   s_entries[j].var_name, *(bool*)s_entries[j].ptr ? 1 : 0);
                        break;
                    case CFG_TYPE_INT:
                        fprintf(file, "%s=%d\n",   s_entries[j].var_name, *(int*)s_entries[j].ptr);
                        break;
                    case CFG_TYPE_FLOAT:
                        fprintf(file, "%s=%.2f\n", s_entries[j].var_name, *(float*)s_entries[j].ptr);
                        break;
                    case CFG_TYPE_STR:
                        fprintf(file, "%s=%s\n",   s_entries[j].var_name, (char*)s_entries[j].ptr);
                        break;
                }
            }
        }
        fprintf(file, "\n");
    }

    fclose(file);
    printf("[derect_config] Saved data conf to %s \n", filename);
}

// 从文件读取并恢复配置
void Config_Load(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) return;
    
    printf("[derect_config] Loading data conf from %s \n", filename);

    char line[128];
    char current_mod[32] = "";

    while (fgets(line, sizeof(line), file)) {
        char* trimmed = trim_whitespace(line);
        if (trimmed[0] == '\0' || trimmed[0] == ';') continue; // 跳过空行和注释

        // 读取 [ModuleName] 节点
        if (trimmed[0] == '[' && trimmed[strlen(trimmed) - 1] == ']') {
            sscanf(trimmed, "[%31[^]]]", current_mod);
            continue;
        }

        // 解析 key=value
        char* eq = strchr(trimmed, '=');
        if (eq && current_mod[0] != '\0') {
            *eq = '\0';
            char* key = trim_whitespace(trimmed);
            char* val = trim_whitespace(eq + 1);

            // 处理模块开启状态
            if (strcmp(key, "enabled") == 0) {
                bool is_enabled = (atoi(val) != 0);
                Module* mod = Module_Find(current_mod);
                if (mod) {
                    if (is_enabled && !mod->enabled && mod->on_enable) mod->on_enable();
                    if (!is_enabled && mod->enabled && mod->on_disable) mod->on_disable();
                    mod->enabled = is_enabled;
                    printf("[derect_config] Process Module config: %s \n", mod->name);
                }
            } else if (strcmp(key, "shortcut") == 0) {
                Module* mod = Module_Find(current_mod);
                if (mod) {
                    Module_SetShortcut(mod, val);
                    printf("[derect_config] Load Module shortcut: %s -> %s\n", mod->name, mod->shortcut);
                }
            } else {
                // 处理注册的变量
                for (int i = 0; i < s_entry_count; i++) {
                    if (strcmp(s_entries[i].mod_name, current_mod) == 0 &&
                        strcmp(s_entries[i].var_name, key) == 0) {
                        switch (s_entries[i].type) {
                            case CFG_TYPE_BOOL:
                                *(bool*)s_entries[i].ptr = (atoi(val) != 0);
                                break;
                            case CFG_TYPE_INT:
                                *(int*)s_entries[i].ptr = atoi(val);
                                break;
                            case CFG_TYPE_FLOAT:
                                *(float*)s_entries[i].ptr = (float)atof(val);
                                break;
                            case CFG_TYPE_STR:
                                if (s_entries[i].size > 0) {
                                    strncpy((char*)s_entries[i].ptr, val, (size_t)(s_entries[i].size - 1));
                                    ((char*)s_entries[i].ptr)[s_entries[i].size - 1] = '\0';
                                }
                                break;
                        }
                    }
                }
            }
        }
    }

    fclose(file);
    printf("[derect_config] Loaded data conf from %s \n", filename);
}

// 批量注册
void Config_RegisterModuleOptions(const char* mod_name, const ConfigOption options[], int count) {
    for (int i = 0; i < count; i++) {
        const ConfigOption* opt = &options[i];
        switch (opt->type) {
            case OPT_BOOL:
                Config_RegisterBool(mod_name, opt->key, (bool*)opt->ptr);
                break;
            case OPT_INT:
                Config_RegisterInt(mod_name, opt->key, (int*)opt->ptr);
                break;
            case OPT_FLOAT:
                Config_RegisterFloat(mod_name, opt->key, (float*)opt->ptr);
                break;
            case OPT_STR:
                Config_RegisterStr(mod_name, opt->key, (char*)opt->ptr, opt->str_max);
                break;
        }
    }
}

// render ui
void Config_RenderOptions(const ConfigOption options[], int count) {
    bool changed = false;

    for (int i = 0; i < count; i++) {
        const ConfigOption* opt = &options[i];
        
        igPushID_Str(opt->key);

        switch (opt->type) {
            case OPT_BOOL:
                // 点击勾选框时返回 true
                if (igCheckbox(opt->label, (bool*)opt->ptr)) {
                    changed = true;
                }
                break;

            case OPT_INT:
                // 拖动滑动条时返回 true
                if (igSliderInt(
                    opt->label, 
                    (int*)opt->ptr, 
                    (int)opt->min, 
                    (int)opt->max, 
                    opt->format ? opt->format : "%d", 
                    0
                )) {
                    // 如果不希望拖动过程频繁写磁盘，可用：
                    // if (igIsItemDeactivatedAfterEdit()) changed = true;
                    changed = true; 
                }
                break;

            case OPT_FLOAT:
                if (igSliderFloat(
                    opt->label, 
                    (float*)opt->ptr, 
                    opt->min, 
                    opt->max, 
                    opt->format ? opt->format : "%.1f", 
                    0
                )) {
                    changed = true;
                }
                break;
            
            case OPT_STR:
                // 字符串输入框 (str_max 为缓冲区大小，含结尾 '\0')
                if (opt->str_max > 0) {
                    if (igInputText(opt->label, (char*)opt->ptr, (size_t)opt->str_max, 0, NULL, NULL)) {
                        changed = true;
                    }
                }
                break;
        }

        igPopID();
    }

    // 只要有任意配置改动，立刻自动写入文件
    if (changed) {
        Config_Save("/storage/emulated/0/com.toad1145.derectcoopdxcn/config.ini");
    }
}