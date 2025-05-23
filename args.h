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

//  Pass in the number of assertions, followed by sets of test cases and messages for each assertion.
//  Pass in NULL for a message to default to the usage message.
//  Be sure to call this after calling setFlagsFromNamedArgs() to get data from the user. This function is designed to validate command line arguments.
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

#define MUTUALLY_EXCLUSIVE(varName1, varName2)\
    !(varName1.hasValue && varName2.hasValue)

#define NO_DEFAULT_VALUE {0} // Set default argument to 0, for readability purposes.

#define NONE // Empty and does nothing. For semantics.

#define USAGE_MESSAGE NULL  // For use in argAssert.

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//  SECTION: How to Use This Library
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//  Before using this library, ensure your main() function accepts an argc and argv as parameters.
//  This library parses those parameters using string formatters and sets variables to their values accordingly.

//  It's likely the first thing you will want to do is declare a usage message for your program.
//  The setUsageMessage() function allows you to do this; it sets a usage message that caps out at 1023 characters.
//  It accepts a string with formatters in it. This might look something like:
//  setUsageMessage("USAGE: %s -n Arg1 -t Arg2", basename(argv[0]));

//  The usage() function will print out this message and terminate the program.

//  As for arguments, they are stored in structs and variables of their respective type.
//  Structs serve to keep track of whether an argument has been specified by the user or not.
//  To simplify the declaration of arguments, argInit and basicArgInit() were created.
//  Simply call argInit() or basicArgInit() with both a variable name to declare, type information, a default value, and a bitmask representing toggled flags.
//  To write explicitly that a variable should not have a default value, use the NO_DEFAULT_VALUE macro in place of a default value.

//  argInit() and basicArgInit() will create a struct from the variable name entered into them.
//  Keep in mind the resulting struct does not contain type information about the variable you declare a container for the argument.
//  To access the data the user entered, add "Value" to the end of the struct's name.
//  For example, when declaring an int argument via basicArgInit(int, intArg, 1, NO_FLAGS):
//      intArg is a struct.
//      intArgValue is an int where the value of the argument is stored.
//      intArg contains a void pointer to intArgValue.

//  The value of an argument should only be accessed after setting the variables' values in accordance with user input.
//  Keep in mind variables must be initialized via argInit() or basicArgInit() before being set.
//  To set argument values from the user, use the setFlagsFromNamedArgs() function. This function accepts the argc and argv parameters and a string formatter for arguments.
//  setFlagsFromNamedArgs accepts both flag parameters and string formatters associated with them.
//  That formatter might look like: "-v:%s" followed by a string argument struct.
//  This would mean that the flag -v should be followed by a string:
//      example.exe -v <INSERT_STRING_HERE>

//  Below is an example of the initializer and setter in conjunction.
//  Keeping everything in mind, the string variable would need to be initialized first:
//  basicArgInit(char *, myString, "default", NO_FLAGS);
//  All arguments should be initialized before setting them, so let's add an int argument also:
//  basicArgInit(int, myInt, 0, NO_FLAGS);

//  Then, these values can be set using the setFlagsFromNamedArgs() function:
//  setFlagsFromNamedArgs(argc, argv, "-v:%s -i:%d", myString, myInt);
//  The %s formatter corresponds to the argument struct myString and the %d formatter corresponds to the myInt struct.

//  Assuming the resulting program is ran with the following arguments:
//      example.exe -v yes -i 5
//  myStringValue will contain the string "yes" and myIntValue will contain the number 5.

//  We also declared default values for those, so if we instead ran:
//      example.exe
//  myStringValue would contain the string "default" and myIntValue would contain the number 5.

//  If this is all the functionality you need, you're done!
//  However, you may want to make an argument required or limit which values the user can set it to, especially one that is initialized with NO_DEFAULT_VALUE.
//  argAssert() is designed for this; it accepts the number of argument assertions as an argument. All assertions after that are two arguments each:
//  The first argument should be the condition that must be met. This can be any expression which can be evaluated as a zero versus nonzero value.
//  If this condition is that the argument is required, use REQUIRED_ARGUMENT() with the corresponding argument struct.
//  If two arguments should not be set at the same time, use MUTUALLY_EXCLUSIVE() with the corresponding structs.
//  The second argument should be the message to display if the condition is not met. Set this to USAGE_MESSAGE to use the usage message instead.

//  For example:
//  argAssert(3, 
//          myIntValue > -1, "Int 1 must not be negative",
//          REQUIRED_ARGUMENT(myInt), USAGE_MESSAGE,
//          REQUIRED_ARGUMENT(myString), USAGE_MESSAGE
//  );
//  This will print "Int 1 must not be negative" if a value less than or equal to -1 is given.
//  The usage message will show if myInt or myString are not given values by the user.

//  To specify arguments that make the program do something entirely different, primarily running a single function and then terminating, call the argumentOverrideCallbacks()
//  function. This function accepts the argument count, argument vector, flags, and function pointers associated with them.
//  For example, to declare arguments for a help-displaying function and another random helper function:
//  argumentOverrideCallbacks(argc, argv, "-h -r", &help, &randomHelperFunction);

//  Another feature this library supports is nameless arguments. Nameless arguments are passed in to the program without a flag.
//  These arguments should always come before named arguments to prevent argument ambiguity.

//  To use nameless arguments alongside named arguments, initialize both first:
//  basicArgInit(int, namelessArg, 0, NAMELESS_ARG);
//  basicArgInit(char, namedArg, 'a', NO_FLAGS);

//  Then, set the values for nameless arguments first:
//  setFlagsFromNamelessArgs(argc, argv, "%d", &namelessArg);

//  Lastly, set the values for named arguments:
//  setFlagsFromNamedArgs(argc, argv, "-n:%d", &namedArg);

//  Keep in mind that nameless arguments are required regardless if they are enforced with an assertion or not.
//  Furthermore, they are assigned to argument variables based on their order. Make sure they line up correctly when you set their values!
//  Nameless arguments can be used in assertions the same way as named arguments.
