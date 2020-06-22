#include "fs.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "defs.h"

o_uint32_tp fs_load_uint32_buffer(
    const char* fname
) {
    FILE* file;
    long filelen;
    size_t buffer_size;
    uint32_t* buffer;
    const char* modes = "rb";
    o_uint32_tp result;

    file = fopen(fname, modes);
    if (file != NULL) {
        if (fseek(file, 0, SEEK_END) == 0) {
            filelen = ftell(file);
            if (filelen != -1L) {
                if (filelen != 0) {
                    rewind(file);
                    buffer_size = filelen * sizeof(uint32_t);
                    buffer = (uint32_t*)malloc(buffer_size);
                    if (buffer != NULL) {
                        if (fread(buffer, filelen, 1, file) == 1) {
                            if (fclose(file) == 0) {
                                result.value = buffer;
                                result.err = NULL;
                            } else {
                                result.err = err_new_file(
                                    L"couldn't close the file",
                                    ERR_FILE_T_CANT_CLOSE, fname, modes, NULL);
                            }
                        } else {
                            result.err = err_new_file(
                                L"failed to read the file into the buffer",
                                ERR_FILE_T_CANT_READ, fname, modes, NULL);
                        }
                    } else {
                        result.err =
                            err_new_file(
                                L"failed to allocate buffer",
                                ERR_FILE_T_GENERAL, fname, modes,
                                    err_new_malloc(NULL, buffer_size, NULL));
                    }
                } else {
                    result.err = err_new_file(
                        L"file is empty",
                        ERR_FILE_T_EMPTY, fname, modes, NULL);
                }
            } else {
                result.err = err_new_file(
                    L"couldn't get the length of the file",
                    ERR_FILE_T_GENERAL, fname, modes, NULL);
            }
        } else {
            result.err = err_new_file(
                L"couldn't go to the end of the file",
                ERR_FILE_T_GENERAL, fname, modes, NULL);
        }
    } else {
        result.err = err_new_file(
            L"couldn't open the file",
            ERR_FILE_T_CANT_OPEN, fname, modes, NULL);
    }

    return result;
}
