#include "args.h"
#include <signal.h>

#ifndef _MSC_VER
    #include <libgen.h>
#else
    #define basename(x) x
#endif

int main(int argc, char *argv[]) { // Example code
    setUsageMessage("USAGE: %s [number] -n [number] -t [string] <-b> <-c> <-z [float]>", basename(argv[0]));
    basicArgInit(int, namelessArg, NO_DEFAULT_VALUE, NAMELESS_ARG);
    basicArgInit(int, intArg, NO_DEFAULT_VALUE, NO_FLAGS);
    basicArgInit(float, floatArg, 0.1f, NO_FLAGS);
    basicArgInit(char, boolArg1, 0, NO_FLAGS);
    basicArgInit(char, boolArg2, 1, NO_FLAGS);
    argInit(char, stringArg, [100], NO_DEFAULT_VALUE, NO_FLAGS);
    setFlagsFromArgs(argc, argv, "nameless:%d -n:%d -t:%10s --term:%20s -z:%f -b:bool -c:bool", &namelessArg, &intArg, &stringArg, &stringArg, &floatArg, &boolArg1, &boolArg2);
    argAssert(4, 
        intArgValue > -1, "Int 1 must not be negative",
        REQUIRED_ARGUMENT(intArg), USAGE_MESSAGE,
        REQUIRED_ARGUMENT(stringArg), USAGE_MESSAGE,
        MUTUALLY_EXCLUSIVE(boolArg1, boolArg2), "Both booleans can't be toggled at the same time"
    );
    printf("namelessArg: %d, intArg: %d, stringArg: %s, float: %f, bool1: %d, bool2: %d\n", namelessArgValue, intArgValue, stringArgValue, floatArgValue, boolArg1Value, boolArg2Value);
    return 0;
}