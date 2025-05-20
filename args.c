#include "args.h"

int main(int argc, char *argv[]) {
    int myArg = 0;
    char myString[100] = "default";
    float thingy = 0.1f;
    int boolean1 = 0;
    int boolean2 = 1;
    setFlagsFromArgs(argc, argv, "-n:%d -t:%99s -z:%f -b:bool -c:bool", &myArg, &myString, &thingy, &boolean1, &boolean2);
    require(2, 
        myArg > -1,              "Int 1 must be positive",
        strcmp(myString, "yes"), "myString cannot be yes"
    );
    printf("myInt: %d, myString: %s, float: %f, bool1: %d, bool2: %d\n", myArg, myString, thingy, boolean1, boolean2);
    return 0;
}