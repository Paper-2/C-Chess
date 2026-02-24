#ifndef PATHS_H
#define PATHS_H

#include <stddef.h>

// Initialize the base path (call once at startup)
// Returns 0 on success, -1 on failure
int paths_init(void);

// Get the base directory where the executable is located
const char* paths_get_base_dir(void);

// Resolve a relative path to an absolute path based on executable location
// result must be at least MAX_PATH bytes
// Returns result on success, NULL on failure
char* paths_resolve(const char* relative_path, char* result, size_t result_size);

// Convenience function to get asset path
// result must be at least MAX_PATH bytes
char* paths_get_asset(const char* asset_name, char* result, size_t result_size);

#endif // PATHS_H
