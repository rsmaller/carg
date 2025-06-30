#include "args.h"

int globalIntArgValue = 0;

void help(void) {
    printf("Help message here\n");
}

void help2(void) {
    printf("Help 2 message here\n");
}

int main(int argc, char *argv[]) { // Example code
    //  Argument Initialization
    libcargInit(argc, argv);
    basicArgInit(int, positionalArg, NO_DEFAULT_VALUE, POSITIONAL_ARG, NO_USAGE_STRING);
    basicArgInit(int, positionalArg2, NO_DEFAULT_VALUE, POSITIONAL_ARG, NO_USAGE_STRING);
    basicArgInit(int, intArg, NO_DEFAULT_VALUE, NO_FLAGS, "-n");
    adjustArgumentCursor(&intArg, &globalIntArgValue);
    basicArgInit(int, keywordIntArg, NO_DEFAULT_VALUE, NO_FLAGS, "-k");
    basicArgInit(float, floatArg, 0.5f, NO_FLAGS, "-z");
    basicArgInit(bool, boolArg1, 0, BOOLEAN_ARG, "-b");
    basicArgInit(bool, boolArg2, 1, BOOLEAN_ARG, "-c");
    basicArgInit(bool, boolArg3, 0, BOOLEAN_ARG, "--xarg");

    heapArgInit(char *, stringArg, NONE, NO_FLAGS, 21 * sizeof(char), "-t|--term"); // Heap string.
    heapArgInit(char *, positionalStringArg, NONE, POSITIONAL_ARG, 100 * sizeof(char), NO_USAGE_STRING); // Heap string.
    heapArgInit(int *, multiIntArg, NONE, MULTI_ARG, sizeof(int), NO_USAGE_STRING);
    pointerArgInit(char, multiStringArg2, [1000], "default", MULTI_ARG, "-ff"); // Stack string.

    basicArgInit(bool, thing1, 0, BOOLEAN_ARG, NO_USAGE_STRING);
    basicArgInit(bool, thing2, 0, BOOLEAN_ARG, NO_USAGE_STRING);
    basicArgInit(bool, thing3, 0, BOOLEAN_ARG, NO_USAGE_STRING);
    basicArgInit(bool, thing4, 0, BOOLEAN_ARG, NO_USAGE_STRING);
    basicArgInit(bool, thing5, 0, BOOLEAN_ARG, NO_USAGE_STRING);
    basicArgInit(bool, thing6, 0, BOOLEAN_ARG, NO_USAGE_STRING);
    basicArgInit(bool, thing7, 0, BOOLEAN_ARG, NO_USAGE_STRING);
    basicArgInit(bool, thing8, 0, BOOLEAN_ARG, NO_USAGE_STRING);
    pointerArgInit(char, thing20, [100], "thing20_default", NO_FLAGS, NO_USAGE_STRING);
    basicArgInit(int, thing21, 0, NO_FLAGS, NO_USAGE_STRING);
    basicArgInit(int, thing22, 0, NO_FLAGS, NO_USAGE_STRING);

    nestedBooleanArgumentInit(&thing1, "thing1", NO_FLAGS); {
        nestBooleanArgument(&thing1, &thing2, "thing2");
        nestBooleanArgument(&thing1, &thing4, "thing4");
        nestArgument(&thing1, &thing21, "thing21", "%d"); {
            nestArgument(&thing21, &thing22, "thing22", "%d");
        }
    }

    nestedBooleanArgumentInit(&thing3, "thing3", ENFORCE_NESTING_ORDER); {
        nestBooleanArgument(&thing3, &thing4, "thing4");
        nestBooleanArgument(&thing3, &thing5, "thing5"); {
            nestBooleanArgument(&thing5, &thing6, "thing6"); {
                nestBooleanArgument(&thing6, &thing7, "thing7");
                nestBooleanArgument(&thing6, &thing8, "thing8");
            }
        }
    }

    nestedArgumentInit(&thing20, "thing20", NO_FLAGS, "%99s");
    //  thing1 -> thing2 or thing4 (not both)
    //  thing3 -> thing4 or thing5 (not both)
        // thing5 -> thing6
            // thing6 -> thing7
            // thing6 -> thing8

    usageMessageAutoGenerate();

    //  Argument Setting
    argumentOverrideCallbacks("-h -h2", help, help2);
    setFlagsFromNestedArgs(3, &thing1, &thing3, &thing20);
    setFlagsFromPositionalArgs("%d %d %20s", &positionalArg, &positionalArg2, &positionalStringArg);
    setFlagsFromNamedArgs("-n:%d -t:%10s --term:%20s -ff:%999[^\n] -z:%f --xarg:bool -k:%d --mynum:%d",
        &intArg, &stringArg, &stringArg, &multiStringArg2, &floatArg, &boolArg3, &keywordIntArg, &multiIntArg);
    setFlagsFromGroupedBooleanArgs("-bc", &boolArg1, &boolArg2);
    setDefaultFlagsFromEnv("OS:%999[^\n] PATH:%7s", &multiStringArg2, &stringArg);

    //  Assertion, previews, and termination
    libcargValidate();
    argAssert(5,
        globalIntArgValue > -1, "-n arg must be greater than or equal to 0",
        requiredArgument(intArg), USAGE_MESSAGE,
        mutuallyExclusive(boolArg1, boolArg3), "-b and --xarg cannot be toggled at the same time",
        mutuallyRequired(boolArg1, boolArg2), "Boolean 1 requires boolean 2 to be toggled",
        positionalArgValue > 0, "First argument must be positive"
    );

    //  Testing arguments
    printf("Basic arguments - positional arg count: %d, positionalArg: %d, positionalArg2: %d, positionalStringArg: %s, intArg: %d[%d], keywordIntArg: %d[%d], stringArg: %s[%d], stringArg2: %s[%d], float: %f[%d], bool1: %d[%d], bool2: %d[%d], bool3: %d[%d]\n",
        positionalArgCount, positionalArgValue, positionalArg2Value,
        positionalStringArgValue, globalIntArgValue, intArg.argvIndexFound,
        keywordIntArgValue, keywordIntArg.argvIndexFound,
        stringArgValue, stringArg.argvIndexFound, multiStringArg2Value,
        multiStringArg2.argvIndexFound, floatArgValue, floatArg.argvIndexFound,
        boolArg1Value, boolArg1.argvIndexFound, boolArg2Value, boolArg2.argvIndexFound,
        boolArg3Value, boolArg3.argvIndexFound);
    printf("\nNested arguments - thing1: %d, thing2: %d, thing3: %d, thing4: %d, thing5: %d, thing6: %d, thing7: %d thing8: %d, thing20: %s, thing21: %d, thing22: %d\n",
        thing1Value, thing2Value, thing3Value, thing4Value, thing5Value, thing6Value, thing7Value, thing8Value, thing20Value, thing21Value, thing22Value);
    // printOutNonStringArgument(&multiIntArg, int); // Uncomment these to see how the argument printing functions work.
    // printOutStringArgument(&stringArg);
    // printOutNonStringMultiArgument(&multiIntArg, int);
    // printOutStringMultiArgument(&multiStringArg2);
    libcargTerminate();
    return 0;
}
