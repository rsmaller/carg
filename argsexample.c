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
    heapArgInit(char *, stringArg2, NONE, NO_FLAGS, 21 * sizeof(char), "-t|--term"); // Heap string.
    heapArgDefaultValue(&stringArg2, "default2", strlen("default2"));
    heapArgInit(char *, positionalStringArg, NONE, POSITIONAL_ARG, 100 * sizeof(char), NO_USAGE_STRING); // Heap string.
    heapArgInit(int *, multiIntArg, NONE, MULTI_ARG, sizeof(int), NO_USAGE_STRING);
    pointerArgInit(char, multiStringArg2, [1000], "default", MULTI_ARG, "-ff"); // Stack string.

    basicArgInit(bool, nestedArg1, 0, BOOLEAN_ARG, NO_USAGE_STRING);
    basicArgInit(bool, nestedArg2, 0, BOOLEAN_ARG, NO_USAGE_STRING);
    basicArgInit(bool, nestedArg3, 0, BOOLEAN_ARG, NO_USAGE_STRING);
    basicArgInit(bool, nestedArg4, 0, BOOLEAN_ARG, NO_USAGE_STRING);
    basicArgInit(bool, nestedArg5, 0, BOOLEAN_ARG, NO_USAGE_STRING);
    basicArgInit(bool, nestedArg6, 0, BOOLEAN_ARG, NO_USAGE_STRING);
    basicArgInit(bool, nestedArg7, 0, BOOLEAN_ARG, NO_USAGE_STRING);
    basicArgInit(bool, nestedArg8, 0, BOOLEAN_ARG, NO_USAGE_STRING);
    pointerArgInit(char, nestedArg20, [100], "nestedArg20_default", NO_FLAGS, NO_USAGE_STRING);
    basicArgInit(int, nestedArg21, 0, NO_FLAGS, NO_USAGE_STRING);
    basicArgInit(int, nestedArg22, 0, NO_FLAGS, NO_USAGE_STRING);

    nestedBooleanArgumentInit(&nestedArg1, "nestedArg1", NO_FLAGS); {
        nestBooleanArgument(&nestedArg1, &nestedArg2, "nestedArg2");
        nestBooleanArgument(&nestedArg1, &nestedArg4, "nestedArg4");
        nestArgument(&nestedArg1, &nestedArg21, "nestedArg21", "%d"); {
            nestArgument(&nestedArg21, &nestedArg22, "nestedArg22", "%d");
        }
    }

    nestedBooleanArgumentInit(&nestedArg3, "nestedArg3", ENFORCE_NESTING_ORDER); {
        nestBooleanArgument(&nestedArg3, &nestedArg4, "nestedArg4");
        nestBooleanArgument(&nestedArg3, &nestedArg5, "nestedArg5"); {
            nestBooleanArgument(&nestedArg5, &nestedArg6, "nestedArg6"); {
                nestBooleanArgument(&nestedArg6, &nestedArg7, "nestedArg7");
                nestBooleanArgument(&nestedArg6, &nestedArg8, "nestedArg8");
            }
        }
    }

    nestedArgumentInit(&nestedArg20, "nestedArg20", NO_FLAGS, "%99s");
    //  nestedArg1 -> nestedArg2 or nestedArg4 (not both)
    //  nestedArg3 -> nestedArg4 or nestedArg5 (not both)
        // nestedArg5 -> nestedArg6
            // nestedArg6 -> nestedArg7
            // nestedArg6 -> nestedArg8

    usageMessageAutoGenerate();

    //  Argument Setting
    argumentOverrideCallbacks("-h -h2", help, help2);
    setFlagsFromNestedArgs(3, &nestedArg1, &nestedArg3, &nestedArg20);
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
    printf("Basic arguments - positional arg count: %d, positionalArg: %d, positionalArg2: %d, positionalStringArg: %s, intArg: %d[%d], keywordIntArg: %d[%d], stringArg: %s[%d], stringArg2: %s[%d], multiStringArg2: %s[%d], float: %f[%d], bool1: %d[%d], bool2: %d[%d], bool3: %d[%d]\n",
        positionalArgCount, positionalArgValue, positionalArg2Value,
        positionalStringArgValue, globalIntArgValue, intArg.argvIndexFound,
        keywordIntArgValue, keywordIntArg.argvIndexFound,
        stringArgValue, stringArg.argvIndexFound, stringArg2Value, stringArg2.argvIndexFound, multiStringArg2Value,
        multiStringArg2.argvIndexFound, floatArgValue, floatArg.argvIndexFound,
        boolArg1Value, boolArg1.argvIndexFound, boolArg2Value, boolArg2.argvIndexFound,
        boolArg3Value, boolArg3.argvIndexFound);
    printf("\nNested arguments - nestedArg1: %d, nestedArg2: %d, nestedArg3: %d, nestedArg4: %d, nestedArg5: %d, nestedArg6: %d, nestedArg7: %d nestedArg8: %d, nestedArg20: %s, nestedArg21: %d, nestedArg22: %d\n",
        nestedArg1Value, nestedArg2Value, nestedArg3Value, nestedArg4Value, nestedArg5Value, nestedArg6Value, nestedArg7Value, nestedArg8Value, nestedArg20Value, nestedArg21Value, nestedArg22Value);
    // printOutNonStringArgument(&multiIntArg, int); // Uncomment these to see how the argument printing functions work.
    // printOutStringArgument(&stringArg);
    // printOutNonStringMultiArgument(&multiIntArg, int);
    // printOutStringMultiArgument(&multiStringArg2);
    libcargTerminate();
    return 0;
}
