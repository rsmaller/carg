#pragma once
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h> // NOLINT
#include <stdarg.h>
#include <inttypes.h>
#define MEM_DEBUG_DISABLE
#include "memdebug.h"

#define CARG_USAGE_STRING_SIZE ((size_t)2048U)

void         _carg_usage_default(void);

int              _cargArgCount                         = 1;
int              _cargPositionalArgCount               = 0;
int *            _cargSetArgs                          = NULL;
char **          _cargArgVector                        = NULL;
argArray         _cargAllArgs                          = {.size = 0, .fillIndex = -1, .array = NULL};
char             _cargUsageString[CARG_USAGE_STRING_SIZE]     = "Please specify a usage message in your client code. You can do this via carg_set_usage_message() or carg_usage_message_autogen().";
char *           _cargUsageStringCursor                = _cargUsageString;
char * const     _cargUsageStringEnd                   = _cargUsageString + CARG_USAGE_STRING_SIZE - 1;
uint64_t         _cargInternalFlags                    = 0;
CargCallbackFunc _carg_usage_ptr                       = _carg_usage_default;


inline CargArgContainer *carg_arg_create(void *argVarPtr, size_t varSize, uint64_t flagsArg, const char usageStringArg[]) {
    _carg_flag_conditional(CARG_INITIALIZED, true, "Attempt to initialize argument before library initialization. Please fix this!\n");
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
    if (HAS_FLAG(flagsArg, POSITIONAL_ARG)) _cargPositionalArgCount++;
    if (_cargAllArgs.array) {
        _cargAllArgs.fillIndex++;
        if (_cargAllArgs.fillIndex >= (int)(_cargAllArgs.size / 2)){
            _cargAllArgs.size *= 2;
            CargArgContainer **argArrayReallocation = _realloc(_cargAllArgs.array, _cargAllArgs.size * sizeof(*argArrayReallocation));
            _carg_heap_check(argArrayReallocation);
            _cargAllArgs.array = argArrayReallocation;
        }
        _cargAllArgs.array[_cargAllArgs.fillIndex] = constructedArgument;
    } else {
        _cargAllArgs.array = (CargArgContainer **)_malloc(sizeof(*_cargAllArgs.array) * 4);
        _carg_heap_check(_cargAllArgs.array);
        _cargAllArgs.array[0] = constructedArgument;
        _cargAllArgs.fillIndex++;
        _cargAllArgs.size = 4;
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

inline int _carg_test_printf(char *formatter, ...) {
    va_list args;
    va_start(args, formatter);
    const int returnValue = vsnprintf(NULL, 0, formatter, args);
    va_end(args);
    return returnValue;
}

int _carg_test_vsnprintf(const char *formatter, va_list args) { // NOLINT
    const int returnValue = vsnprintf(NULL, 0, formatter, args);
    return returnValue;
}

inline int _carg_secure_sprintf_concat(char * const startPointer, char * const endPointer, const char **cursor, const char * const formatter, ...) {
    if (startPointer > endPointer) return 0;
    if (*endPointer) {
        _carg_error("endPointer 0x%" PRIxPTR " does not point to a null terminator.\n", (uintptr_t)endPointer);
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

inline int _carg_secure_vsprintf_concat(char * const startPointer, char * const endPointer, const char **cursor, const char *formatter, va_list argsToCopy) {
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

inline char *_carg_strdup(const char *str) {
    const size_t size = strlen(str);
    char *returnVal = (char *)_malloc(sizeof(*returnVal) * (size + 1));
    strncpy(returnVal, str, size);
    returnVal[size] = '\0';
    return returnVal;
}

inline void carg_set_usage_message(const char * const format, ...) {
    va_list args;
    va_start(args, format);
    _carg_flag_conditional(CARG_USAGE_MESSAGE_SET, false, "Usage message set by user twice. Please fix this!\n");
    _carg_secure_vsprintf_concat(_cargUsageStringCursor, _cargUsageStringEnd, &_cargUsageStringCursor, format, args);
    SET_FLAG(_cargInternalFlags, CARG_USAGE_MESSAGE_SET);
    va_end(args);
}

inline void carg_usage_message_autogen(void) {
    _carg_flag_conditional(CARG_INITIALIZED, true, "Usage message auto-generated before library initialization. Please fix this!\n");
    _carg_flag_conditional(CARG_USAGE_MESSAGE_SET, false, "Usage message set by user twice. Please fix this!\n");
    _carg_secure_sprintf_concat(_cargUsageStringCursor, _cargUsageStringEnd, &_cargUsageStringCursor, "%s%s ", "Usage: ", carg_basename(_cargArgVector[0]));
    _carg_print_positional_usage_buffer();
    _carg_print_non_positional_usage_buffer();
    SET_FLAG(_cargInternalFlags, CARG_USAGE_MESSAGE_SET);
}

inline void carg_set_usage_function(CargCallbackFunc usageFunc) {
    _carg_flag_conditional(CARG_USAGE_MESSAGE_SET, false, "Usage message set by user twice. Please fix this!\n");
    _carg_usage_ptr = usageFunc;
    SET_FLAG(_cargInternalFlags, CARG_USAGE_MESSAGE_SET);
}

inline void carg_usage(void) {
    _carg_usage_ptr();
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

inline void carg_init(int argc, char **argv) {
    _cargArgCount = argc;
    _cargArgVector = (char **)_calloc(_cargArgCount, sizeof(*_cargArgVector));
    _carg_heap_check(_cargArgVector);
    for (int i=0; i<_cargArgCount; i++) {
        char *allocation = _carg_strdup(argv[i]);
        _carg_heap_check(allocation);
        _cargArgVector[i] = allocation;
    }
    _cargSetArgs = (int *)_calloc(_cargArgCount, sizeof(*_cargSetArgs));
    _carg_heap_check(_cargSetArgs);
    SET_FLAG(_cargInternalFlags, CARG_INITIALIZED);
}

inline void carg_heap_default_value(const CargArgContainer *heapArg, const void *val, size_t bytes) {
    if (!HAS_FLAG(heapArg -> flags, HEAP_ALLOCATED)) {
        _carg_error("Heap argument default value setter called on non-heap-allocated argument. Please fix this!\n");
    }
    memcpy(heapArg -> valueContainer.value, val, bytes);
}

inline void carg_set_named_args(const char * const format, ...) {
    _carg_flag_conditional(CARG_INITIALIZED, true, "Setter called before library initialization. Please fix this!\n");
    _carg_flag_conditional(CARG_NAMED_ARGS_SET, false, "Named args initializer called multiple times. Please fix this!\n");
    _carg_flag_conditional(CARG_GROUPED_ARGS_SET, false, "Grouped args initializer called before named args initializer. Please fix this!\n");
    _carg_flag_conditional(CARG_ASSERTIONS_SET, false, "Assertions set before all arguments were initialized. Please fix this!\n");
    va_list args;
    va_start(args, format);
    for (int i=_cargPositionalArgCount + 1; i<_cargArgCount; i++) {
        _carg_reference_named_arg_formatter(i, format, args);
    }
    va_end(args);
    SET_FLAG(_cargInternalFlags, CARG_NAMED_ARGS_SET);
}

inline void carg_set_positional_args(const char * const format, ...) {
    _carg_flag_conditional(CARG_INITIALIZED, true, "Setter called before library initialization. Please fix this!\n");
    _carg_flag_conditional(CARG_POSITIONAL_ARGS_SET, false, "Positional args initializer called multiple times. Please fix this!\n");
    _carg_flag_conditional(CARG_GROUPED_ARGS_SET, false, "Grouped args initializer called before positional args initializer. Please fix this!\n");
    _carg_flag_conditional(CARG_ASSERTIONS_SET, false, "Assertions set before all arguments were initialized. Please fix this!\n");
    if (_cargArgCount <= _cargPositionalArgCount) carg_usage();
    void *formatToTokenizeAllocation = _carg_strdup(format);
    _carg_heap_check(formatToTokenizeAllocation);
    char *tokenSavePointer = NULL;
    char *formatToTokenize = (char *)formatToTokenizeAllocation;
    void *varDataPtr = NULL;
    va_list args;
    va_start(args, format);
    for (int i=1; i<_cargPositionalArgCount+1; i++) {
        CargArgContainer *currentArg = va_arg(args, CargArgContainer *);
        _carg_reference_positional_arg_formatter(currentArg, i, &formatToTokenizeAllocation, &formatToTokenize, &tokenSavePointer, &varDataPtr);
    }
    _carg_free_nullify(&formatToTokenizeAllocation);
    va_end(args);
    SET_FLAG(_cargInternalFlags, CARG_POSITIONAL_ARGS_SET);
}

inline void carg_set_grouped_boolean_args(const char * const format, ...) {
    _carg_flag_conditional(CARG_INITIALIZED, true, "Setter called before library initialization. Please fix this!\n");
    _carg_flag_conditional(CARG_GROUPED_ARGS_SET, false, "Grouped args initializer called multiple times. Please fix this!\n");
    _carg_flag_conditional(CARG_ASSERTIONS_SET, false, "Assertions set before all arguments were initialized. Please fix this!\n");
    const char prefixChar = format[0];
    const char *noPrefixFormat = format + 1;
    va_list args;
    va_start(args, format);
    bool *varDataPtr = NULL;
    for (int i=1; i<_cargArgCount; i++) {
        if (_cargArgVector[i][0] != prefixChar || _cargSetArgs[i]) continue;
        if (strlen(_cargArgVector[i]) > 1 && _cargArgVector[i][1] == prefixChar) continue;
        for (size_t j=0; j<strlen(noPrefixFormat); j++) {
            _carg_reference_grouped_boolean_arg_formatter(i, j, noPrefixFormat, &varDataPtr, args);
        }
    }
    SET_FLAG(_cargInternalFlags, CARG_GROUPED_ARGS_SET);
    va_end(args);
}

inline void carg_set_env_defaults(const char * const format, ...) {
    _carg_flag_conditional(CARG_INITIALIZED, true, "Setter called before library initialization. Please fix this!\n");
    va_list args;
    va_start(args, format);
    void *formatTokenAllocation = _carg_strdup(format);
    _carg_heap_check(formatTokenAllocation);
    char *formatCopy = (char *)formatTokenAllocation;
    char *tokenSavePointer = NULL;
    _carg_set_env_defaults_internal(&formatCopy, &tokenSavePointer, &formatTokenAllocation, args);
    va_end(args);
    _carg_free_nullify(&formatTokenAllocation);
}

inline void carg_set_nested_args(const int nestedArgumentCount, ...) {
    _carg_flag_conditional(CARG_INITIALIZED, true, "Setter called before library initialization. Please fix this!\n");
    _carg_flag_conditional(CARG_NESTED_ARGS_SET, false, "Nested args initializer called multiple times. Please fix this!\n");
    _carg_flag_conditional(CARG_GROUPED_ARGS_SET, false, "Grouped args initializer called before nested args initializer. Please fix this!\n");
    va_list args;
    va_start(args, nestedArgumentCount);
    for (int x=0; x<nestedArgumentCount; x++) {
        CargArgContainer *argRoot = va_arg(args, CargArgContainer *);
        if (!HAS_FLAG(argRoot -> flags, NESTED_ARG)) {
            _carg_error("Nested flag setter called on non-nested argument. Fix this!\n");
        }
        if (!HAS_FLAG(argRoot -> flags, NESTED_ARG_ROOT)) {
            _carg_error("Nested flag setter called on non-root nested argument. Fix this!\n");
        }
        if (argRoot -> hasValue) {
            _carg_error("Root nested element was set multiple times. Fix this!\n");
        }
        const CargArgContainer *argCursor = argRoot;
        if (!_carg_set_nested_arg_internal(argRoot)) continue;
        for (int i=0; i <= argCursor -> nestedArgFillIndex; i++) {
            if (_carg_set_nested_arg_internal(argCursor -> nestedArgs[i])) {
                argCursor = argCursor -> nestedArgs[i];
                i = -1;
            }
        }
    }
    SET_FLAG(_cargInternalFlags, CARG_NESTED_ARGS_SET);
}

inline CargArgContainer *carg_nested_boolean_container_create(CargArgContainer *arg, const char *nestedArgString, const uint64_t flagsArg) {
    if (!HAS_FLAG(arg -> flags, BOOLEAN_ARG)) {
        _carg_error("Boolean nested argument initializer called on non-boolean flag. Fix this!\n");
    }
    SET_FLAG(arg -> flags, NESTED_ARG | NESTED_ARG_ROOT | flagsArg);
    arg -> nestedArgString = nestedArgString;
    return arg;
}

inline CargArgContainer *carg_nest_boolean_container(CargArgContainer *nestIn, CargArgContainer *argToNest, const char *nestedArgString) {
    if (!HAS_FLAG(nestIn -> flags, BOOLEAN_ARG) || !HAS_FLAG(argToNest -> flags, BOOLEAN_ARG)) {
        _carg_error("Only boolean arguments can be nested with this nesting function. Fix this!\n");
    }
    if (nestIn -> nestedArgs && nestIn -> nestedArgFillIndex >= (int)nestIn -> nestedArgArraySize / 2) {
        nestIn -> nestedArgArraySize *= 2;
        nestIn -> nestedArgs = (CargArgContainer **)_realloc(nestIn -> nestedArgs, nestIn -> nestedArgArraySize * sizeof(*nestIn -> nestedArgs));
    } else if (!nestIn -> nestedArgs) {
        nestIn -> nestedArgs = (CargArgContainer **)_calloc(4, sizeof(*nestIn -> nestedArgs));
        nestIn -> nestedArgArraySize = 4;
    }
    _carg_heap_check(nestIn -> nestedArgs);
    argToNest -> nestedArgString = nestedArgString;
    SET_FLAG(argToNest -> flags, NESTED_ARG);
    nestIn -> nestedArgs[++nestIn -> nestedArgFillIndex] = argToNest;
    argToNest -> parentArg = nestIn;
    return argToNest;
}

inline CargArgContainer *carg_nested_container_create(CargArgContainer *arg, const char *nestedArgString, const uint64_t flagsArg, const char * const format) {
    if (HAS_FLAG(arg -> flags, BOOLEAN_ARG)) {
        _carg_error("Non-boolean nested argument initializer called on boolean flag. Fix this!\n");
    }
    strncpy(arg -> formatterUsed, format, sizeof(arg -> formatterUsed) - 1);
    SET_FLAG(arg -> flags, NESTED_ARG | NESTED_ARG_ROOT | flagsArg);
    arg -> nestedArgString = nestedArgString;
    return arg;
}

inline CargArgContainer *carg_nest_container(CargArgContainer *nestIn, CargArgContainer *argToNest, const char *nestedArgString, const char * const format) {
    if (HAS_FLAG(argToNest -> flags, BOOLEAN_ARG)) {
        _carg_error("Only non-boolean arguments can be nested with this nesting function. Fix this!\n");
    }
    if (nestIn -> nestedArgs && nestIn -> nestedArgFillIndex >= (int)nestIn -> nestedArgArraySize / 2) {
        nestIn -> nestedArgArraySize *= 2;
        nestIn -> nestedArgs = (CargArgContainer **)_realloc(nestIn -> nestedArgs, nestIn -> nestedArgArraySize * sizeof(*nestIn -> nestedArgs));
    } else if (!nestIn -> nestedArgs) {
        nestIn -> nestedArgs = (CargArgContainer **)_calloc(4, sizeof(*nestIn -> nestedArgs));
        nestIn -> nestedArgArraySize = 4;
    }
    _carg_heap_check(nestIn -> nestedArgs);
    argToNest -> nestedArgString = nestedArgString;
    strncpy(argToNest -> formatterUsed, format, sizeof(argToNest -> formatterUsed) - 1);
    SET_FLAG(argToNest -> flags, NESTED_ARG);
    nestIn -> nestedArgs[++nestIn -> nestedArgFillIndex] = argToNest;
    argToNest -> parentArg = nestIn;
    return argToNest;
}

inline void carg_override_callbacks(const char * const format, ...) {
    _carg_flag_conditional(CARG_INITIALIZED, true, "Argument override called before library initialization. Please fix this!\n");
    _carg_flag_conditional(CARG_OVERRIDE_CALLBACKS_SET, false, "Override callback args initializer called multiple times. Please fix this!\n");
    _carg_flag_conditional(CARG_ASSERTIONS_SET | CARG_NAMED_ARGS_SET | CARG_POSITIONAL_ARGS_SET | CARG_GROUPED_ARGS_SET | CARG_NESTED_ARGS_SET, false, "Callback override initialized after arguments were set. Fix this!\n");
    _carg_flag_conditional(CARG_ASSERTIONS_SET, false, "Assertions set before all arguments were initialized. Please fix this!\n");
    if (_cargArgCount < 2) return;
    char *formatToTokenize = _carg_strtok_string_init(format);
    _carg_heap_check(formatToTokenize);
    const char *currentFlag = NULL;
    CargCallbackFunc functionCursor = NULL;
    va_list args;
    va_list argsCopy;
    va_start(args, format);
    va_copy(argsCopy, args);
    for (int i=1; i<_cargArgCount; i++) {
        _carg_strtok_register_string(formatToTokenize);
        while ((currentFlag = strtok(NULL, " "))) {
            functionCursor = va_arg(argsCopy, CargCallbackFunc);
            if (_carg_cmp_flag(currentFlag, _cargArgVector[i])) {
                functionCursor();
                _carg_free_nullify(&formatToTokenize);
                carg_terminate();
                exit(EXIT_SUCCESS);
            }
        }
        _carg_free_nullify(&formatToTokenize);
        va_copy(argsCopy, args);
        formatToTokenize = _carg_strtok_string_init(format);
        _carg_heap_check(formatToTokenize);
    }
    _carg_free_nullify(&formatToTokenize);
    va_end(args);
    SET_FLAG(_cargInternalFlags, CARG_OVERRIDE_CALLBACKS_SET);
}

inline void carg_arg_assert(const int assertionCount, ...) {
    _carg_flag_conditional(CARG_ASSERTIONS_SET, false, "Assertion args initializer called multiple times. Please fix this!\n");
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
    SET_FLAG(_cargInternalFlags, CARG_ASSERTIONS_SET);
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

inline void *carg_fetch_multi_arg_entry(const CargArgContainer *container, int index) {
    if (container -> multiArgIndex < index) {
        _carg_error("%d is past the bounds of the multi argument being accessed (max index is %d)\n", index, container -> multiArgIndex);
    }
    const CargMultiArgContainer *cursor = &container -> valueContainer;
    for (int i=0; i<index; i++) {
        cursor = cursor -> next;
    }
    return cursor -> value;
}

inline void carg_validate(void) {
    _carg_flag_conditional(CARG_ASSERTIONS_SET | CARG_NAMED_ARGS_SET | CARG_POSITIONAL_ARGS_SET | CARG_GROUPED_ARGS_SET | CARG_NESTED_ARGS_SET, true, "Argument validator called before arguments were set. Fix this!\n");
    bool errorFound = false;
    for (int i=1; i<_cargArgCount; i++) {
        if (!_cargSetArgs[i]) {
            fprintf(stderr, "Error: Unknown option \"%s\"\n", _cargArgVector[i]);
            errorFound = true;
        }
    }
    for (int i=0; i<=_cargAllArgs.fillIndex; i++) {
        const CargArgContainer *arg = _cargAllArgs.array[i];
        if (!arg -> parentArg || !arg -> hasValue) continue;
        if (!HAS_FLAG(arg -> parentArg -> flags, BOOLEAN_ARG) && arg -> parentArg -> argvIndexFound == arg -> argvIndexFound - 1)
            carg_usage();
        if (HAS_FLAG(arg -> flags, ENFORCE_NESTING_ORDER) && arg -> parentArg -> argvIndexFound >= arg -> argvIndexFound)
            carg_usage();
        if (HAS_FLAG(arg-> flags, ENFORCE_STRICT_NESTING_ORDER) && arg -> parentArg -> argvIndexFound != arg -> argvIndexFound - 1)
            carg_usage();
    }
    if (errorFound) {
        carg_terminate();
        exit(EXIT_SUCCESS);
    }
}

inline void carg_terminate(void) {
    if (HAS_FLAG(_cargInternalFlags, CARG_INITIALIZED)) {
        if (_cargAllArgs.array) {
            for (int i=0; i<=_cargAllArgs.fillIndex; i++) {
                if (_cargAllArgs.array[i] -> valueContainer.value && HAS_FLAG(_cargAllArgs.array[i] -> flags, HEAP_ALLOCATED)) {
                    _carg_free_nullify(&_cargAllArgs.array[i] -> valueContainer.value);
                }
                if (_cargAllArgs.array[i] -> nestedArgs) {
                    _carg_free_nullify(&_cargAllArgs.array[i] -> nestedArgs);
                }
                if (_cargAllArgs.array[i] -> valueContainer.next) {
                    _carg_free_nullify(&_cargAllArgs.array[i] -> valueContainer.next -> value);
                    CargMultiArgContainer *cursor = _cargAllArgs.array[i] -> valueContainer.next -> next;
                    CargMultiArgContainer *cursorToFree = _cargAllArgs.array[i] -> valueContainer.next;
                    _carg_free_nullify(&cursorToFree);
                    while (cursor) {
                        _carg_free_nullify(&cursor -> value);
                        cursorToFree = cursor;
                        cursor = cursor -> next;
                        _carg_free_nullify(&cursorToFree);
                    }
                }
                free(_cargAllArgs.array[i]);
            }
            _carg_free_nullify(&_cargAllArgs.array);
        }
        _carg_free_nullify(&_cargSetArgs);
        if (_cargArgVector) {
            for (int i=0; i<_cargArgCount; i++) {
                _carg_free_nullify(&_cargArgVector[i]);
            }
            _carg_free_nullify(&_cargArgVector);
        }
    }
    CLEAR_FLAG(_cargInternalFlags, CARG_INITIALIZED);
}

inline void _carg_flag_conditional(uint64_t flag, bool truthiness, const char * const errorMessage) {
    if ((bool)HAS_FLAG(_cargInternalFlags, flag) != truthiness) {
        _carg_error(errorMessage);
    }
}

inline void _carg_error(const char * const formatter, ...) {
    va_list args;
    va_start(args, formatter);
    fprintf(stderr, "cargError: ");
    vfprintf(stderr, formatter, args);
    va_end(args);
    carg_terminate();
    exit(EXIT_FAILURE);
}

inline void _carg_heap_check(void *ptr) {
    if (!ptr) {
        printf("Heap allocation failure. Terminating\n");
        carg_terminate();
        exit(EXIT_FAILURE);
    }
}

inline void _carg_free_nullify(void *ptr) {
    if (*(void **)ptr) {
        free(*(void **)ptr);
        *(void **)ptr = NULL;
    }
}

inline char *_carg_strtok_string_init(const char * const str) {
    char *returnVal = (char *)_calloc(strlen(str) + 4, sizeof(*returnVal));
    strcpy(returnVal, str);
    memmove(returnVal + 3, returnVal, strlen(returnVal));
    memcpy(returnVal, "\x03\x01\x03", 3);
    return returnVal;
}

inline void _carg_strtok_register_string(char *str) {
    strtok(str, "\x03");
}

inline int _carg_cmp_flag(const char * const argument, const char * const parameter) {
    return !strcmp(argument, parameter);
}

inline int _carg_is_flag(const char * const formatter, const char * const toCheck) {
    char *formatToTokenize = _carg_strtok_string_init(formatter);
    _carg_heap_check(formatToTokenize);
    _carg_strtok_register_string(formatToTokenize);
    while (1) {
        char *flagToken = strtok(NULL, ": ");
        strtok(NULL, ": "); // Discard formatter item
        if (!flagToken) {
            break;
        }
        if (_carg_cmp_flag(toCheck, flagToken)) {
            _carg_free_nullify(&formatToTokenize);
            return 1;
        }
    }
    _carg_free_nullify(&formatToTokenize);
    return 0;
}

inline void _carg_usage_default(void) {
    printf("%s\n", _cargUsageString);
}

inline bool _carg_adjust_multi_arg_setter(CargArgContainer *currentArg, void **varDataPtr) {
    if (HAS_FLAG(currentArg -> flags, MULTI_ARG) && currentArg -> hasValue) {
        CargMultiArgContainer *multiArgCursor = &currentArg->valueContainer;
        while (multiArgCursor -> next) {
            multiArgCursor = multiArgCursor -> next;
        }
        multiArgCursor -> next = (CargMultiArgContainer *)_malloc(sizeof(*multiArgCursor -> next));
        _carg_heap_check(multiArgCursor -> next);
        multiArgCursor -> next -> next = NULL;
        multiArgCursor -> next -> value = _malloc(currentArg -> valueSize);
        _carg_heap_check(multiArgCursor -> next -> value);
        *varDataPtr = multiArgCursor -> next -> value;
        currentArg -> multiArgIndex++;
        return true;
    }
    return false;
}

inline void _carg_set_named_arg_internal(CargArgContainer *currentArg, void **varDataPtr, const int argIndex, const char *formatToken, const char *argToFormat, char **formatToTokenize, char **argumentFlagToCompare) {
    if (!currentArg) return;
    if (currentArg -> hasValue && !HAS_FLAG(currentArg -> flags, MULTI_ARG)) {
        carg_usage();
    }
    if (!_carg_adjust_multi_arg_setter(currentArg, varDataPtr)) {
        *varDataPtr = currentArg -> valueContainer.value;
    }
    if (!*varDataPtr) return;
    if (_carg_cmp_flag(formatToken, "bool")) {
        if (!HAS_FLAG(currentArg -> flags, BOOLEAN_ARG)) {
            _carg_error("Argument struct does not contain the BOOLEAN_ARG flag; argument items should be initialized with this flag for readability.\n");
        }
        *(bool *)*varDataPtr = !*(bool *)*varDataPtr; // Flip flag from its default value. Boolean flags are expected to be chars with a default value.
        currentArg -> hasValue = 1;
        _cargSetArgs[argIndex] = currentArg -> hasValue;
    } else {
        if (argIndex >= _cargArgCount - 1 && !(carg_string_contains_char(_cargArgVector[argIndex], '=') >= 0 && strcmp(formatToken, "bool"))) carg_usage();
        currentArg -> hasValue = sscanf(argToFormat, formatToken, *varDataPtr); // If an argument is passed in that does not match its formatter, the value remains default.
        _cargSetArgs[argIndex] = currentArg -> hasValue;
        if (!(carg_string_contains_char(_cargArgVector[argIndex], '=') >= 0 && strcmp(formatToken, "bool"))) _cargSetArgs[argIndex + 1] = currentArg -> hasValue;
        if (!currentArg -> hasValue) {
            _carg_free_nullify(formatToTokenize);
            _carg_free_nullify(argumentFlagToCompare);
            carg_usage();
        }
    }
    currentArg -> argvIndexFound = argIndex;
    if (formatToken) strncpy(currentArg -> formatterUsed, formatToken, CARG_MAX_FORMATTER_SIZE - 1);
}

inline void _carg_validate_formatter_extended(const char * const formatToken) {
    if (formatToken && (!_carg_cmp_flag(formatToken, "bool") && formatToken[0] != '%' || carg_string_contains_char(formatToken + 1, '%') >= 0)) {
        _carg_error("Cannot parse format token %s\n", formatToken);
    }
}

inline void _carg_validate_formatter(const char * const formatToken) {
    if (formatToken && (formatToken[0] != '%' || carg_string_contains_char(formatToken + 1, '%') >= 0)) {
        _carg_error("Cannot parse format token %s\n", formatToken);
    }
}

inline void _carg_validate_flag(const char * const flagToken) {
    if (flagToken && carg_string_contains_char(flagToken, '%') > -1) {
        _carg_error("Cannot parse flag token %s\n", flagToken);
    }
}

inline bool _carg_adjust_named_assign(const int argIndex, const char * const formatToken, const char * const flagToken, const char **argToFormat, char *argumentFlagToCompare) {
    if (argumentFlagToCompare && carg_string_contains_char(_cargArgVector[argIndex], '=') >= 0 && strcmp(formatToken, "bool")) {
        int ncompare = 0;
        while (_cargArgVector[argIndex][ncompare] != '=') ncompare++;
        if (strncmp(flagToken, _cargArgVector[argIndex], ncompare)) return false;
        argumentFlagToCompare[ncompare] = '\0';
        *argToFormat = _cargArgVector[argIndex]+ncompare+1;
    }
    return true;
}

inline void _carg_reference_named_arg_formatter(const int argIndex, const char *format, va_list args) { // NOLINT
    if (_cargSetArgs[argIndex]) return;
    va_list argsCopy;
    va_copy(argsCopy, args);
    char *formatToTokenize = _carg_strtok_string_init(format);
    _carg_heap_check(formatToTokenize);
    void *varDataPtr = NULL;
    char *argumentFlagToCompare = _carg_strdup(_cargArgVector[argIndex]);
    _carg_heap_check(argumentFlagToCompare);
    const char *argToFormat;
    if (argIndex < _cargArgCount - 1) argToFormat = _cargArgVector[argIndex + 1];
    else argToFormat = NULL;
    _carg_strtok_register_string(formatToTokenize);
    while (1) {
        const char *flagToken = strtok(NULL, ":");
        const char *formatToken = strtok(NULL, " ");
        if (!flagToken) break;
        _carg_validate_flag(flagToken);
        _carg_validate_formatter_extended(formatToken);
        CargArgContainer *currentArg = va_arg(argsCopy, CargArgContainer *);
        if (!_carg_adjust_named_assign(argIndex, formatToken, flagToken, &argToFormat, argumentFlagToCompare)) continue;
        if (!strcmp(flagToken, argumentFlagToCompare)) {
            _carg_set_named_arg_internal(currentArg, &varDataPtr, argIndex, formatToken, argToFormat, &formatToTokenize, &argumentFlagToCompare);
            break;
        }
    }
    _carg_free_nullify(&formatToTokenize);
    _carg_free_nullify(&argumentFlagToCompare);
}

inline void _carg_reference_positional_arg_formatter(CargArgContainer *currentArg, const int i, void **formatToTokenizeAllocation, char **formatToTokenize, char **tokenSavePointer, void **varDataPtr) {
    *formatToTokenize = strtok_r(*formatToTokenize, " ", tokenSavePointer);
    if (!HAS_FLAG(currentArg -> flags, POSITIONAL_ARG)) {
        _carg_error("Positional arg setter called on named argument. Please fix this!\n");
    }
    *varDataPtr = currentArg -> valueContainer.value;
    currentArg -> hasValue = sscanf(_cargArgVector[i], *formatToTokenize, *varDataPtr);
    _cargSetArgs[i] = currentArg -> hasValue;
    if (!currentArg -> hasValue) {
        _carg_free_nullify(formatToTokenizeAllocation);
        carg_usage();
    }
    currentArg -> argvIndexFound = i;
    if (*formatToTokenize) {
        strncpy(currentArg -> formatterUsed, *formatToTokenize, CARG_MAX_FORMATTER_SIZE - 1);
    }
    *formatToTokenize = *tokenSavePointer;
}

inline void _carg_reference_grouped_boolean_arg_formatter(const int i, const size_t j, const char *noPrefixFormat, bool **varDataPtr, va_list args) {
    va_list argsCopy;
    va_copy(argsCopy, args);
    if (carg_string_contains_char(_cargArgVector[i], noPrefixFormat[j]) >= 0) {
        CargArgContainer *currentArg = NULL;
        for (size_t k=0; k<=j; k++) {
            currentArg = va_arg(argsCopy, CargArgContainer *);
            *varDataPtr = (bool *)currentArg -> valueContainer.value;
        }
        va_end(argsCopy);
        va_copy(argsCopy, args);
        if (*varDataPtr && currentArg) {
            currentArg -> hasValue = 1;
            _cargSetArgs[i] = currentArg -> hasValue;
            currentArg -> argvIndexFound = i;
            **varDataPtr = !**varDataPtr;
        }
    }
    va_end(argsCopy);
}

inline int _carg_set_nested_arg_internal(CargArgContainer *currentArg) {
    if (!currentArg) return 0;
    if (!HAS_FLAG(currentArg -> flags, NESTED_ARG)) {
        _carg_error("Nested flag setter called on non-nested argument. Fix this!\n");
    }
    if (currentArg -> hasValue) return 0;
    for (int i=_cargPositionalArgCount+1; i<_cargArgCount; i++) {
        if (!strcmp(currentArg -> nestedArgString, _cargArgVector[i])) {
            if (HAS_FLAG(currentArg -> flags, BOOLEAN_ARG)) {
                *(bool *)currentArg -> valueContainer.value = !*(bool *)currentArg -> valueContainer.value;
                currentArg -> hasValue = 1;
            } else {
                if (i >= _cargArgCount - 1) carg_usage();
                currentArg -> hasValue = sscanf(_cargArgVector[i+1], currentArg -> formatterUsed, currentArg -> valueContainer.value);
                _cargSetArgs[i+1] = currentArg -> hasValue;
            }
            _cargSetArgs[i] = currentArg -> hasValue;
            currentArg -> argvIndexFound = i;
            return 1;
        }
    }
    return 0;
}

inline void _carg_set_env_defaults_internal(char **stringToTokenize, char **tokenSavePointer, void **stringAllocation, va_list args) {
    va_list argsCopy;
    va_copy(argsCopy, args);
    CargArgContainer *currentArg = NULL;
    while (1) {
        char *envVarToken = strtok_r(*stringToTokenize, ":", tokenSavePointer);
        char *formatToken = strtok_r(NULL, " ", tokenSavePointer);
        *stringToTokenize = *tokenSavePointer;
        if (!envVarToken || !formatToken) break;
        _carg_validate_flag(envVarToken);
        _carg_validate_formatter(formatToken);
        const char *envVarValue = getenv(envVarToken);
        currentArg = va_arg(argsCopy, CargArgContainer *);
        if (!envVarValue) continue;
        if (!currentArg -> hasValue) {
            currentArg -> hasValue = sscanf(envVarValue, formatToken, currentArg -> valueContainer.value);
        }
        if (!currentArg -> hasValue) {
            _carg_free_nullify(stringAllocation);
            _carg_error("Unable to grab environment variable %s\n", envVarToken);
        }
        strncpy(currentArg -> formatterUsed, formatToken, CARG_MAX_FORMATTER_SIZE - 1);
    }
    va_end(argsCopy);
}

inline void _carg_print_positional_usage_buffer(void) {
    for (int i=0; i<=_cargAllArgs.fillIndex; i++) {
        if (!_cargAllArgs.array[i]) break;
        if (!HAS_FLAG(_cargAllArgs.array[i] -> flags, POSITIONAL_ARG)) continue;
        if (_cargAllArgs.array[i] -> usageString && _cargAllArgs.array[i] -> usageString[0]) _carg_secure_sprintf_concat(_cargUsageStringCursor, _cargUsageStringEnd, &_cargUsageStringCursor, "%s ", _cargAllArgs.array[i]->usageString);
        else if (_cargAllArgs.array[i] -> nestedArgString && _cargAllArgs.array[i] -> nestedArgString[0]) _carg_secure_sprintf_concat(_cargUsageStringCursor, _cargUsageStringEnd, &_cargUsageStringCursor, "%s ", _cargAllArgs.array[i]->nestedArgString);
    }
}

inline void _carg_print_non_positional_usage_buffer(void) {
    for (int i=0; i<=_cargAllArgs.fillIndex; i++) {
        if (HAS_FLAG(_cargAllArgs.array[i] -> flags, BOOLEAN_ARG && _cargAllArgs.array[i] -> usageString[0])) {
            _carg_secure_sprintf_concat(_cargUsageStringCursor, _cargUsageStringEnd, &_cargUsageStringCursor, "%s ", _cargAllArgs.array[i]->usageString);
        }
    }
    for (int i=0; i<=_cargAllArgs.fillIndex; i++) {
        if (HAS_FLAG(_cargAllArgs.array[i] -> flags, NESTED_ARG_ROOT)) {
            _carg_secure_sprintf_concat(_cargUsageStringCursor, _cargUsageStringEnd, &_cargUsageStringCursor, "%s ", _cargAllArgs.array[i]->nestedArgString);
        }
    }
    for (int i=0; i<=_cargAllArgs.fillIndex; i++) {
        if (HAS_FLAG(_cargAllArgs.array[i] -> flags, POSITIONAL_ARG)) continue;
        if (HAS_FLAG(_cargAllArgs.array[i] -> flags, BOOLEAN_ARG) && !_cargAllArgs.array[i] -> nestedArgString[0]) {
            continue;
        }
        if (_cargAllArgs.array[i] -> usageString[0]) {
            _carg_secure_sprintf_concat(_cargUsageStringCursor, _cargUsageStringEnd, &_cargUsageStringCursor, "%s ", _cargAllArgs.array[i] -> usageString);
        }
        else if (_cargAllArgs.array[i] -> nestedArgString[0]) {}
        else {}
    }
}

#define CARG_PRINT_STRING_ARG(argument) do {\
    carg_print_container_data(argument);\
    printf("\tValue: ");\
    if (carg_string_contains_char((argument) -> formatterUsed, '\n') >= 0) printf("%s", (char *)(argument) -> valueContainer.value); /* This allows the string scanf() formatter with spaces to work. */\
    else printf((argument) -> formatterUsed, (char *)(argument) -> valueContainer.value);\
    printf("\n");\
} while (0)

#define CARG_PRINT_NON_STRING_ARG(argument, typeArg) do {\
    carg_print_container_data(argument);\
    printf("\tValue: ");\
    printf((argument) -> formatterUsed, *(typeArg *)(argument) -> valueContainer.value);\
    printf("\n");\
} while (0)

#define CARG_PRINT_NON_STRING_MULTI_ARG(argument, typeArg) do {\
    CARG_PRINT_NON_STRING_ARG(argument, typeArg);\
    if (!(argument) -> formatterUsed[0]) break;\
    CargMultiArgContainer *cursor = argument -> valueContainer.next;\
    while (cursor) {\
        printf("\tValue: ");\
        printf((argument) -> formatterUsed, *(typeArg *)cursor -> value);\
        printf("\n");\
        cursor = cursor -> next;\
    }\
} while (0)

#define CARG_PRINT_STRING_MULTI_ARG(argument) do {\
    CARG_PRINT_STRING_ARG(argument);\
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
