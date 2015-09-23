#pragma once

#ifdef __cplusplus
extern "C" {
#endif

__EXPORT int eparam_save(const char *search_string, const char *filename, const char *mode);
__EXPORT int eparam_load(const char *filename);

#ifdef __cplusplus
}
#endif
