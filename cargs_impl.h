#pragma once
#ifndef CARGS_IMPL_H
    #define CARGS_IMPL_H
#endif
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <inttypes.h>

inline ArgContainer *carg_arg_create(void *argMemory, size_t expectedSize, uint64_t flagsArg, const char usageStringArg[]) {
    _carg_flag_conditional(LIBCARGS_INITIALIZED, true, "Attempt to initialize argument before library initialization. Please fix this!\n");
    ArgContainer *constructedArgument = (ArgContainer *)malloc(sizeof(ArgContainer));
    const ArgContainer constructedArgumentInternal = {
        .valueContainer = {
            .next = NULL,
            .value = argMemory
        },
        .multiArgIndex = 0,
        .valueSize = expectedSize,
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
    memcpy(constructedArgument, &constructedArgumentInternal, sizeof(ArgContainer));
    if (HAS_FLAG(flagsArg, POSITIONAL_ARG)) positionalArgCount++;
    if (allArgs.array) {
        allArgs.fillIndex++;
        if (allArgs.fillIndex >= (int)(allArgs.size / 2)){
            allArgs.size *= 2;
            void *argArrayReallocation = realloc(allArgs.array, allArgs.size * sizeof(ArgContainer *));
            _carg_heap_check(argArrayReallocation);
            allArgs.array = (ArgContainer **)argArrayReallocation;
        }
        allArgs.array[allArgs.fillIndex] = constructedArgument;
    } else {
        allArgs.array = (ArgContainer **)malloc(sizeof(ArgContainer *) * 4);
        _carg_heap_check(allArgs.array);
        allArgs.array[0] = constructedArgument;
        allArgs.fillIndex++;
        allArgs.size = 4;
    }
    return constructedArgument;
}

inline char *carg_string_contains_substr(char *testString, const char *substring) {
    while (strlen(testString) >= strlen(substring)) {
        if (!strncmp(testString, substring, strlen(substring))) {
            return testString;
        }
        testString++;
    }
    return NULL;
}

inline int carg_string_contains_char(const char *testString, const char subchar) {
    if (!testString) return -1;
    for (int i=0; i<strlen(testString); i++) {
        if (testString[i] == subchar) {
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

inline void carg_set_usage_message(const char *formatter, ...) {
    va_list args;
    va_start(args, formatter);
    _carg_flag_conditional(USAGE_MESSAGE_SET, false, "Usage message set by user twice. Please fix this!\n");
    secure_vsprintf_concat(usageStringCursor, usageStringEnd, &usageStringCursor, formatter, args);
    SET_FLAG(cargInternalFlags, USAGE_MESSAGE_SET);
    va_end(args);
}

inline void carg_usage_message_autogen(void) {
    _carg_flag_conditional(LIBCARGS_INITIALIZED, true, "Usage message auto-generated before library initialization. Please fix this!\n");
    _carg_flag_conditional(USAGE_MESSAGE_SET, false, "Usage message set by user twice. Please fix this!\n");
    secure_sprintf_concat(usageStringCursor, usageStringEnd, &usageStringCursor, "%s%s ", "Usage: ", carg_basename(argVector[0]));
    _carg_print_positional_usage_buffer();
    _carg_print_non_positional_usage_buffer();
    SET_FLAG(cargInternalFlags, USAGE_MESSAGE_SET);
}

inline void carg_set_usage_function(CargCallbackFunc funcArg) {
    _carg_flag_conditional(USAGE_MESSAGE_SET, false, "Usage message set by user twice. Please fix this!\n");
    _carg_usage_ptr = funcArg;
    SET_FLAG(cargInternalFlags, USAGE_MESSAGE_SET);
}

inline void usage(void) {
    _carg_usage_ptr();
    carg_terminate();
    exit(EXIT_SUCCESS);
}

inline void carg_init(int argc, char **argv) {
    argCount = argc;
    argVector = (char **)calloc(argCount, sizeof(char *));
    _carg_heap_check(argVector);
    for (int i=0; i<argCount; i++) {
        char *allocation = carg_strdup(argv[i]);
        _carg_heap_check(allocation);
        argVector[i] = allocation;
    }
    setArgs = (int *)calloc(argCount, sizeof(int));
    _carg_heap_check(setArgs);
    SET_FLAG(cargInternalFlags, LIBCARGS_INITIALIZED);
}

inline void carg_heap_default_value(const ArgContainer *varName, const void *val, size_t bytes) {
    if (!HAS_FLAG(varName -> flags, HEAP_ALLOCATED)) {
        _carg_error("Heap argument default value setter called on non-heap-allocated argument. Please fix this!\n");
    }
    memcpy(varName -> valueContainer.value, val, bytes);
}

inline void carg_set_named_args(const char * const format, ...) {
    _carg_flag_conditional(LIBCARGS_INITIALIZED, true, "Setter called before library initialization. Please fix this!\n");
    _carg_flag_conditional(NAMED_ARGS_SET, false, "Named args initializer called multiple times. Please fix this!\n");
    _carg_flag_conditional(GROUPED_ARGS_SET, false, "Grouped args initializer called before named args initializer. Please fix this!\n");
    _carg_flag_conditional(ASSERTIONS_SET, false, "Assertions set before all arguments were initialized. Please fix this!\n");
    va_list args;
    va_start(args, format);
    for (int i=positionalArgCount + 1; i<argCount; i++) {
        _carg_reference_named_arg_formatter(i, format, args);
    }
    va_end(args);
    SET_FLAG(cargInternalFlags, NAMED_ARGS_SET);
}

inline void carg_set_positional_args(const char *format, ...) {
    _carg_flag_conditional(LIBCARGS_INITIALIZED, true, "Setter called before library initialization. Please fix this!\n");
    _carg_flag_conditional(POSITIONAL_ARGS_SET, false, "Positional args initializer called multiple times. Please fix this!\n");
    _carg_flag_conditional(GROUPED_ARGS_SET, false, "Grouped args initializer called before positional args initializer. Please fix this!\n");
    _carg_flag_conditional(ASSERTIONS_SET, false, "Assertions set before all arguments were initialized. Please fix this!\n");
    if (argCount <= positionalArgCount) usage();
    void *formatToTokenizeAllocation = carg_strdup(format);
    _carg_heap_check(formatToTokenizeAllocation);
    char *tokenSavePointer = NULL;
    char *formatToTokenize = (char *)formatToTokenizeAllocation;
    void *flagCopierPointer = NULL;
    va_list args;
    va_start(args, format);
    for (int i=1; i<positionalArgCount+1; i++) {
        ArgContainer *currentArg = va_arg(args, ArgContainer *);
        _carg_reference_positional_arg_formatter(currentArg, i, &formatToTokenizeAllocation, &formatToTokenize, &tokenSavePointer, &flagCopierPointer);
    }
    _carg_free_nullify(&formatToTokenizeAllocation);
    va_end(args);
    SET_FLAG(cargInternalFlags, POSITIONAL_ARGS_SET);
}

inline void carg_set_grouped_boolean_args(const char *format, ...) {
    _carg_flag_conditional(LIBCARGS_INITIALIZED, true, "Setter called before library initialization. Please fix this!\n");
    _carg_flag_conditional(GROUPED_ARGS_SET, false, "Grouped args initializer called multiple times. Please fix this!\n");
    _carg_flag_conditional(ASSERTIONS_SET, false, "Assertions set before all arguments were initialized. Please fix this!\n");
    const char prefixChar = format[0];
    const char *noPrefixFormat = format + 1;
    va_list args;
    va_start(args, format);
    bool *flagCopierPointer = NULL;
    for (int i=1; i<argCount; i++) {
        if (argVector[i][0] != prefixChar || setArgs[i]) continue;
        if (strlen(argVector[i]) > 1 && argVector[i][1] == prefixChar) continue;
        for (size_t j=0; j<strlen(noPrefixFormat); j++) {
            _carg_reference_grouped_boolean_arg_formatter(i, j, noPrefixFormat, &flagCopierPointer, args);
        }
    }
    SET_FLAG(cargInternalFlags, GROUPED_ARGS_SET);
    va_end(args);
}

inline void carg_set_env_defaults(const char * const format, ...) {
    _carg_flag_conditional(LIBCARGS_INITIALIZED, true, "Setter called before library initialization. Please fix this!\n");
    va_list args;
    va_start(args, format);
    void *formatTokenAllocation = carg_strdup(format);
    _carg_heap_check(formatTokenAllocation);
    char *formatCopy = (char *)formatTokenAllocation;
    char *tokenSavePointer = NULL;
    _carg_set_env_defaults_internal(&formatCopy, &tokenSavePointer, &formatTokenAllocation, args);
    va_end(args);
    _carg_free_nullify(&formatTokenAllocation);
}

inline void carg_set_nested_args(const int nestedArgumentCount, ...) {
    _carg_flag_conditional(LIBCARGS_INITIALIZED, true, "Setter called before library initialization. Please fix this!\n");
    _carg_flag_conditional(NESTED_ARGS_SET, false, "Nested args initializer called multiple times. Please fix this!\n");
    _carg_flag_conditional(GROUPED_ARGS_SET, false, "Grouped args initializer called before nested args initializer. Please fix this!\n");
    va_list args;
    va_start(args, nestedArgumentCount);
    for (int x=0; x<nestedArgumentCount; x++) {
        ArgContainer *argRoot = va_arg(args, ArgContainer *);
        if (!HAS_FLAG(argRoot -> flags, NESTED_ARG)) {
            _carg_error("Nested flag setter called on non-nested argument. Fix this!\n");
        }
        if (!HAS_FLAG(argRoot -> flags, NESTED_ARG_ROOT)) {
            _carg_error("Nested flag setter called on non-root nested argument. Fix this!\n");
        }
        if (argRoot -> hasValue) {
            _carg_error("Root nested element was set multiple times. Fix this!\n");
        }
        const ArgContainer *argCursor = argRoot;
        if (!_carg_set_nested_arg_internal(argRoot)) continue;
        for (int i=0; i <= argCursor -> nestedArgFillIndex; i++) {
            if (_carg_set_nested_arg_internal(argCursor -> nestedArgs[i])) {
                argCursor = argCursor -> nestedArgs[i];
                i = -1;
            }
        }
    }
    SET_FLAG(cargInternalFlags, NESTED_ARGS_SET);
}

inline ArgContainer *carg_nested_boolean_container_create(ArgContainer *arg, const char *argString, const uint64_t flagsArg) {
    if (!HAS_FLAG(arg -> flags, BOOLEAN_ARG)) {
        _carg_error("Boolean nested argument initializer called on non-boolean flag. Fix this!\n");
    }
    SET_FLAG(arg -> flags, NESTED_ARG | NESTED_ARG_ROOT | flagsArg);
    arg -> nestedArgString = argString;
    return arg;
}

inline ArgContainer *carg_nest_boolean_container(ArgContainer *nestIn, ArgContainer *argToNest, const char *nestedArgString) {
    if (!HAS_FLAG(nestIn -> flags, BOOLEAN_ARG) || !HAS_FLAG(argToNest -> flags, BOOLEAN_ARG)) {
        _carg_error("Only boolean arguments can be nested with this nesting function. Fix this!\n");
    }
    if (nestIn -> nestedArgs && nestIn -> nestedArgFillIndex >= (int)nestIn -> nestedArgArraySize / 2) {
        nestIn -> nestedArgArraySize *= 2;
        nestIn -> nestedArgs = (ArgContainer **)realloc(nestIn -> nestedArgs, nestIn -> nestedArgArraySize * sizeof(ArgContainer *));
        _carg_heap_check(nestIn -> nestedArgs);
    } else if (!nestIn -> nestedArgs) {
        nestIn -> nestedArgs = (ArgContainer **)calloc(4, sizeof(ArgContainer *));
        _carg_heap_check(nestIn -> nestedArgs);
        nestIn -> nestedArgArraySize = 4;
    }
    argToNest -> nestedArgString = nestedArgString;
    argToNest -> flags |= nestIn -> flags;
    SET_FLAG(argToNest -> flags, NESTED_ARG | BOOLEAN_ARG);
    CLEAR_FLAG(argToNest -> flags, NESTED_ARG_ROOT);
    nestIn -> nestedArgs[++nestIn -> nestedArgFillIndex] = argToNest;
    argToNest -> parentArg = nestIn;
    return argToNest;
}

inline ArgContainer *carg_nested_container_create(ArgContainer *arg, const char *argString, const uint64_t flagsArg, const char * const formatterToUse) {
    if (HAS_FLAG(arg -> flags, BOOLEAN_ARG)) {
        _carg_error("Non-boolean nested argument initializer called on boolean flag. Fix this!\n");
    }
    strncpy(arg -> formatterUsed, formatterToUse, sizeof(arg -> formatterUsed) - 1);
    SET_FLAG(arg -> flags, NESTED_ARG | NESTED_ARG_ROOT | flagsArg);
    arg -> nestedArgString = argString;
    return arg;
}

inline ArgContainer *carg_nest_container(ArgContainer *nestIn, ArgContainer *argToNest, const char *nestedArgString, const char * const formatterToUse) {
    if (HAS_FLAG(argToNest -> flags, BOOLEAN_ARG)) {
        _carg_error("Only non-boolean arguments can be nested with this nesting function. Fix this!\n");
    }
    if (nestIn -> nestedArgs && nestIn -> nestedArgFillIndex >= (int)nestIn -> nestedArgArraySize / 2) {
        nestIn -> nestedArgArraySize *= 2;
        nestIn -> nestedArgs = (ArgContainer **)realloc(nestIn -> nestedArgs, nestIn -> nestedArgArraySize * sizeof(ArgContainer *));
        _carg_heap_check(nestIn -> nestedArgs);
    } else if (!nestIn -> nestedArgs) {
        nestIn -> nestedArgs = (ArgContainer **)calloc(4, sizeof(ArgContainer *));
        _carg_heap_check(nestIn -> nestedArgs);
        nestIn -> nestedArgArraySize = 4;
    }
    argToNest -> nestedArgString = nestedArgString;
    argToNest -> flags |= nestIn -> flags;
    strncpy(argToNest -> formatterUsed, formatterToUse, sizeof(argToNest -> formatterUsed) - 1);
    SET_FLAG(argToNest -> flags, NESTED_ARG);
    CLEAR_FLAG(argToNest -> flags, NESTED_ARG_ROOT);
    CLEAR_FLAG(argToNest -> flags, BOOLEAN_ARG);
    nestIn -> nestedArgs[++nestIn -> nestedArgFillIndex] = argToNest;
    argToNest -> parentArg = nestIn;
    return argToNest;
}

inline void carg_override_callbacks(const char *format, ...) {
    _carg_flag_conditional(LIBCARGS_INITIALIZED, true, "Argument override called before library initialization. Please fix this!\n");
    _carg_flag_conditional(OVERRIDE_CALLBACKS_SET, false, "Override callback args initializer called multiple times. Please fix this!\n");
    _carg_flag_conditional(ASSERTIONS_SET | NAMED_ARGS_SET | POSITIONAL_ARGS_SET | GROUPED_ARGS_SET | NESTED_ARGS_SET, false, "Callback override initialized after arguments were set. Fix this!\n");
    _carg_flag_conditional(ASSERTIONS_SET, false, "Assertions set before all arguments were initialized. Please fix this!\n");
    if (argCount < 2) return;
    char *formatToTokenize = _carg_strtok_string_init(format);
    _carg_heap_check(formatToTokenize);
    const char *currentFlag = NULL;
    CargCallbackFunc functionCursor = NULL;
    va_list args;
    va_list argsCopy;
    va_start(args, format);
    va_copy(argsCopy, args);
    for (int i=1; i<argCount; i++) {
        _carg_strtok_register_string(formatToTokenize);
        while ((currentFlag = strtok(NULL, " "))) {
            functionCursor = va_arg(argsCopy, CargCallbackFunc);
            if (_carg_cmp_flag(currentFlag, argVector[i])) {
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
    SET_FLAG(cargInternalFlags, OVERRIDE_CALLBACKS_SET);
}

inline void carg_arg_assert(const int assertionCount, ...) {
    _carg_flag_conditional(ASSERTIONS_SET, false, "Assertion args initializer called multiple times. Please fix this!\n");
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
            usage();
        }
    }
    va_end(args);
    SET_FLAG(cargInternalFlags, ASSERTIONS_SET);
}

inline void carg_print_container_data(ArgContainer *container) {
    printf("Argument: \n");
    printf("\tSize: %zu\n", container -> valueSize);
    printf("\tFlags: %" PRIu64 "\n", container -> flags);
    printf("\tFound At: %d\n", container -> argvIndexFound);
    printf("\tHas Value: %d\n", container -> hasValue);
    if (container -> usageString[0]) printf("\tUsage String: %s\n", container -> usageString);
    printf("\tFormatter Used: %s\n", container -> formatterUsed);
    if (container -> nestedArgString[0]) printf("\tNested Argument String: %s\n", container -> nestedArgString);
}

inline void *carg_fetch_multi_arg_entry(ArgContainer *container, int index) {
    if (container -> multiArgIndex < index) {
        _carg_error("%d is past the bounds of the multi argument being accessed (max index is %d)\n", index, container -> multiArgIndex);
    }
    MultiArgContainer *cursor = &container -> valueContainer;
    for (int i=0; i<index; i++) {
        cursor = cursor -> next;
    }
    return cursor -> value;
}

inline void carg_validate(void) {
    _carg_flag_conditional(ASSERTIONS_SET | NAMED_ARGS_SET | POSITIONAL_ARGS_SET | GROUPED_ARGS_SET | NESTED_ARGS_SET, true, "Argument validator called before arguments were set. Fix this!\n");
    bool errorFound = false;
    for (int i=1; i<argCount; i++) {
        if (!setArgs[i]) {
            fprintf(stderr, "Error: Unknown option \"%s\"\n", argVector[i]);
            errorFound = true;
        }
    }
    for (int i=0; i<=allArgs.fillIndex; i++) {
        const ArgContainer *arg = allArgs.array[i];
        if (!arg -> parentArg || !arg -> hasValue) continue;
        if (!HAS_FLAG(arg -> parentArg -> flags, BOOLEAN_ARG) && arg -> parentArg -> argvIndexFound == arg -> argvIndexFound - 1)
            usage();
        if (HAS_FLAG(arg -> flags, ENFORCE_NESTING_ORDER) && arg -> parentArg -> argvIndexFound >= arg -> argvIndexFound)
            usage();
        if (HAS_FLAG(arg-> flags, ENFORCE_STRICT_NESTING_ORDER) && arg -> parentArg -> argvIndexFound != arg -> argvIndexFound - 1)
            usage();
    }
    if (errorFound) {
        carg_terminate();
        exit(EXIT_SUCCESS);
    }
}

inline void carg_terminate(void) {
    if (HAS_FLAG(cargInternalFlags, LIBCARGS_INITIALIZED)) {
        if (allArgs.array) {
            for (int i=0; i<=allArgs.fillIndex; i++) {
                if (allArgs.array[i] -> valueContainer.value && HAS_FLAG(allArgs.array[i] -> flags, HEAP_ALLOCATED)) {
                    _carg_free_nullify(&allArgs.array[i] -> valueContainer.value);
                }
                if (allArgs.array[i] -> nestedArgs) {
                    _carg_free_nullify(&allArgs.array[i] -> nestedArgs);
                }
                if (allArgs.array[i] -> valueContainer.next) {
                    _carg_free_nullify(&allArgs.array[i] -> valueContainer.next -> value);
                    MultiArgContainer *cursor = allArgs.array[i] -> valueContainer.next -> next;
                    MultiArgContainer *cursorToFree = allArgs.array[i] -> valueContainer.next;
                    _carg_free_nullify(&cursorToFree);
                    while (cursor) {
                        _carg_free_nullify(&cursor -> value);
                        cursorToFree = cursor;
                        cursor = cursor -> next;
                        _carg_free_nullify(&cursorToFree);
                    }
                }
                free(allArgs.array[i]);
            }
            _carg_free_nullify(&allArgs.array);
        }
        _carg_free_nullify(&setArgs);
        if (argVector) {
            for (int i=0; i<argCount; i++) {
                _carg_free_nullify(&argVector[i]);
            }
            _carg_free_nullify(&argVector);
        }
    }
    CLEAR_FLAG(cargInternalFlags, LIBCARGS_INITIALIZED);
}

inline int test_printf(char *formatter, ...) {
    va_list args;
    va_start(args, formatter);
    const int returnValue = vsnprintf(NULL, 0, formatter, args);
    va_end(args);
    return returnValue;
}

int test_vsnprintf(const char *formatter, va_list args) { // NOLINT
    const int returnValue = vsnprintf(NULL, 0, formatter, args);
    return returnValue;
}

inline int secure_sprintf_concat(char * const startPointer, char * const endPointer, char **cursor, const char * const formatter, ...) {
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

inline int secure_vsprintf_concat(char * const startPointer, char * const endPointer, char **cursor, const char *formatter, va_list argsToCopy) {
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

inline char *carg_strdup(const char *str) {
    const size_t size = strlen(str);
    char *returnVal = (char *)malloc(sizeof(char) * (size + 1));
    strncpy(returnVal, str, size);
    returnVal[size] = '\0';
    return returnVal;
}

inline void _carg_flag_conditional(uint64_t flag, bool truthiness, const char * const errorMessage) {
    if ((bool)HAS_FLAG(cargInternalFlags, flag) != truthiness) {
        _carg_error(errorMessage);
    }
}

inline void _carg_error(const char *formatter, ...) {
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

inline char *_carg_strtok_string_init(const char *str) {
    char *returnVal = (char *)calloc(strlen(str) + 4, sizeof(char));
    strcpy(returnVal, str);
    memmove(returnVal + 3, returnVal, strlen(returnVal));
    memcpy(returnVal, "\x03\x01\x03", 3);
    return returnVal;
}

inline void _carg_strtok_register_string(char *str) {
    strtok(str, "\x03");
}

inline int _carg_cmp_flag(const char *argument, const char *parameter) {
    return !strcmp(argument, parameter);
}

inline int _carg_is_flag(const char *formatter, const char *toCheck) {
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
    printf("%s\n", usageString);
}

inline bool _carg_adjust_multi_arg_setter(ArgContainer *currentArg, void **flagCopierPointer) {
    if (HAS_FLAG(currentArg -> flags, MULTI_ARG) && currentArg -> hasValue) {
        MultiArgContainer *multiArgCursor = &currentArg->valueContainer;
        while (multiArgCursor -> next) {
            multiArgCursor = multiArgCursor -> next;
        }
        multiArgCursor -> next = (MultiArgContainer *)malloc(sizeof(MultiArgContainer));
        _carg_heap_check(multiArgCursor -> next);
        multiArgCursor -> next -> next = NULL;
        multiArgCursor -> next -> value = malloc(currentArg -> valueSize);
        _carg_heap_check(multiArgCursor -> next -> value);
        *flagCopierPointer = multiArgCursor -> next -> value;
        currentArg -> multiArgIndex++;
        return true;
    }
    return false;
}

inline void _carg_set_named_arg_internal(ArgContainer *currentArg, void **flagCopierPointer, const int argIndex, const char *formatToken, const char *argToFormat, char **formatToTokenize, char **argumentFlagToCompare) {
    if (!currentArg) return;
    if (currentArg -> hasValue && !HAS_FLAG(currentArg -> flags, MULTI_ARG)) {
        usage();
    }
    if (!_carg_adjust_multi_arg_setter(currentArg, flagCopierPointer)) {
        *flagCopierPointer = currentArg -> valueContainer.value;
    }
    if (!*flagCopierPointer) return;
    if (_carg_cmp_flag(formatToken, "bool")) {
        if (!HAS_FLAG(currentArg -> flags, BOOLEAN_ARG)) {
            _carg_error("Argument struct does not contain the BOOLEAN_ARG flag; argument items should be initialized with this flag for readability.\n");
        }
        *(bool *)*flagCopierPointer = !*(bool *)*flagCopierPointer; // Flip flag from its default value. Boolean flags are expected to be chars with a default value.
        currentArg -> hasValue = 1;
        setArgs[argIndex] = currentArg -> hasValue;
    } else {
        if (argIndex >= argCount - 1 && !(carg_string_contains_char(argVector[argIndex], '=') >= 0 && strcmp(formatToken, "bool"))) usage();
        currentArg -> hasValue = sscanf(argToFormat, formatToken, *flagCopierPointer); // If an argument is passed in that does not match its formatter, the value remains default.
        setArgs[argIndex] = currentArg -> hasValue;
        if (!(carg_string_contains_char(argVector[argIndex], '=') >= 0 && strcmp(formatToken, "bool"))) setArgs[argIndex + 1] = currentArg -> hasValue;
        if (!currentArg -> hasValue) {
            _carg_free_nullify(formatToTokenize);
            _carg_free_nullify(argumentFlagToCompare);
            usage();
        }
    }
    currentArg -> argvIndexFound = argIndex;
    if (formatToken) strncpy(currentArg -> formatterUsed, formatToken, maxFormatterSize - 1);
}

inline void _carg_validate_formatter_extended(const char *formatToken) {
    if (formatToken && !_carg_cmp_flag(formatToken, "bool") && formatToken[0] != '%') {
        _carg_error("Cannot parse token %s\n", formatToken);
    }
}

inline void _carg_validate_formatter(const char *formatToken) {
    if (formatToken && formatToken[0] != '%') {
        _carg_error("Cannot parse token %s\n", formatToken);
    }
}

inline void _validateFlag(const char *flagToken) {
    if (flagToken && carg_string_contains_char(flagToken, '%') > -1) {
        _carg_error("Cannot parse token %s\n", flagToken);
    }
}

inline bool _carg_adjust_named_assign(const int argIndex, const char *formatToken, const char *flagToken, const char **argToFormat, char *argumentFlagToCompare) {
    if (argumentFlagToCompare && carg_string_contains_char(argVector[argIndex], '=') >= 0 && strcmp(formatToken, "bool")) {
        int ncompare = 0;
        while (argVector[argIndex][ncompare] != '=') ncompare++;
        if (strncmp(flagToken, argVector[argIndex], ncompare)) return false;
        argumentFlagToCompare[ncompare] = '\0';
        *argToFormat = argVector[argIndex]+ncompare+1;
    }
    return true;
}

inline void _carg_reference_named_arg_formatter(const int argIndex, const char *format, va_list args) { // NOLINT
    if (setArgs[argIndex]) return;
    va_list argsCopy;
    va_copy(argsCopy, args);
    char *formatToTokenize = _carg_strtok_string_init(format);
    _carg_heap_check(formatToTokenize);
    void *flagCopierPointer = NULL;
    char *argumentFlagToCompare = carg_strdup(argVector[argIndex]);
    _carg_heap_check(argumentFlagToCompare);
    const char *argToFormat;
    if (argIndex < argCount - 1) argToFormat = argVector[argIndex + 1];
    else argToFormat = NULL;
    _carg_strtok_register_string(formatToTokenize);
    while (1) {
        const char *flagToken = strtok(NULL, ": ");
        const char *formatToken = strtok(NULL, ": ");
        if (!flagToken) break;
        _validateFlag(flagToken);
        _carg_validate_formatter_extended(formatToken);
        ArgContainer *currentArg = va_arg(argsCopy, ArgContainer *);
        if (!_carg_adjust_named_assign(argIndex, formatToken, flagToken, &argToFormat, argumentFlagToCompare)) continue;
        if (!strcmp(flagToken, argumentFlagToCompare)) {
            _carg_set_named_arg_internal(currentArg, &flagCopierPointer, argIndex, formatToken, argToFormat, &formatToTokenize, &argumentFlagToCompare);
            break;
        }
    }
    _carg_free_nullify(&formatToTokenize);
    _carg_free_nullify(&argumentFlagToCompare);
}

inline void _carg_reference_positional_arg_formatter(ArgContainer *currentArg, const int i, void **formatToTokenizeAllocation, char **formatToTokenize, char **tokenSavePointer, void **flagCopierPointer) {
    *formatToTokenize = strtok_r(*formatToTokenize, " ", tokenSavePointer);
    if (!HAS_FLAG(currentArg -> flags, POSITIONAL_ARG)) {
        _carg_error("Positional arg setter called on named argument. Please fix this!\n");
    }
    *flagCopierPointer = currentArg -> valueContainer.value;
    currentArg -> hasValue = sscanf(argVector[i], *formatToTokenize, *flagCopierPointer);
    setArgs[i] = currentArg -> hasValue;
    if (!currentArg -> hasValue) {
        _carg_free_nullify(formatToTokenizeAllocation);
        usage();
    }
    currentArg -> argvIndexFound = i;
    if (*formatToTokenize) {
        strncpy(currentArg -> formatterUsed, *formatToTokenize, maxFormatterSize - 1);
    }
    *formatToTokenize = *tokenSavePointer;
}

inline void _carg_reference_grouped_boolean_arg_formatter(const int i, const size_t j, const char *noPrefixFormat, bool **flagCopierPointer, va_list args) {
    va_list argsCopy;
    va_copy(argsCopy, args);
    if (carg_string_contains_char(argVector[i], noPrefixFormat[j]) >= 0) {
        ArgContainer *currentArg = NULL;
        for (size_t k=0; k<=j; k++) {
            currentArg = va_arg(argsCopy, ArgContainer *);
            *flagCopierPointer = (bool *)currentArg -> valueContainer.value;
        }
        va_end(argsCopy);
        va_copy(argsCopy, args);
        if (*flagCopierPointer && currentArg) {
            currentArg -> hasValue = 1;
            setArgs[i] = currentArg -> hasValue;
            currentArg -> argvIndexFound = i;
            **flagCopierPointer = !**flagCopierPointer;
        }
    }
    va_end(argsCopy);
}

inline int _carg_set_nested_arg_internal(ArgContainer *arg) {
    if (!arg) return 0;
    if (!HAS_FLAG(arg -> flags, NESTED_ARG)) {
        _carg_error("Nested flag setter called on non-nested argument. Fix this!\n");
    }
    if (arg -> hasValue) return 0;
    for (int i=positionalArgCount+1; i<argCount; i++) {
        if (!strcmp(arg -> nestedArgString, argVector[i])) {
            if (HAS_FLAG(arg -> flags, BOOLEAN_ARG)) {
                *(bool *)arg -> valueContainer.value = !*(bool *)arg -> valueContainer.value;
                arg -> hasValue = 1;
            } else {
                if (i >= argCount - 1) usage();
                arg -> hasValue = sscanf(argVector[i+1], arg -> formatterUsed, arg -> valueContainer.value);
                setArgs[i+1] = arg -> hasValue;
            }
            setArgs[i] = arg -> hasValue;
            arg -> argvIndexFound = i;
            return 1;
        }
    }
    return 0;
}

inline void _carg_set_env_defaults_internal(char **stringToTokenize, char **tokenSavePointer, void **stringAllocation, va_list args) {
    va_list argsCopy;
    va_copy(argsCopy, args);
    ArgContainer *currentArg = NULL;
    while (1) {
        char *envVarToken = strtok_r(*stringToTokenize, ": ", tokenSavePointer);
        char *formatToken = strtok_r(NULL, ": ", tokenSavePointer);
        *stringToTokenize = *tokenSavePointer;
        _validateFlag(envVarToken);
        _carg_validate_formatter(formatToken);
        if (!envVarToken || !formatToken) break;
        const char *envVarValue = getenv(envVarToken);
        currentArg = va_arg(argsCopy, ArgContainer *);
        if (!envVarValue) continue;
        if (!currentArg -> hasValue) {
            currentArg -> hasValue = sscanf(envVarValue, formatToken, currentArg -> valueContainer.value);
        }
        if (!currentArg -> hasValue) {
            _carg_free_nullify(stringAllocation);
            _carg_error("Unable to grab environment variable %s\n", envVarToken);
        }
        strncpy(currentArg -> formatterUsed, formatToken, maxFormatterSize - 1);
    }
    va_end(argsCopy);
}

inline void _carg_print_positional_usage_buffer(void) {
    for (int i=0; i<=allArgs.fillIndex; i++) {
        if (!allArgs.array[i]) break;
        if (!HAS_FLAG(allArgs.array[i] -> flags, POSITIONAL_ARG)) continue;
        if (allArgs.array[i] -> usageString && allArgs.array[i] -> usageString[0]) secure_sprintf_concat(usageStringCursor, usageStringEnd, &usageStringCursor, "%s ", allArgs.array[i]->usageString);
        else if (allArgs.array[i] -> nestedArgString && allArgs.array[i] -> nestedArgString[0]) secure_sprintf_concat(usageStringCursor, usageStringEnd, &usageStringCursor, "%s ", allArgs.array[i]->nestedArgString);
    }
}

inline void _carg_print_non_positional_usage_buffer(void) {
    for (int i=0; i<=allArgs.fillIndex; i++) {
        if (HAS_FLAG(allArgs.array[i] -> flags, BOOLEAN_ARG && allArgs.array[i] -> usageString[0])) {
            secure_sprintf_concat(usageStringCursor, usageStringEnd, &usageStringCursor, "%s ", allArgs.array[i]->usageString);
        }
    }
    for (int i=0; i<=allArgs.fillIndex; i++) {
        if (HAS_FLAG(allArgs.array[i] -> flags, NESTED_ARG_ROOT)) {
            secure_sprintf_concat(usageStringCursor, usageStringEnd, &usageStringCursor, "%s ", allArgs.array[i]->nestedArgString);
        }
    }
    for (int i=0; i<=allArgs.fillIndex; i++) {
        if (HAS_FLAG(allArgs.array[i] -> flags, POSITIONAL_ARG)) continue;
        if (HAS_FLAG(allArgs.array[i] -> flags, BOOLEAN_ARG) && !allArgs.array[i] -> nestedArgString[0]) {
            continue;
        }
        if (allArgs.array[i] -> usageString[0]) {
            secure_sprintf_concat(usageStringCursor, usageStringEnd, &usageStringCursor, "%s ", allArgs.array[i] -> usageString);
        }
        else if (allArgs.array[i] -> nestedArgString[0]) {}
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
    MultiArgContainer *cursor = argument -> valueContainer.next;\
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
    MultiArgContainer *cursor = argument -> valueContainer.next;\
    while (cursor) {\
        printf("\tValue: ");\
        if (carg_string_contains_char((argument) -> formatterUsed, '\n') >= 0) printf("%s", (cursor -> value)); /* This allows the string scanf() formatter with spaces to work. */\
        else printf((argument) -> formatterUsed, cursor -> value);\
        printf("\n");\
        cursor = cursor -> next;\
    }\
} while (0)
