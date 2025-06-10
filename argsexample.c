#include "args.h"

int globalIntArgValue = 0;

void help(void) {
    printf("Help message here\n");
}

void help2(void) {
    printf("Help 2 message here\n");
}

int main(const int argc, char *argv[]) { // Example code
    libcargInit(argc, argv);
    setUsageMessage("USAGE: %s [number] [number] [string] -n [number] -t [string] <-b|c> <--xarg> <-ff [string]> <-z [float]>", basename(argv[0]));

    basicArgInit(int, namelessArg, NO_DEFAULT_VALUE, NAMELESS_ARG);
    basicArgInit(int, namelessArg2, NO_DEFAULT_VALUE, NAMELESS_ARG);
    basicArgInit(int, intArg, NO_DEFAULT_VALUE, NO_FLAGS);
    adjustArgumentCursor(&intArg, &globalIntArgValue);
    basicArgInit(int, keywordIntArg, NO_DEFAULT_VALUE, NO_FLAGS);
    basicArgInit(float, floatArg, 0.5f, NO_FLAGS);
    basicArgInit(bool, boolArg1, 0, BOOLEAN_ARG);
    basicArgInit(bool, boolArg2, 1, BOOLEAN_ARG);
    basicArgInit(bool, boolArg3, 0, BOOLEAN_ARG);

    basicArgInit(bool, thing1, 0, BOOLEAN_ARG);
    basicArgInit(bool, thing2, 0, BOOLEAN_ARG);
    basicArgInit(bool, thing3, 0, BOOLEAN_ARG);
    basicArgInit(bool, thing4, 0, BOOLEAN_ARG);
    basicArgInit(bool, thing5, 0, BOOLEAN_ARG);
    basicArgInit(bool, thing6, 0, BOOLEAN_ARG);
    basicArgInit(bool, thing7, 0, BOOLEAN_ARG);
    basicArgInit(bool, thing8, 0, BOOLEAN_ARG);

    nestedArgumentInit(&thing1, "thing1", NO_FLAGS); {
        nestArgument(&thing1, &thing2, "thing2");
        nestArgument(&thing1, &thing4, "thing4");
    }

    nestedArgumentInit(&thing3, "thing3", ENFORCE_NESTING_ORDER); {
        nestArgument(&thing3, &thing4, "thing4");
        nestArgument(&thing3, &thing5, "thing5"); {
            nestArgument(&thing5, &thing6, "thing6"); {
                nestArgument(&thing6, &thing7, "thing7");
                nestArgument(&thing6, &thing8, "thing8");
            }
        }
    }
    //  thing1 -> thing2 or thing4 (not both)
    //  thing3 -> thing4 or thing5 (not both)
        // thing5 -> thing6
            // thing6 -> thing7
            // thing6 -> thing8

    heapArgInit(char *, stringArg, NONE, NO_FLAGS, 100 * sizeof(char)); // Heap string.
    heapArgInit(char *, namelessStringArg, NONE, NAMELESS_ARG, 100 * sizeof(char)); // Heap string.
    argInit(char, stringArg2, [100], "default", NO_FLAGS); // Stack string.

    argumentOverrideCallbacks("-h -h2", &help, &help2);
    setFlagsFromNestedArgs(2, &thing1, &thing3);
    setFlagsFromNamelessArgs("%d %d %20s", &namelessArg, &namelessArg2, &namelessStringArg);
    setFlagsFromNamedArgs("-n:%d -t:%10s --term:%20[^\n] -ff:%10s -z:%f --xarg:bool -k:%d", &intArg, &stringArg, &stringArg, &stringArg2, &floatArg, &boolArg3, &keywordIntArg);
    setFlagsFromGroupedBooleanArgs("-bc", &boolArg1, &boolArg2);
    argAssert(6,
        intArgValue > -1, "Int 1 must be at least 0",
        REQUIRED_ARGUMENT(intArg), USAGE_MESSAGE,
        REQUIRED_ARGUMENT(stringArg), USAGE_MESSAGE,
        MUTUALLY_EXCLUSIVE(boolArg1, boolArg3), "Booleans -b and --xarg can't be toggled at the same time",
        MUTUALLY_REQUIRED(boolArg1, boolArg2), "Boolean 1 requires boolean 2 to be toggled",
        namelessArgValue > 0, "Nameless int 1 must be positive"
    );
    printf("Basic arguments - nameless arg count: %d, namelessArg: %d, namelessArg2: %d, namelessStringArg: %s, intArg: %d[%d], keywordIntArg: %d[%d], stringArg: %s[%d], stringArg2: %s[%d], float: %f[%d], bool1: %d[%d], bool2: %d[%d], bool3: %d[%d]\n",
        namelessArgCount, namelessArgValue, namelessArg2Value,
        namelessStringArgValue, globalIntArgValue, intArg.argvIndexFound,
        keywordIntArgValue, keywordIntArg.argvIndexFound,
        stringArgValue, stringArg.argvIndexFound, stringArg2Value,
        stringArg2.argvIndexFound, floatArgValue, floatArg.argvIndexFound,
        boolArg1Value, boolArg1.argvIndexFound, boolArg2Value, boolArg2.argvIndexFound,
        boolArg3Value, boolArg3.argvIndexFound);
    printf("\nNested arguments - thing1: %d, thing2: %d, thing3: %d, thing4: %d, thing5: %d, thing6: %d, thing7: %d, thing8: %d\n", thing1Value, thing2Value, thing3Value, thing4Value, thing5Value, thing6Value, thing7Value, thing8Value);
    free(stringArgValue); // Free heap strings.
    free(namelessStringArgValue);
    return 0;
}
