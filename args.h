//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//  SECTION: Includes, Type Definitions, and Function Prototypes
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// ReSharper disable CppNonInlineFunctionDefinitionInHeaderFile
#pragma once
#ifdef _MSC_VER
    #define strtok_r strtok_s
    #define strdup _strdup
    #define _CRT_SECURE_NO_WARNINGS // sscanf() is required for this project.
    #pragma warning(disable:4003) // Some variadic macros in this library do not use their variadic arguments.
#endif
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>

#define MAX_FORMATTER_SIZE 128

typedef struct multiArgLinkedList {
    struct multiArgLinkedList *next;
    void *value;
} multiArgLinkedList;

typedef struct argStruct {
    multiArgLinkedList valueContainer;
    size_t valueSize; // For non-heap arguments, this will contain the size of the holder variable. For heap-allocated arguments, this will contain the size of heap-allocated memory.
    bool hasValue;
    int argvIndexFound; // The index where an argument is found. This stores the argv index of the flag, not the value associated with it. Set to -1 when not found.
    int flags;

    const char * const type;
    const char * const usageString;
    char formatterUsed[MAX_FORMATTER_SIZE];

    char *nestedArgString;
    int nestedArgFillIndex;
    size_t nestedArgArraySize;
    struct argStruct *parentArg;
    struct argStruct **nestedArgs;
} argStruct;

typedef struct argArray {
    size_t size;
    int fillIndex;
    argStruct **array;
} argArray;

typedef void(*voidFuncPtr)(void); // Some syntax highlighters don't like seeing function pointer parentheses in a macro.

void libcargTerminate(void);

void _usageDefault(void); // The standard function to print the usage message.

void (*_usagePointer)(void) = _usageDefault; // Usage function cursor which may be changed by the client code.

void usage(void); // The function which should be called after setting usage information to display a usage message.

char *contains(char *, const char *);

const char *basename(const char *);

int charInString(const char *, char);

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//  SECTION: Global Variables and Definitions
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#define usageStringSize 2048U

int argCount = 1;

char **argVector = NULL;

argArray allArgs = {.fillIndex = -1, .array = NULL, .size = 0};

int *setArgs = NULL;

int positionalArgCount = 0;

char usageString[usageStringSize] = "Please specify a usage message in your client code. You can do this via setUsageMessage() or usageMessageAutoGenerate().";

char *usageStringCursor = usageString; // The default usage message should be immediately overwritten when a usage message setter is called.

char * const usageStringEnd = usageString + usageStringSize;

uint64_t libcargInternalFlags = 0;

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//  SECTION: Flags, Flag Checkers, and Initializer Macros
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//  Generic Flags.
#define NO_FLAGS (0ULL) // Set no flags when creating an argument.

#define NO_DEFAULT_VALUE {0} // Set default argument to 0 in arg initializers, for readability purposes.

#define NONE // Empty and does nothing. For semantics.

//  Function Initializer Flags for libcargInternalFlags. (Do not set these manually; they are here to give warnings when something goes wrong!)
#define LIBCARGS_INITIALIZED (1ULL << 0ULL)

#define NAMED_ARGS_SET (1ULL<<1ULL)

#define POSITIONAL_ARGS_SET (1ULL<<2ULL)

#define GROUPED_ARGS_SET (1ULL<<3ULL)

#define NESTED_ARGS_SET (1ULL<<4ULL)

#define OVERRIDE_CALLBACKS_SET (1ULL<<5ULL)

#define ASSERTIONS_SET (1ULL<<6ULL)

#define USAGE_MESSAGE_SET (1ULL<<7ULL)

//  Internal Argument Flags. (These should be set by functions and not the user.)
#define HEAP_ALLOCATED (1ULL<<0ULL)

#define NESTED_ARG (1ULL<<1ULL)

#define NESTED_ARG_ROOT (1ULL<<2ULL)

//  Argument Flags. (These should be set by the user.)
#define POSITIONAL_ARG (1ULL<<3ULL)

#define BOOLEAN_ARG (1ULL<<4ULL)

#define ENFORCE_NESTING_ORDER (1ULL<<5ULL)

#define ENFORCE_STRICT_NESTING_ORDER (1ULL<<6ULL)

#define MULTI_ARG (1ULL<<7ULL)

//  Getters and Setters.
#define hasFlag(item, flag) (item & flag)

#define setFlag(item, flag) (item |= flag)

#define clearFlag(item, flag) (item &= ~flag)

#define toggleFlag(item, flag) (item ^= flag)

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//  SECTION: Assertion Macros
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//  This is designed to be used in the argAssert() function to assert that an argument cannot have a default value.
#define REQUIRED_ARGUMENT(varName) varName.hasValue

//  Assert that two arguments cannot be declared by the user at the same time.
#define MUTUALLY_EXCLUSIVE(varName1, varName2) !(varName1.hasValue && varName2.hasValue)

//  Assert that if the first argument is used, the second must also be used.
#define MUTUALLY_REQUIRED(varName1, varName2) varName1.hasValue ? varName2.hasValue : 1

// For use in argAssert.
#define USAGE_MESSAGE NULL

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//  SECTION: Internal Macros
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#define VA_ARG_1(arg1, ...) arg1

#define VA_ARG_2(arg1, arg2, ...) arg2

#define EXPAND(x) x

#define EXPAND_R(x) EXPAND(x)

#define TOKEN_TO_STRING(x) #x

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//  SECTION: Internal Functions and Definitions
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//  This function will return the number of characters that would be written from a series of string formatters.
//  However, it will not write to anything.
int test_printf(char *formatter, ...) {
    va_list args;
    va_start(args, formatter);
    const int returnValue = vsnprintf(NULL, 0, formatter, args);
    va_end(args);
    return returnValue;
}

//  This function will return the number of characters that would be written from a series of string formatters.
int test_vsnprintf(const char *formatter, va_list args) {
    const int returnValue = vsnprintf(NULL, 0, formatter, args);
    return returnValue;
}

//  This function keeps track of a start and end pointer of a string. The end pointer of the string should be the start
//  of the string plus the size of the string, or in other words, the index right after the expected location of the
//  null terminator in a string.
int secure_sprintf(char * const startPointer, char * const endPointer, char **cursor, char *formatter, ...) {
    if (startPointer > endPointer) return 0;
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

#define _checkForAssertion() do {\
    if (hasFlag(libcargInternalFlags, ASSERTIONS_SET)) {\
        fprintf(stderr, "Error: Assertion initializer called before args are initialized. Please fix this!\n");\
        libcargTerminate();\
        exit(EXIT_FAILURE);\
    }\
} while (0)

//  A semantic wrapper to compare flags against their parameters.
//  For internal use only.
int _compareFlag(const char *argument, const char *parameter) {
    return !strcmp(argument, parameter);
}

int _isFlag(const char *formatter, const char *toCheck) {
    char *internalFormatter = strdup(formatter);
    void *internalFormatterAllocation = internalFormatter;
    char *savePointer = NULL;
    while (1) {
        char *flagItem = strtok_r(internalFormatter, ": ", &savePointer);
        strtok_r(NULL, ": ", &savePointer); // Discard formatter item
        internalFormatter = savePointer;
        if (!flagItem) {
            break;
        }
        if (_compareFlag(toCheck, flagItem)) {
            return 1;
        }
    }
    free(internalFormatterAllocation);
    return 0;
}

//  Prints the usage message.
void _usageDefault(void) {
    printf("%s\n", usageString);
}

//  Fetches how many arguments are expected based on string formatter tokenization.
//  For internal use only.
int _getArgCountFromFormatter(char *argFormatter) {
    int returnVal = 1;
    char *savePointer = NULL;
    strtok_r(argFormatter, " ", &savePointer);
    while (strtok_r(NULL, " ", &savePointer)) returnVal++;
    return returnVal;
}

//  Checks a va_list passed in from setFlagsFromNamedArgs() to set arguments accordingly.
//  For internal use only.
void _checkArgAgainstFormatter(const int argIndex, const char *argFormatter, va_list outerArgs) { // NOLINT
    if (setArgs[argIndex]) return;
    va_list formatterArgs;
    va_copy(formatterArgs, outerArgs);
    char *internalFormatter = strdup(argFormatter);
    void *internalFormatterAllocation = internalFormatter;
    char *savePointer = NULL;
    void *flagCopierPointer = NULL;
    char *argumentFlagToCompare = strdup(argVector[argIndex]);
    const char *formatItemToCopy = argVector[argIndex + 1];
    while (1) {
        const char *flagItem = strtok_r(internalFormatter, ": ", &savePointer);
        const char *formatterItem = strtok_r(NULL, ": ", &savePointer);
        internalFormatter = savePointer;
        if (!flagItem) break;
        argStruct *currentArg = va_arg(formatterArgs, argStruct *);
        if (charInString(argVector[argIndex], '=') >= 0 && strcmp(formatterItem, "bool")) {
            int ncompare = 0;
            while (argVector[argIndex][ncompare] != '=') ncompare++;
            if (strncmp(flagItem,argVector[argIndex], ncompare)) continue;
            argumentFlagToCompare[ncompare] = '\0';
            formatItemToCopy = argVector[argIndex]+ncompare+1;
        }
        if (!strcmp(flagItem, argumentFlagToCompare)) {
            if (!currentArg) return;
            if (currentArg -> hasValue && !hasFlag(currentArg -> flags, MULTI_ARG)) usage(); // Duplicate named arguments
            if (hasFlag(currentArg -> flags, MULTI_ARG) && currentArg -> hasValue) {
                multiArgLinkedList *multiArgCursor = &currentArg->valueContainer;
                while (multiArgCursor -> next) {
                    multiArgCursor = multiArgCursor -> next;
                }
                multiArgCursor -> next = (multiArgLinkedList *)malloc(sizeof(multiArgLinkedList));\
                multiArgCursor -> next -> next = NULL;
                multiArgCursor -> next -> value = malloc(currentArg -> valueSize);
                flagCopierPointer = multiArgCursor -> next -> value;
            } else {
                flagCopierPointer = currentArg -> valueContainer.value;
            }

            if (!flagCopierPointer) return;
            if (_compareFlag(formatterItem, "bool")) {
                if (!hasFlag(currentArg -> flags, BOOLEAN_ARG)) {
                    fprintf(stderr, "Argument struct does not contain the BOOLEAN_ARG flag; argument items should be initialized with this flag for readability.\n");
                    libcargTerminate();
                    exit(EXIT_FAILURE);
                }
                *(bool *)flagCopierPointer = !*(bool *)flagCopierPointer; // Flip flag from its default value. Boolean flags are expected to be chars with a default value.
                currentArg -> hasValue = 1;
                setArgs[argIndex] = currentArg -> hasValue;
            } else {
                if (argIndex >= argCount - 1 && !(charInString(argVector[argIndex], '=') >= 0 && strcmp(formatterItem, "bool"))) usage();
                currentArg -> hasValue = sscanf(formatItemToCopy, formatterItem, flagCopierPointer); // If an argument is passed in that does not match its formatter, the value remains default.
                setArgs[argIndex] = currentArg -> hasValue;
                if (!(charInString(argVector[argIndex], '=') >= 0 && strcmp(formatterItem, "bool"))) setArgs[argIndex + 1] = currentArg -> hasValue;
                if (!currentArg -> hasValue) {
                    usage();
                }
            }
            currentArg -> argvIndexFound = argIndex;
            if (formatterItem) strncpy(currentArg -> formatterUsed, formatterItem, MAX_FORMATTER_SIZE - 1);
            break;
        }
    }
    free(internalFormatterAllocation);
    free(argumentFlagToCompare);
}

int _setFlagFromNestedArgInternal(argStruct *arg) {
    if (!arg) return 0;
    if (!hasFlag(arg -> flags, NESTED_ARG)) {
        fprintf(stderr, "Error: Nested flag setter called on non-nested argument. Fix this!\n");
        libcargTerminate();
        exit(EXIT_FAILURE);
    }
    if (arg -> hasValue) return 0;
    for (int i=positionalArgCount+1; i<argCount; i++) {
        if (!strcmp(arg -> nestedArgString, argVector[i])) {
            if (hasFlag(arg -> flags, BOOLEAN_ARG)) {
                *(bool *)arg -> valueContainer.value = !*(bool *)arg -> valueContainer.value;
                arg -> hasValue = 1;
            } else {
                if (i >= argCount - 1) usage();
                arg -> hasValue = sscanf(argVector[i+1], arg -> formatterUsed, arg -> valueContainer.value);
                setArgs[i+1] = arg -> hasValue;
            }
            setArgs[i] = arg -> hasValue;
            arg -> argvIndexFound = i;
            if (arg -> parentArg && !hasFlag(arg -> parentArg -> flags, BOOLEAN_ARG) && arg -> parentArg -> argvIndexFound == arg -> argvIndexFound - 1) usage();
            if (arg -> parentArg && !hasFlag(arg -> parentArg -> flags, BOOLEAN_ARG) && hasFlag(arg -> parentArg -> flags, ENFORCE_STRICT_NESTING_ORDER) && arg -> parentArg -> argvIndexFound != arg -> argvIndexFound - 2) usage(); {}
            if (hasFlag(arg -> flags, ENFORCE_STRICT_NESTING_ORDER) && arg -> parentArg && arg -> parentArg -> hasValue && arg -> parentArg -> argvIndexFound != arg -> argvIndexFound - 1) {
                usage();
            }
            if (hasFlag(arg -> flags, ENFORCE_NESTING_ORDER) && arg -> parentArg && arg -> parentArg -> hasValue && arg -> parentArg -> argvIndexFound >= arg -> argvIndexFound) {
                usage();
            }
            return 1;
        }
    }
    return 0;
}

void _printAllPositionalArgsToUsageBuffer(void) {
    for (int i=0; i<=allArgs.fillIndex; i++) {
        if (!allArgs.array[i]) break;
        if (!hasFlag(allArgs.array[i] -> flags, POSITIONAL_ARG)) continue;
        if (allArgs.array[i] -> usageString[0]) secure_sprintf(usageStringCursor, usageStringEnd, &usageStringCursor, "%s ", allArgs.array[i]->usageString);
        else if (allArgs.array[i] -> nestedArgString[0]) secure_sprintf(usageStringCursor, usageStringEnd, &usageStringCursor, "%s ", allArgs.array[i]->nestedArgString);
        secure_sprintf(usageStringCursor, usageStringEnd, &usageStringCursor, "%s ",allArgs.array[i]->type);
    }
}

void _printAllNonPositionalArgsToUsageBuffer(void) {
    for (int i=0; i<=allArgs.fillIndex; i++) {
        if (hasFlag(allArgs.array[i] -> flags, BOOLEAN_ARG && allArgs.array[i] -> usageString[0])) secure_sprintf(usageStringCursor, usageStringEnd, &usageStringCursor, "%s ", allArgs.array[i]->usageString);
    }
    for (int i=0; i<=allArgs.fillIndex; i++) {
        if (hasFlag(allArgs.array[i] -> flags, NESTED_ARG_ROOT)) secure_sprintf(usageStringCursor, usageStringEnd, &usageStringCursor, "%s ", allArgs.array[i]->nestedArgString);
    }
    for (int i=0; i<=allArgs.fillIndex; i++) {
        if (hasFlag(allArgs.array[i] -> flags, POSITIONAL_ARG)) continue;
        if (hasFlag(allArgs.array[i] -> flags, BOOLEAN_ARG) && !allArgs.array[i] -> nestedArgString[0]) {
            continue;
        }
        if (allArgs.array[i] -> usageString[0]) {
            secure_sprintf(usageStringCursor, usageStringEnd, &usageStringCursor, "%s ", allArgs.array[i]->usageString);
            secure_sprintf(usageStringCursor, usageStringEnd, &usageStringCursor, "%s ",allArgs.array[i]->type);
        }
        else if (allArgs.array[i] -> nestedArgString[0]) {
        } else {
            secure_sprintf(usageStringCursor, usageStringEnd, &usageStringCursor, "%s ",allArgs.array[i]->type);
        }
    }
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//  SECTION: User-Facing Functions and Definitions
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//  Check if a substring is present in a string.
//  Returns a pointer to where the substring starts in the string.
//  If the substring does not exist in the parent string, return NULL.
char *contains(char *testString, const char *substring) {
    while (strlen(testString) >= strlen(substring)) {
        if (!strncmp(testString, substring, strlen(substring))) {
            return testString;
        }
        testString++;
    }
    return NULL;
}

//  Returns the index where a character is present in a string.
//  Returns -1 if the character is not present in the string.
int charInString(const char *testString, const char subchar) {
    for (int i=0; i<strlen(testString); i++) {
        if (testString[i] == subchar) {
            return i;
        }
    }
    return -1;
}

//  Fetches the basename of a file from a path.
const char *basename(const char * const pathStart) {
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

//  This function uses a string formatter to generate a usage message to be used when usage() is called.
#define setUsageMessage(...) do { \
    if (hasFlag(libcargInternalFlags, USAGE_MESSAGE_SET)) {\
        fprintf(stderr, "Error: usage message set by user twice. Please fix this!\n");\
        libcargTerminate();\
        exit(EXIT_FAILURE);\
    }\
    secure_sprintf(usageStringCursor, usageStringEnd, &usageStringCursor, __VA_ARGS__);\
    setFlag(libcargInternalFlags, USAGE_MESSAGE_SET);\
} while (0)

//  Generates a usage message automatically based on data entered to argument constructors. Make sure to call this after all arguments have been initialized.
//  setUsageMessage() should not be used alongside this function, otherwise it would obfuscate the generated usage message generated here.
void usageMessageAutoGenerate(void) {
    if (!hasFlag(libcargInternalFlags, LIBCARGS_INITIALIZED)) {
        fprintf(stderr, "Error: usage message auto-generated before library initialization. Please fix this!\n");
        libcargTerminate();
        exit(EXIT_FAILURE);
    }
    if (hasFlag(libcargInternalFlags, USAGE_MESSAGE_SET)) {
        fprintf(stderr, "Error: usage message set by user twice. Please fix this!\n");
        libcargTerminate();
        exit(EXIT_FAILURE);
    }
    secure_sprintf(usageStringCursor, usageStringEnd, &usageStringCursor, "%s%s ", "Usage: ", basename(argVector[0]));
    _printAllPositionalArgsToUsageBuffer();
    _printAllNonPositionalArgsToUsageBuffer();
    setFlag(libcargInternalFlags, USAGE_MESSAGE_SET);
}

//  This function will set a function to call for the usage message. This function is implemented for versatility.
//  However, using it is not recommended unless there is some requisite functionality to the client code which the usage message generation in this library does not provide.
void setUsageFunction(void (*funcArg)(void)) {
    if (hasFlag(libcargInternalFlags, USAGE_MESSAGE_SET)) {
        fprintf(stderr, "Err or: usage message set by user twice. Please fix this!\n");
        libcargTerminate();
        exit(EXIT_FAILURE);
    }
    _usagePointer = funcArg;
    setFlag(libcargInternalFlags, USAGE_MESSAGE_SET);
}

//  This function will display the set usage message or call the set usage function and then terminate the program.
void usage(void) {
    _usagePointer();
    libcargTerminate();
    exit(EXIT_SUCCESS);
}

void libcargInit(const int argc, const char * const * const argv){
    argCount = argc;
    argVector = (char **)malloc(sizeof(char *) * argCount);
    if (!argVector) {
        printf("Heap allocation failure. Terminating\n");
        libcargTerminate();
        exit(EXIT_FAILURE);
    }
    memcpy(argVector, argv, sizeof(char *) * argCount);
    setArgs = (int *)calloc(argCount, sizeof(int));
    setFlag(libcargInternalFlags, LIBCARGS_INITIALIZED);
}

//  This macro creates a struct that contains the variable information as well as whether the argument has already been specified or not.
//  It is designed to take the name of a variable which it uses to make a struct containing the variable type.
//  The value can later be accessed via the name arg1Value. This is achieved with token pasting.
//  For variables with basic types, they can be declared with basicArgInit(type, name, value) instead.
//  For any kind of pointer without heap allocation, use pointerArgInit().
//  For any kind of pointer with heap allocation, use heapArgInit().
//  Finally, a variadic string argument may be passed to this macro to set a usage string with the usageMessageAutoGenerate() function.
#define argInit(leftType, varName, rightType, val, flagsArg, ...)\
    if (!hasFlag(libcargInternalFlags, LIBCARGS_INITIALIZED)) {\
        fprintf(stderr, "Error: attempt to initialize argument before library initialization. Please fix this!\n");\
        libcargTerminate();\
        exit(EXIT_FAILURE);\
    }\
    leftType varName##Value rightType = val;\
    argStruct varName = (argStruct) {\
            .valueContainer = {.value = &varName##Value, .next = NULL},\
            .hasValue = 0,\
            .flags = flagsArg,\
            .argvIndexFound = -1,\
            .nestedArgString = "",\
            .valueSize = sizeof(varName##Value),\
            .nestedArgFillIndex = -1,\
            .formatterUsed = {0},\
            .usageString = "" VA_ARG_1(__VA_ARGS__),\
            .parentArg = NULL,\
            .nestedArgs = NULL,\
            .nestedArgArraySize = 0,\
            .type = "<" TOKEN_TO_STRING(leftType) TOKEN_TO_STRING(rightType) ">"\
    };\
    if (hasFlag(flagsArg, POSITIONAL_ARG)) positionalArgCount++;\
    if (allArgs.array) {\
        allArgs.fillIndex++;\
        if (allArgs.fillIndex >= (allArgs.size / 2)){\
            allArgs.size *= 2;\
            void *argArrayReallocation = realloc(allArgs.array, allArgs.size * sizeof(argStruct *));\
            if (!argArrayReallocation) {\
                printf("Heap allocation failure. Terminating\n");\
                libcargTerminate();\
                exit(EXIT_FAILURE);\
            }\
            allArgs.array = (argStruct **)argArrayReallocation;\
        }\
        allArgs.array[allArgs.fillIndex] = &varName;\
    } else {\
        allArgs.array = (argStruct **)malloc(sizeof(argStruct *) * 4);\
        if (!allArgs.array) {\
            printf("Heap allocation failure. Terminating\n");\
            libcargTerminate();\
            exit(EXIT_FAILURE);\
        }\
        allArgs.array[0] = &varName;\
        allArgs.fillIndex++;\
        allArgs.size = 4;\
    }

//  This macro is for initializing arguments which point to heap-allocated memory.
//  Make sure to free the pointer in the varName##Value variable when finished using this argument.
//  Keep in mind that heap-allocated arguments cannot be directly initialized with a default value in this macro.
#define heapArgInit(leftType, varName, rightType, flagsArg, size, ...)\
    argInit(leftType, varName, rightType, NO_DEFAULT_VALUE, ((flagsArg) | (HEAP_ALLOCATED)), __VA_ARGS__)\
    void *varName##Ptr = malloc(size);\
    memset(varName##Ptr, 0, size);\
    varName##Value = varName##Ptr;\
    varName.valueContainer = (multiArgLinkedList) {.value = varName##Ptr, .next = NULL};\
    varName.valueSize = size;

//  This macro is for initializing arguments which point to memory that does not need to be freed by this library.
#define pointerArgInit(leftType, varName, rightType, val, flagsArg, ...)\
    argInit(leftType, varName, rightType, val, flagsArg, __VA_ARGS__)\
    varName.valueContainer = (multiArgLinkedList) {.value = varName##Value, .next = NULL};\
    varName.valueSize = sizeof(varName##Value);

//  A wrapper for argInit().
#define basicArgInit(type, varName, value, flagsArg, ...)\
    argInit(type, varName, NONE, value, flagsArg, __VA_ARGS__)

//  Changes where the value of an argument is saved to. Ensure the readjusted pointer is of the correct type.
//  This is useful for saving an argument value in a global variable.
//  If a heap-allocated argument is readjusted, the old memory it pointed to will be freed.
//  This will only change the variable where the argument is saved, meaning the save variable will not follow the "Value" convention the rest of this code follows unless you name it accordingly.
void adjustArgumentCursor(argStruct *arg, void *newItem) {
    if (hasFlag(arg->flags, HEAP_ALLOCATED)) free(arg -> valueContainer.value);
    arg -> valueContainer.value = newItem;
}

//  Pass in the argument count, argument vector, and all argument structs generated from the argInit()
//  and basicArgInit() functions to set them based on the argument vector.
void setFlagsFromNamedArgs(const char * const argFormatter, ...) {
    if (!hasFlag(libcargInternalFlags, LIBCARGS_INITIALIZED)) {
        fprintf(stderr, "Error: setter called before library initialization. Please fix this!\n");
        libcargTerminate();
        exit(EXIT_FAILURE);
    }
    if (hasFlag(libcargInternalFlags, NAMED_ARGS_SET)) {
        fprintf(stderr, "Error: Named args initializer called multiple times. Please fix this!\n");
        libcargTerminate();
        exit(EXIT_FAILURE);
    }
    if (hasFlag(libcargInternalFlags, GROUPED_ARGS_SET)) {
        fprintf(stderr, "Error: Grouped args initializer called before named args initializer. Please fix this!\n");
        libcargTerminate();
        exit(EXIT_FAILURE);
    }
    _checkForAssertion();
    va_list formatterArgs;
    va_start(formatterArgs, argFormatter);
    for (int i=positionalArgCount + 1; i<argCount; i++) {
        _checkArgAgainstFormatter(i, argFormatter, formatterArgs);
    }
    va_end(formatterArgs);
    setFlag(libcargInternalFlags, NAMED_ARGS_SET);
}

//  This sets values for positional arguments in mostly the same format as setFlagsFromNamedArgs().
//  However, this function is for arguments without preceding flags; therefore, flags should not be included in the formatter.
void setFlagsFromPositionalArgs(const char *argFormatter, ...) {
    if (!hasFlag(libcargInternalFlags, LIBCARGS_INITIALIZED)) {
        fprintf(stderr, "Error: setter called before library initialization. Please fix this!\n");
        libcargTerminate();
        exit(EXIT_FAILURE);
    }
    if (hasFlag(libcargInternalFlags, POSITIONAL_ARGS_SET)) {
        fprintf(stderr, "Error: Positional args initializer called multiple times. Please fix this!\n");
        libcargTerminate();
        exit(EXIT_FAILURE);
    }
    if (hasFlag(libcargInternalFlags, GROUPED_ARGS_SET)) {
        fprintf(stderr, "Error: Grouped args initializer called before positional args initializer. Please fix this!\n");
        libcargTerminate();
        exit(EXIT_FAILURE);
    }
    _checkForAssertion();
    if (argCount <= positionalArgCount) usage();
    char *internalFormatter = strdup(argFormatter);
    void *internalFormatterAllocation = internalFormatter;
    char *savePointer = NULL;
    const char *currentFormatter = NULL;
    void *flagCopierPointer = NULL;
    va_list formatterArgs;
    va_start(formatterArgs, argFormatter);
    for (int i=1; i<positionalArgCount+1; i++) {
        currentFormatter = strtok_r(internalFormatter, " ", &savePointer);
        internalFormatter = savePointer;
        argStruct *currentArg = va_arg(formatterArgs, argStruct *);
        if (!hasFlag(currentArg -> flags, POSITIONAL_ARG)) {
            fprintf(stderr, "Error: Positional arg setter called on named arg. Please fix this!\n");
            libcargTerminate();
            exit(EXIT_FAILURE);
        }
        flagCopierPointer = currentArg -> valueContainer.value;
        currentArg -> hasValue = sscanf(argVector[i], currentFormatter, flagCopierPointer);
        setArgs[i] = currentArg -> hasValue;
        if (!currentArg -> hasValue) {
            usage();
        }
        currentArg -> argvIndexFound = i;
        if (currentFormatter) {
            strncpy(currentArg -> formatterUsed, currentFormatter, MAX_FORMATTER_SIZE - 1);
        }
    }
    free(internalFormatterAllocation);
    va_end(formatterArgs);
    setFlag(libcargInternalFlags, POSITIONAL_ARGS_SET);
}

//  Creates boolean flags, which should be individual characters, that can be grouped under one flag in any order.
//  Ex: -b and -c can be grouped as -bc or -cb to toggle both flags.
//  To make a non-groupable boolean flag, use the setFlagsFromNamedArgs() function with the bool formatter.
//  This function takes a string containing a one-character prefix and each flag represented by a single character.
//  This function also takes in the argument structs to set, like the other string formatter-esque functions.
//  The argument structs each correspond to a character in the flag string, so the order matters!
void setFlagsFromGroupedBooleanArgs(const char *argFormatter, ...) {
    if (!hasFlag(libcargInternalFlags, LIBCARGS_INITIALIZED)) {
        fprintf(stderr, "Error: setter called before library initialization. Please fix this!\n");
        libcargTerminate();
        exit(EXIT_FAILURE);
    }
    if (hasFlag(libcargInternalFlags, GROUPED_ARGS_SET)) {
        fprintf(stderr, "Error: Grouped args initializer called multiple times. Please fix this!\n");
        libcargTerminate();
        exit(EXIT_FAILURE);
    }
    _checkForAssertion();
    const char prefixChar = argFormatter[0];
    const char *noPrefixArgFormatter = argFormatter + 1;
    va_list formatterArgs;
    va_list formatterArgsSaveCopy;
    va_start(formatterArgs, argFormatter);
    va_copy(formatterArgsSaveCopy, formatterArgs);
    bool *flagCopierPointer = NULL;
    argStruct* currentArg = NULL;
    for (int i=1; i<argCount; i++) {
        if (argVector[i][0] != prefixChar || setArgs[i]) continue;
        if (strlen(argVector[i]) > 1 && argVector[i][1] == prefixChar) continue;
        for (int j=0; j<strlen(noPrefixArgFormatter); j++) {
            if (charInString(argVector[i], noPrefixArgFormatter[j]) >= 0) {
                for (int k=0; k<=j; k++) {
                    currentArg = va_arg(formatterArgs, argStruct *);
                    flagCopierPointer = (bool *)currentArg -> valueContainer.value;
                }
                va_end(formatterArgs);
                va_copy(formatterArgs, formatterArgsSaveCopy);
                if (flagCopierPointer && currentArg) {
                    currentArg -> hasValue = 1;
                    setArgs[i] = currentArg -> hasValue;
                    currentArg -> argvIndexFound = i;
                    *flagCopierPointer = !*flagCopierPointer;
                }
            }
        }
    }
    va_end(formatterArgs);
    va_end(formatterArgsSaveCopy);
    setFlag(libcargInternalFlags, GROUPED_ARGS_SET);
}

//  Use this to nest a boolean argument within a root argument or another nested argument.
argStruct *nestBooleanArgument(argStruct *nestIn, argStruct *argToNest, char *nestedArgString) {
    if (!hasFlag(nestIn -> flags, BOOLEAN_ARG) || !hasFlag(argToNest -> flags, BOOLEAN_ARG)) {
        fprintf(stderr, "Error: only boolean arguments can be nested with this nesting function. Fix this!\n");
        libcargTerminate();
        exit(EXIT_FAILURE);
    }
    if (nestIn -> nestedArgs && nestIn -> nestedArgFillIndex >= nestIn -> nestedArgArraySize / 2) {
          nestIn -> nestedArgArraySize *= 2;
          nestIn -> nestedArgs = (argStruct **)realloc(nestIn -> nestedArgs, nestIn -> nestedArgArraySize * sizeof(argStruct *));
    } else if (!nestIn -> nestedArgs) {
        nestIn -> nestedArgs = (argStruct **)calloc(4, sizeof(argStruct *));
        nestIn -> nestedArgArraySize = 4;
    }
    argToNest -> nestedArgString = nestedArgString;
    setFlag(argToNest -> flags, NESTED_ARG);
    clearFlag(argToNest -> flags, NESTED_ARG_ROOT);
    nestIn -> nestedArgs[++nestIn -> nestedArgFillIndex] = argToNest;
    argToNest -> parentArg = nestIn;
    return argToNest;
}

//  Use this with a boolean argument struct to declare it as the root of a series of nested arguments.
//  Every nested element, including the root, must use a plain string to identify its flag.
argStruct *nestedBooleanArgumentInit(argStruct *arg, char *argString, const int flagsArg) {
    if (!hasFlag(arg -> flags, BOOLEAN_ARG)) {
        fprintf(stderr, "Error: Boolean nested argument initializer called on non-boolean flag. Fix this!\n");
        libcargTerminate();
        exit(EXIT_FAILURE);
    }
    setFlag(arg -> flags, NESTED_ARG | NESTED_ARG_ROOT | flagsArg);
    arg -> nestedArgString = argString;
    return arg;
}

//  Use this with a non-boolean argument struct to declare it as the root of a series of nested arguments.
//  Every nested element, including the root, must use a plain string to identify its flag.
argStruct *nestedArgumentInit(argStruct *arg, char *argString, const int flagsArg, const char * const formatterToUse) {
    if (hasFlag(arg -> flags, BOOLEAN_ARG)) {
        fprintf(stderr, "Error: Non-boolean nested argument initializer called on boolean flag. Fix this!\n");
        libcargTerminate();
        exit(EXIT_FAILURE);
    }
    strncpy(arg -> formatterUsed, formatterToUse, sizeof(arg -> formatterUsed) - 1);
    setFlag(arg -> flags, NESTED_ARG | NESTED_ARG_ROOT | flagsArg);
    arg -> nestedArgString = argString;
    return arg;
}

//  Use this to nest an argument within a root argument or another nested argument.
argStruct *nestArgument(argStruct *nestIn, argStruct *argToNest, char *nestedArgString, const char * const formatterToUse) {
    if (hasFlag(argToNest -> flags, BOOLEAN_ARG)) {
        fprintf(stderr, "Error: only non-boolean arguments can be nested with this nesting function. Fix this!\n");
        libcargTerminate();
        exit(EXIT_FAILURE);
    }
    if (nestIn -> nestedArgs && nestIn -> nestedArgFillIndex >= nestIn -> nestedArgArraySize / 2) {
        nestIn -> nestedArgArraySize *= 2;
        nestIn -> nestedArgs = (argStruct **)realloc(nestIn -> nestedArgs, nestIn -> nestedArgArraySize * sizeof(argStruct *));
    } else if (!nestIn -> nestedArgs) {
        nestIn -> nestedArgs = (argStruct **)calloc(4, sizeof(argStruct *));
        nestIn -> nestedArgArraySize = 4;
    }
    argToNest -> nestedArgString = nestedArgString;
    strncpy(argToNest -> formatterUsed, formatterToUse, sizeof(argToNest -> formatterUsed) - 1);
    setFlag(argToNest -> flags, NESTED_ARG);
    clearFlag(argToNest -> flags, NESTED_ARG_ROOT);
    nestIn -> nestedArgs[++nestIn -> nestedArgFillIndex] = argToNest;
    argToNest -> parentArg = nestIn;
    return argToNest;
}

//  Uses nested arguments to set flags. Only use this with nested argument roots.
//  To make a nested argument, first declare a few boolean arguments with one of the argument initializer functions.
//  Select one of them to be the root of the nesting and call nestedArgumentInit() on it.
//  Then, call nestArgument() with the root argument and another argument.
//  Note that this is not designed to handle multiple arguments with the same level of nesting from one root node.
//  A logical line must follow from a graph of options for the flags to be toggled accordingly.
void setFlagsFromNestedArgs(const int nestedArgumentCount, ...) {
    if (!hasFlag(libcargInternalFlags, LIBCARGS_INITIALIZED)) {
        fprintf(stderr, "Error: setter called before library initialization. Please fix this!\n");
        libcargTerminate();
        exit(EXIT_FAILURE);
    }
    if (hasFlag(libcargInternalFlags, NESTED_ARGS_SET)) {
        fprintf(stderr, "Error: Nested args initializer called multiple times. Please fix this!\n");
        libcargTerminate();
        exit(EXIT_FAILURE);
    }
    if (hasFlag(libcargInternalFlags, GROUPED_ARGS_SET)) {
        fprintf(stderr, "Error: Grouped args initializer called before nested args initializer. Please fix this!\n");
        libcargTerminate();
        exit(EXIT_FAILURE);
    }
    va_list args;
    va_start(args, nestedArgumentCount);
    for (int x=0; x<nestedArgumentCount; x++) {
        argStruct *argRoot = va_arg(args, argStruct *);
        if (!hasFlag(argRoot -> flags, NESTED_ARG)) {
            fprintf(stderr, "Error: Nested flag setter called on non-nested argument. Fix this!\n");
            libcargTerminate();
            exit(EXIT_FAILURE);
        }
        if (!hasFlag(argRoot -> flags, NESTED_ARG_ROOT)) {
            fprintf(stderr, "Error: Nested flag setter called on non-root nested argument. Fix this!\n");
            libcargTerminate();
            exit(EXIT_FAILURE);
        }
        if (argRoot -> hasValue) {
            fprintf(stderr, "Error: Root nested element is set multiple times. Fix this!\n");
            libcargTerminate();
            exit(EXIT_FAILURE);
        }
        const argStruct *argCursor = argRoot;
        if (!_setFlagFromNestedArgInternal(argRoot)) continue;
        for (int i=0; i <= argCursor -> nestedArgFillIndex; i++) {
            if (_setFlagFromNestedArgInternal(argCursor -> nestedArgs[i])) {
                argCursor = argCursor -> nestedArgs[i];
                i=-1; // i will get incremented to 0 right after this iteration.
            }
        }
    }
    setFlag(libcargInternalFlags, NESTED_ARGS_SET);
}

//  This function will set named args based on environment variables assuming their values have not been set elsewhere.
//  Pass in a formatter with environment variable names and scanf() formatters, separated by a colon.
//  Each pair should be separated by a space.
//  Lastly, the function should be given variadic argument structs (Ex. "PATH:%s OS:%s", &string1, &string2)
void setDefaultFlagsFromEnv(const char * const argFormatter, ...) {
    if (!hasFlag(libcargInternalFlags, LIBCARGS_INITIALIZED)) {
        fprintf(stderr, "Error: setter called before library initialization. Please fix this!\n");
        libcargTerminate();
        exit(EXIT_FAILURE);
    }
    va_list args;
    va_start(args, argFormatter);
    char *argFormatterTokenCopy = strdup(argFormatter);
    void *argFormatterTokenAllocation = argFormatterTokenCopy;
    char *savePointer = NULL;
    argStruct *currentArg = NULL;
    while (1) {
        char *envVarName = strtok_r(argFormatterTokenCopy, ": ", &savePointer);
        char *formatter = strtok_r(NULL, ": ", &savePointer);
        argFormatterTokenCopy = savePointer;
        if (!envVarName || !formatter) break;
        const char *envVarValue = getenv(envVarName);
        currentArg = va_arg(args, argStruct *);
        if (!envVarValue) continue;
        if (!currentArg -> hasValue) {
            currentArg -> hasValue = sscanf(envVarValue, formatter, currentArg -> valueContainer.value);
        }
        if (!currentArg -> hasValue) {
            fprintf(stderr, "Error: unable to grab environment variable %s\n", envVarName);
        }
        strncpy(currentArg -> formatterUsed, formatter, MAX_FORMATTER_SIZE - 1);
    }
    va_end(args);
    free(argFormatterTokenAllocation);
}

//  Call this before any other argument setter functions. 
//  This accepts the argument count and vector, a set of flags, and a series of functions to call corresponding with each flag.
//  If one of these functions is called, the program will terminate.
//  These arguments will override any other arguments passed in.
void argumentOverrideCallbacks(const char *argFormatter, ...) {
    if (!hasFlag(libcargInternalFlags, LIBCARGS_INITIALIZED)) {
        fprintf(stderr, "Error: argument override called before library initialization. Please fix this!\n");
        libcargTerminate();
        exit(EXIT_FAILURE);
    }
    if (hasFlag(libcargInternalFlags, OVERRIDE_CALLBACKS_SET)) {
        fprintf(stderr, "Error: Override callback args initializer called multiple times. Please fix this!\n");
        libcargTerminate();
        exit(EXIT_FAILURE);
    }
    if (hasFlag(libcargInternalFlags, ASSERTIONS_SET) || hasFlag(libcargInternalFlags, NAMED_ARGS_SET) || hasFlag(libcargInternalFlags, POSITIONAL_ARGS_SET) ||
        hasFlag(libcargInternalFlags, GROUPED_ARGS_SET) || hasFlag(libcargInternalFlags, NESTED_ARGS_SET)) {
        fprintf(stderr, "Error: Callback override initialized after arguments were set. Fix this!\n");
        libcargTerminate();
        exit(EXIT_FAILURE);
    }
    _checkForAssertion();
    if (argCount < 2) return;
    char *internalFormatter = strdup(argFormatter);
    void *internalFormatterAllocation = internalFormatter;
    char *savePointer = NULL;
    const char *currentFlag = NULL;
    voidFuncPtr functionCursor = NULL;
    va_list args;
    va_list args_copy;
    va_start(args, argFormatter);
    va_copy(args_copy, args);
    for (int i=1; i<argCount; i++) {
        while ((currentFlag = strtok_r(internalFormatter, " ", &savePointer))) {
            internalFormatter = savePointer;
            functionCursor = va_arg(args_copy, voidFuncPtr);
            if (_compareFlag(currentFlag, argVector[i])) {
                functionCursor();
                free(internalFormatterAllocation);
                libcargTerminate();
                exit(EXIT_SUCCESS);
            }
        }
        free(internalFormatterAllocation);
        va_copy(args_copy, args);
        internalFormatter = strdup(argFormatter);
        internalFormatterAllocation = internalFormatter;
        savePointer = NULL;
    }
    free(internalFormatterAllocation);
    va_end(args);
    setFlag(libcargInternalFlags, OVERRIDE_CALLBACKS_SET);
}

//  Pass in the number of assertions, followed by sets of test cases and messages to print if the assertion fails.
//  Pass in NULL for a message to default to the usage message.
//  Be sure to call this after calling setFlagsFromNamedArgs() to get data from the user.
//  This function is designed to validate command line arguments.
void argAssert(const int assertionCount, ...) {
    if (hasFlag(libcargInternalFlags, ASSERTIONS_SET)) {
        fprintf(stderr, "Error: Assertion args initializer called multiple times. Please fix this!\n");
        libcargTerminate();
        exit(EXIT_FAILURE);
    }
    va_list args;                         
    va_start(args, assertionCount);
    for (int i=0; i<assertionCount; i++) {
        const int expression = va_arg(args, int);
        char *message = va_arg(args, char *);
        if (!expression) {
            if (message) {
                printf("%s\n", message);
                libcargTerminate();
                exit(EXIT_SUCCESS);
            }
            va_end(args);
            usage();
        }
    }
    va_end(args);
    setFlag(libcargInternalFlags, ASSERTIONS_SET);
}

//  Prints out data about a single argument which has a pointer type.
//  This function is primarily for strings, or data that does not need to be passed as a dereferenced value before printing.
//  Type information is obfuscated in each argument, so make sure to pass the correct type into this macro.
#define printOutPointerArgument(argument, typeArg) do {\
    printf("Argument passed in as %s:\n", (#argument));\
    printf("\tType: %s\n", (argument) -> type);\
    printf("\tSize: %llu\n", (argument) -> valueSize);\
    printf("\tFlags: %d\n", (argument) -> flags);\
    printf("\tFound At: %d\n", (argument) -> argvIndexFound);\
    printf("\tHas Value: %d\n", (argument) -> hasValue);\
    if ((argument) -> usageString[0]) printf("\tUsage String: %s\n", (argument) -> usageString);\
    printf("\tFormatter Used: %s\n", (argument) -> formatterUsed);\
    if ((argument) -> nestedArgString[0]) printf("\tNested Argument String: %s\n", (argument) -> nestedArgString);\
    printf("\tValue: ");\
    if (charInString((argument) -> formatterUsed, '\n')) printf("%s", (typeArg)(argument) -> valueContainer.value); /* This allows the string scanf() formatter with spaces to work. */\
    else printf((argument) -> formatterUsed, (typeArg)(argument) -> valueContainer.value);\
    printf("\n");\
} while (0)

//  Prints out data about a single argument which has a non-pointer type.
//  If you want to print out a pointer argument's dereferenced value, you can use this function.
//  This is particularly useful when printing out an element from something like an integer array.
//  Type information is obfuscated in each argument, so make sure to pass the correct type into this macro.
#define printOutNonPointerArgument(argument, typeArg) do {\
    printf("Argument passed in as %s:\n", (#argument));\
    printf("\tType: %s\n", (argument) -> type);\
    printf("\tSize: %llu\n", (argument) -> valueSize);\
    printf("\tFlags: %d\n", (argument) -> flags);\
    printf("\tFound At: %d\n", (argument) -> argvIndexFound);\
    printf("\tHas Value: %d\n", (argument) -> hasValue);\
    if ((argument) -> usageString[0]) printf("\tUsage String: %s\n", (argument) -> usageString);\
    printf("\tFormatter Used: %s\n", (argument) -> formatterUsed);\
    if ((argument) -> nestedArgString[0]) printf("\tNested Argument String: %s\n", (argument) -> nestedArgString);\
    printf("\tValue: ");\
    printf((argument) -> formatterUsed, *(typeArg *)(argument) -> valueContainer.value);\
    printf("\n");\
} while (0)

#define printOutNonPointerMultiArgument(argument, typeArg) do {\
    printOutNonPointerArgument(argument, typeArg);\
    if (!(argument) -> formatterUsed[0]) break;\
    multiArgLinkedList *cursor = *argument.valueContainer.next;\
    while (cursor) {\
        printf("\tValue: ");\
        printf((argument) -> formatterUsed, *(typeArg)cursor -> value);\
        printf("\n");\
        cursor = cursor -> next;\
    }\
} while (0)

#define printOutPointerMultiArgument(argument, typeArg) do {\
printOutPointerArgument(argument, typeArg);\
    if (!(argument) -> formatterUsed[0]) break;\
    multiArgLinkedList *cursor = *argument.valueContainer.next;\
    while (cursor) {\
        printf("\tValue: ");\
        if (charInString((argument) -> formatterUsed, '\n')) printf("%s", (typeArg)cursor -> value); /* This allows the string scanf() formatter with spaces to work. */\
        else printf((argument) -> formatterUsed, (typeArg)cursor -> value);\
        printf("\n");\
        cursor = cursor -> next;\
    }\
} while (0)

//  Do a final sweep to catch any arguments that have been passed but were not used.
//  This function is not required for this library to work.
//  However, this function is useful for pointing out redundant arguments to the end user.
void libcargValidate(void) {
    if (!(hasFlag(libcargInternalFlags, ASSERTIONS_SET) || hasFlag(libcargInternalFlags, NAMED_ARGS_SET) || hasFlag(libcargInternalFlags, POSITIONAL_ARGS_SET) ||
        hasFlag(libcargInternalFlags, GROUPED_ARGS_SET) || hasFlag(libcargInternalFlags, NESTED_ARGS_SET))) {
        fprintf(stderr, "Error: Argument validator called before arguments were set. Fix this!\n");
        libcargTerminate();
        exit(EXIT_FAILURE);
        }
    bool errorFound = false;
    for (int i=1; i<argCount; i++) {
        if (!setArgs[i]) {
            fprintf(stderr, "Error: unknown option: \"%s\"\n", argVector[i]);
            errorFound = true;
        }
    }
    if (errorFound) {
        libcargTerminate();
        exit(EXIT_SUCCESS);
    }
}

//  This function will clear all heap allocations this library uses, including heap-allocated arguments.
//  Call this after arguments have been fully parsed to avoid memory leaks.
void libcargTerminate(void) {
    if (hasFlag(libcargInternalFlags, LIBCARGS_INITIALIZED)) {
        if (allArgs.array) {
            for (int i=0; i<=allArgs.fillIndex; i++) {
                if (allArgs.array[i] -> valueContainer.value && hasFlag(allArgs.array[i] -> flags, HEAP_ALLOCATED)) {
                    free(allArgs.array[i] -> valueContainer.value);
                }
                if (allArgs.array[i] -> nestedArgs) {
                    free(allArgs.array[i] -> nestedArgs);
                }
                if (allArgs.array[i] -> valueContainer.next) {
                    multiArgLinkedList *cursor = allArgs.array[i] -> valueContainer.next -> next;
                    void *cursorToFree = allArgs.array[i] -> valueContainer.next;
                    free(cursorToFree);
                    while (cursor) {
                        cursorToFree = cursor;
                        cursor = cursor -> next;
                        free(cursorToFree);
                    }
                }
            }
            free(allArgs.array);
        }
        if (setArgs) free(setArgs);
        if (argVector) free(argVector);
    }
    clearFlag(libcargInternalFlags, LIBCARGS_INITIALIZED);
}

// Reset macro definitions to not interfere with other included libraries.
#ifdef _MSC_VER 
    #undef strtok_r
    #undef strdup
    #undef _CRT_SECURE_NO_WARNINGS
#endif