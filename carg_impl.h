#pragma once
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h> // NOLINT
#include <stdarg.h> // NOLINT
#include <inttypes.h>
#define MEM_DEBUG_DISABLE

#include "memdebug.h"

#ifdef _MSC_VER
    #define strtok_r strtok_s
#endif

CargContext *cargDefaultContext;

inline CargArgContainer *carg_arg_create(void *argVarPtr, const size_t varSize, const uint64_t flagsArg, const char usageStringArg[]) {
    internal_carg_flag_conditional(CARG_INITIALIZED, true, "Attempt to initialize argument before library initialization. Please fix this!\n");
    CargArgContainer *constructedArgument = (CargArgContainer *)_malloc(sizeof(*constructedArgument));
    const CargArgContainer constructedArgumentInternal = {
        .valueContainer = {
            .next = NULL,
            .value = argVarPtr
        },
        .multiArgIndex = 0,
        .valueSize = varSize,
        .hasValue = false,
        .argvIndexFound = -1,
        .flags = flagsArg,
        .usageString = usageStringArg,
        .formatterUsed = {0},
        .nestedArgString = "",
        .nestedArgFillIndex = -1,
        .nestedArgArraySize = 0,
        .parentArg = NULL,
        .nestedArgs = NULL
    };
    memcpy(constructedArgument, &constructedArgumentInternal, sizeof(CargArgContainer));
    if (HAS_FLAG(flagsArg, CARG_ITEM_POSITIONAL)) cargDefaultContext -> cargPositionalArgCount++;
    if (cargDefaultContext -> internal_cargAllArgs.array) {
        cargDefaultContext -> internal_cargAllArgs.fillIndex++;
        if (cargDefaultContext -> internal_cargAllArgs.fillIndex >= (int)(cargDefaultContext -> internal_cargAllArgs.size / 2)){
            cargDefaultContext -> internal_cargAllArgs.size *= 2;
            CargArgContainer **CargArgArrayReallocation = (CargArgContainer **)_realloc(cargDefaultContext -> internal_cargAllArgs.array, cargDefaultContext -> internal_cargAllArgs.size * sizeof(*CargArgArrayReallocation));
            internal_carg_heap_check(CargArgArrayReallocation);
            cargDefaultContext -> internal_cargAllArgs.array = CargArgArrayReallocation;
        }
        cargDefaultContext -> internal_cargAllArgs.array[cargDefaultContext -> internal_cargAllArgs.fillIndex] = constructedArgument;
    } else {
        cargDefaultContext -> internal_cargAllArgs.array = (CargArgContainer **)_malloc(sizeof(*cargDefaultContext -> internal_cargAllArgs.array) * 4);
        internal_carg_heap_check(cargDefaultContext -> internal_cargAllArgs.array);
        cargDefaultContext -> internal_cargAllArgs.array[0] = constructedArgument;
        cargDefaultContext -> internal_cargAllArgs.fillIndex++;
        cargDefaultContext -> internal_cargAllArgs.size = 4;
    }
    return constructedArgument;
}

inline char *carg_string_contains_substr(char *container, const char * const substr) {
    while (strlen(container) >= strlen(substr)) {
        if (!strncmp(container, substr, strlen(substr))) {
            return container;
        }
        container++;
    }
    return NULL;
}

inline int carg_string_contains_char(const char * const container, const char subchar) {
    if (!container) return -1;
    for (int i=0; i<strlen(container); i++) {
        if (container[i] == subchar) {
            return i;
        }
    }
    return -1;
}

inline const char *carg_basename(const char * const pathStart) {
    const char * const pathEnd = pathStart + strlen(pathStart);
    const char *result = pathEnd;
    while (result > pathStart && *result != '/' && *result != '\\') {
        result--;
    }
    if (result < pathEnd && (result[0] == '\\' || result[0] == '/')) {
        result++;
    }
    return result;
}

inline int internal_carg_test_printf(char *formatter, ...) {
    va_list args;
    va_start(args, formatter);
    const int returnValue = vsnprintf(NULL, 0, formatter, args);
    va_end(args);
    return returnValue;
}

int internal_carg_test_vsnprintf(const char *formatter, va_list args) { // NOLINT
    const int returnValue = vsnprintf(NULL, 0, formatter, args);
    return returnValue;
}

inline int internal_carg_secure_sprintf_concat(char * const startPointer, char * const endPointer, char **cursor, const char * const formatter, ...) {
    if (startPointer > endPointer) return 0;
    if (*endPointer) {
        internal_carg_error("endPointer 0x%" PRIxPTR " does not point to a null terminator.\n", (uintptr_t)endPointer);
    }
    va_list args;
    va_start(args, formatter);
    const int returnValue = vsnprintf(startPointer, endPointer - startPointer, formatter, args);
    if (*cursor + returnValue > endPointer) {
        *cursor = endPointer;
    } else {
        *cursor += returnValue;
    }
    *(endPointer - 1) = '\0';
    va_end(args);
    return returnValue;
}

inline int internal_carg_secure_vsprintf_concat(char * const startPointer, char * const endPointer, char **cursor, const char *formatter, va_list argsToCopy) {
    if (startPointer > endPointer) return 0;
    va_list argsCopy;
    va_copy(argsCopy, argsToCopy);
    const int returnValue = vsnprintf(startPointer, endPointer - startPointer, formatter, argsCopy);
    if (*cursor + returnValue > endPointer) {
        *cursor = endPointer;
    } else {
        *cursor += returnValue;
    }
    *(endPointer - 1) = '\0';
    va_end(argsCopy);
    return returnValue;
}

inline char *internal_carg_strdup(const char *str) {
    const size_t size = strlen(str);
    char *returnVal = (char *)_malloc(sizeof(*returnVal) * (size + 1));
    strncpy(returnVal, str, size);
    returnVal[size] = '\0';
    return returnVal;
}

inline void carg_set_usage_message(const char * const format, ...) {
    va_list args;
    va_start(args, format);
    internal_carg_flag_conditional(CARG_USAGE_MESSAGE_SET, false, "Usage message set by user twice. Please fix this!\n");
    internal_carg_secure_vsprintf_concat(cargDefaultContext -> internal_cargUsageStringCursor, cargDefaultContext -> internal_cargUsageStringEnd, &cargDefaultContext -> internal_cargUsageStringCursor, format, args);
    SET_FLAG(cargDefaultContext -> internal_cargInternalFlags, CARG_USAGE_MESSAGE_SET);
    va_end(args);
}

inline void carg_usage_message_autogen(void) {
    internal_carg_flag_conditional(CARG_INITIALIZED, true, "Usage message auto-generated before library initialization. Please fix this!\n");
    internal_carg_flag_conditional(CARG_USAGE_MESSAGE_SET, false, "Usage message set by user twice. Please fix this!\n");
    internal_carg_secure_sprintf_concat(cargDefaultContext -> internal_cargUsageStringCursor, cargDefaultContext -> internal_cargUsageStringEnd, &cargDefaultContext -> internal_cargUsageStringCursor, "%s%s ", "Usage: ", carg_basename(cargDefaultContext -> internal_cargArgVector[0]));
    internal_carg_print_positional_usage_buffer();
    internal_carg_print_non_positional_usage_buffer();
    SET_FLAG(cargDefaultContext -> internal_cargInternalFlags, CARG_USAGE_MESSAGE_SET);
}

inline void carg_set_usage_function(const CargCallbackFunc usageFunc) {
    internal_carg_flag_conditional(CARG_USAGE_MESSAGE_SET, false, "Usage message set by user twice. Please fix this!\n");
    cargDefaultContext -> internal_carg_usage_ptr = usageFunc;
    SET_FLAG(cargDefaultContext -> internal_cargInternalFlags, CARG_USAGE_MESSAGE_SET);
}

inline void carg_usage(void) {
    cargDefaultContext -> internal_carg_usage_ptr();
    carg_terminate();
    exit(EXIT_SUCCESS);
}

inline bool carg_required(const CargArgContainer *arg) {
    return arg -> hasValue;
}

inline bool carg_mutually_exclusive(const CargArgContainer *arg1, const CargArgContainer *arg2) {
    return !(arg1 -> hasValue && arg2 -> hasValue);
}

inline bool carg_mutually_required(const CargArgContainer *arg1, const CargArgContainer *arg2) {
    return arg1 -> hasValue ? arg2 -> hasValue : 1;
}

inline void carg_init(const int argc, char **argv) {
    if (cargDefaultContext) {
        internal_carg_error("carg initialized multiple times consecutively without termination.");
    }
    cargDefaultContext = (CargContext*)_malloc(sizeof(CargContext));
    CargContext cargContextConstructor = {
        .cargArgCount                   = argc,
        .cargPositionalArgCount         = 0,
        .internal_cargSetArgs           = (int *)_calloc(sizeof(int), argc),
        .internal_cargArgVector         = (char **)_calloc(sizeof(char *), argc),
        .internal_cargAllArgs           = {.size = 0, .fillIndex = -1, .array = NULL},
        .internal_cargUsageString       = (char *)_calloc(sizeof(char), CARG_USAGE_STRING_SIZE),
        .internal_cargUsageStringCursor = NULL,
        .internal_cargUsageStringEnd    = NULL,
        .internal_cargInternalFlags     = 0,
        .internal_carg_usage_ptr        = internal_carg_usage_default
    };
    cargContextConstructor.internal_cargUsageStringCursor = cargContextConstructor.internal_cargUsageString;
    cargContextConstructor.internal_cargUsageStringEnd = cargContextConstructor.internal_cargUsageString + CARG_USAGE_STRING_SIZE - 1;
    internal_carg_heap_check(cargDefaultContext);
    internal_carg_heap_check(cargContextConstructor.internal_cargArgVector);
    internal_carg_heap_check(cargContextConstructor.internal_cargSetArgs);
    internal_carg_heap_check(cargContextConstructor.internal_cargUsageString);
    strncpy(cargContextConstructor.internal_cargUsageString, "Please specify a usage message in your client code. You can do this via carg_set_usage_message() or carg_usage_message_autogen().", CARG_USAGE_STRING_SIZE);
    memcpy(cargDefaultContext, &cargContextConstructor, sizeof(CargContext));
    for (int i=0; i<cargDefaultContext -> cargArgCount; i++) {
        char *allocation = internal_carg_strdup(argv[i]);
        internal_carg_heap_check(allocation);
        cargDefaultContext -> internal_cargArgVector[i] = allocation;
    }

    internal_carg_heap_check(cargDefaultContext -> internal_cargSetArgs);
    SET_FLAG(cargDefaultContext -> internal_cargInternalFlags, CARG_INITIALIZED);
}

inline void carg_heap_default_value(const CargArgContainer *heapArg, const void *val, const size_t bytes) {
    if (!HAS_FLAG(heapArg -> flags, CARG_ITEM_HEAP_ALLOCATED)) {
        internal_carg_error("Heap argument default value setter called on non-heap-allocated argument. Please fix this!\n");
    }
    memcpy(heapArg -> valueContainer.value, val, bytes);
}

inline void carg_set_named_args(const char * const format, ...) {
    internal_carg_flag_conditional(CARG_INITIALIZED, true, "Setter called before library initialization. Please fix this!\n");
    internal_carg_flag_conditional(CARG_NAMED_ARGS_SET, false, "Named args initializer called multiple times. Please fix this!\n");
    internal_carg_flag_conditional(CARG_GROUPED_ARGS_SET, false, "Grouped args initializer called before named args initializer. Please fix this!\n");
    internal_carg_flag_conditional(CARG_ASSERTIONS_SET, false, "Assertions set before all arguments were initialized. Please fix this!\n");
    va_list args;
    va_start(args, format);
    for (int i=cargDefaultContext -> cargPositionalArgCount + 1; i<cargDefaultContext -> cargArgCount; i++) {
        internal_carg_reference_named_arg_formatter(i, format, args);
    }
    va_end(args);
    SET_FLAG(cargDefaultContext -> internal_cargInternalFlags, CARG_NAMED_ARGS_SET);
}

inline void carg_set_positional_args(const char * const format, ...) {
    internal_carg_flag_conditional(CARG_INITIALIZED, true, "Setter called before library initialization. Please fix this!\n");
    internal_carg_flag_conditional(CARG_POSITIONAL_ARGS_SET, false, "Positional args initializer called multiple times. Please fix this!\n");
    internal_carg_flag_conditional(CARG_GROUPED_ARGS_SET, false, "Grouped args initializer called before positional args initializer. Please fix this!\n");
    internal_carg_flag_conditional(CARG_ASSERTIONS_SET, false, "Assertions set before all arguments were initialized. Please fix this!\n");
    if (cargDefaultContext -> cargArgCount <= cargDefaultContext -> cargPositionalArgCount) carg_usage();
    void *formatToTokenizeAllocation = internal_carg_strdup(format);
    internal_carg_heap_check(formatToTokenizeAllocation);
    char *tokenSavePointer = NULL;
    char *formatToTokenize = (char *)formatToTokenizeAllocation;
    void *varDataPtr = NULL;
    va_list args;
    va_start(args, format);
    for (int i=1; i<cargDefaultContext -> cargPositionalArgCount+1; i++) {
        CargArgContainer *currentArg = va_arg(args, CargArgContainer *);
        internal_carg_reference_positional_arg_formatter(currentArg, i, &formatToTokenizeAllocation, &formatToTokenize, &tokenSavePointer, &varDataPtr);
    }
    internal_carg_free_nullify(&formatToTokenizeAllocation);
    va_end(args);
    SET_FLAG(cargDefaultContext -> internal_cargInternalFlags, CARG_POSITIONAL_ARGS_SET);
}

inline void carg_set_grouped_boolean_args(const char * const format, ...) {
    internal_carg_flag_conditional(CARG_INITIALIZED, true, "Setter called before library initialization. Please fix this!\n");
    internal_carg_flag_conditional(CARG_GROUPED_ARGS_SET, false, "Grouped args initializer called multiple times. Please fix this!\n");
    internal_carg_flag_conditional(CARG_ASSERTIONS_SET, false, "Assertions set before all arguments were initialized. Please fix this!\n");
    const char prefixChar = format[0];
    const char *noPrefixFormat = format + 1;
    va_list args;
    va_start(args, format);
    bool *varDataPtr = NULL;
    for (int i=1; i<cargDefaultContext -> cargArgCount; i++) {
        if (cargDefaultContext -> internal_cargArgVector[i][0] != prefixChar || cargDefaultContext -> internal_cargSetArgs[i]) continue;
        if (strlen(cargDefaultContext -> internal_cargArgVector[i]) > 1 && cargDefaultContext -> internal_cargArgVector[i][1] == prefixChar) continue;
        for (size_t j=0; j<strlen(noPrefixFormat); j++) {
            internal_carg_reference_grouped_boolean_arg_formatter(i, j, noPrefixFormat, &varDataPtr, args);
        }
    }
    SET_FLAG(cargDefaultContext -> internal_cargInternalFlags, CARG_GROUPED_ARGS_SET);
    va_end(args);
}

inline void carg_set_env_defaults(const char * const format, ...) {
    internal_carg_flag_conditional(CARG_INITIALIZED, true, "Setter called before library initialization. Please fix this!\n");
    va_list args;
    va_start(args, format);
    void *formatTokenAllocation = internal_carg_strdup(format);
    internal_carg_heap_check(formatTokenAllocation);
    char *formatCopy = (char *)formatTokenAllocation;
    char *tokenSavePointer = NULL;
    internal_carg_set_env_defaults(&formatCopy, &tokenSavePointer, &formatTokenAllocation, args);
    va_end(args);
    internal_carg_free_nullify(&formatTokenAllocation);
}

inline void carg_set_nested_args(const int nestedArgumentCount, ...) {
    internal_carg_flag_conditional(CARG_INITIALIZED, true, "Setter called before library initialization. Please fix this!\n");
    internal_carg_flag_conditional(CARG_NESTED_ARGS_SET, false, "Nested args initializer called multiple times. Please fix this!\n");
    internal_carg_flag_conditional(CARG_GROUPED_ARGS_SET, false, "Grouped args initializer called before nested args initializer. Please fix this!\n");
    va_list args;
    va_start(args, nestedArgumentCount);
    for (int x=0; x<nestedArgumentCount; x++) {
        CargArgContainer *argRoot = va_arg(args, CargArgContainer *);
        if (!HAS_FLAG(argRoot -> flags, CARG_ITEM_NESTED)) {
            internal_carg_error("Nested flag setter called on non-nested argument. Fix this!\n");
        }
        if (!HAS_FLAG(argRoot -> flags, CARG_ITEM_NESTED_ROOT)) {
            internal_carg_error("Nested flag setter called on non-root nested argument. Fix this!\n");
        }
        if (argRoot -> hasValue) {
            internal_carg_error("Root nested element was set multiple times. Fix this!\n");
        }
        const CargArgContainer *argCursor = argRoot;
        if (!internal_carg_set_nested_arg(argRoot)) continue;
        for (int i=0; i <= argCursor -> nestedArgFillIndex; i++) {
            if (internal_carg_set_nested_arg(argCursor -> nestedArgs[i])) {
                argCursor = argCursor -> nestedArgs[i];
                i = -1;
            }
        }
    }
    SET_FLAG(cargDefaultContext -> internal_cargInternalFlags, CARG_NESTED_ARGS_SET);
}

inline CargArgContainer *carg_nested_boolean_container_create(CargArgContainer *arg, const char *nestedArgString, const uint64_t flagsArg) {
    if (!HAS_FLAG(arg -> flags, CARG_ITEM_BOOLEAN)) {
        internal_carg_error("Boolean nested argument initializer called on non-boolean flag. Fix this!\n");
    }
    SET_FLAG(arg -> flags, CARG_ITEM_NESTED | CARG_ITEM_NESTED_ROOT | flagsArg);
    arg -> nestedArgString = nestedArgString;
    return arg;
}

inline CargArgContainer *carg_nest_boolean_container(CargArgContainer *nestIn, CargArgContainer *argToNest, const char *nestedArgString) {
    if (!HAS_FLAG(nestIn -> flags, CARG_ITEM_BOOLEAN) || !HAS_FLAG(argToNest -> flags, CARG_ITEM_BOOLEAN)) {
        internal_carg_error("Only boolean arguments can be nested with this nesting function. Fix this!\n");
    }
    if (nestIn -> nestedArgs && nestIn -> nestedArgFillIndex >= (int)nestIn -> nestedArgArraySize / 2) {
        nestIn -> nestedArgArraySize *= 2;
        nestIn -> nestedArgs = (CargArgContainer **)_realloc(nestIn -> nestedArgs, nestIn -> nestedArgArraySize * sizeof(*nestIn -> nestedArgs));
    } else if (!nestIn -> nestedArgs) {
        nestIn -> nestedArgs = (CargArgContainer **)_calloc(4, sizeof(*nestIn -> nestedArgs));
        nestIn -> nestedArgArraySize = 4;
    }
    internal_carg_heap_check(nestIn -> nestedArgs);
    argToNest -> nestedArgString = nestedArgString;
    SET_FLAG(argToNest -> flags, CARG_ITEM_NESTED);
    nestIn -> nestedArgs[++nestIn -> nestedArgFillIndex] = argToNest;
    argToNest -> parentArg = nestIn;
    return argToNest;
}

inline CargArgContainer *carg_nested_container_create(CargArgContainer *arg, const char *nestedArgString, const uint64_t flagsArg, const char * const format) {
    if (HAS_FLAG(arg -> flags, CARG_ITEM_BOOLEAN)) {
        internal_carg_error("Non-boolean nested argument initializer called on boolean flag. Fix this!\n");
    }
    strncpy(arg -> formatterUsed, format, sizeof(arg -> formatterUsed) - 1);
    SET_FLAG(arg -> flags, CARG_ITEM_NESTED | CARG_ITEM_NESTED_ROOT | flagsArg);
    arg -> nestedArgString = nestedArgString;
    return arg;
}

inline CargArgContainer *carg_nest_container(CargArgContainer *nestIn, CargArgContainer *argToNest, const char *nestedArgString, const char * const format) {
    if (HAS_FLAG(argToNest -> flags, CARG_ITEM_BOOLEAN)) {
        internal_carg_error("Only non-boolean arguments can be nested with this nesting function. Fix this!\n");
    }
    if (nestIn -> nestedArgs && nestIn -> nestedArgFillIndex >= (int)nestIn -> nestedArgArraySize / 2) {
        nestIn -> nestedArgArraySize *= 2;
        nestIn -> nestedArgs = (CargArgContainer **)_realloc(nestIn -> nestedArgs, nestIn -> nestedArgArraySize * sizeof(*nestIn -> nestedArgs));
    } else if (!nestIn -> nestedArgs) {
        nestIn -> nestedArgs = (CargArgContainer **)_calloc(4, sizeof(*nestIn -> nestedArgs));
        nestIn -> nestedArgArraySize = 4;
    }
    internal_carg_heap_check(nestIn -> nestedArgs);
    argToNest -> nestedArgString = nestedArgString;
    strncpy(argToNest -> formatterUsed, format, sizeof(argToNest -> formatterUsed) - 1);
    SET_FLAG(argToNest -> flags, CARG_ITEM_NESTED);
    nestIn -> nestedArgs[++nestIn -> nestedArgFillIndex] = argToNest;
    argToNest -> parentArg = nestIn;
    return argToNest;
}

inline void carg_override_callbacks(const char * const format, ...) {
    internal_carg_flag_conditional(CARG_INITIALIZED, true, "Argument override called before library initialization. Please fix this!\n");
    internal_carg_flag_conditional(CARG_OVERRIDE_CALLBACKS_SET, false, "Override callback args initializer called multiple times. Please fix this!\n");
    internal_carg_flag_conditional(CARG_ASSERTIONS_SET | CARG_NAMED_ARGS_SET | CARG_POSITIONAL_ARGS_SET | CARG_GROUPED_ARGS_SET | CARG_NESTED_ARGS_SET, false, "Callback override initialized after arguments were set. Fix this!\n");
    internal_carg_flag_conditional(CARG_ASSERTIONS_SET, false, "Assertions set before all arguments were initialized. Please fix this!\n");
    if (cargDefaultContext -> cargArgCount < 2) return;
    char *formatToTokenize = internal_carg_strtok_string_init(format);
    internal_carg_heap_check(formatToTokenize);
    const char *currentFlag = NULL;
    CargCallbackFunc functionCursor = NULL;
    va_list args;
    va_list argsCopy;
    va_start(args, format);
    va_copy(argsCopy, args);
    for (int i=1; i<cargDefaultContext -> cargArgCount; i++) {
        internal_carg_strtok_register_string(formatToTokenize);
        while ((currentFlag = strtok(NULL, " "))) {
            functionCursor = va_arg(argsCopy, CargCallbackFunc);
            if (internal_carg_cmp_flag(currentFlag, cargDefaultContext -> internal_cargArgVector[i])) {
                functionCursor();
                internal_carg_free_nullify(&formatToTokenize);
                carg_terminate();
                exit(EXIT_SUCCESS);
            }
        }
        internal_carg_free_nullify(&formatToTokenize);
        va_copy(argsCopy, args);
        formatToTokenize = internal_carg_strtok_string_init(format);
        internal_carg_heap_check(formatToTokenize);
    }
    internal_carg_free_nullify(&formatToTokenize);
    va_end(args);
    SET_FLAG(cargDefaultContext -> internal_cargInternalFlags, CARG_OVERRIDE_CALLBACKS_SET);
}

inline void carg_arg_assert(const int assertionCount, ...) {
    internal_carg_flag_conditional(CARG_ASSERTIONS_SET, false, "Assertion args initializer called multiple times. Please fix this!\n");
    va_list args;
    va_start(args, assertionCount);
    for (int i=0; i<assertionCount; i++) {
        const int expression = va_arg(args, int);
        char *message = va_arg(args, char *);
        if (!expression) {
            if (message) {
                printf("%s\n", message);
                carg_terminate();
                exit(EXIT_SUCCESS);
            }
            va_end(args);
            carg_usage();
        }
    }
    va_end(args);
    SET_FLAG(cargDefaultContext -> internal_cargInternalFlags, CARG_ASSERTIONS_SET);
}

inline void carg_print_container_data(const CargArgContainer *container) {
    printf("Argument: \n");
    printf("\tSize: %zu\n", container -> valueSize);
    printf("\tFlags: %" PRIu64 "\n", container -> flags);
    printf("\tFound At: %d\n", container -> argvIndexFound);
    printf("\tHas Value: %d\n", container -> hasValue);
    if (container -> usageString[0]) printf("\tUsage String: %s\n", container -> usageString);
    printf("\tFormatter Used: %s\n", container -> formatterUsed);
    if (container -> nestedArgString[0]) printf("\tNested Argument String: %s\n", container -> nestedArgString);
}

inline void *carg_fetch_multi_arg_entry(const CargArgContainer *container, const int index) {
    if (container -> multiArgIndex < index) {
        internal_carg_error("%d is past the bounds of the multi argument being accessed (max index is %d)\n", index, container -> multiArgIndex);
    }
    const CargMultiArgContainer *cursor = &container -> valueContainer;
    for (int i=0; i<index; i++) {
        cursor = cursor -> next;
    }
    return cursor -> value;
}

inline void carg_validate(void) {
    internal_carg_flag_conditional(CARG_ASSERTIONS_SET | CARG_NAMED_ARGS_SET | CARG_POSITIONAL_ARGS_SET | CARG_GROUPED_ARGS_SET | CARG_NESTED_ARGS_SET, true, "Argument validator called before arguments were set. Fix this!\n");
    bool errorFound = false;
    for (int i=1; i<cargDefaultContext -> cargArgCount; i++) {
        if (!cargDefaultContext -> internal_cargSetArgs[i]) {
            fprintf(stderr, "Error: Unknown option \"%s\"\n", cargDefaultContext -> internal_cargArgVector[i]);
            errorFound = true;
        }
    }
    for (int i=0; i<=cargDefaultContext -> internal_cargAllArgs.fillIndex; i++) {
        const CargArgContainer *arg = cargDefaultContext -> internal_cargAllArgs.array[i];
        if (!arg -> parentArg || !arg -> hasValue) continue;
        if (!HAS_FLAG(arg -> parentArg -> flags, CARG_ITEM_BOOLEAN) && arg -> parentArg -> argvIndexFound == arg -> argvIndexFound - 1)
            carg_usage();
        if (HAS_FLAG(arg -> flags, CARG_ITEM_ENFORCE_NESTING_ORDER) && arg -> parentArg -> argvIndexFound >= arg -> argvIndexFound)
            carg_usage();
        if (HAS_FLAG(arg-> flags, CARG_ITEM_ENFORCE_STRICT_NESTING_ORDER) && arg -> parentArg -> argvIndexFound != arg -> argvIndexFound - 1)
            carg_usage();
    }
    if (errorFound) {
        carg_terminate();
        exit(EXIT_SUCCESS);
    }
}

inline void carg_terminate(void) {
    if (HAS_FLAG(cargDefaultContext -> internal_cargInternalFlags, CARG_INITIALIZED)) {
        if (cargDefaultContext -> internal_cargAllArgs.array) {
            for (int i=0; i<=cargDefaultContext -> internal_cargAllArgs.fillIndex; i++) {
                if (cargDefaultContext -> internal_cargAllArgs.array[i] -> valueContainer.value && HAS_FLAG(cargDefaultContext -> internal_cargAllArgs.array[i] -> flags, CARG_ITEM_HEAP_ALLOCATED)) {
                    internal_carg_free_nullify(&cargDefaultContext -> internal_cargAllArgs.array[i] -> valueContainer.value);
                }
                if (cargDefaultContext -> internal_cargAllArgs.array[i] -> nestedArgs) {
                    internal_carg_free_nullify(&cargDefaultContext -> internal_cargAllArgs.array[i] -> nestedArgs);
                }
                if (cargDefaultContext -> internal_cargAllArgs.array[i] -> valueContainer.next) {
                    internal_carg_free_nullify(&cargDefaultContext -> internal_cargAllArgs.array[i] -> valueContainer.next -> value);
                    CargMultiArgContainer *cursor = cargDefaultContext -> internal_cargAllArgs.array[i] -> valueContainer.next -> next;
                    CargMultiArgContainer *cursorToFree = cargDefaultContext -> internal_cargAllArgs.array[i] -> valueContainer.next;
                    internal_carg_free_nullify(&cursorToFree);
                    while (cursor) {
                        internal_carg_free_nullify(&cursor -> value);
                        cursorToFree = cursor;
                        cursor = cursor -> next;
                        internal_carg_free_nullify(&cursorToFree);
                    }
                }
                free(cargDefaultContext -> internal_cargAllArgs.array[i]);
            }
            internal_carg_free_nullify(&cargDefaultContext -> internal_cargAllArgs.array);
        }
        internal_carg_free_nullify(&cargDefaultContext -> internal_cargSetArgs);
        if (cargDefaultContext -> internal_cargArgVector) {
            for (int i=0; i<cargDefaultContext -> cargArgCount; i++) {
                internal_carg_free_nullify(&cargDefaultContext -> internal_cargArgVector[i]);
            }
            internal_carg_free_nullify(&cargDefaultContext -> internal_cargArgVector);
        }
    }
    internal_carg_free_nullify(&cargDefaultContext -> internal_cargUsageString);
    internal_carg_free_nullify(&cargDefaultContext);
}

inline void internal_carg_flag_conditional(const uint64_t flag, const bool truthiness, const char * const errorMessage) {
    if ((bool)HAS_FLAG(cargDefaultContext -> internal_cargInternalFlags, flag) != truthiness) {
        internal_carg_error(errorMessage);
    }
}

inline void internal_carg_error(const char * const formatter, ...) {
    va_list args;
    va_start(args, formatter);
    fprintf(stderr, "cargError: ");
    vfprintf(stderr, formatter, args);
    va_end(args);
    carg_terminate();
    exit(EXIT_FAILURE);
}

inline void internal_carg_heap_check(void *ptr) {
    if (!ptr) {
        printf("Heap allocation failure. Terminating\n");
        carg_terminate();
        exit(EXIT_FAILURE);
    }
}

inline void internal_carg_free_nullify(void *ptr) {
    if (*(void **)ptr) {
        free(*(void **)ptr);
        *(void **)ptr = NULL;
    }
}

inline char *internal_carg_strtok_string_init(const char * const str) {
    char *returnVal = (char *)_calloc(strlen(str) + 4, sizeof(*returnVal));
    strcpy(returnVal, str);
    memmove(returnVal + 3, returnVal, strlen(returnVal));
    memcpy(returnVal, "\x03\x01\x03", 3);
    return returnVal;
}

inline void internal_carg_strtok_register_string(char *str) {
    strtok(str, "\x03");
}

inline int internal_carg_cmp_flag(const char * const argument, const char * const parameter) {
    return !strcmp(argument, parameter);
}

inline int internal_carg_is_flag(const char * const formatter, const char * const toCheck) {
    char *formatToTokenize = internal_carg_strtok_string_init(formatter);
    internal_carg_heap_check(formatToTokenize);
    internal_carg_strtok_register_string(formatToTokenize);
    while (1) {
        char *flagToken = strtok(NULL, ": ");
        strtok(NULL, ": "); // Discard formatter item
        if (!flagToken) {
            break;
        }
        if (internal_carg_cmp_flag(toCheck, flagToken)) {
            internal_carg_free_nullify(&formatToTokenize);
            return 1;
        }
    }
    internal_carg_free_nullify(&formatToTokenize);
    return 0;
}

inline void internal_carg_usage_default(void) {
    printf("%s\n", cargDefaultContext -> internal_cargUsageString);
}

inline bool internal_carg_adjust_multi_arg_setter(CargArgContainer *currentArg, void **varDataPtr) {
    if (HAS_FLAG(currentArg -> flags, CARG_ITEM_MULTI) && currentArg -> hasValue) {
        CargMultiArgContainer *multiArgCursor = &currentArg->valueContainer;
        while (multiArgCursor -> next) {
            multiArgCursor = multiArgCursor -> next;
        }
        multiArgCursor -> next = (CargMultiArgContainer *)_malloc(sizeof(*multiArgCursor -> next));
        internal_carg_heap_check(multiArgCursor -> next);
        multiArgCursor -> next -> next = NULL;
        multiArgCursor -> next -> value = _malloc(currentArg -> valueSize);
        internal_carg_heap_check(multiArgCursor -> next -> value);
        *varDataPtr = multiArgCursor -> next -> value;
        currentArg -> multiArgIndex++;
        return true;
    }
    return false;
}

inline void internal_carg_set_named_arg(CargArgContainer *currentArg, void **varDataPtr, const int argIndex, const char *formatToken, const char *argToFormat, char **formatToTokenize, char **argumentFlagToCompare) {
    if (!currentArg) return;
    if (currentArg -> hasValue && !HAS_FLAG(currentArg -> flags, CARG_ITEM_MULTI)) {
        carg_usage();
    }
    if (!internal_carg_adjust_multi_arg_setter(currentArg, varDataPtr)) {
        *varDataPtr = currentArg -> valueContainer.value;
    }
    if (!*varDataPtr) return;
    if (internal_carg_cmp_flag(formatToken, "bool")) {
        if (!HAS_FLAG(currentArg -> flags, CARG_ITEM_BOOLEAN)) {
            internal_carg_error("Argument struct does not contain the CARG_ITEM_BOOLEAN flag; argument items should be initialized with this flag for readability.\n");
        }
        *(bool *)*varDataPtr = !*(bool *)*varDataPtr; // Flip flag from its default value. Boolean flags are expected to be chars with a default value.
        currentArg -> hasValue = 1;
        cargDefaultContext -> internal_cargSetArgs[argIndex] = currentArg -> hasValue;
    } else {
        if (argIndex >= cargDefaultContext -> cargArgCount - 1 && !(carg_string_contains_char(cargDefaultContext -> internal_cargArgVector[argIndex], '=') >= 0 && strcmp(formatToken, "bool"))) carg_usage();
        currentArg -> hasValue = sscanf(argToFormat, formatToken, *varDataPtr); // If an argument is passed in that does not match its formatter, the value remains default.
        cargDefaultContext -> internal_cargSetArgs[argIndex] = currentArg -> hasValue;
        if (!(carg_string_contains_char(cargDefaultContext -> internal_cargArgVector[argIndex], '=') >= 0 && strcmp(formatToken, "bool"))) cargDefaultContext -> internal_cargSetArgs[argIndex + 1] = currentArg -> hasValue;
        if (!currentArg -> hasValue) {
            internal_carg_free_nullify(formatToTokenize);
            internal_carg_free_nullify(argumentFlagToCompare);
            carg_usage();
        }
    }
    currentArg -> argvIndexFound = argIndex;
    if (formatToken) strncpy(currentArg -> formatterUsed, formatToken, CARG_MAX_FORMATTER_SIZE - 1);
}

inline void internal_carg_validate_formatter_extended(const char * const formatToken) {
    if (formatToken && (!internal_carg_cmp_flag(formatToken, "bool") && formatToken[0] != '%' || carg_string_contains_char(formatToken + 1, '%') >= 0)) {
        internal_carg_error("Cannot parse format token %s\n", formatToken);
    }
}

inline void internal_carg_validate_formatter(const char * const formatToken) {
    if (formatToken && (formatToken[0] != '%' || carg_string_contains_char(formatToken + 1, '%') >= 0)) {
        internal_carg_error("Cannot parse format token %s\n", formatToken);
    }
}

inline void internal_carg_validate_flag(const char * const flagToken) {
    if (flagToken && carg_string_contains_char(flagToken, '%') > -1) {
        internal_carg_error("Cannot parse flag token %s\n", flagToken);
    }
}

inline bool internal_carg_adjust_named_assign(const int argIndex, const char * const formatToken, const char * const flagToken, const char **argToFormat, char *argumentFlagToCompare) {
    if (argumentFlagToCompare && carg_string_contains_char(cargDefaultContext -> internal_cargArgVector[argIndex], '=') >= 0 && strcmp(formatToken, "bool")) {
        int ncompare = 0;
        while (cargDefaultContext -> internal_cargArgVector[argIndex][ncompare] != '=') ncompare++;
        if (strncmp(flagToken, cargDefaultContext -> internal_cargArgVector[argIndex], ncompare)) return false;
        argumentFlagToCompare[ncompare] = '\0';
        *argToFormat = cargDefaultContext -> internal_cargArgVector[argIndex]+ncompare+1;
    }
    return true;
}

inline void internal_carg_reference_named_arg_formatter(const int argIndex, const char *format, va_list args) { // NOLINT
    if (cargDefaultContext -> internal_cargSetArgs[argIndex]) return;
    va_list argsCopy;
    va_copy(argsCopy, args);
    char *formatToTokenize = internal_carg_strtok_string_init(format);
    internal_carg_heap_check(formatToTokenize);
    void *varDataPtr = NULL;
    char *argumentFlagToCompare = internal_carg_strdup(cargDefaultContext -> internal_cargArgVector[argIndex]);
    internal_carg_heap_check(argumentFlagToCompare);
    const char *argToFormat;
    if (argIndex < cargDefaultContext -> cargArgCount - 1) argToFormat = cargDefaultContext -> internal_cargArgVector[argIndex + 1];
    else argToFormat = NULL;
    internal_carg_strtok_register_string(formatToTokenize);
    while (1) {
        const char *flagToken = strtok(NULL, ":");
        const char *formatToken = strtok(NULL, " ");
        if (!flagToken) break;
        internal_carg_validate_flag(flagToken);
        internal_carg_validate_formatter_extended(formatToken);
        CargArgContainer *currentArg = va_arg(argsCopy, CargArgContainer *);
        if (!internal_carg_adjust_named_assign(argIndex, formatToken, flagToken, &argToFormat, argumentFlagToCompare)) continue;
        if (!strcmp(flagToken, argumentFlagToCompare)) {
            internal_carg_set_named_arg(currentArg, &varDataPtr, argIndex, formatToken, argToFormat, &formatToTokenize, &argumentFlagToCompare);
            break;
        }
    }
    internal_carg_free_nullify(&formatToTokenize);
    internal_carg_free_nullify(&argumentFlagToCompare);
}

inline void internal_carg_reference_positional_arg_formatter(CargArgContainer *currentArg, const int i, void **formatToTokenizeAllocation, char **formatToTokenize, char **tokenSavePointer, void **varDataPtr) {
    *formatToTokenize = strtok_r(*formatToTokenize, " ", tokenSavePointer);
    if (!HAS_FLAG(currentArg -> flags, CARG_ITEM_POSITIONAL)) {
        internal_carg_error("Positional arg setter called on named argument. Please fix this!\n");
    }
    *varDataPtr = currentArg -> valueContainer.value;
    currentArg -> hasValue = sscanf(cargDefaultContext -> internal_cargArgVector[i], *formatToTokenize, *varDataPtr);
    cargDefaultContext -> internal_cargSetArgs[i] = currentArg -> hasValue;
    if (!currentArg -> hasValue) {
        internal_carg_free_nullify(formatToTokenizeAllocation);
        carg_usage();
    }
    currentArg -> argvIndexFound = i;
    if (*formatToTokenize) {
        strncpy(currentArg -> formatterUsed, *formatToTokenize, CARG_MAX_FORMATTER_SIZE - 1);
    }
    *formatToTokenize = *tokenSavePointer;
}

inline void internal_carg_reference_grouped_boolean_arg_formatter(const int i, const size_t j, const char *noPrefixFormat, bool **varDataPtr, va_list args) {
    va_list argsCopy;
    va_copy(argsCopy, args);
    if (carg_string_contains_char(cargDefaultContext -> internal_cargArgVector[i], noPrefixFormat[j]) >= 0) {
        CargArgContainer *currentArg = NULL;
        for (size_t k=0; k<=j; k++) {
            currentArg = va_arg(argsCopy, CargArgContainer *);
            *varDataPtr = (bool *)currentArg -> valueContainer.value;
        }
        va_end(argsCopy);
        va_copy(argsCopy, args);
        if (*varDataPtr && currentArg) {
            currentArg -> hasValue = 1;
            cargDefaultContext -> internal_cargSetArgs[i] = currentArg -> hasValue;
            currentArg -> argvIndexFound = i;
            **varDataPtr = !**varDataPtr;
        }
    }
    va_end(argsCopy);
}

inline int internal_carg_set_nested_arg(CargArgContainer *currentArg) {
    if (!currentArg) return 0;
    if (!HAS_FLAG(currentArg -> flags, CARG_ITEM_NESTED)) {
        internal_carg_error("Nested flag setter called on non-nested argument. Fix this!\n");
    }
    if (currentArg -> hasValue) return 0;
    for (int i=cargDefaultContext -> cargPositionalArgCount+1; i<cargDefaultContext -> cargArgCount; i++) {
        if (!strcmp(currentArg -> nestedArgString, cargDefaultContext -> internal_cargArgVector[i])) {
            if (HAS_FLAG(currentArg -> flags, CARG_ITEM_BOOLEAN)) {
                *(bool *)currentArg -> valueContainer.value = !*(bool *)currentArg -> valueContainer.value;
                currentArg -> hasValue = 1;
            } else {
                if (i >= cargDefaultContext -> cargArgCount - 1) carg_usage();
                currentArg -> hasValue = sscanf(cargDefaultContext -> internal_cargArgVector[i+1], currentArg -> formatterUsed, currentArg -> valueContainer.value);
                cargDefaultContext -> internal_cargSetArgs[i+1] = currentArg -> hasValue;
            }
            cargDefaultContext -> internal_cargSetArgs[i] = currentArg -> hasValue;
            currentArg -> argvIndexFound = i;
            return 1;
        }
    }
    return 0;
}

inline void internal_carg_set_env_defaults(char **stringToTokenize, char **tokenSavePointer, void **stringAllocation, va_list args) {
    va_list argsCopy;
    va_copy(argsCopy, args);
    CargArgContainer *currentArg = NULL;
    while (1) {
        char *envVarToken = strtok_r(*stringToTokenize, ":", tokenSavePointer);
        char *formatToken = strtok_r(NULL, " ", tokenSavePointer);
        *stringToTokenize = *tokenSavePointer;
        if (!envVarToken || !formatToken) break;
        internal_carg_validate_flag(envVarToken);
        internal_carg_validate_formatter(formatToken);
        const char *envVarValue = getenv(envVarToken);
        currentArg = va_arg(argsCopy, CargArgContainer *);
        if (!envVarValue) continue;
        if (!currentArg -> hasValue) {
            currentArg -> hasValue = sscanf(envVarValue, formatToken, currentArg -> valueContainer.value);
        }
        if (!currentArg -> hasValue) {
            internal_carg_free_nullify(stringAllocation);
            internal_carg_error("Unable to grab environment variable %s\n", envVarToken);
        }
        strncpy(currentArg -> formatterUsed, formatToken, CARG_MAX_FORMATTER_SIZE - 1);
    }
    va_end(argsCopy);
}

inline void internal_carg_print_positional_usage_buffer(void) {
    for (int i=0; i<=cargDefaultContext -> internal_cargAllArgs.fillIndex; i++) {
        if (!cargDefaultContext -> internal_cargAllArgs.array[i]) break;
        if (!HAS_FLAG(cargDefaultContext -> internal_cargAllArgs.array[i] -> flags, CARG_ITEM_POSITIONAL)) continue;
        if (cargDefaultContext -> internal_cargAllArgs.array[i] -> usageString && cargDefaultContext -> internal_cargAllArgs.array[i] -> usageString[0]) internal_carg_secure_sprintf_concat(cargDefaultContext -> internal_cargUsageStringCursor, cargDefaultContext -> internal_cargUsageStringEnd, &cargDefaultContext -> internal_cargUsageStringCursor, "%s ", cargDefaultContext -> internal_cargAllArgs.array[i]->usageString);
        else if (cargDefaultContext -> internal_cargAllArgs.array[i] -> nestedArgString && cargDefaultContext -> internal_cargAllArgs.array[i] -> nestedArgString[0]) internal_carg_secure_sprintf_concat(cargDefaultContext -> internal_cargUsageStringCursor, cargDefaultContext -> internal_cargUsageStringEnd, &cargDefaultContext -> internal_cargUsageStringCursor, "%s ", cargDefaultContext -> internal_cargAllArgs.array[i]->nestedArgString);
    }
}

inline void internal_carg_print_non_positional_usage_buffer(void) {
    for (int i=0; i<=cargDefaultContext -> internal_cargAllArgs.fillIndex; i++) {
        if (HAS_FLAG(cargDefaultContext -> internal_cargAllArgs.array[i] -> flags, CARG_ITEM_BOOLEAN && cargDefaultContext -> internal_cargAllArgs.array[i] -> usageString[0])) {
            internal_carg_secure_sprintf_concat(cargDefaultContext -> internal_cargUsageStringCursor, cargDefaultContext -> internal_cargUsageStringEnd, &cargDefaultContext -> internal_cargUsageStringCursor, "%s ", cargDefaultContext -> internal_cargAllArgs.array[i]->usageString);
        }
    }
    for (int i=0; i<=cargDefaultContext -> internal_cargAllArgs.fillIndex; i++) {
        if (HAS_FLAG(cargDefaultContext -> internal_cargAllArgs.array[i] -> flags, CARG_ITEM_NESTED_ROOT)) {
            internal_carg_secure_sprintf_concat(cargDefaultContext -> internal_cargUsageStringCursor, cargDefaultContext -> internal_cargUsageStringEnd, &cargDefaultContext -> internal_cargUsageStringCursor, "%s ", cargDefaultContext -> internal_cargAllArgs.array[i]->nestedArgString);
        }
    }
    for (int i=0; i<=cargDefaultContext -> internal_cargAllArgs.fillIndex; i++) {
        if (HAS_FLAG(cargDefaultContext -> internal_cargAllArgs.array[i] -> flags, CARG_ITEM_POSITIONAL)) continue;
        if (HAS_FLAG(cargDefaultContext -> internal_cargAllArgs.array[i] -> flags, CARG_ITEM_BOOLEAN) && !cargDefaultContext -> internal_cargAllArgs.array[i] -> nestedArgString[0]) {
            continue;
        }
        if (cargDefaultContext -> internal_cargAllArgs.array[i] -> usageString[0]) {
            internal_carg_secure_sprintf_concat(cargDefaultContext -> internal_cargUsageStringCursor, cargDefaultContext -> internal_cargUsageStringEnd, &cargDefaultContext -> internal_cargUsageStringCursor, "%s ", cargDefaultContext -> internal_cargAllArgs.array[i] -> usageString);
        }
        else if (cargDefaultContext -> internal_cargAllArgs.array[i] -> nestedArgString[0]) {}
        else {}
    }
}

#define CARG_PRINT_STRING_ITEM(argument) do {\
    carg_print_container_data(argument);\
    printf("\tValue: ");\
    if (carg_string_contains_char((argument) -> formatterUsed, '\n') >= 0) printf("%s", (char *)(argument) -> valueContainer.value); /* This allows the string scanf() formatter with spaces to work. */\
    else printf((argument) -> formatterUsed, (char *)(argument) -> valueContainer.value);\
    printf("\n");\
} while (0)

#define CARG_PRINT_NON_STRING_ITEM(argument, typeArg) do {\
    carg_print_container_data(argument);\
    printf("\tValue: ");\
    printf((argument) -> formatterUsed, *(typeArg *)(argument) -> valueContainer.value);\
    printf("\n");\
} while (0)

#define CARG_PRINT_NON_STRING_MULTI_ITEM(argument, typeArg) do {\
    CARG_PRINT_NON_STRING_ITEM(argument, typeArg);\
    if (!(argument) -> formatterUsed[0]) break;\
    CargMultiArgContainer *cursor = argument -> valueContainer.next;\
    while (cursor) {\
        printf("\tValue: ");\
        printf((argument) -> formatterUsed, *(typeArg *)cursor -> value);\
        printf("\n");\
        cursor = cursor -> next;\
    }\
} while (0)

#define CARG_PRINT_STRING_MULTI_ITEM(argument) do {\
    CARG_PRINT_STRING_ITEM(argument);\
    if (!(argument) -> formatterUsed[0]) break;\
    CargMultiArgContainer *cursor = argument -> valueContainer.next;\
    while (cursor) {\
        printf("\tValue: ");\
        if (carg_string_contains_char((argument) -> formatterUsed, '\n') >= 0) printf("%s", (cursor -> value)); /* This allows the string scanf() formatter with spaces to work. */\
        else printf((argument) -> formatterUsed, cursor -> value);\
        printf("\n");\
        cursor = cursor -> next;\
    }\
} while (0)

// Reset macro definitions to not interfere with other included libraries.
#ifdef _MSC_VER
    #undef strtok_r
    #undef _CRT_SECURE_NO_WARNINGS
#endif