#pragma once

#include <inttypes.h>
#include <nvs.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define STR_BUF_LEN 100
#define BLB_BUF_LEN 500

esp_err_t storageInit();

esp_err_t storageSetInt(const char *tag, int32_t val);

esp_err_t storageGetInt(const char *tag, int32_t *val, int32_t def);

esp_err_t storageSetString(const char *tag, const char *val);

esp_err_t storageGetString(const char *tag, char *val, const char *def, size_t maxLen, bool *trn);

esp_err_t storageSetBlob(const char *tag, void *ptr, size_t len);

esp_err_t storageGetBlob(const char *tag, void *ptr, size_t *len, size_t maxLen, bool *trn);

esp_err_t storageDelete(const char *tag);

esp_err_t storageRegisterCommands();
