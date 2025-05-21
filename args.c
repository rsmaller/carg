#include "args.h"
#include <libgen.h>

int main(int argc, char *argv[]) { // Example code
    setUsageMessage("USAGE: %s -n [number] -t [string] <-b> <-c> <-z [float]>", basename(argv[0]));
    argInit(int, myArg, NO_DEFAULT_VALUE);
    argInit(char, myString[100], NO_DEFAULT_VALUE);
    argInit(float, thingy, 0.1f);
    argInit(char, boolean1, 0);
    argInit(char, boolean2, 1);
    setFlagsFromArgs(argc, argv, "-n:%d -t:%99s -z:%f -b:bool -c:bool", &myArg, &myString, &thingy, &boolean1, &boolean2);
    require(3, 
        myArg > -1, "Int 1 must be positive",
        REQUIRE_NO_DEFAULT_VALUE(myArg), NULL,
        REQUIRE_NO_DEFAULT_VALUE(myString), NULL
    );
    printf("myInt: %d, myString: %s, float: %f, bool1: %d, bool2: %d\n", myArg, myString, thingy, boolean1, boolean2);
    return 0;
}