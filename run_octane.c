#include "quickjs/quickjs.h"
#include "quickjs/quickjs-libc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

double total = 0;

int Execute(JSContext *ctx, const char *filename, int eval_flags) {
    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        return 1;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = (char *)malloc(file_size + 1);
    if (buffer == NULL) {
        fclose(file);
        fprintf(stderr, "Memory allocation error\n");
        return 1;
    }

    size_t bytesRead = fread(buffer, 1, file_size, file);
    fclose(file);

    if (bytesRead != file_size) {
        fprintf(stderr, "Error reading file: %s\n", filename);
        printf("bytesRead: %zu, file_size: %ld\n", bytesRead, file_size);
        free(buffer);
        return 1;
    }

    buffer[file_size] = '\0';

    JSValue ret_val = JS_Eval(ctx, buffer, file_size, filename, eval_flags);

    if (JS_IsException(ret_val)) {
        printf("Error %s\n", JS_ToCString(ctx, JS_GetException(ctx)));
        JS_FreeValue(ctx, ret_val);
        free(buffer);
        return 1;
    }

    JS_FreeValue(ctx, ret_val);
    free(buffer);

    return 0;
}

int main(int argc, char **argv) {
    JSRuntime *rt = JS_NewRuntime();
    JSContext *ctx = JS_NewContext(rt);

    /* system modules */
    js_init_module_std(ctx, "std");

    JS_SetMaxStackSize(rt, 864 * 1024); // 864 KB

    Execute(ctx, "octane/base.js", JS_EVAL_TYPE_GLOBAL);
    Execute(ctx, "octane/richards", JS_EVAL_TYPE_GLOBAL);
    Execute(ctx, "octane/deltablue.js", JS_EVAL_TYPE_GLOBAL);
    Execute(ctx, "octane/crypto.js", JS_EVAL_TYPE_GLOBAL);
    Execute(ctx, "octane/raytrace.js", JS_EVAL_TYPE_GLOBAL);
    Execute(ctx, "octane/earley-boyer.js", JS_EVAL_TYPE_GLOBAL);
    Execute(ctx, "octane/regexp.js", JS_EVAL_TYPE_GLOBAL);
    Execute(ctx, "octane/splay.js", JS_EVAL_TYPE_GLOBAL);
    Execute(ctx, "octane/navier-stokes.js", JS_EVAL_TYPE_GLOBAL);
    Execute(ctx, "octane/pdfjs.js", JS_EVAL_TYPE_GLOBAL);
    Execute(ctx, "octane/mandreel.js", JS_EVAL_TYPE_GLOBAL);
    Execute(ctx, "octane/gbemu-part1.js", JS_EVAL_TYPE_GLOBAL);
    Execute(ctx, "octane/gbemu-part2.js", JS_EVAL_TYPE_GLOBAL);
    Execute(ctx, "octane/code-load.js", JS_EVAL_TYPE_GLOBAL);
    Execute(ctx, "octane/box2d.js", JS_EVAL_TYPE_GLOBAL);
    Execute(ctx, "octane/zlib.js", JS_EVAL_TYPE_GLOBAL);
    Execute(ctx, "octane/zlib-data.js", JS_EVAL_TYPE_GLOBAL);
    Execute(ctx, "octane/typescript.js", JS_EVAL_TYPE_GLOBAL);
    Execute(ctx, "octane/typescript-input.js", JS_EVAL_TYPE_GLOBAL);
    Execute(ctx, "octane/typescript-compiler.js", JS_EVAL_TYPE_GLOBAL);

    Execute(ctx, "run_octane.js", JS_EVAL_TYPE_MODULE);

    JS_FreeContext(ctx);

    return 0;
}
