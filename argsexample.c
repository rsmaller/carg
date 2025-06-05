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
    basicArgInit(float, floatArg, 0.5f, NO_FLAGS);
    basicArgInit(char, boolArg1, 0, BOOLEAN_ARG);
    basicArgInit(char, boolArg2, 1, BOOLEAN_ARG);
    basicArgInit(char, boolArg3, 0, BOOLEAN_ARG);
    heapArgInit(char *, stringArg, NONE, NO_FLAGS, 100 * sizeof(char)); // Heap string.
    heapArgInit(char *, namelessStringArg, NONE, NAMELESS_ARG, 100 * sizeof(char)); // Heap string.
    argInit(char, stringArg2, [100], "default", NO_FLAGS); // Stack string.
    argumentOverrideCallbacks("-h -h2", &help, &help2);
    setFlagsFromNamelessArgs("%d %d %20s", &namelessArg, &namelessArg2, &namelessStringArg);
    setFlagsFromNamedArgs("-n:%d -t:%10s --term:%20[^\n] -ff:%10s -z:%f --xarg:bool", &intArg, &stringArg, &stringArg, &stringArg2, &floatArg, &boolArg3);
    setFlagsFromGroupedBooleanArgs("-bc", &boolArg1, &boolArg2);
    argAssert(5,
        intArgValue > -1, "Int 1 must be at least 0",
        REQUIRED_ARGUMENT(intArg), USAGE_MESSAGE,
        REQUIRED_ARGUMENT(stringArg), USAGE_MESSAGE,
        MUTUALLY_EXCLUSIVE(boolArg1, boolArg3), "Booleans -b and --xarg can't be toggled at the same time",
        namelessArgValue > 0, "Nameless int 1 must be positive"
    );
    printf("nameless arg count: %d, namelessArg: %d, namelessArg2: %d, namelessStringArg: %s, intArg: %d, stringArg: %s, stringArg2: %s, float: %f, bool1: %d, bool2: %d, bool3: %d\n",
        namelessArgCount, namelessArgValue, namelessArg2Value, namelessStringArgValue, globalIntArgValue, stringArgValue, stringArg2Value, floatArgValue, boolArg1Value, boolArg2Value,
        boolArg3Value);
    free(stringArgValue); // Free heap strings.
    free(namelessStringArgValue);
    return 0;
}
