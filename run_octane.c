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

#ifdef NO_INCLUDE_DIR
#include "quickjs.h"
#include "quickjs-libc.h"
#else
#include "quickjs/quickjs.h"
#include "quickjs/quickjs-libc.h"
#endif

void Execute(JSContext *ctx, const char *path, const char *filename, int eval_flags) {
    char pathname[1024];

    snprintf(pathname, sizeof pathname, "%s%s", path, filename);
    filename = pathname;

    FILE *file = fopen(filename, "r");
    /* assumes max test script size */
    static char buf[8 << 20]; // 8 MB
    if (file == NULL) {
        fprintf(stderr, "Cannot open file: %s\n", filename);
        return;
    }
    size_t nread = fread(buf, 1, sizeof(buf), file);
    fclose(file);
    if (nread == 0) {
        fprintf(stderr, "fread failed: %s\n", filename);
        return;
    }
    if (nread == sizeof(buf)) {
        fprintf(stderr, "file too large: %s\n", filename);
        return;
    }
    buf[nread] = '\0';

    JSValue ret_val = JS_Eval(ctx, buf, nread, filename, eval_flags);
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
    }
    JS_FreeValue(ctx, ret_val);
    return;
}

int main(int argc, char **argv) {
    JSRuntime *rt = JS_NewRuntime();
    JSContext *ctx = JS_NewContext(rt);
    const char *path = "";
    int argnum = 1;

    /* system modules */
    js_init_module_std(ctx, "std");
    /* disable stack limit */
    JS_SetMaxStackSize(rt, 0);

    if (argc > 1) {
        size_t len = strlen(argv[1]);
        if (len == 0 || argv[1][len - 1] == '/') {
            path = argv[1];
            argnum = 2;
        }
    }

    Execute(ctx, path, "octane/base.js", JS_EVAL_TYPE_GLOBAL);

    static const char *testfile[] = {
        "octane/richards.js",
        "octane/deltablue.js",
        "octane/crypto.js",
        "octane/raytrace.js",
        "octane/earley-boyer.js",
        "octane/regexp.js",
        "octane/splay.js",
        "octane/navier-stokes.js",
        "octane/pdfjs.js",
        "octane/mandreel.js",
        "octane/gbemu-part1.js",
        "octane/gbemu-part2.js",
        "octane/code-load.js",
        "octane/box2d.js",
        "octane/zlib.js",
        "octane/zlib-data.js",
        "octane/typescript.js",
        "octane/typescript-input.js",
        "octane/typescript-compiler.js",
    };
    int testfile_number = sizeof(testfile) / sizeof(testfile[0]);

    for (int test = 0; test < testfile_number; test++) {
        const char *filename = testfile[test];
        if (argc > argnum) {
            int found = 0;
            for (int i = argnum; i < argc; i++) {
                if (strstr(filename, argv[i])) {
                    found = 1;
                    break;
                }
            }
            if (!found) {
                printf("Skipping %s%s\n", path, filename);
                continue;
            }
        }
        Execute(ctx, path, filename, JS_EVAL_TYPE_GLOBAL);
    }

    Execute(ctx, path, "run_octane.js", JS_EVAL_TYPE_MODULE);

    JS_FreeContext(ctx);

    return 0;
}
