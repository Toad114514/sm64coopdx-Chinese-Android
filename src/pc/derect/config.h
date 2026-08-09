#ifndef DERECT_CONFIG_H
#define DERECT_CONFIG_H

#include <stdbool.h>

typedef enum {
    OPT_BOOL,
    OPT_INT,
    OPT_FLOAT,
    OPT_STR,
} OptionType;

// 单个配置项定义
typedef struct {
    const char* key;        // INI 配置文件中的 Key (如 "range")
    const char* label;      // ImGui 显示的中文标签 (如 "攻击距离")
    OptionType type;        // 类型
    void* ptr;              // 变量指针
    float min;              // 滑动条最小值 (仅 Int/Float 生效)
    float max;              // 滑动条最大值 (仅 Int/Float 生效)
    const char* format;     // 格式化字符串 (如 "%.0f"，填 NULL 使用默认)
    int str_max;            // 字符串缓冲区最大长度 (仅 Str 生效)
} ConfigOption;

// 辅助宏：快捷定义配置项
#define BIND_BOOL(key, label, ptr) \
    { key, label, OPT_BOOL, ptr, 0, 0, NULL, 0 }

#define BIND_INT(key, label, ptr, min, max, fmt) \
    { key, label, OPT_INT, ptr, (float)(min), (float)(max), fmt, 0 }

#define BIND_FLOAT(key, label, ptr, min, max, fmt) \
    { key, label, OPT_FLOAT, ptr, min, max, fmt, 0 }

// str_max: ptr 指向的字符串缓冲区大小 (需预留 1 字节给结尾 '\0')
#define BIND_STR(key, label, ptr, str_max) \
    { key, label, OPT_STR, ptr, 0, 0, NULL, str_max }


// 注册不同类型的变量以支持自动读写存档
void Config_RegisterBool(const char* mod_name, const char* var_name, bool* ptr);
void Config_RegisterInt(const char* mod_name, const char* var_name, int* ptr);
void Config_RegisterFloat(const char* mod_name, const char* var_name, float* ptr);
void Config_RegisterStr(const char* mod_name, const char* var_name, char* ptr, int str_max);

void Config_RegisterModuleOptions(const char* mod_name, const ConfigOption options[], int count);
void Config_RenderOptions(const ConfigOption options[], int count);

// 保存与加载配置
void Config_Save(const char* filename);
void Config_Load(const char* filename);

#endif