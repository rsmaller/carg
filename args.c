#include "args.h"
#include <libgen.h>

int main(int argc, char *argv[]) { // Example code
    setUsageMessage("USAGE: %s -n [number] -t [string] <-b> <-c> <-z [float]>", basename(argv[0]));
    basicArgInit(int, myInt, NO_DEFAULT_VALUE);
    basicArgInit(float, thingy, 0.1f);
    basicArgInit(char, boolean1, 0);
    basicArgInit(char, boolean2, 1);
    argInit(char, myString, [100], NO_DEFAULT_VALUE);
    setFlagsFromArgs(argc, argv, "-n:%d -t:%99s --term:%99s -z:%f -b:bool -c:bool", &myInt, &myString, &myString, &thingy, &boolean1, &boolean2);
    argAssert(3, 
        myInt > -1, "Int 1 must not be negative",
        REQUIRED_ARGUMENT(myInt), NULL,
        REQUIRED_ARGUMENT(myString), NULL
    );
    printf("myInt: %d, myString: %s, float: %f, bool1: %d, bool2: %d\n", myInt, myString, thingy, boolean1, boolean2);
    return 0;
}