#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h> 
#include <stdarg.h>
#include <inttypes.h>
#define CARG_IMPL
#include "carg.h"

CARG_EXPORT CargContext *cargDefaultContext;

inline int carg_string_contains_char(const char * const container, const char subchar) {
    if (!container) return -1;
    const size_t len = strlen(container);
    for (int i=0; i<len; i++) {
        if (container[i] == subchar) {
            return i;
        }
    }
    return -1;
}

static char *internal_carg_strtok_reentrant(char *str, const char *delim, char **savePtr) {
    if (!savePtr) return NULL;
    if (!str) str = *savePtr;
    if (!str || !*str || !delim) return NULL;
    while (*str && carg_string_contains_char(delim, *str) != -1) {
        str++;
    }
    char *ret = str;
    while (*str && carg_string_contains_char(delim, *str) == -1) {
        str++;
    }
    if (*str) {
        *str = '\0';
        *savePtr = ++str;
    } else {
        *savePtr = NULL;
    }
    if (*ret) {
        return ret;
    }
    return NULL;
}

static void internal_carg_heap_check(const void *ptr) {
    if (!ptr) {
        printf("Heap allocation failure. Terminating\n");
        carg_terminate();
        exit(EXIT_FAILURE);
    }
}

static int internal_carg_cmp_flag(const char * const argument, const char * const parameter) {
    return !strcmp(argument, parameter);
}

inline void carg_validate_context(const CargContext *cargLocalContext) {
    if (!cargLocalContext) {
        internal_carg_error("carg context not initialized!");
    }
}

inline CargArgContainer *carg_arg_create_ts(CargContext *cargLocalContext, void *argVarPtr, const size_t varSize, const uint64_t flagsArg, const char usageStringArg[]) {
    carg_validate_context(cargLocalContext);
    internal_carg_flag_conditional_ts(cargLocalContext, CARG_INITIALIZED, true, "Attempt to initialize argument before library initialization. Please fix this!\n");
    CargArgContainer *constructedArgument = (CargArgContainer *)malloc(sizeof(*constructedArgument));
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
    if (internal_carg_has_flag(flagsArg, CARG_ITEM_POSITIONAL)) cargLocalContext -> cargPositionalArgCount++;
    if (cargLocalContext -> internal_cargAllArgs.array) {
        cargLocalContext -> internal_cargAllArgs.fillIndex++;
        if (cargLocalContext -> internal_cargAllArgs.fillIndex >= (int)(cargLocalContext -> internal_cargAllArgs.size / 2)){
            cargLocalContext -> internal_cargAllArgs.size *= 2;
            CargArgContainer **CargArgArrayReallocation = (CargArgContainer **)realloc(cargLocalContext -> internal_cargAllArgs.array, cargLocalContext -> internal_cargAllArgs.size * sizeof(*CargArgArrayReallocation));
            internal_carg_heap_check(CargArgArrayReallocation);
            cargLocalContext -> internal_cargAllArgs.array = CargArgArrayReallocation;
        }
        cargLocalContext -> internal_cargAllArgs.array[cargLocalContext -> internal_cargAllArgs.fillIndex] = constructedArgument;
    } else {
        cargLocalContext -> internal_cargAllArgs.array = (CargArgContainer **)malloc(sizeof(*cargLocalContext -> internal_cargAllArgs.array) * 4);
        internal_carg_heap_check(cargLocalContext -> internal_cargAllArgs.array);
        cargLocalContext -> internal_cargAllArgs.array[0] = constructedArgument;
        cargLocalContext -> internal_cargAllArgs.fillIndex++;
        cargLocalContext -> internal_cargAllArgs.size = 4;
    }
    return constructedArgument;
}

inline CargArgContainer *carg_arg_create(void *argVarPtr, const size_t varSize, const uint64_t flagsArg, const char usageStringArg[]) {
    return carg_arg_create_ts(cargDefaultContext, argVarPtr, varSize, flagsArg, usageStringArg);
}

inline const char *carg_string_contains_substr(const char *container, const char * const substr) {
    if (!container || !substr) return NULL;
    const size_t substrlen = strlen(substr);
    size_t containerlen = strlen(container);
    while (containerlen >= substrlen) {
        if (!strncmp(container, substr, substrlen)) {
            return container;
        }
        container++;
        containerlen--;
    }
    return NULL;
}

inline const char *carg_basename(const char * const container) {
    const char * const pathEnd = container + strlen(container);
    const char *result = pathEnd;
    while (result > container && *result != '/' && *result != '\\') {
        result--;
    }
    if (result < pathEnd && (result[0] == '\\' || result[0] == '/')) {
        result++;
    }
    return result;
}

//  For future use.
// static int internal_carg_test_printf(const char *formatter, ...) {
//     va_list args;
//     va_start(args, formatter);
//     const int returnValue = vsnprintf(NULL, 0, formatter, args);
//     va_end(args);
//     return returnValue;
// }
//
// static int internal_carg_test_vsnprintf(const char *formatter, va_list args) {
//     const int returnValue = vsnprintf(NULL, 0, formatter, args);
//     return returnValue;
// }

static int internal_carg_secure_sprintf_concat(char * const startPointer, char * const endPointer, char **cursor, const char * const formatter, ...) {
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

static int internal_carg_secure_vsprintf_concat(char * const startPointer, char * const endPointer, char **cursor, const char *formatter, va_list argsToCopy) {
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

static char *internal_carg_strdup(const char *str) {
    const size_t size = strlen(str);
    char *returnVal = (char *)malloc(sizeof(*returnVal) * (size + 1));
    strncpy(returnVal, str, size);
    returnVal[size] = '\0';
    return returnVal;
}

inline void carg_set_usage_message_tsv(CargContext *cargLocalContext, const char * const format, va_list args) {
    carg_validate_context(cargLocalContext);
    va_list argsCopy;
    va_copy(argsCopy, args);
    internal_carg_flag_conditional_ts(cargLocalContext, CARG_USAGE_MESSAGE_SET, false, "Usage message set by user twice. Please fix this!\n");
    internal_carg_secure_vsprintf_concat(cargLocalContext -> internal_cargUsageStringCursor, cargLocalContext -> internal_cargUsageStringEnd, &cargLocalContext -> internal_cargUsageStringCursor, format, argsCopy);
    internal_carg_set_flag(&cargLocalContext -> internal_cargInternalFlags, CARG_USAGE_MESSAGE_SET);
    va_end(argsCopy);
}

inline void carg_set_usage_message_ts(CargContext *cargLocalContext, const char * const format, ...) {
    va_list args;
    va_start(args, format);
    carg_set_usage_message_tsv(cargLocalContext, format, args);
    va_end(args);
}

inline void carg_set_usage_message(const char * const format, ...) {
    va_list args;
    va_start(args, format);
    carg_set_usage_message_tsv(cargDefaultContext, format, args);
    va_end(args);
}

inline void carg_usage_message_autogen_ts(CargContext *cargLocalContext) {
    carg_validate_context(cargLocalContext);
    internal_carg_flag_conditional_ts(cargLocalContext, CARG_INITIALIZED, true, "Usage message auto-generated before library initialization. Please fix this!\n");
    internal_carg_flag_conditional_ts(cargLocalContext, CARG_USAGE_MESSAGE_SET, false, "Usage message set by user twice. Please fix this!\n");
    internal_carg_secure_sprintf_concat(cargLocalContext -> internal_cargUsageStringCursor, cargLocalContext -> internal_cargUsageStringEnd, &cargLocalContext -> internal_cargUsageStringCursor, "%s%s ", "Usage: ", carg_basename(cargLocalContext -> internal_cargArgVector[0]));
    internal_carg_print_positional_usage_buffer_ts(cargLocalContext);
    internal_carg_print_non_positional_usage_buffer_ts(cargLocalContext);
    internal_carg_set_flag(&cargLocalContext -> internal_cargInternalFlags, CARG_USAGE_MESSAGE_SET);
}

inline void carg_usage_message_autogen(void) {
    carg_usage_message_autogen_ts(cargDefaultContext);
}

inline void carg_set_usage_function_ts(CargContext *cargLocalContext, CargUsageFunc usageFunc) {
    carg_validate_context(cargLocalContext);
    internal_carg_flag_conditional_ts(cargLocalContext, CARG_USAGE_MESSAGE_SET, false, "Usage message set by user twice. Please fix this!\n");
    cargLocalContext -> internal_carg_usage_ptr = usageFunc;
    internal_carg_set_flag(&cargLocalContext -> internal_cargInternalFlags, CARG_USAGE_MESSAGE_SET);
}

inline void carg_set_usage_function(CargUsageFunc usageFunc) {
    carg_set_usage_function_ts(cargDefaultContext, usageFunc);
}

inline void carg_usage_ts(const CargContext *cargLocalContext) {
    carg_validate_context(cargLocalContext);
    cargLocalContext -> internal_carg_usage_ptr(cargLocalContext);
    carg_terminate();
    exit(EXIT_SUCCESS);
}

inline void carg_usage(void) {
    carg_usage_ts(cargDefaultContext);
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

inline void carg_init_ts(CargContext **cargLocalContext, const int argc, char **argv) {
    if (*cargLocalContext) {
        internal_carg_error("carg initialized multiple times consecutively without termination.");
    }
    *cargLocalContext = (CargContext*)malloc(sizeof(CargContext));
    carg_validate_context(*cargLocalContext);
    CargContext cargContextConstructor = {
        .cargArgCount                   = argc,
        .cargPositionalArgCount         = 0,
        .internal_cargSetArgs           = (int *)calloc(sizeof(int), argc),
        .internal_cargArgVector         = (char **)calloc(sizeof(char *), argc),
        .internal_cargAllArgs           = {.size = 0, .fillIndex = -1, .array = NULL},
        .internal_cargUsageString       = (char *)calloc(sizeof(char), CARG_USAGE_STRING_SIZE),
        .internal_cargUsageStringCursor = NULL,
        .internal_cargUsageStringEnd    = NULL,
        .internal_cargInternalFlags     = 0,
        .internal_carg_usage_ptr        = internal_carg_usage_default_ts
    };
    cargContextConstructor.internal_cargUsageStringCursor = cargContextConstructor.internal_cargUsageString;
    cargContextConstructor.internal_cargUsageStringEnd = cargContextConstructor.internal_cargUsageString + CARG_USAGE_STRING_SIZE - 1;
    internal_carg_heap_check(*cargLocalContext);
    internal_carg_heap_check(cargContextConstructor.internal_cargArgVector);
    internal_carg_heap_check(cargContextConstructor.internal_cargSetArgs);
    internal_carg_heap_check(cargContextConstructor.internal_cargUsageString);
    strncpy(cargContextConstructor.internal_cargUsageString, "Please specify a usage message in your client code. You can do this via carg_set_usage_message() or carg_usage_message_autogen().", CARG_USAGE_STRING_SIZE - 1);
    memcpy(*cargLocalContext, &cargContextConstructor, sizeof(CargContext));
    for (int i=0; i<(*cargLocalContext) -> cargArgCount; i++) {
        char *allocation = internal_carg_strdup(argv[i]);
        internal_carg_heap_check(allocation);
        (*cargLocalContext) -> internal_cargArgVector[i] = allocation;
    }
    internal_carg_heap_check((*cargLocalContext) -> internal_cargSetArgs);
    internal_carg_set_flag(&(*cargLocalContext) -> internal_cargInternalFlags, CARG_INITIALIZED);
}

inline void carg_init(const int argc, char **argv) {
    carg_init_ts(&cargDefaultContext, argc, argv);
}

inline void carg_heap_default_value(const CargArgContainer *heapArg, const void *val, const size_t bytes) {
    if (!internal_carg_has_flag(heapArg -> flags, CARG_ITEM_HEAP_ALLOCATED)) {
        internal_carg_error("Heap argument default value setter called on non-heap-allocated argument. Please fix this!\n");
    }
    memcpy(heapArg -> valueContainer.value, val, bytes);
}

inline void carg_set_named_args_tsv(CargContext *cargLocalContext, const char * const format, va_list args) {
    carg_validate_context(cargLocalContext);
    internal_carg_flag_conditional_ts(cargLocalContext, CARG_INITIALIZED, true, "Setter called before library initialization. Please fix this!\n");
    internal_carg_flag_conditional_ts(cargLocalContext, CARG_NAMED_ARGS_SET, false, "Named args initializer called multiple times. Please fix this!\n");
    internal_carg_flag_conditional_ts(cargLocalContext, CARG_GROUPED_ARGS_SET, false, "Grouped args initializer called before named args initializer. Please fix this!\n");
    internal_carg_flag_conditional_ts(cargLocalContext, CARG_ASSERTIONS_SET, false, "Assertions set before all arguments were initialized. Please fix this!\n");
    for (int i=cargLocalContext -> cargPositionalArgCount + 1; i<cargLocalContext -> cargArgCount; i++) {
        va_list argsCopy;
        va_copy(argsCopy, args);
        internal_carg_reference_named_arg_formatter_ts(cargLocalContext, i, format, argsCopy);
        va_end(argsCopy);
    }
    internal_carg_set_flag(&cargLocalContext -> internal_cargInternalFlags, CARG_NAMED_ARGS_SET);
}

inline void carg_set_named_args_ts(CargContext *cargLocalContext, const char * const format, ...) {
    va_list args;
    va_start(args, format);
    carg_set_named_args_tsv(cargLocalContext, format, args);
    va_end(args);
}

inline void carg_set_named_args(const char * const format, ...) {
    va_list args;
    va_start(args, format);
    carg_set_named_args_tsv(cargDefaultContext, format, args);
    va_end(args);
}

inline void carg_set_positional_args_tsv(CargContext *cargLocalContext, const char * const format, va_list args) {
    carg_validate_context(cargLocalContext);
    internal_carg_flag_conditional_ts(cargLocalContext, CARG_INITIALIZED, true, "Setter called before library initialization. Please fix this!\n");
    internal_carg_flag_conditional_ts(cargLocalContext, CARG_POSITIONAL_ARGS_SET, false, "Positional args initializer called multiple times. Please fix this!\n");
    internal_carg_flag_conditional_ts(cargLocalContext, CARG_GROUPED_ARGS_SET, false, "Grouped args initializer called before positional args initializer. Please fix this!\n");
    internal_carg_flag_conditional_ts(cargLocalContext, CARG_ASSERTIONS_SET, false, "Assertions set before all arguments were initialized. Please fix this!\n");
    if (cargLocalContext -> cargArgCount <= cargLocalContext -> cargPositionalArgCount) carg_usage();
    void *formatToTokenizeAllocation = internal_carg_strdup(format);
    internal_carg_heap_check(formatToTokenizeAllocation);
    char *tokenSavePointer = NULL;
    char *formatToTokenize = (char *)formatToTokenizeAllocation;
    void *varDataPtr = NULL;
    va_list argsCopy;
    va_copy(argsCopy, args);
    for (int i=1; i<cargLocalContext -> cargPositionalArgCount+1; i++) {
        CargArgContainer *currentArg = va_arg(argsCopy, CargArgContainer *);
        internal_carg_reference_positional_arg_formatter_ts(cargLocalContext, currentArg, i, &formatToTokenizeAllocation, &formatToTokenize, &tokenSavePointer, &varDataPtr);
    }
    va_end(argsCopy);
    internal_carg_free_nullify(&formatToTokenizeAllocation);
    internal_carg_set_flag(&cargLocalContext -> internal_cargInternalFlags, CARG_POSITIONAL_ARGS_SET);
}

inline void carg_set_positional_args_ts(CargContext *cargLocalContext, const char * const format, ...) {
    va_list args;
    va_start(args, format);
    carg_set_positional_args_tsv(cargLocalContext, format, args);
    va_end(args);
}

inline void carg_set_positional_args(const char * const format, ...) {
    va_list args;
    va_start(args, format);
    carg_set_positional_args_tsv(cargDefaultContext, format, args);
    va_end(args);
}

inline void carg_set_grouped_boolean_args_tsv(CargContext *cargLocalContext, const char * const format, va_list args) {
    carg_validate_context(cargLocalContext);
    internal_carg_flag_conditional_ts(cargLocalContext, CARG_INITIALIZED, true, "Setter called before library initialization. Please fix this!\n");
    internal_carg_flag_conditional_ts(cargLocalContext, CARG_GROUPED_ARGS_SET, false, "Grouped args initializer called multiple times. Please fix this!\n");
    internal_carg_flag_conditional_ts(cargLocalContext, CARG_ASSERTIONS_SET, false, "Assertions set before all arguments were initialized. Please fix this!\n");
    const char prefixChar = format[0];
    const char *noPrefixFormat = format + 1;
    va_list argsCopy;
    va_copy(argsCopy, args);
    bool *varDataPtr = NULL;
    for (int i=1; i<cargLocalContext -> cargArgCount; i++) {
        if (cargLocalContext -> internal_cargArgVector[i][0] != prefixChar || cargLocalContext -> internal_cargSetArgs[i]) continue;
        if (strlen(cargLocalContext -> internal_cargArgVector[i]) > 1 && cargLocalContext -> internal_cargArgVector[i][1] == prefixChar) continue;
        for (size_t j=0; j<strlen(noPrefixFormat); j++) {
            internal_carg_reference_grouped_boolean_arg_formatter_ts(cargLocalContext, i, j, noPrefixFormat, &varDataPtr, argsCopy);
        }
    }
    internal_carg_set_flag(&cargLocalContext -> internal_cargInternalFlags, CARG_GROUPED_ARGS_SET);
    va_end(argsCopy);
}

inline void carg_set_grouped_boolean_args_ts(CargContext *cargLocalContext, const char * const format, ...) {
    va_list args;
    va_start(args, format);
    carg_set_grouped_boolean_args_tsv(cargLocalContext, format, args);
    va_end(args);
}

inline void carg_set_grouped_boolean_args(const char * const format, ...) {
    va_list args;
    va_start(args, format);
    carg_set_grouped_boolean_args_tsv(cargDefaultContext, format, args);
    va_end(args);
}

inline void carg_set_env_defaults_tsv(const CargContext *cargLocalContext, const char * const format, va_list args) {
    carg_validate_context(cargLocalContext);
    internal_carg_flag_conditional_ts(cargLocalContext, CARG_INITIALIZED, true, "Setter called before library initialization. Please fix this!\n");
    va_list argsCopy;
    va_copy(argsCopy, args);
    void *formatTokenAllocation = internal_carg_strdup(format);
    internal_carg_heap_check(formatTokenAllocation);
    char *formatCopy = (char *)formatTokenAllocation;
    char *tokenSavePointer = NULL;
    internal_carg_set_env_defaults_ts(cargLocalContext, &formatCopy, &tokenSavePointer, &formatTokenAllocation, argsCopy);
    va_end(argsCopy);
    internal_carg_free_nullify(&formatTokenAllocation);
}

inline void carg_set_env_defaults_ts(const CargContext *cargLocalContext, const char * const format, ...) {
    va_list args;
    va_start(args, format);
    carg_set_env_defaults_tsv(cargLocalContext, format, args);
    va_end(args);
}

inline void carg_set_env_defaults(const char * const format, ...) {
    va_list args;
    va_start(args, format);
    carg_set_env_defaults_tsv(cargDefaultContext, format, args);
    va_end(args);
}

inline void carg_set_nested_args_tsv(CargContext *cargLocalContext, const int nestedArgumentCount, va_list args) {
    carg_validate_context(cargLocalContext);
    internal_carg_flag_conditional_ts(cargLocalContext, CARG_INITIALIZED, true, "Setter called before library initialization. Please fix this!\n");
    internal_carg_flag_conditional_ts(cargLocalContext, CARG_NESTED_ARGS_SET, false, "Nested args initializer called multiple times. Please fix this!\n");
    internal_carg_flag_conditional_ts(cargLocalContext, CARG_GROUPED_ARGS_SET, false, "Grouped args initializer called before nested args initializer. Please fix this!\n");
    va_list argsCopy;
    va_copy(argsCopy, args);
    for (int x=0; x<nestedArgumentCount; x++) {
        CargArgContainer *argRoot = va_arg(argsCopy, CargArgContainer *);
        if (!internal_carg_has_flag(argRoot -> flags, CARG_ITEM_NESTED)) {
            internal_carg_error("Nested flag setter called on non-nested argument. Fix this!\n");
        }
        if (!internal_carg_has_flag(argRoot -> flags, CARG_ITEM_NESTED_ROOT)) {
            internal_carg_error("Nested flag setter called on non-root nested argument. Fix this!\n");
        }
        if (argRoot -> hasValue) {
            internal_carg_error("Root nested element was set multiple times. Fix this!\n");
        }
        const CargArgContainer *argCursor = argRoot;
        if (!internal_carg_set_nested_arg_ts(cargLocalContext, argRoot)) continue;
        for (int i=0; i <= argCursor -> nestedArgFillIndex; i++) {
            if (internal_carg_set_nested_arg_ts(cargLocalContext, argCursor -> nestedArgs[i])) {
                argCursor = argCursor -> nestedArgs[i];
                i = -1;
            }
        }
    }
    internal_carg_set_flag(&cargLocalContext -> internal_cargInternalFlags, CARG_NESTED_ARGS_SET);
    va_end(argsCopy);
}

inline void carg_set_nested_args_ts(CargContext *cargLocalContext, const int nestedArgumentCount, ...) {
    va_list args;
    va_start(args, nestedArgumentCount);
    carg_set_nested_args_tsv(cargLocalContext, nestedArgumentCount, args);
    va_end(args);
}

inline void carg_set_nested_args(const int nestedArgumentCount, ...) {
    va_list args;
    va_start(args, nestedArgumentCount);
    carg_set_nested_args_tsv(cargDefaultContext, nestedArgumentCount, args);
    va_end(args);
}

inline CargArgContainer *carg_nested_boolean_container_create(CargArgContainer *arg, const char *nestedArgString, const uint64_t flagsArg) {
    if (!internal_carg_has_flag(arg -> flags, CARG_ITEM_BOOLEAN)) {
        internal_carg_error("Boolean nested argument initializer called on non-boolean flag. Fix this!\n");
    }
    internal_carg_set_flag(&arg -> flags, CARG_ITEM_NESTED | CARG_ITEM_NESTED_ROOT | flagsArg);
    arg -> nestedArgString = nestedArgString;
    return arg;
}

inline CargArgContainer *carg_nest_boolean_container(CargArgContainer *nestIn, CargArgContainer *argToNest, const char *nestedArgString) {
    if (!internal_carg_has_flag(argToNest -> flags, CARG_ITEM_BOOLEAN)) {
        internal_carg_error("Only boolean arguments can be nested with this nesting function. Fix this!\n");
    }
    if (nestIn -> nestedArgs && nestIn -> nestedArgFillIndex >= (int)nestIn -> nestedArgArraySize / 2) {
        void *newAllocation = realloc(nestIn -> nestedArgs, nestIn -> nestedArgArraySize * sizeof(*nestIn -> nestedArgs));;
        if (newAllocation) nestIn -> nestedArgs = (CargArgContainer **)newAllocation;
        else internal_carg_free_nullify(&nestIn -> nestedArgs);
        nestIn -> nestedArgArraySize *= 2;
    } else if (!nestIn -> nestedArgs) {
        nestIn -> nestedArgs = (CargArgContainer **)calloc(4, sizeof(*nestIn -> nestedArgs));
        nestIn -> nestedArgArraySize = 4;
    }
    internal_carg_heap_check(nestIn -> nestedArgs);
    argToNest -> nestedArgString = nestedArgString;
    internal_carg_set_flag(&argToNest -> flags, CARG_ITEM_NESTED);
    nestIn -> nestedArgs[++nestIn -> nestedArgFillIndex] = argToNest;
    argToNest -> parentArg = nestIn;
    return argToNest;
}

inline CargArgContainer *carg_nested_container_create(CargArgContainer *arg, const char *nestedArgString, const uint64_t flagsArg, const char * const format) {
    if (internal_carg_has_flag(arg -> flags, CARG_ITEM_BOOLEAN)) {
        internal_carg_error("Non-boolean nested argument initializer called on boolean flag. Fix this!\n");
    }
    strncpy(arg -> formatterUsed, format, sizeof(arg -> formatterUsed) - 1);
    internal_carg_set_flag(&arg -> flags, CARG_ITEM_NESTED | CARG_ITEM_NESTED_ROOT | flagsArg);
    arg -> nestedArgString = nestedArgString;
    return arg;
}

inline CargArgContainer *carg_nest_container(CargArgContainer *nestIn, CargArgContainer *argToNest, const char *nestedArgString, const char * const format) {
    if (internal_carg_has_flag(argToNest -> flags, CARG_ITEM_BOOLEAN)) {
        internal_carg_error("Only non-boolean arguments can be nested with this nesting function. Fix this!\n");
    }
    if (nestIn -> nestedArgs && nestIn -> nestedArgFillIndex >= (int)nestIn -> nestedArgArraySize / 2) {
        void *newAllocation = realloc(nestIn -> nestedArgs, nestIn -> nestedArgArraySize * sizeof(*nestIn -> nestedArgs));;
        if (newAllocation) nestIn -> nestedArgs = (CargArgContainer **)newAllocation;
        else internal_carg_free_nullify(&nestIn -> nestedArgs);
        nestIn -> nestedArgArraySize *= 2;
    } else if (!nestIn -> nestedArgs) {
        nestIn -> nestedArgs = (CargArgContainer **)calloc(4, sizeof(*nestIn -> nestedArgs));
        nestIn -> nestedArgArraySize = 4;
    }
    internal_carg_heap_check(nestIn -> nestedArgs);
    argToNest -> nestedArgString = nestedArgString;
    strncpy(argToNest -> formatterUsed, format, sizeof(argToNest -> formatterUsed) - 1);
    internal_carg_set_flag(&argToNest -> flags, CARG_ITEM_NESTED);
    nestIn -> nestedArgs[++nestIn -> nestedArgFillIndex] = argToNest;
    argToNest -> parentArg = nestIn;
    return argToNest;
}

inline void carg_override_callbacks_tsv(CargContext *cargLocalContext, const char * const format, va_list args) {
    carg_validate_context(cargLocalContext);
    internal_carg_flag_conditional_ts(cargLocalContext, CARG_INITIALIZED, true, "Argument override called before library initialization. Please fix this!\n");
    internal_carg_flag_conditional_ts(cargLocalContext, CARG_OVERRIDE_CALLBACKS_SET, false, "Override callback args initializer called multiple times. Please fix this!\n");
    internal_carg_flag_conditional_ts(cargLocalContext, CARG_ASSERTIONS_SET | CARG_NAMED_ARGS_SET | CARG_POSITIONAL_ARGS_SET | CARG_GROUPED_ARGS_SET | CARG_NESTED_ARGS_SET, false, "Callback override initialized after arguments were set. Fix this!\n");
    internal_carg_flag_conditional_ts(cargLocalContext, CARG_ASSERTIONS_SET, false, "Assertions set before all arguments were initialized. Please fix this!\n");
    if (cargLocalContext -> cargArgCount < 2) return;
    const char *currentFlag = NULL;
    CargCallbackFunc functionCursor = NULL;
    va_list argsCopy;
    va_copy(argsCopy, args);
    char *formatToTokenize = internal_carg_strdup(format);
    char *savePtr = formatToTokenize;
    while ((currentFlag = internal_carg_strtok_reentrant(savePtr, " ", &savePtr))) {
        functionCursor = va_arg(argsCopy, CargCallbackFunc);
        for (int i=1; i<cargLocalContext -> cargArgCount; i++) {
            if (internal_carg_cmp_flag(currentFlag, cargLocalContext -> internal_cargArgVector[i])) {
                functionCursor();
                internal_carg_free_nullify(&formatToTokenize);
                carg_terminate();
                exit(EXIT_SUCCESS);
            }
        }
    }
    internal_carg_free_nullify(&formatToTokenize);
    va_end(argsCopy);
    internal_carg_set_flag(&cargLocalContext -> internal_cargInternalFlags, CARG_OVERRIDE_CALLBACKS_SET);
}

inline void carg_override_callbacks_ts(CargContext *cargLocalContext, const char * const format, ...) {
    va_list args;
    va_start(args, format);
    carg_override_callbacks_tsv(cargLocalContext, format, args);
    va_end(args);
}

inline void carg_override_callbacks(const char * const format, ...) {
    va_list args;
    va_start(args, format);
    carg_override_callbacks_tsv(cargDefaultContext, format, args);
    va_end(args);
}

inline void carg_arg_assert_tsv(CargContext *cargLocalContext, const int assertionCount, va_list args) {
    carg_validate_context(cargLocalContext);
    internal_carg_flag_conditional_ts(cargLocalContext, CARG_ASSERTIONS_SET, false, "Assertion args initializer called multiple times. Please fix this!\n");
    va_list argsCopy;
    va_copy(argsCopy, args);
    for (int i=0; i<assertionCount; i++) {
        const int expression = va_arg(argsCopy, int);
        char *message = va_arg(argsCopy, char *);
        if (!expression) {
            if (message) {
                printf("%s\n", message);
                carg_terminate();
                exit(EXIT_SUCCESS);
            }
            va_end(argsCopy);
            carg_usage();
        }
    }
    va_end(argsCopy);
    internal_carg_set_flag(&cargLocalContext -> internal_cargInternalFlags, CARG_ASSERTIONS_SET);
}

inline void carg_arg_assert_ts(CargContext *cargLocalContext, const int assertionCount, ...) {
    va_list args;
    va_start(args, assertionCount);
    carg_arg_assert_tsv(cargLocalContext, assertionCount, args);
    va_end(args);
}

inline void carg_arg_assert(const int assertionCount, ...) {
    va_list args;
    va_start(args, assertionCount);
    carg_arg_assert_tsv(cargDefaultContext, assertionCount, args);
    va_end(args);
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

inline void carg_validate_ts(const CargContext *cargLocalContext) {
    carg_validate_context(cargLocalContext);
    internal_carg_flag_conditional_ts(cargLocalContext, CARG_ASSERTIONS_SET | CARG_NAMED_ARGS_SET | CARG_POSITIONAL_ARGS_SET | CARG_GROUPED_ARGS_SET | CARG_NESTED_ARGS_SET, true, "Argument validator called before arguments were set. Fix this!\n");
    bool errorFound = false;
    for (int i=1; i<cargLocalContext -> cargArgCount; i++) {
        if (!cargLocalContext -> internal_cargSetArgs[i]) {
            fprintf(stderr, "Error: Unknown option \"%s\"\n", cargLocalContext -> internal_cargArgVector[i]);
            errorFound = true;
        }
    }
    for (int i=0; i<=cargLocalContext -> internal_cargAllArgs.fillIndex; i++) {
        const CargArgContainer *arg = cargLocalContext -> internal_cargAllArgs.array[i];
        if (!arg -> parentArg || !arg -> hasValue) continue;
        if (!internal_carg_has_flag(arg -> parentArg -> flags, CARG_ITEM_BOOLEAN) && arg -> parentArg -> argvIndexFound == arg -> argvIndexFound - 1)
            carg_usage();
        if (internal_carg_has_flag(arg -> flags, CARG_ITEM_ENFORCE_NESTING_ORDER) && arg -> parentArg -> argvIndexFound >= arg -> argvIndexFound)
            carg_usage();
        if (internal_carg_has_flag(arg-> flags, CARG_ITEM_ENFORCE_STRICT_NESTING_ORDER) && arg -> parentArg -> argvIndexFound != arg -> argvIndexFound - 1)
            carg_usage();
    }
    if (errorFound) {
        carg_terminate();
        exit(EXIT_SUCCESS);
    }
}

inline void carg_validate(void) {
    carg_validate_ts(cargDefaultContext);
}

inline void carg_terminate_ts(const CargContext *cargLocalContext) {
    if (internal_carg_has_flag(cargLocalContext -> internal_cargInternalFlags, CARG_INITIALIZED)) {
        if (cargLocalContext -> internal_cargAllArgs.array) {
            for (int i=0; i<=cargLocalContext -> internal_cargAllArgs.fillIndex; i++) {
                if (cargLocalContext -> internal_cargAllArgs.array[i] -> valueContainer.value && internal_carg_has_flag(cargLocalContext -> internal_cargAllArgs.array[i] -> flags, CARG_ITEM_HEAP_ALLOCATED)) {
                    internal_carg_free_nullify(&cargLocalContext -> internal_cargAllArgs.array[i] -> valueContainer.value);
                }
                if (cargLocalContext -> internal_cargAllArgs.array[i] -> nestedArgs) {
                    internal_carg_free_nullify(&cargLocalContext -> internal_cargAllArgs.array[i] -> nestedArgs);
                }
                if (cargLocalContext -> internal_cargAllArgs.array[i] -> valueContainer.next) {
                    internal_carg_free_nullify(&cargLocalContext -> internal_cargAllArgs.array[i] -> valueContainer.next -> value);
                    CargMultiArgContainer *cursor = cargLocalContext -> internal_cargAllArgs.array[i] -> valueContainer.next -> next;
                    CargMultiArgContainer *cursorToFree = cargLocalContext -> internal_cargAllArgs.array[i] -> valueContainer.next;
                    internal_carg_free_nullify(&cursorToFree);
                    while (cursor) {
                        internal_carg_free_nullify(&cursor -> value);
                        cursorToFree = cursor;
                        cursor = cursor -> next;
                        internal_carg_free_nullify(&cursorToFree);
                    }
                }
                free(cargLocalContext -> internal_cargAllArgs.array[i]);
            }
            internal_carg_free_nullify(&cargLocalContext -> internal_cargAllArgs.array);
        }
        internal_carg_free_nullify(&cargLocalContext -> internal_cargSetArgs);
        if (cargLocalContext -> internal_cargArgVector) {
            for (int i=0; i<cargLocalContext -> cargArgCount; i++) {
                internal_carg_free_nullify(&cargLocalContext -> internal_cargArgVector[i]);
            }
            internal_carg_free_nullify(&cargLocalContext -> internal_cargArgVector);
        }
    }
    internal_carg_free_nullify(&cargLocalContext -> internal_cargUsageString);
    internal_carg_free_nullify(&cargLocalContext);
}

inline void carg_terminate(void) {
    carg_terminate_ts(cargDefaultContext);
}

static void internal_carg_flag_conditional_ts(const CargContext *cargLocalContext, const uint64_t flag, const bool truthiness, const char * const errorMessage) {
    carg_validate_context(cargLocalContext);
    if ((bool)internal_carg_has_flag(cargLocalContext -> internal_cargInternalFlags, flag) != truthiness) {
        internal_carg_error(errorMessage);
    }
}

static void internal_carg_error(const char * const formatter, ...) {
    va_list args;
    va_start(args, formatter);
    fprintf(stderr, "cargError: ");
    vfprintf(stderr, formatter, args);
    va_end(args);
    carg_terminate();
    exit(EXIT_FAILURE);
}

static void internal_carg_free_nullify(const void *ptr) {
    if (*(void **)ptr) {
        free(*(void **)ptr);
        *(void **)ptr = NULL;
    }
}

//  For future use.
// static int internal_carg_is_flag(const char * const formatter, const char * const toCheck) {
//     char *formatToTokenize = internal_carg_strdup(formatter);
//     internal_carg_heap_check(formatToTokenize);
//     char *savePtr = formatToTokenize;
//     while (1) {
//         char *flagToken = internal_carg_strtok_reentrant(savePtr, ": ", &savePtr);
//         internal_carg_strtok_reentrant(savePtr, " ", &savePtr); // Discard formatter item
//         if (!flagToken) {
//             break;
//         }
//         if (internal_carg_cmp_flag(toCheck, flagToken)) {
//             internal_carg_free_nullify(&formatToTokenize);
//             return 1;
//         }
//     }
//     internal_carg_free_nullify(&formatToTokenize);
//     return 0;
// }

static void internal_carg_usage_default_ts(const CargContext *cargLocalContext) {
    carg_validate_context(cargLocalContext);
    printf("%s\n", cargLocalContext -> internal_cargUsageString);
}

static bool internal_carg_adjust_multi_arg_setter_ts(const CargContext *cargLocalContext, CargArgContainer *currentArg, void **varDataPtr) {
    carg_validate_context(cargLocalContext);
    if (internal_carg_has_flag(currentArg -> flags, CARG_ITEM_MULTI) && currentArg -> hasValue) {
        CargMultiArgContainer *multiArgCursor = &currentArg->valueContainer;
        while (multiArgCursor -> next) {
            multiArgCursor = multiArgCursor -> next;
        }
        multiArgCursor -> next = (CargMultiArgContainer *)malloc(sizeof(*multiArgCursor -> next));
        internal_carg_heap_check(multiArgCursor -> next);
        multiArgCursor -> next -> next = NULL;
        multiArgCursor -> next -> value = malloc(currentArg -> valueSize);
        internal_carg_heap_check(multiArgCursor -> next -> value);
        *varDataPtr = multiArgCursor -> next -> value;
        currentArg -> multiArgIndex++;
        return true;
    }
    return false;
}

static void internal_carg_set_named_arg_ts(const CargContext *cargLocalContext, CargArgContainer *currentArg, void **varDataPtr, const int argIndex, const char *formatToken, const char *argToFormat, char **formatToTokenize, char **argumentFlagToCompare) {
    carg_validate_context(cargLocalContext);
    if (!currentArg) return;
    if (currentArg -> hasValue && !internal_carg_has_flag(currentArg -> flags, CARG_ITEM_MULTI)) {
        carg_usage();
    }
    if (!internal_carg_adjust_multi_arg_setter_ts(cargLocalContext, currentArg, varDataPtr)) {
        *varDataPtr = currentArg -> valueContainer.value;
    }
    if (!*varDataPtr) return;
    if (internal_carg_cmp_flag(formatToken, "bool")) {
        if (!internal_carg_has_flag(currentArg -> flags, CARG_ITEM_BOOLEAN)) {
            internal_carg_error("Argument struct does not contain the CARG_ITEM_BOOLEAN flag; argument items should be initialized with this flag for readability.\n");
        }
        *(bool *)*varDataPtr = !*(bool *)*varDataPtr; // Flip flag from its default value. Boolean flags are expected to be chars with a default value.
        currentArg -> hasValue = 1;
        cargLocalContext -> internal_cargSetArgs[argIndex] = currentArg -> hasValue;
    } else {
        if (argIndex >= cargLocalContext -> cargArgCount - 1 && !(carg_string_contains_char(cargLocalContext -> internal_cargArgVector[argIndex], '=') >= 0 && strcmp(formatToken, "bool") != 0)) carg_usage();
        currentArg -> hasValue = sscanf(argToFormat, formatToken, *varDataPtr); // If an argument is passed in that does not match its formatter, the value remains default.
        cargLocalContext -> internal_cargSetArgs[argIndex] = currentArg -> hasValue;
        if (!(carg_string_contains_char(cargLocalContext -> internal_cargArgVector[argIndex], '=') >= 0 && strcmp(formatToken, "bool") != 0)) cargLocalContext -> internal_cargSetArgs[argIndex + 1] = currentArg -> hasValue;
        if (!currentArg -> hasValue) {
            internal_carg_free_nullify(formatToTokenize);
            internal_carg_free_nullify(argumentFlagToCompare);
            carg_usage();
        }
    }
    currentArg -> argvIndexFound = argIndex;
    if (formatToken) strncpy(currentArg -> formatterUsed, formatToken, CARG_MAX_FORMATTER_SIZE - 1);
}

static void internal_carg_validate_formatter_extended(const char * const formatToken) {
    if (formatToken && (!internal_carg_cmp_flag(formatToken, "bool") && formatToken[0] != '%' || carg_string_contains_char(formatToken + 1, '%') >= 0)) {
        internal_carg_error("Cannot parse format token %s\n", formatToken);
    }
}

static void internal_carg_validate_formatter(const char * const formatToken) {
    if (formatToken && (formatToken[0] != '%' || carg_string_contains_char(formatToken + 1, '%') >= 0)) {
        internal_carg_error("Cannot parse format token %s\n", formatToken);
    }
}

static void internal_carg_validate_flag(const char * const flagToken) {
    if (flagToken && carg_string_contains_char(flagToken, '%') > -1) {
        internal_carg_error("Cannot parse flag token %s\n", flagToken);
    }
}

static bool internal_carg_adjust_named_assign_ts(const CargContext *cargLocalContext, const int argIndex, const char * const formatToken, const char * const flagToken, const char **argToFormat, char *argumentFlagToCompare) {
    carg_validate_context(cargLocalContext);
    if (argumentFlagToCompare && carg_string_contains_char(cargLocalContext -> internal_cargArgVector[argIndex], '=') >= 0 && strcmp(formatToken, "bool") != 0) {
        int ncompare = 0;
        while (cargLocalContext -> internal_cargArgVector[argIndex][ncompare] != '=') ncompare++;
        if (strncmp(flagToken, cargLocalContext -> internal_cargArgVector[argIndex], ncompare) != 0) return false;
        argumentFlagToCompare[ncompare] = '\0';
        *argToFormat = cargLocalContext -> internal_cargArgVector[argIndex]+ncompare+1;
    }
    return true;
}

static void internal_carg_reference_named_arg_formatter_ts(const CargContext *cargLocalContext, const int argIndex, const char *format, va_list args) {
    carg_validate_context(cargLocalContext);
    if (cargLocalContext -> internal_cargSetArgs[argIndex]) return;
    char *formatToTokenize = internal_carg_strdup(format);
    internal_carg_heap_check(formatToTokenize);
    void *varDataPtr = NULL;
    char *argumentFlagToCompare = internal_carg_strdup(cargLocalContext -> internal_cargArgVector[argIndex]);
    internal_carg_heap_check(argumentFlagToCompare);
    const char *argToFormat;
    if (argIndex < cargLocalContext -> cargArgCount - 1) argToFormat = cargLocalContext -> internal_cargArgVector[argIndex + 1];
    else argToFormat = NULL;
    char *savePtr = formatToTokenize;
    while (1) {
        const char *flagToken = internal_carg_strtok_reentrant(savePtr, ":", &savePtr);
        const char *formatToken = internal_carg_strtok_reentrant(savePtr, " ", &savePtr);
        if (!flagToken) break;
        internal_carg_validate_flag(flagToken);
        internal_carg_validate_formatter_extended(formatToken);
        CargArgContainer *currentArg = va_arg(args, CargArgContainer *);
        if (!internal_carg_adjust_named_assign_ts(cargLocalContext, argIndex, formatToken, flagToken, &argToFormat, argumentFlagToCompare)) continue;
        if (!strcmp(flagToken, argumentFlagToCompare)) {
            internal_carg_set_named_arg_ts(cargLocalContext, currentArg, &varDataPtr, argIndex, formatToken, argToFormat, &formatToTokenize, &argumentFlagToCompare);
            break;
        }
    }
    internal_carg_free_nullify(&formatToTokenize);
    internal_carg_free_nullify(&argumentFlagToCompare);
}

static void internal_carg_reference_positional_arg_formatter_ts(const CargContext *cargLocalContext, CargArgContainer *currentArg, const int i, void **formatToTokenizeAllocation, char **formatToTokenize, char **tokenSavePointer, void **varDataPtr) {
    carg_validate_context(cargLocalContext);
    *formatToTokenize = internal_carg_strtok_reentrant(*formatToTokenize, " ", tokenSavePointer);
    if (!internal_carg_has_flag(currentArg -> flags, CARG_ITEM_POSITIONAL)) {
        internal_carg_error("Positional arg setter called on named argument. Please fix this!\n");
    }
    *varDataPtr = currentArg -> valueContainer.value;
    currentArg -> hasValue = sscanf(cargLocalContext -> internal_cargArgVector[i], *formatToTokenize, *varDataPtr);
    cargLocalContext -> internal_cargSetArgs[i] = currentArg -> hasValue;
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

static void internal_carg_reference_grouped_boolean_arg_formatter_ts(const CargContext *cargLocalContext, const int i, const size_t j, const char *noPrefixFormat, bool **varDataPtr, va_list args) {
    carg_validate_context(cargLocalContext);
    va_list argsCopy;
    va_copy(argsCopy, args);
    if (carg_string_contains_char(cargLocalContext -> internal_cargArgVector[i], noPrefixFormat[j]) >= 0) {
        CargArgContainer *currentArg = NULL;
        for (size_t k=0; k<=j; k++) {
            currentArg = va_arg(argsCopy, CargArgContainer *);
            *varDataPtr = (bool *)currentArg -> valueContainer.value;
        }
        va_end(argsCopy);
        va_copy(argsCopy, args);
        if (*varDataPtr && currentArg) {
            currentArg -> hasValue = 1;
            cargLocalContext -> internal_cargSetArgs[i] = currentArg -> hasValue;
            currentArg -> argvIndexFound = i;
            **varDataPtr = !**varDataPtr;
        }
    }
    va_end(argsCopy);
}

static int internal_carg_set_nested_arg_ts(const CargContext *cargLocalContext, CargArgContainer *currentArg) {
    carg_validate_context(cargLocalContext);
    if (!currentArg) return 0;
    if (!internal_carg_has_flag(currentArg -> flags, CARG_ITEM_NESTED)) {
        internal_carg_error("Nested flag setter called on non-nested argument. Fix this!\n");
    }
    if (currentArg -> hasValue) return 0;
    for (int i=cargLocalContext -> cargPositionalArgCount+1; i<cargLocalContext -> cargArgCount; i++) {
        if (currentArg -> nestedArgString && cargLocalContext -> internal_cargArgVector[i] && !strcmp(currentArg -> nestedArgString, cargLocalContext -> internal_cargArgVector[i])) {
            if (internal_carg_has_flag(currentArg -> flags, CARG_ITEM_BOOLEAN)) {
                *(bool *)currentArg -> valueContainer.value = !*(bool *)currentArg -> valueContainer.value;
                currentArg -> hasValue = 1;
            } else {
                if (i >= cargLocalContext -> cargArgCount - 1) carg_usage();
                currentArg -> hasValue = sscanf(cargLocalContext -> internal_cargArgVector[i+1], currentArg -> formatterUsed, currentArg -> valueContainer.value);
                cargLocalContext -> internal_cargSetArgs[i+1] = currentArg -> hasValue;
            }
            cargLocalContext -> internal_cargSetArgs[i] = currentArg -> hasValue;
            currentArg -> argvIndexFound = i;
            return 1;
        }
    }
    return 0;
}

static void internal_carg_set_env_defaults_ts(const CargContext *cargLocalContext, char **stringToTokenize, char **tokenSavePointer, void **stringAllocation, va_list args) {
    carg_validate_context(cargLocalContext);
    va_list argsCopy;
    va_copy(argsCopy, args);
    CargArgContainer *currentArg = NULL;
    while (1) {
        char *envVarToken = internal_carg_strtok_reentrant(*stringToTokenize, ":", tokenSavePointer);
        char *formatToken = internal_carg_strtok_reentrant(NULL, " ", tokenSavePointer);
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

static void internal_carg_print_positional_usage_buffer_ts(CargContext *cargLocalContext) {
    carg_validate_context(cargLocalContext);
    for (int i=0; i<=cargLocalContext -> internal_cargAllArgs.fillIndex; i++) {
        if (!cargLocalContext -> internal_cargAllArgs.array[i]) break;
        if (!internal_carg_has_flag(cargLocalContext -> internal_cargAllArgs.array[i] -> flags, CARG_ITEM_POSITIONAL)) continue;
        if (cargLocalContext -> internal_cargAllArgs.array[i] -> usageString && cargLocalContext -> internal_cargAllArgs.array[i] -> usageString[0]) internal_carg_secure_sprintf_concat(cargLocalContext -> internal_cargUsageStringCursor, cargLocalContext -> internal_cargUsageStringEnd, &cargLocalContext -> internal_cargUsageStringCursor, "%s ", cargLocalContext -> internal_cargAllArgs.array[i]->usageString);
        else if (cargLocalContext -> internal_cargAllArgs.array[i] -> nestedArgString && cargLocalContext -> internal_cargAllArgs.array[i] -> nestedArgString[0]) internal_carg_secure_sprintf_concat(cargLocalContext -> internal_cargUsageStringCursor, cargLocalContext -> internal_cargUsageStringEnd, &cargLocalContext -> internal_cargUsageStringCursor, "%s ", cargLocalContext -> internal_cargAllArgs.array[i]->nestedArgString);
    }
}

static void internal_carg_print_non_positional_usage_buffer_ts(CargContext *cargLocalContext) {
    for (int i=0; i<=cargLocalContext -> internal_cargAllArgs.fillIndex; i++) {
        if (internal_carg_has_flag(cargLocalContext -> internal_cargAllArgs.array[i] -> flags, CARG_ITEM_BOOLEAN && cargLocalContext -> internal_cargAllArgs.array[i] -> usageString[0])) {
            internal_carg_secure_sprintf_concat(cargLocalContext -> internal_cargUsageStringCursor, cargLocalContext -> internal_cargUsageStringEnd, &cargLocalContext -> internal_cargUsageStringCursor, "%s ", cargLocalContext -> internal_cargAllArgs.array[i]->usageString);
        }
    }
    for (int i=0; i<=cargLocalContext -> internal_cargAllArgs.fillIndex; i++) {
        if (internal_carg_has_flag(cargLocalContext -> internal_cargAllArgs.array[i] -> flags, CARG_ITEM_NESTED_ROOT)) {
            internal_carg_secure_sprintf_concat(cargLocalContext -> internal_cargUsageStringCursor, cargLocalContext -> internal_cargUsageStringEnd, &cargLocalContext -> internal_cargUsageStringCursor, "%s ", cargLocalContext -> internal_cargAllArgs.array[i]->nestedArgString);
        }
    }
    for (int i=0; i<=cargLocalContext -> internal_cargAllArgs.fillIndex; i++) {
        if (internal_carg_has_flag(cargLocalContext -> internal_cargAllArgs.array[i] -> flags, CARG_ITEM_POSITIONAL)) continue;
        if (internal_carg_has_flag(cargLocalContext -> internal_cargAllArgs.array[i] -> flags, CARG_ITEM_BOOLEAN) && !cargLocalContext -> internal_cargAllArgs.array[i] -> nestedArgString[0]) {
            continue;
        }
        if (cargLocalContext -> internal_cargAllArgs.array[i] -> usageString[0]) {
            internal_carg_secure_sprintf_concat(cargLocalContext -> internal_cargUsageStringCursor, cargLocalContext -> internal_cargUsageStringEnd, &cargLocalContext -> internal_cargUsageStringCursor, "%s ", cargLocalContext -> internal_cargAllArgs.array[i] -> usageString);
        }
    }
}

static bool internal_carg_has_flag(const uint64_t item, const uint64_t flag) {
    return (bool)(item & flag);
}

static void internal_carg_set_flag(uint64_t *item, const uint64_t flag) {
    *item |= flag;
}

//  For future use.
// static void internal_carg_clear_flag(uint64_t *item, const uint64_t flag) {
//     *item &= ~flag;
// }
//
// static void internal_carg_toggle_flag(uint64_t *item, const uint64_t flag) {
//     *item ^= flag;
// }

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
