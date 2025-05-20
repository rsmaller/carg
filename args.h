#pragma once
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>

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
        if (!flagItem) break;
        if (!strcmp(flagItem, argv[*argIndex])) {
            flagCopierPointer = va_arg(formatterArgs, void *);
            if (!strcmp(formatterItem, "bool")) {
                *(int *)flagCopierPointer = !*(int *)flagCopierPointer; // Flip flag from its default value. Boolean flags are expected to be ints with a default value.
            } else {
                sscanf(argv[(*argIndex) + 1], formatterItem, flagCopierPointer);
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

void require(int assertionCount, ...) {
    va_list args;
    va_start(args, assertionCount);
    int expression;
    char *message;
    int exitFlag = 0;
    for (int i=0; i<assertionCount; i++) {
        expression = va_arg(args, int);
        message = va_arg(args, char *);
        if (!(expression)) {
            printf("%s\n", message);
            exitFlag = 1;
        }
    }
    if (exitFlag) exit(0);
    va_end(args);
}

