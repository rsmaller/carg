#pragma once
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <assert.h>

char usageString[1024] = "Please specify a usage message in your client code.";

void usage() {
    printf("%s\n", usageString);
    exit(0);
}

#define setUsageMessage(...) do { \
    snprintf(usageString, 1023, __VA_ARGS__);\
    usageString[1023] = '\0';\
} while (0)

int compareArg(char *argument, char *parameter) {
    return !strcmp(argument, parameter);
}

int getArgCountFromFormatter(char *argFormatter) {
    int returnVal = 1;
    strtok(argFormatter, " ");
    while (strtok(NULL, " ")) returnVal++;
    return returnVal;
}

void checkArgAgainstFormatter(int argc, char *argv[], int *argIndex, const char *argFormatter, va_list formatterArgs) {
    const size_t maxFormatterSize = 524288;
    assert(("Formatter must be smaller than the max formatter size", strlen(argFormatter) < maxFormatterSize));
    char internalFormatterArray[maxFormatterSize];
    char argCountArray[maxFormatterSize];
    char *internalFormatter = internalFormatterArray;
    strncpy(internalFormatter, argFormatter, maxFormatterSize - 1);
    strncpy(argCountArray, argFormatter, maxFormatterSize - 1);
    int argCount = getArgCountFromFormatter(argCountArray);
    int argIncrement = 0;
    char *savePointer;
    char *flagItem;
    char *formatterItem;
    void *flagCopierPointer = NULL;
    while (1) {
        argIncrement++;
        flagItem = strtok_r(internalFormatter, ": ", &savePointer);
        formatterItem = strtok_r(NULL, ": ", &savePointer);
        internalFormatter = savePointer;
        if (!flagItem) {
            break;
        }
        if (!strcmp(flagItem, argv[*argIndex])) {
            flagCopierPointer = va_arg(formatterArgs, void *);
            if (!strcmp(formatterItem, "bool")) {
                *(char *)flagCopierPointer = !*(char *)flagCopierPointer; // Flip flag from its default value. Boolean flags are expected to be chars with a default value.
            } else {
                sscanf(argv[(*argIndex) + 1], formatterItem, flagCopierPointer); // If an argument is passed in that does not match its formatter, the value remains default.
            }
            break;
        }
        if (argIncrement < argCount) va_arg(formatterArgs, void *);
        else break;
    }
}

void setFlagsFromArgs(int argc, char *argv[], const char *argFormatter, ...) {
    va_list formatterArgs;
    va_start(formatterArgs, argFormatter);
    int i;
    for (i=1; i<argc; i++) {
        checkArgAgainstFormatter(argc, argv, &i, argFormatter, formatterArgs);
    }
    va_end(formatterArgs);
    return;
}

void argAssert(int assertionCount, ...) { // Pass in the number of assertions, followed by sets of test cases and messages for each assertion.
    va_list args;                        // Pass in NULL for a message to default to the usage message.
    va_start(args, assertionCount);      // Be sure to call this after calling setFlagsFromArgs() to get data from the user. This function is designed to validate command line arguments.
    int expression;
    char *message;
    int exitFlag = 0;
    for (int i=0; i<assertionCount; i++) {
        expression = va_arg(args, int);
        message = va_arg(args, char *);
        if (!(expression)) {
            if (message) {
                printf("%s\n", message);
                exitFlag = 1;
            } else {
                usage();
            }
        }
    }
    if (exitFlag) exit(0);
    va_end(args);
}

// This macro is designed to be used with the name of the variable declaration freed up so a "default" variable can be created from its name.
// For example, the variable char **myInt[100] = {0} should be declared as argInit(char **, myInt, [100], {0});
// For variables with basic types, they can be declared with basicArgInit(type, name, value) instead.
#define NONE
#define argInit(leftType, varName, rightType, value)\
    leftType varName rightType = value;\
    const leftType default_##varName rightType = value;
#define basicArgInit(type, varName, value)\
    argInit(type, varName, NONE, value)

// This is designed to be used in the require() function to assert that an argument should not have a default value.
// This is NOT designed to handle pointers to heap-allocated memory.
// The default value macros should only be used on array types, not pointer types.
// If you need to use heap memory, memcpy() the data into a heap memory space after fetching it from the user.
#define REQUIRED_ARGUMENT(varName)\
    memcmp(&varName, &default_##varName, sizeof(varName)) 

#define NO_DEFAULT_VALUE {0} // For readability purposes.
