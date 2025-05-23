#pragma once
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <assert.h>

#ifdef _MSC_VER
    #define strtok_r strtok_s
#endif

typedef struct argStruct {
    void *value;
    char hasValue;
    int flags;
} argStruct;

char usageString[1024] = "Please specify a usage message in your client code.";

void usage(void);

#define maxFormatterSize 2048

int namelessArgCount = 0;

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//  SECTION: Internal Functions and Definitions
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//  A semantic wrapper to compare flags against their parameters.
//  For internal use only.
int compareFlag(const char *argument, const char *parameter) {
    return !strcmp(argument, parameter);
}

//  Fetches how many arguments are expected based on string formatter tokenization.
//  For internal use only.
int getArgCountFromFormatter(char *argFormatter) {
    int returnVal = 1;
    char *token;
    token = strtok(argFormatter, " ");
    while ((token = strtok(NULL, " "))) returnVal++;
    return returnVal;
}

int isFlag(const char *formatter, const char *toCheck) {
    // assert(("Formatter must be smaller than the max formatter size", strlen(formatter) < maxFormatterSize));
    char internalFormatterArray[maxFormatterSize];
    char *internalFormatter = internalFormatterArray;
    char *savePointer = NULL;
    char *flagItem;
    char *formatterItem;
    strncpy(internalFormatter, formatter, maxFormatterSize - 1);
    internalFormatter[maxFormatterSize - 1] = '\0';
    while (1) {
        flagItem = strtok_r(internalFormatter, ": ", &savePointer);
        formatterItem = strtok_r(NULL, ": ", &savePointer);
        internalFormatter = savePointer;
        if (!flagItem) {
            break;
        } else
        if (compareFlag(toCheck, flagItem)) {
            return 1;
        }
    }
    return 0;
}

//  Checks a va_list passed in from setFlagsFromNamedArgs() to set arguments accordingly.
//  For internal use only.
void checkArgAgainstFormatter(int argc, char *argv[], int *argIndex, const char *argFormatter, va_list formatterArgs) {
    assert(((void)"Formatter must be smaller than the max formatter size", strlen(argFormatter) < maxFormatterSize));
    char internalFormatterArray[maxFormatterSize];
    char argCountArray[maxFormatterSize];
    char *internalFormatter = internalFormatterArray;
    strncpy(internalFormatter, argFormatter, maxFormatterSize - 1);
    strncpy(argCountArray, argFormatter, maxFormatterSize - 1);
    internalFormatter[maxFormatterSize - 1] = '\0';
    argCountArray[maxFormatterSize - 1] = '\0';
    int argCount = getArgCountFromFormatter(argCountArray);
    int argIncrement = 0;
    char *savePointer = NULL;
    char *flagItem;
    char *formatterItem;
    argStruct *currentArg;
    void *flagCopierPointer = NULL;
    while (1) {
        argIncrement++;
        flagItem = strtok_r(internalFormatter, ": ", &savePointer);
        formatterItem = strtok_r(NULL, ": ", &savePointer);
        internalFormatter = savePointer;
        if (!flagItem) {
            break;
        }
        if (compareFlag(flagItem, argv[*argIndex])) {
            currentArg = va_arg(formatterArgs, argStruct *);
            flagCopierPointer = currentArg -> value;
            currentArg -> hasValue = 1;
            if (compareFlag(formatterItem, "bool")) {
                *(char *)flagCopierPointer = !*(char *)flagCopierPointer; // Flip flag from its default value. Boolean flags are expected to be chars with a default value.
            } else {
                if (*argIndex < argc - 1 && isFlag(argFormatter, argv[(*argIndex)+1])) {
                    usage();
                } else if (*argIndex == argc - 1) {
                    usage();
                } else {
                    sscanf(argv[(*argIndex) + 1], formatterItem, flagCopierPointer); // If an argument is passed in that does not match its formatter, the value remains default.
                }
            }
            break;
        }
        if (argIncrement < argCount) va_arg(formatterArgs, argStruct *);
        else break;
    }
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//  SECTION: User-Facing Functions and Definitions
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//  Flags and Flag Checkers

#define NO_FLAGS 0

#define NAMELESS_ARG 1

#define STRING 2

#define hasFlag(item, flag)\
    (item & flag)

//  Prints the usage message.
void usage() {
    printf("%s\n", usageString);
    exit(0);
}

//  This function uses a string formatter to generate a usage message to be used when usage() is caled.
#define setUsageMessage(...) do { \
    snprintf(usageString, 1023, __VA_ARGS__);\
    usageString[1023] = '\0';\
} while (0)

//  This macro creates a struct that contains the variable information as well as whether the argument has already been specified or not.
//  It is designed to take the name of a variable which it uses to make a struct containing the variable type.
//  For example, the variable char **arg1[100] = {0} should be declared as argInit(char **, myInt, [100], {0});
//  Note that when doing so, the name arg1 means a struct that contains an array of 100 char **.
//  The value can later be accessed via the name arg1Value. This is achieved with token pasting.
//  For variables with basic types, they can be declared with basicArgInit(type, name, value) instead.
#define argInit(leftType, varName, rightType, val, flagsArg)\
    leftType varName##Value rightType = val;\
    argStruct varName = (argStruct) {\
            .value = &varName##Value,\
            .hasValue = 0,\
            .flags = flagsArg\
    };\
    if (hasFlag(flagsArg, NAMELESS_ARG)) namelessArgCount++;

//  A wrapper for argInit().
#define basicArgInit(type, varName, value, flagsArg)\
    argInit(type, varName, NONE, value, flagsArg)

//  Pass in the argument count, argument vector, and all argument structs generated from the argInit()
//  and basicArgInit() functions to set them based on the argument vector.
void setFlagsFromNamedArgs(int argc, char *argv[], const char *argFormatter, ...) {
    if (argc < 2) usage();
    va_list formatterArgs;
    va_start(formatterArgs, argFormatter);
    int i;
    for (i=namelessArgCount; i<argc; i++) {
        checkArgAgainstFormatter(argc, argv, &i, argFormatter, formatterArgs);
    }
    va_end(formatterArgs);
    return;
}

//  This sets values for nameless arguments in mostly the same format as setFlagsFromNamedArgs().
//  However, this function is for arguments without preceding flags; therefore, flags should not be included in the formatter.
void setFlagsFromNamelessArgs(int argc, char *argv[], const char *argFormatter, ...) {
    if (argc < 2) usage();
    char internalFormatter[maxFormatterSize];
    strncpy(internalFormatter, argFormatter, maxFormatterSize);
    char *currentFormatter = NULL;
    void *flagCopierPointer = NULL;
    argStruct *currentArg;
    va_list formatterArgs;
    va_start(formatterArgs, argFormatter);
    for (int i=1; i<namelessArgCount+1; i++) {
        currentFormatter = strtok(internalFormatter, " ");
        currentArg = va_arg(formatterArgs, argStruct *);
        flagCopierPointer = currentArg -> value;
        currentArg -> hasValue = 1;
        sscanf(argv[i], currentFormatter, flagCopierPointer);
    }
    va_end(formatterArgs);
}

//  Call this before any other argument setter functions. 
//  This accepts the argument count and vector, a set of flags, and a series of functions to call corresponding with each flag.
//  If one of these functions is called, the program will terminate.
//  These arguments will override any other arguments passed in.
void argumentOverrideCallbacks(int argc, char *argv[], const char *argFormatter, ...) {
    if (argc < 2) return;
    char internalFormatter[maxFormatterSize];
    strncpy(internalFormatter, argFormatter, maxFormatterSize);
    char *internalFormatterPointer = internalFormatter;
    char *savePointer = NULL;
    char *currentFlag = NULL;
    void (*functionCursor)(void) = NULL;
    va_list args;
    va_start(args, argFormatter);
    for (int i=1; i<argc; i++) {
        while ((currentFlag = strtok_r(internalFormatterPointer, " ", &savePointer))) {
            internalFormatterPointer = savePointer;
            functionCursor = va_arg(args, void(*)(void));
            if (compareFlag(currentFlag, argv[i])) {
                functionCursor();
                exit(0);
            } else {
                continue;
            }
        }
    }
    va_end(args);
}

//  Pass in the number of assertions, followed by sets of test cases and messages to print if the assertion fails.
//  Pass in NULL for a message to default to the usage message.
//  Be sure to call this after calling setFlagsFromNamedArgs() to get data from the user.
//  This function is designed to validate command line arguments.
void argAssert(int assertionCount, ...) { 
    va_list args;                         
    va_start(args, assertionCount);       
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

//  This is designed to be used in the argAssert() function to assert that an argument cannot have a default value.
#define REQUIRED_ARGUMENT(varName)\
    varName.hasValue

//  Assert that two arguments cannot be declared by the user at the same time.
#define MUTUALLY_EXCLUSIVE(varName1, varName2)\
    !(varName1.hasValue && varName2.hasValue)

#define NO_DEFAULT_VALUE {0} // Set default argument to 0, for readability purposes.

#define NONE // Empty and does nothing. For semantics.

#define USAGE_MESSAGE NULL  // For use in argAssert.