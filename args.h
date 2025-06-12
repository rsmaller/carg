//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//  SECTION: Includes, Type Definitions, and Function Prototypes
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// ReSharper disable CppNonInlineFunctionDefinitionInHeaderFile
#pragma once
#ifdef _MSC_VER
    #define strtok_r strtok_s
    #define _CRT_SECURE_NO_WARNINGS // sscanf() is required for this project.
#endif
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <stdbool.h>

#define MAX_ARG_NESTING 256

typedef struct argStruct {
    void *value;
    bool hasValue;
    int argvIndexFound; // The index where an argument is found. This stores the argv index of the flag, not the value associated with it. Set to -1 when not found.
    int flags;
    char *nestedArgString;
    const char * const type;
    const char * const usageString;
    int nestedArgFillIndex;
    struct argStruct *parentArg;
    struct argStruct *nestedArgs[MAX_ARG_NESTING];
} argStruct;

typedef struct argArray {
    int fillIndex;
    argStruct **array;
} argArray;

typedef void(*voidFuncPtr)(void); // Some syntax highlighters don't like seeing function pointer parentheses in a macro.

void _usageDefault(void);

//  Prints the usage message.
void (*usage)(void) = _usageDefault;

char *contains(char *, const char *);

char *basename(char *);

int charInString(const char *, char);

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//  SECTION: Global Variables and Definitions
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

int argCount = 1;

char **argVector = NULL;

argArray allArgs = (argArray) {.fillIndex = -1, .array = NULL};

int namelessArgCount = 0;

char usageString[1024] = "Please specify a usage message in your client code.";

uint64_t libcargInternalFlags = 0;

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//  SECTION: Flags, Flag Checkers, and Initializer Macros
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//  Generic Flags.
#define NO_FLAGS (0ULL) // Set no flags when creating an argument.

#define NO_DEFAULT_VALUE {0} // Set default argument to 0 in arg initializers, for readability purposes.

#define NONE // Empty and does nothing. For semantics.

//  Function Initializer Flags for libcargInternalFlags. (Do not set these manually; they are here to give warnings when something goes wrong!)
#define NAMED_ARGS_SET (1ULL<<0ULL)

#define NAMELESS_ARGS_SET (1ULL<<1ULL)

#define GROUPED_ARGS_SET (1ULL<<2ULL)

#define NESTED_ARGS_SET (1ULL<<3ULL)

#define OVERRIDE_CALLBACKS_SET (1ULL<<4ULL)

#define ASSERTIONS_SET (1ULL<<5ULL)

#define USAGE_MESSAGE_SET (1ULL<<6ULL)

//  Internal Argument Flags. (These should be set by functions and not the user.)
#define HEAP_ALLOCATED (1ULL<<0ULL)

#define NESTED_ARG (1ULL<<1ULL)

#define NESTED_ARG_ROOT (1ULL<<2ULL)

//  Argument Flags. (These should be set by the user.)
#define NAMELESS_ARG (1ULL<<3ULL)

#define BOOLEAN_ARG (1ULL<<4ULL)

#define ENFORCE_NESTING_ORDER (1ULL<5ULL)

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

#define VA_ARG_2(arg1, arg2 ...) arg2

#define EXPAND(x) x

#define EXPAND_R(x) EXPAND(x)

#define TOKEN_TO_STRING(x) #x

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//  SECTION: Internal Functions and Definitions
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#define checkForAssertion() do {\
    if (hasFlag(libcargInternalFlags, ASSERTIONS_SET)) {\
        printf("Error: Assertion initializer called before args are initialized. Please fix this!\n");\
        exit(0);\
    }\
} while (0)

//  A semantic wrapper to compare flags against their parameters.
//  For internal use only.
int compareFlag(const char *argument, const char *parameter) {
    return !strcmp(argument, parameter);
}

int isFlag(const char *formatter, const char *toCheck) {
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
        if (compareFlag(toCheck, flagItem)) {
            return 1;
        }
    }
    free(internalFormatterAllocation);
    return 0;
}

//  Prints the usage message.
void _usageDefault(void) {
    printf("%s\n", usageString);
    exit(0);
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
void _checkArgAgainstFormatter(const int argIndex, const char *argFormatter, va_list outerArgs) {
    if (isFlag(argFormatter, argVector[argIndex]) && argIndex == argCount - 1) usage();
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
        argStruct *currentArg = va_arg(formatterArgs, argStruct *);
        if (!flagItem) {
            break;
        }
        if (charInString(argVector[argIndex], '=') >= 0 && strcmp(formatterItem, "bool")) {
            int ncompare = 0;
            while (argVector[argIndex][ncompare] != '=') ncompare++;
            if (strncmp(flagItem,argVector[argIndex], ncompare)) continue;
            argumentFlagToCompare[ncompare] = '\0';
            formatItemToCopy = argVector[argIndex]+ncompare+1;
        }
        if (!strcmp(flagItem, argumentFlagToCompare)) {
            if (!currentArg) return;
            flagCopierPointer = currentArg -> value;
            if (!flagCopierPointer) return;
            if (currentArg -> hasValue) usage();
            if (compareFlag(formatterItem, "bool")) {
                assert(((void)"Argument struct does not contain the BOOLEAN_ARG flag; argument items should be initialized with this flag for readability.",
                        hasFlag(currentArg -> flags, BOOLEAN_ARG)));
                *(bool *)flagCopierPointer = !*(bool *)flagCopierPointer; // Flip flag from its default value. Boolean flags are expected to be chars with a default value.
                currentArg -> hasValue = 1;
            } else {
                currentArg -> hasValue = sscanf(formatItemToCopy, formatterItem, flagCopierPointer); // If an argument is passed in that does not match its formatter, the value remains default.
                if (!currentArg -> hasValue) {
                    usage();
                }
            }
            currentArg -> argvIndexFound = argIndex;
            break;
        }
    }
    free(internalFormatterAllocation);
    free(argumentFlagToCompare);
}

int _setFlagFromNestedArgInternal(argStruct *arg) {
    if (!arg) return 0;
    if (!hasFlag(arg -> flags, NESTED_ARG)) {
        printf("Error: Nested flag setter called on non-nested argument. Fix this!\n");
        exit(0);
    }
    if (arg -> hasValue) return 0;
    for (int i=namelessArgCount; i<argCount; i++) {
        if (!strcmp(arg -> nestedArgString, argVector[i])) {
            *(bool *)arg -> value = !*(bool *)arg -> value;
            arg -> hasValue = 1;
            arg -> argvIndexFound = i;
            if (hasFlag(arg -> flags, ENFORCE_NESTING_ORDER) && arg -> parentArg && arg -> parentArg -> hasValue && arg -> parentArg -> argvIndexFound >= arg -> argvIndexFound) {
                usage();
            }
            return 1;
        }
    }
    return 0;
}

void _printAllNamelessArgs(void) {
    for (int i=0; i<=allArgs.fillIndex; i++) {
        if (!allArgs.array[i]) break;
        if (!hasFlag(allArgs.array[i] -> flags, NAMELESS_ARG)) continue;
        if (allArgs.array[i] -> usageString[0]) printf("%s ", allArgs.array[i]->usageString);
        else if (allArgs.array[i] -> nestedArgString[0]) printf("%s ", allArgs.array[i]->nestedArgString);
        printf("%s ",allArgs.array[i]->type);
    }
}

void _printAllNonNamelessArgs(void) {
    for (int i=0; i<=allArgs.fillIndex; i++) {
        if (hasFlag(allArgs.array[i] -> flags, BOOLEAN_ARG && allArgs.array[i] -> usageString[0])) printf("%s ", allArgs.array[i]->usageString);
    }
    for (int i=0; i<=allArgs.fillIndex; i++) {
        if (hasFlag(allArgs.array[i] -> flags, NESTED_ARG_ROOT)) printf("%s ", allArgs.array[i]->nestedArgString);
    }
    for (int i=0; i<=allArgs.fillIndex; i++) {
        if (hasFlag(allArgs.array[i] -> flags, NAMELESS_ARG)) continue;
        if (hasFlag(allArgs.array[i] -> flags, BOOLEAN_ARG) && !allArgs.array[i] -> nestedArgString[0]) {
            continue;
        }
        if (allArgs.array[i] -> usageString[0]) {
            printf("%s ", allArgs.array[i]->usageString);
            printf("%s ",allArgs.array[i]->type);
        }
        else if (allArgs.array[i] -> nestedArgString[0]) {
        } else {
            printf("%s ",allArgs.array[i]->type);
        }
    }
}

void _printAllArgs(void) {
    _printAllNamelessArgs();
    _printAllNonNamelessArgs();
    printf("\n");
}

void _usageAutoGen(void) {
    printf("Usage: %s ", basename(argVector[0]));
    _printAllArgs();
    exit(0);
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
char *basename(char * const filePath) {
    char *valueToTokenize = strdup(filePath);
    char *valueCursor = valueToTokenize;
    char *savePointer = NULL;
    while (strtok_r(valueCursor, "\\/", &savePointer) && savePointer && *savePointer != '\0') {
        valueCursor = savePointer;
    }
    const char * const returnArithmetic = valueToTokenize; // Prevent IDEs from complaining about using freed pointers in return arithmetic.
    free(valueToTokenize);
    return filePath + (valueCursor - returnArithmetic);
}

//  This function uses a string formatter to generate a usage message to be used when usage() is called.
#define setUsageMessage(...) do { \
    if (hasFlag(libcargInternalFlags, USAGE_MESSAGE_SET)) {\
        printf("Error: usage message set by user twice. Please fix this!\n");\
        exit(0);\
    }\
    setFlag(libcargInternalFlags, USAGE_MESSAGE_SET);\
    snprintf(usageString, 1023, __VA_ARGS__);\
    usageString[1023] = '\0';\
} while (0)

void libcargInit(const int argc, char **argv){
    argCount = argc;
    argVector = argv;
}

//  This macro creates a struct that contains the variable information as well as whether the argument has already been specified or not.
//  It is designed to take the name of a variable which it uses to make a struct containing the variable type.
//  For example, the variable char **arg1[100] = {0} should be declared as argInit(char **, myInt, [100], {0});
//  Note that when doing so, the name arg1 means a struct that contains an array of 100 char **.
//  The value can later be accessed via the name arg1Value. This is achieved with token pasting.
//  For variables with basic types, they can be declared with basicArgInit(type, name, value) instead.
//  Finally, a variadic string argument may be passed to this macro to set a usage string with the usageMesasgeAutoGenerate() function.
#define argInit(leftType, varName, rightType, val, flagsArg, ...)\
    leftType varName##Value rightType = val;\
    argStruct varName = (argStruct) {\
            .value = &varName##Value,\
            .hasValue = 0,\
            .flags = flagsArg,\
            .argvIndexFound = -1,\
            .nestedArgString = "",\
            .nestedArgFillIndex = -1,\
            .usageString = "" VA_ARG_1(__VA_ARGS__),\
            .parentArg = NULL,\
            .nestedArgs = {0},\
            .type = "<" TOKEN_TO_STRING(leftType) TOKEN_TO_STRING(rightType) ">"\
    };\
    if (hasFlag(flagsArg, NAMELESS_ARG)) namelessArgCount++;\
    if (allArgs.array) {\
        void *argArrayReallocation = realloc(allArgs.array, (++allArgs.fillIndex + 1) * sizeof(argStruct *));\
        if (!argArrayReallocation) {\
            printf("Heap allocation failure. Terminating\n");\
            free(allArgs.array);\
            exit(0);\
        }\
        allArgs.array = (argStruct **)argArrayReallocation;\
        allArgs.array[allArgs.fillIndex] = &varName;\
    } else {\
        allArgs.array = (argStruct **)malloc(sizeof(argStruct *));\
        if (!allArgs.array) {\
            printf("Heap allocation failure. Terminating\n");\
            exit(0);\
        }\
        allArgs.array[0] = &varName;\
        allArgs.fillIndex++;\
    }


//  This macro is for initializing arguments which point to heap-allocated memory.
//  Make sure to free the pointer in the varName##Value variable when finished using this argument.
//  Keep in mind that heap-allocated arguments cannot be directly initialized with a default value in this macro.
#define heapArgInit(leftType, varName, rightType, flagsArg, size, ...)\
    argInit(leftType, varName, rightType, NO_DEFAULT_VALUE, flagsArg | HEAP_ALLOCATED, __VA_ARGS__)\
    void *varName##Ptr = malloc(size);\
    memset(varName##Ptr, 0, size);\
    varName##Value = varName##Ptr;\
    varName.value = varName##Ptr;

//  A wrapper for argInit().
#define basicArgInit(type, varName, value, flagsArg, ...)\
    argInit(type, varName, NONE, value, flagsArg, __VA_ARGS__)

//  Changes where the value of an argument is saved to. Ensure the readjusted pointer is of the correct type.
//  This is useful for saving an argument value in a global variable.
//  If a heap-allocated argument is readjusted, the old memory it pointed to will be freed.
//  This will only change the variable where the argument is saved, meaning the save variable will not follow the "Value" convention the rest of this code follows unless you name it accordingly.
void adjustArgumentCursor(argStruct *arg, void *newItem) {
    if (hasFlag(arg->flags, HEAP_ALLOCATED)) free(arg -> value);
    arg -> value = newItem;
}

//  Pass in the argument count, argument vector, and all argument structs generated from the argInit()
//  and basicArgInit() functions to set them based on the argument vector.
void setFlagsFromNamedArgs(const char * const argFormatter, ...) {
    if (hasFlag(libcargInternalFlags, NAMED_ARGS_SET)) {
        printf("Error: Named args initializer called multiple times. Please fix this!\n");
        exit(0);
    }
    checkForAssertion();
    va_list formatterArgs;
    va_start(formatterArgs, argFormatter);
    for (int i=namelessArgCount + 1; i<argCount; i++) {
        _checkArgAgainstFormatter(i, argFormatter, formatterArgs);
    }
    va_end(formatterArgs);
    setFlag(libcargInternalFlags, NAMED_ARGS_SET);
}

//  This sets values for nameless arguments in mostly the same formats as setFlagsFromNamedArgs().
//  However, this function is for arguments without preceding flags; therefore, flags should not be included in the formatter.
void setFlagsFromNamelessArgs(const char *argFormatter, ...) {
    if (hasFlag(libcargInternalFlags, NAMELESS_ARGS_SET)) {
        printf("Error: Nameless args initializer called multiple times. Please fix this!\n");
        exit(0);
    }
    checkForAssertion();
    if (argCount <= namelessArgCount) usage();
    char *internalFormatter = strdup(argFormatter);
    void *internalFormatterAllocation = internalFormatter;
    char *savePointer = NULL;
    const char *currentFormatter = NULL;
    void *flagCopierPointer = NULL;
    va_list formatterArgs;
    va_start(formatterArgs, argFormatter);
    for (int i=1; i<namelessArgCount+1; i++) {
        currentFormatter = strtok_r(internalFormatter, " ", &savePointer);
        internalFormatter = savePointer;
        argStruct *currentArg = va_arg(formatterArgs, argStruct *);
        flagCopierPointer = currentArg -> value;
        currentArg -> hasValue = sscanf(argVector[i], currentFormatter, flagCopierPointer);
        if (!currentArg -> hasValue) {
            usage();
        }
        currentArg -> argvIndexFound = i;
    }
    free(internalFormatterAllocation);
    va_end(formatterArgs);
    setFlag(libcargInternalFlags, NAMELESS_ARGS_SET);
}

//  Creates boolean flags, which should be individual characters, that can be grouped under one flag in any order.
//  Ex: -b and -c can be grouped as -bc or -cb to toggle both flags.
//  To make a non-groupable boolean flag, use the setFlagsFromNamedArgs() function with the bool formatter.
//  This function takes a string containing a one-character prefix and each flag represented by a single character.
//  This function also takes in the argument structs to set, like the other string formatter-esque functions.
//  The argument structs each correspond to a character in the flag string, so the order matters!
void setFlagsFromGroupedBooleanArgs(const char *argFormatter, ...) {
    if (hasFlag(libcargInternalFlags, GROUPED_ARGS_SET)) {
        printf("Error: Grouped args initializer called multiple times. Please fix this!\n");
        exit(0);
    }
    checkForAssertion();
    const char prefixChar = argFormatter[0];
    const char *noPrefixArgFormatter = argFormatter + 1;
    va_list formatterArgs;
    va_list formatterArgsSaveCopy;
    va_start(formatterArgs, argFormatter);
    va_copy(formatterArgsSaveCopy, formatterArgs);
    bool *flagCopierPointer = NULL;
    argStruct* currentArg = NULL;
    for (int i=1; i<argCount; i++) {
        if (argVector[i][0] != prefixChar || argVector[i][1] == prefixChar) continue;
        for (int j=0; j<strlen(noPrefixArgFormatter); j++) {
            if (charInString(argVector[i], noPrefixArgFormatter[j]) >= 0) {
                for (int k=0; k<=j; k++) {
                    currentArg = va_arg(formatterArgs, argStruct *);
                    flagCopierPointer = (bool *)currentArg -> value;
                }
                va_end(formatterArgs);
                va_copy(formatterArgs, formatterArgsSaveCopy);
                if (flagCopierPointer && currentArg) {
                    currentArg -> hasValue = 1;
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

//  Use this to nest an argument within a root argument or another nested argument.
argStruct *nestArgument(argStruct *nestIn, argStruct *argToNest, char *nestedArgString) {
    if (!hasFlag(nestIn -> flags, BOOLEAN_ARG) || !hasFlag(argToNest -> flags, BOOLEAN_ARG)) {
        printf("Error: only boolean arguments can be safely nested. Fix this!\n");
        exit(0);
    }
    if (nestIn -> nestedArgFillIndex >= MAX_ARG_NESTING - 1) {
        printf("Error: too many arguments have been nested into this option!\n");
        exit(0);
    }
    argToNest -> nestedArgString = nestedArgString;
    argToNest -> flags = nestIn -> flags;
    setFlag(argToNest -> flags, NESTED_ARG);
    clearFlag(argToNest -> flags, NESTED_ARG_ROOT);
    nestIn -> nestedArgs[++nestIn -> nestedArgFillIndex] = argToNest;
    argToNest -> parentArg = nestIn;
    return argToNest;
}

//  Use this with a boolean argument struct to declare it as the root of a series of nested arguments.
//  Every nested element, including the root, must use a plain string to identify its flag.
argStruct *nestedArgumentInit(argStruct *arg, char *argString, int flagsArg) {
    if (!hasFlag(arg -> flags, BOOLEAN_ARG)) {
        printf("Error: Nested argument initializer called on non-boolean flag. Fix this!\n");
        exit(0);
    }
    setFlag(arg -> flags, NESTED_ARG | NESTED_ARG_ROOT | flagsArg);
    arg -> nestedArgString = argString;
    return arg;
}

//  Uses nested arguments to set flags. Only use this with nested argument roots.
//  To make a nested argument, first declare a few boolean arguments with one of the argument initializer functions.
//  Select one of them to be the root of the nesting and call nestedArgumentInit() on it.
//  Then, call nestArgument() with the root argument and another argument.
//  Note that this is not designed to handle multiple arguments with the same level of nesting from one root node.
//  A logical line must follow from a graph of options for the flags to be toggled accordingly.
void setFlagsFromNestedArgs(const int nestedArgumentCount, ...) {
    if (hasFlag(libcargInternalFlags, NESTED_ARGS_SET)) {
        printf("Error: Nested args initializer called multiple times. Please fix this!\n");
        exit(0);
    }
    va_list args;
    va_start(args, nestedArgumentCount);
    for (int x=0; x<nestedArgumentCount; x++) {
        argStruct *argRoot = va_arg(args, argStruct *);
        if (!hasFlag(argRoot -> flags, NESTED_ARG)) {
            printf("Error: Nested flag setter called on non-nested argument. Fix this!\n");
            exit(0);
        }
        if (!hasFlag(argRoot -> flags, NESTED_ARG_ROOT)) {
            printf("Error: Nested flag setter called on non-root nested argument. Fix this!\n");
            exit(0);
        }
        if (argRoot -> hasValue) {
            printf("Error: Root nested element is set multiple times. Fix this!\n");
            exit(0);
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
        currentArg = va_arg(args, argStruct *);
        if (!envVarName || !formatter) break;
        const char *envVarValue = getenv(envVarName);
        if (!envVarValue) continue;
        if (!currentArg -> hasValue) {
            currentArg -> hasValue = sscanf(envVarValue, formatter, currentArg -> value);
        }
        if (!currentArg -> hasValue) {
            usage();
        }
    }
    va_end(args);
    free(argFormatterTokenAllocation);
}

//  Call this before any other argument setter functions. 
//  This accepts the argument count and vector, a set of flags, and a series of functions to call corresponding with each flag.
//  If one of these functions is called, the program will terminate.
//  These arguments will override any other arguments passed in.
void argumentOverrideCallbacks(const char *argFormatter, ...) {
    if (hasFlag(libcargInternalFlags, OVERRIDE_CALLBACKS_SET)) {
        printf("Error: Override callback args initializer called multiple times. Please fix this!\n");
        exit(0);
    }
    checkForAssertion();
    if (argCount < 2) return;
    char *internalFormatter = strdup(argFormatter);
    void *internalFormatterAllocation = internalFormatter;
    char *savePointer = NULL;
    const char *currentFlag = NULL;
    voidFuncPtr functionCursor = NULL;
    va_list args;
    va_start(args, argFormatter);
    for (int i=1; i<argCount; i++) {
        while ((currentFlag = strtok_r(internalFormatter, " ", &savePointer))) {
            internalFormatter = savePointer;
            functionCursor = va_arg (args, voidFuncPtr);
            if (compareFlag(currentFlag, argVector[i])) {
                functionCursor();
                exit(0);
            }
        }
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
        printf("Error: Assertion args initializer called multiple times. Please fix this!\n");
        exit(0);
    }
    va_list args;                         
    va_start(args, assertionCount);
    for (int i=0; i<assertionCount; i++) {
        const int expression = va_arg(args, int);
        char *message = va_arg(args, char *);
        if (!expression) {
            if (message) {
                printf("%s\n", message);
                exit(0);
            }
            va_end(args);
            usage();
        }
    }
    va_end(args);
    setFlag(libcargInternalFlags, ASSERTIONS_SET);
}

//  Generates a usage message automatically based on data entered to argument constructors. Make sure to call this after all arguments have been initialized.
//  setUsageMessage() should not be used alongside this function, otherwise it would obfuscate the generated usage message generated here.
void usageMessageAutoGenerate(void) {
    if (hasFlag(libcargInternalFlags, USAGE_MESSAGE_SET)) {\
        printf("Error: usage message set by user twice. Please fix this!\n");\
        exit(0);\
    }\
    setFlag(libcargInternalFlags, USAGE_MESSAGE_SET);\
    usage = _usageAutoGen;
}

void libcargTerminate(void) {
    if (allArgs.array) free(allArgs.array);
}