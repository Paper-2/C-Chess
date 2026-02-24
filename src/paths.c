#include "paths.h"
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define PATH_SEP '\\'
#define PATH_SEP_STR "\\"
#else
#include <unistd.h>
#include <libgen.h>
#include <limits.h>
#define MAX_PATH PATH_MAX
#define PATH_SEP '/'
#define PATH_SEP_STR "/"
#endif

static char g_base_dir[MAX_PATH] = {0};
static int g_initialized = 0;

int paths_init(void) {
    if (g_initialized) return 0;

#ifdef _WIN32
    char exe_path[MAX_PATH];
    DWORD len = GetModuleFileNameA(NULL, exe_path, MAX_PATH);
    if (len == 0 || len >= MAX_PATH) {
        fprintf(stderr, "Failed to get executable path\n");
        return -1;
    }
    
    char* last_sep = strrchr(exe_path, PATH_SEP);
    if (last_sep) {
        *last_sep = '\0';
    }
    strncpy(g_base_dir, exe_path, MAX_PATH - 1);
#else
    char exe_path[MAX_PATH];
    ssize_t len = readlink("/proc/self/exe", exe_path, MAX_PATH - 1);
    if (len == -1) {
        fprintf(stderr, "Failed to get executable path\n");
        return -1;
    }
    exe_path[len] = '\0';
    
    char* dir = dirname(exe_path);
    strncpy(g_base_dir, dir, MAX_PATH - 1);
#endif

    g_initialized = 1;
    printf("Base directory: %s\n", g_base_dir);
    return 0;
}

const char* paths_get_base_dir(void) {
    if (!g_initialized) {
        paths_init();
    }
    return g_base_dir;
}

char* paths_resolve(const char* relative_path, char* result, size_t result_size) {
    int written;
    char* p;
    
    if (!g_initialized) {
        if (paths_init() != 0) return NULL;
    }
    if (!relative_path || !result || result_size == 0) return NULL;

    if (relative_path[0] == '.' && (relative_path[1] == '/' || relative_path[1] == '\\')) {
        relative_path += 2;
    }

    written = snprintf(result, result_size, "%s%s%s", g_base_dir, PATH_SEP_STR, relative_path);
    if (written < 0 || (size_t)written >= result_size) {
        return NULL;
    }

#ifdef _WIN32
    for (p = result; *p; p++) {
        if (*p == '/') *p = '\\';
    }
#endif

    return result;
}

char* paths_get_asset(const char* asset_name, char* result, size_t result_size) {
    char relative[MAX_PATH];
    snprintf(relative, sizeof(relative), "data%sassets%s%s", PATH_SEP_STR, PATH_SEP_STR, asset_name);
    return paths_resolve(relative, result, result_size);
}
