#include "args.h"
#include <signal.h>

#ifndef _MSC_VER
    #include <libgen.h>
#else
    #define basename(x) x
#endif

void help(void) {
    printf("Help message here\n");
}

void help2(void) {
    printf("Help 2 message here\n");
}

int main(int argc, char *argv[]) { // Example code
    setUsageMessage("USAGE: %s [number] [number] -n [number] -t [string] <-b> <-c> <-z [float]>", basename(argv[0]));
    basicArgInit(int, namelessArg, NO_DEFAULT_VALUE, NAMELESS_ARG);
    basicArgInit(int, namelessArg2, NO_DEFAULT_VALUE, NAMELESS_ARG);
    basicArgInit(int, intArg, NO_DEFAULT_VALUE, NO_FLAGS);
    basicArgInit(float, floatArg, 0.1f, NO_FLAGS);
    basicArgInit(char, boolArg1, 0, NO_FLAGS);
    basicArgInit(char, boolArg2, 1, NO_FLAGS);
    argInit(char, stringArg, [100], NO_DEFAULT_VALUE, NO_FLAGS);
    argumentOverrideCallbacks(argc, argv, "-h -h2", &help, &help2);
    setFlagsFromNamelessArgs(argc, argv, "%d %d", &namelessArg, &namelessArg2);
    setFlagsFromNamedArgs(argc, argv, "-n:%d -t:%10s --term:%20s -z:%f -b:bool -c:bool", &intArg, &stringArg, &stringArg, &floatArg, &boolArg1, &boolArg2);
    argAssert(3,
        intArgValue > -1, "Int 1 must not be negative",
        // REQUIRED_ARGUMENT(intArg), USAGE_MESSAGE,
        // REQUIRED_ARGUMENT(stringArg), USAGE_MESSAGE,
        MUTUALLY_EXCLUSIVE(boolArg1, boolArg2), "Both booleans can't be toggled at the same time",
        namelessArgValue > 0, "Nameless int 1 must be positive"
    );
    printf("namelessArg: %d, namelessArg2: %d, intArg: %d, stringArg: %s, float: %f, bool1: %d, bool2: %d\n", namelessArgValue, namelessArg2Value, intArgValue, stringArgValue, floatArgValue, boolArg1Value, boolArg2Value);
    return 0;
}