#include <string.h>
#include "argtable3/argtable3.h"
#include "esp_console.h"
#include "nvs.h"

#include "storage_wrapper.h"
#include "hexdump.h"

static struct {
    struct arg_str *tag;
    struct arg_end *end;
} getIntArgs, getStringArgs, deleteArgs;

static struct {
    struct arg_str *tag;
    struct arg_int *width;
    struct arg_end *end;
} getBlobArgs;

static struct {
    struct arg_str *tag;
    struct arg_int *val;
    struct arg_end *end;
} setIntArgs;

static struct {
    struct arg_str *tag;
    struct arg_str *val;
    struct arg_end *end;
} setStringArgs;

static struct {
    struct arg_lit *all;
    struct arg_end *end;
} infoArgs;

static int getInt(int argc, char **argv) {
    int nerrors = arg_parse(argc, argv, (void **)&getIntArgs);
    if (nerrors != 0) {
        arg_print_errors(stderr, getIntArgs.end, argv[0]);
        return 1;
    }

    int32_t val;

    esp_err_t err = storageGetInt(getIntArgs.tag->sval[0], &val, 0);

    if (err == ESP_OK) {
        printf("Read success: %ld\n", val);
    } else {
        printf("Read error: %s\n", esp_err_to_name(err));
    }

    return err;
}

static int setInt(int argc, char **argv) {
    int nerrors = arg_parse(argc, argv, (void **)&setIntArgs);
    if (nerrors != 0) {
        arg_print_errors(stderr, setIntArgs.end, argv[0]);
        return 1;
    }

    esp_err_t err = storageSetInt(setIntArgs.tag->sval[0], setIntArgs.val->ival[0]);

    if (err == ESP_OK) {
        printf("Write success\n");
    } else {
        printf("Write error: %s\n", esp_err_to_name(err));
    }

    return err;
}

static int getString(int argc, char **argv) {
    int nerrors = arg_parse(argc, argv, (void **)&getStringArgs);
    if (nerrors != 0) {
        arg_print_errors(stderr, getStringArgs.end, argv[0]);
        return 1;
    }

    char buf[STR_BUF_LEN];
    bool truncated;

    esp_err_t err = storageGetString(getStringArgs.tag->sval[0], buf, NULL, STR_BUF_LEN - 1, &truncated);

    if (err == ESP_OK) {
        printf("Read success: %s\n", buf);
        if (truncated) {
            printf("Truncated to first %d chars\n", STR_BUF_LEN - 1);
        }
    } else {
        printf("Read error: %s\n", esp_err_to_name(err));
    }

    return err;
}

static int setString(int argc, char **argv) {
    int nerrors = arg_parse(argc, argv, (void **)&setStringArgs);
    if (nerrors != 0) {
        arg_print_errors(stderr, setStringArgs.end, argv[0]);
        return 1;
    }

    esp_err_t err = storageSetString(setStringArgs.tag->sval[0], setStringArgs.val->sval[0]);
    // esp_err_t err = storageSetBlob(setStringArgs.tag->sval[0], "This is a hardcoded string\n\n\t\t\0\0aabbcc---\nWith a length of 69; nice.", 69);

    if (err == ESP_OK) {
        printf("Write success\n");
    } else {
        printf("Write error: %s\n", esp_err_to_name(err));
    }

    return err;
}

static int getBlob(int argc, char **argv) {
    int nerrors = arg_parse(argc, argv, (void **)&getBlobArgs);
    if (nerrors != 0) {
        arg_print_errors(stderr, getBlobArgs.end, argv[0]);
        return 1;
    }

    int hexdumpWidth = 8;
    if (getBlobArgs.width->count == 1) {
        hexdumpWidth = getBlobArgs.width->ival[0];
    }

    char *buf = malloc(BLB_BUF_LEN);
    bool truncated;
    size_t len;

    esp_err_t err = storageGetBlob(getBlobArgs.tag->sval[0], buf, &len, BLB_BUF_LEN, &truncated);

    if (err == ESP_OK) {
        printf("Read success: %d bytes\n", len);
        if (truncated) {
            printf("Truncated to first %d bytes\n", BLB_BUF_LEN);
        }
        hexdump(buf, len, hexdumpWidth);
    } else {
        printf("Read error: %s\n", esp_err_to_name(err));
    }

    free(buf);

    return err;
}

static int delete(int argc, char **argv) {
    int nerrors = arg_parse(argc, argv, (void **)&deleteArgs);
    if (nerrors != 0) {
        arg_print_errors(stderr, deleteArgs.end, argv[0]);
        return 1;
    }

    esp_err_t err = storageDelete(deleteArgs.tag->sval[0]);

    if (err == ESP_OK) {
        printf("Delete success\n");
    } else {
        printf("Delete error: %s\n", esp_err_to_name(err));
    }

    return err;
}

static char *typeToString(nvs_type_t type) {
    switch (type) {
        case NVS_TYPE_U8:
            return "U8";
        case NVS_TYPE_I8:
            return "I8";
        case NVS_TYPE_U16:
            return "U16";
        case NVS_TYPE_I16:
            return "I16";
        case NVS_TYPE_U32:
            return "U32";
        case NVS_TYPE_I32:
            return "I32";
        case NVS_TYPE_U64:
            return "U64";
        case NVS_TYPE_I64:
            return "I64";
        case NVS_TYPE_STR:
            return "STR";
        case NVS_TYPE_BLOB:
            return "BLOB";
        default:
            return "UNKNOWN";
    }
}

static int printInfo(int argc, char **argv) {
    int nerrors = arg_parse(argc, argv, (void **)&infoArgs);
    if (nerrors != 0) {
        arg_print_errors(stderr, infoArgs.end, argv[0]);
        return 1;
    }

    printf("Storage stored items:\n");
    nvs_iterator_t it = NULL;
    esp_err_t res = nvs_entry_find("nvs", infoArgs.all->count == 1 ? NULL : "storage", NVS_TYPE_ANY, &it);
    while (res == ESP_OK) {
        nvs_entry_info_t info;
        nvs_entry_info(it, &info); // Can omit error check if parameters are guaranteed to be non-NULL
        if (infoArgs.all->count == 1) {
            printf("Tag: %s, type %s, namespace %s\n", info.key, typeToString(info.type), info.namespace_name);
        } else {
            printf("Tag: %s, type '%s'\n", info.key, typeToString(info.type));
        }
        res = nvs_entry_next(&it);
    }
    nvs_release_iterator(it);

    return 0;
}

esp_err_t storageRegisterCommands() {
    getIntArgs.tag = arg_str1("t", "tag", "<string>", "Tag for data");
    getIntArgs.end = arg_end(1);
    esp_console_cmd_t getIntCmd = {
        .command = "storage-get-int",
        .help = "Gets int from storage",
        .argtable = &getIntArgs,
        .func = &getInt,
    };
    esp_err_t err = esp_console_cmd_register(&getIntCmd);

    if (err != ESP_OK) {
        return err;
    }

    setIntArgs.tag = arg_str1("t", "tag", "<string>", "Tag for data");
    setIntArgs.val = arg_int1("v", "val", "<value>", "Value to be stored");
    setIntArgs.end = arg_end(2);
    esp_console_cmd_t setIntCmd = {
        .command = "storage-set-int",
        .help = "Sets int in storage",
        .argtable = &setIntArgs,
        .func = &setInt,
    };
    err = esp_console_cmd_register(&setIntCmd);

    if (err != ESP_OK) {
        return err;
    }

    getStringArgs.tag = arg_str1("t", "tag", "<string>", "Tag for data");
    getStringArgs.end = arg_end(1);
    esp_console_cmd_t getStringCmd = {
        .command = "storage-get-str",
        .help = "Gets string from storage",
        .argtable = &getStringArgs,
        .func = &getString,
    };
    err = esp_console_cmd_register(&getStringCmd);

    if (err != ESP_OK) {
        return err;
    }

    setStringArgs.tag = arg_str1("t", "tag", "<string>", "Tag for data");
    setStringArgs.val = arg_str1("v", "val", "<value>", "Value to be stored");
    setStringArgs.end = arg_end(2);
    esp_console_cmd_t setStringCmd = {
        .command = "storage-set-str",
        .help = "Sets string in storage",
        .argtable = &setStringArgs,
        .func = &setString,
    };
    err = esp_console_cmd_register(&setStringCmd);

    if (err != ESP_OK) {
        return err;
    }

    getBlobArgs.tag = arg_str1("t", "tag", "<string>", "Tag for data");
    getBlobArgs.width = arg_int0("w", "width", "<bytes>", "Width of hexdump in bytes");
    getBlobArgs.end = arg_end(2);
    esp_console_cmd_t getBlobCmd = {
        .command = "storage-get-blb",
        .help = "Gets blob from storage",
        .argtable = &getBlobArgs,
        .func = &getBlob,
    };
    err = esp_console_cmd_register(&getBlobCmd);

    if (err != ESP_OK) {
        return err;
    }

    deleteArgs.tag = arg_str1("t", "tag", "<string>", "Tag to delete");
    deleteArgs.end = arg_end(1);
    esp_console_cmd_t deleteCmd = {
        .command = "storage-del",
        .help = "Deletes from storage",
        .argtable = &deleteArgs,
        .func = &delete,
    };
    err = esp_console_cmd_register(&deleteCmd);

    if (err != ESP_OK) {
        return err;
    }

    infoArgs.all = arg_lit0("a", "all", "Print all key-value pairs");
    infoArgs.end = arg_end(1);
    esp_console_cmd_t infoCmd = {
        .command = "storage-info",
        .help = "Prints information about storage",
        .argtable = &infoArgs,
        .func = &printInfo,
    };
    err = esp_console_cmd_register(&infoCmd);

    return err;
}