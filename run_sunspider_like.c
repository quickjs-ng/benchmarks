/*
 * QuickJS Javascript Engine Benchmark Suite
 *
 * Copyright (c) 2023 Divy Srivastava
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef NO_INCLUDE_DIR
#include "quickjs.h"
#else
#include "quickjs/quickjs.h"
#endif

double total = 0;

int Execute(JSContext *ctx, const char *filename) {
    clock_t start_time, end_time;
    double execution_time;
    /* assumes max test script size */
    static char buf[8 << 20]; // 8 MB

    FILE *file = fopen(filename, "r");
    if (file == NULL)
        return -1;
    size_t nread = fread(buf, 1, sizeof(buf), file);
    fclose(file);
    if (nread == 0) {
        fprintf(stderr, "fread failed: %s\n", filename);
        return -1;
    }
    if (nread == sizeof(buf)) {
        fprintf(stderr, "file too large: %s\n", filename);
        return -1;
    }
    buf[nread] = '\0';

    start_time = clock();
    JSValue ret_val = JS_Eval(ctx, buf, nread, filename, JS_EVAL_TYPE_GLOBAL);
    end_time = clock();
    execution_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
    total += execution_time;

    printf("%s (%.4f seconds)\n", filename, execution_time);

    if (JS_IsException(ret_val)) {
        JSValue exception = JS_GetException(ctx);
        if (JS_IsError(ctx, exception)) {
            JSValue stack;
            const char *stack_str;

            stack = JS_GetPropertyStr(ctx, exception, "stack");
            if (!JS_IsUndefined(stack)) {
                stack_str = JS_ToCString(ctx, stack);
                if (stack_str) {
                    printf("%s\n", stack_str);
                    JS_FreeCString(ctx, stack_str);
                }
            }
            JS_FreeValue(ctx, stack);
        }
        JS_FreeValue(ctx, exception);
        return -1;
    }

    JS_FreeValue(ctx, ret_val);
    return 0;
}

void RunTest(JSContext* ctx, char* path, char* buffer) {
    char fullpath[256];

    snprintf(fullpath, sizeof(fullpath), "%s%s-data.js", path, buffer);
    /* Pass, only used for kraken. */
    Execute(ctx, fullpath);

    snprintf(fullpath, sizeof(fullpath), "%s%s.js", path, buffer);
    if (Execute(ctx, fullpath) != 0) {
        fprintf(stderr, "Error executing file: %s\n", buffer);
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <benchmark folder> [optional: name]\n", argv[0]);
        return 1;
    }

    JSRuntime *rt = JS_NewRuntime();
    JSContext *ctx = JS_NewContext(rt);

    /* disable stack limit */
    JS_SetMaxStackSize(rt, 0);

    char *path = argv[1];
    char filename[256];
    snprintf(filename, sizeof(filename), "%s%s", path, "LIST");

    FILE *listFile = fopen(filename, "r");
    if (listFile == NULL) {
        fprintf(stderr, "Error opening file: %s\n", filename);
        return 1;
    }

    char buffer[100]; // Adjust the buffer size as needed
    while (fgets(buffer, sizeof(buffer), listFile) != NULL) {
        size_t i = strcspn(buffer, "\n");
        buffer[i] = '\0';
        if (argc == 3 && strcmp(argv[2], buffer) != 0) {
            printf("Skipping %s\n", buffer);
            continue;
        }
        RunTest(ctx, path, buffer);
        JS_RunGC(rt);
    }

    printf("\nTotal (ms): %.4f\n", total * 1000);

    fclose(listFile);
    JS_FreeContext(ctx);

    return 0;
}
