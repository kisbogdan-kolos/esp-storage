#include <stdbool.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"

#include "storage_wrapper.h"

static const char *TAG = "storagw-wrapper";

/**
 * Opens the NVS storage.
 * @param mode Mode to open the storage in.
 * @param storage Pointer to the storage handle.
 * @return ESP_OK on success, error code otherwise.
 */
static esp_err_t open(nvs_open_mode_t mode, nvs_handle_t *storage) {
    esp_err_t err = nvs_open("storage", mode, storage);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
    }
    return err;
}

/**
 * Closes the NVS storage.
 * @param storage Pointer to the storage handle.
 */
static void close(nvs_handle_t *storage) {
    nvs_close(*storage);
}

/**
 * Initializes the NVS storage.
 * @return ESP_OK on success, error code otherwise.
 */
esp_err_t storageInit() {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        err = nvs_flash_init();
    }

    return err;
}

/**
 * Sets an integer value in the storage.
 * @param tag Tag of the value.
 * @param val Value to set.
 * @return ESP_OK on success, error code otherwise.
 */
esp_err_t storageSetInt(const char *tag, int32_t val) {
    nvs_handle_t storage;
    esp_err_t err = open(NVS_READWRITE, &storage);
    if (err != ESP_OK) {
        return err;
    }

    esp_err_t err2 = nvs_set_i32(storage, tag, val);
    esp_err_t err3 = nvs_commit(storage);

    close(&storage);

    return err2 == ESP_OK ? err3 : err2;
}

/**
 * Gets an integer value from the storage.
 * @param tag Tag of the value.
 * @param val Pointer to the value.
 * @param def Default value if the tag is not found.
 * @return ESP_OK on success, error code otherwise.
 */
esp_err_t storageGetInt(const char *tag, int32_t *val, int32_t def) {
    nvs_handle_t storage;
    esp_err_t err = open(NVS_READONLY, &storage);
    if (err != ESP_OK) {
        return err;
    }

    esp_err_t err2 = nvs_get_i32(storage, tag, val);
    if (err2 == ESP_ERR_NVS_NOT_FOUND) {
        *val = def;
        err2 = ESP_OK;
    }

    close(&storage);

    return err2;
}

/**
 * Sets a string value in the storage.
 * @param tag Tag of the value.
 * @param val Value to set.
 * @return ESP_OK on success, error code otherwise.
 */
esp_err_t storageSetString(const char *tag, const char *val) {
    nvs_handle_t storage;
    esp_err_t err = open(NVS_READWRITE, &storage);
    if (err != ESP_OK) {
        return err;
    }

    esp_err_t err2 = nvs_set_str(storage, tag, val);
    esp_err_t err3 = nvs_commit(storage);

    close(&storage);

    return err2 == ESP_OK ? err3 : err2;
}

/**
 * Gets a string value from the storage.
 * @param tag Tag of the value.
 * @param val String to copy data to.
 * @param def Default value if the tag is not found. If NULL, the out buffer won't be modified.
 * @param maxLen Maximum length of the string. Should be set to the size of the val buffer. Includes the null terminator.
 * @param trn Pointer to a boolean that is set to true if the string was truncated. (Did not fit into the buffer.) Can be NULL.
 * @return ESP_OK on success, error code otherwise.
 */
esp_err_t storageGetString(const char *tag, char *val, const char *def, size_t maxLen, bool *trn) {
    nvs_handle_t storage;
    esp_err_t err = open(NVS_READONLY, &storage);
    if (err != ESP_OK) {
        return err;
    }

    size_t siz;
    esp_err_t err2 = nvs_get_str(storage, tag, NULL, &siz);
    if (err2 == ESP_ERR_NVS_NOT_FOUND && def != NULL) {
        strncpy(val, def, maxLen - 1);
        val[maxLen - 1] = '\0';
        close(&storage);
        return ESP_OK;
    }

    esp_err_t err3;
    if (err2 == ESP_OK) {
        if (siz <= maxLen + 1) {
            err3 = nvs_get_str(storage, tag, val, &siz);
            if (trn != NULL)
                *trn = false;
        } else {
            char *dst = malloc(siz * sizeof(char));
            err3 = nvs_get_str(storage, tag, dst, &siz);
            if (err3 == ESP_OK) {
                strncpy(val, dst, maxLen);
                val[maxLen - 1] = '\0';
                if (trn != NULL)
                    *trn = true;
            }
            free(dst);
        }
    }

    close(&storage);

    return err2 == ESP_OK ? err3 : err2;
}

/**
 * Sets a blob value in the storage.
 * @param tag Tag of the value.
 * @param ptr Pointer to the data.
 * @param len Length of the data.
 * @return ESP_OK on success, error code otherwise.
 */
esp_err_t storageSetBlob(const char *tag, void *ptr, size_t len) {
    nvs_handle_t storage;
    esp_err_t err = open(NVS_READWRITE, &storage);
    if (err != ESP_OK) {
        return err;
    }

    esp_err_t err2 = nvs_set_blob(storage, tag, ptr, len);
    esp_err_t err3 = nvs_commit(storage);

    close(&storage);

    return err2 == ESP_OK ? err3 : err2;
}

/**
 * Gets a blob value from the storage.
 * @param tag Tag of the value.
 * @param ptr Pointer to the data.
 * @param len Pointer to the length of the data.
 * @param maxLen Maximum length of the data. Should be set to the size of the ptr buffer.
 * @param trn Pointer to a boolean that is set to true if the data was truncated. (Did not fit into the buffer.) Can be NULL.
 * @return ESP_OK on success, error code otherwise.
 */
esp_err_t storageGetBlob(const char *tag, void *ptr, size_t *len, size_t maxLen, bool *trn) {
    nvs_handle_t storage;
    esp_err_t err = open(NVS_READONLY, &storage);
    if (err != ESP_OK) {
        return err;
    }

    size_t siz;
    esp_err_t err2 = nvs_get_blob(storage, tag, NULL, &siz);

    esp_err_t err3;
    if (err2 == ESP_OK) {
        if (siz <= maxLen) {
            err3 = nvs_get_blob(storage, tag, ptr, &siz);
            if (trn != NULL)
                *trn = false;
            *len = siz;
        } else {
            char *dst = malloc(siz);
            err3 = nvs_get_blob(storage, tag, dst, &siz);
            if (err3 == ESP_OK) {
                memcpy(ptr, dst, maxLen);
                *len = maxLen;
                if (trn != NULL)
                    *trn = true;
            }
            free(dst);
        }
    }

    close(&storage);

    return err2 == ESP_OK ? err3 : err2;
}

/**
 * Deletes a value from the storage. Data type is not checked.
 * @param tag Tag of the value.
 * @return ESP_OK on success, error code otherwise.
 */
esp_err_t storageDelete(const char *tag) {
    nvs_handle_t storage;
    esp_err_t err = open(NVS_READWRITE, &storage);
    if (err != ESP_OK) {
        return err;
    }

    esp_err_t err2 = nvs_erase_key(storage, tag);
    esp_err_t err3 = nvs_commit(storage);

    close(&storage);

    return err2 == ESP_OK ? err3 : err2;
}
