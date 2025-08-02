#include "carg.h"

void help(void) {
    printf("Help message here\n");
}

void help2(void) {
    printf("Help 2 message here\n");
}

int main(const int argc, char *argv[]) { // Example code
    //  Argument Initialization
    carg_init(argc, argv);

    int                 positionalArgValue  = 0;
    CargArgContainer   *positionalArg       = carg_arg_create(&positionalArgValue, sizeof(int), CARG_ITEM_POSITIONAL, "<number>");
    int                 positionalArg2Value = 0;
    CargArgContainer   *positionalArg2      = carg_arg_create(&positionalArg2Value, sizeof(int), CARG_ITEM_POSITIONAL, "<number>");
    char               *positionalStringArgValue = (char *)calloc(sizeof(char), 100);
    CargArgContainer   *positionalStringArg      = carg_arg_create(positionalStringArgValue, sizeof(char) * 100, CARG_ITEM_POSITIONAL | CARG_ITEM_HEAP_ALLOCATED, "<string>");

    int                 intArgValue        = 0;
    CargArgContainer   *intArg             = carg_arg_create(&intArgValue, sizeof(int), CARG_ITEM_NO_FLAGS, "-n <number>");
    int                 keywordIntArgValue = 0;
    CargArgContainer   *keywordIntArg      = carg_arg_create(&keywordIntArgValue, sizeof(int), CARG_ITEM_NO_FLAGS, "-k <number>");
    float               floatArgValue      = 0.5f;
    CargArgContainer   *floatArg           = carg_arg_create(&floatArgValue, sizeof(float), CARG_ITEM_NO_FLAGS, "-z <float>");
    bool                boolArg1Value      = false;
    CargArgContainer   *boolArg1           = carg_arg_create(&boolArg1Value, sizeof(bool), CARG_ITEM_BOOLEAN, "-b");
    bool                boolArg2Value      = true;
    CargArgContainer   *boolArg2           = carg_arg_create(&boolArg2Value, sizeof(bool), CARG_ITEM_BOOLEAN, "-c");
    bool                boolArg3Value      = false;
    CargArgContainer   *boolArg3           = carg_arg_create(&boolArg3Value, sizeof(bool), CARG_ITEM_BOOLEAN, "--xarg");
    char               *stringArgValue     = (char *)malloc(21 * sizeof(char));
    CargArgContainer   *stringArg          = carg_arg_create(stringArgValue, sizeof(char) * 21, CARG_ITEM_HEAP_ALLOCATED, "-t|--term <string>");

    bool                nestedArg1Value = false;
    CargArgContainer   *nestedArg1      = carg_arg_create(&nestedArg1Value, sizeof(bool), CARG_ITEM_BOOLEAN, "");
    bool                nestedArg2Value = false;
    CargArgContainer   *nestedArg2      = carg_arg_create(&nestedArg2Value, sizeof(bool), CARG_ITEM_BOOLEAN, "");
    bool                nestedArg3Value = false;
    CargArgContainer   *nestedArg3      = carg_arg_create(&nestedArg3Value, sizeof(bool), CARG_ITEM_BOOLEAN, "");
    int                nestedArg4Value  = 27;
    CargArgContainer   *nestedArg4      = carg_arg_create(&nestedArg4Value, sizeof(bool), CARG_ITEM_NO_FLAGS, "");
    bool                nestedArg5Value = false;
    CargArgContainer   *nestedArg5      = carg_arg_create(&nestedArg5Value, sizeof(bool), CARG_ITEM_BOOLEAN, "");
    bool                nestedArg6Value = false;
    CargArgContainer   *nestedArg6      = carg_arg_create(&nestedArg6Value, sizeof(bool), CARG_ITEM_BOOLEAN, "");
    bool                nestedArg7Value = false;
    CargArgContainer   *nestedArg7      = carg_arg_create(&nestedArg7Value, sizeof(bool), CARG_ITEM_BOOLEAN, "");
    bool                nestedArg8Value = false;
    CargArgContainer   *nestedArg8      = carg_arg_create(&nestedArg8Value, sizeof(bool), CARG_ITEM_BOOLEAN, "");

    char                nestedArg20Value[100] = "nestedArg20_default";
    CargArgContainer   *nestedArg20           = carg_arg_create(nestedArg20Value, sizeof(nestedArg20Value), CARG_ITEM_NO_FLAGS, "");
    int                 nestedArg21Value      = 0;
    CargArgContainer   *nestedArg21           = carg_arg_create(&nestedArg21Value, sizeof(int), CARG_ITEM_NO_FLAGS, "");
    int                 nestedArg22Value      = 0;
    CargArgContainer   *nestedArg22           = carg_arg_create(&nestedArg22Value, sizeof(int), CARG_ITEM_NO_FLAGS, "");

    int                *multiIntArgValue           = (int *)malloc(sizeof(int));
    CargArgContainer   *multiIntArg                = carg_arg_create(multiIntArgValue, sizeof(int), CARG_ITEM_HEAP_ALLOCATED | CARG_ITEM_MULTI, "");
    char                multiStringArgValue[1000] = "default";
    CargArgContainer   *multiStringArg            = carg_arg_create(multiStringArgValue, sizeof(multiStringArgValue), CARG_ITEM_MULTI, "-ff <string>");

    carg_nested_boolean_container_create(nestedArg1, "nestedArg1", CARG_ITEM_ENFORCE_NESTING_ORDER); {
        carg_nest_boolean_container(nestedArg1, nestedArg2, "nestedArg2");
        carg_nest_container(nestedArg1, nestedArg21, "nestedArg21", "%d"); {
            carg_nest_container(nestedArg21, nestedArg22, "nestedArg22", "%d");
        }
    }

    carg_nested_boolean_container_create(nestedArg3, "nestedArg3", CARG_ITEM_ENFORCE_STRICT_NESTING_ORDER); {
        carg_nest_container(nestedArg3, nestedArg4, "nestedArg4", "%d");
        carg_nest_boolean_container(nestedArg3, nestedArg5, "nestedArg5"); {
            carg_nest_boolean_container(nestedArg5, nestedArg6, "nestedArg6"); {
                carg_nest_boolean_container(nestedArg6, nestedArg7, "nestedArg7");
                carg_nest_boolean_container(nestedArg6, nestedArg8, "nestedArg8");
            }
        }
    }

    carg_nested_container_create(nestedArg20, "nestedArg20", CARG_ITEM_NO_FLAGS, "%99s");
     // nestedArg1 -> nestedArg2 or nestedArg21 (not both)
     //    nestedArg21 -> nestedArg22
     // nestedArg3 -> nestedArg4 or nestedArg5 (not both)
     //    nestedArg5 -> nestedArg6
     //        nestedArg6 -> nestedArg7
     //        nestedArg6 -> nestedArg8

    carg_usage_message_autogen();

     // Argument Setting
    carg_override_callbacks("-h -h2", help, help2);
    carg_set_nested_args(3, nestedArg1, nestedArg3, nestedArg20);
    carg_set_positional_args("%d %d %20s", positionalArg, positionalArg2, positionalStringArg);
    carg_set_named_args("-n:%d -t:%10s --term:%20s -ff:%999[^\n] -z:%f --xarg:bool -k:%d --mynum:%d",
        intArg, stringArg, stringArg, multiStringArg, floatArg, boolArg3, keywordIntArg, multiIntArg);
    carg_set_grouped_boolean_args("-bc", boolArg1, boolArg2);
    carg_set_env_defaults("OS:%999[^\n] PATH:%7s", multiStringArg, stringArg);

    //  Assertion, previews, and termination
    carg_validate();
    carg_arg_assert(5,
        intArgValue > -1, "-n arg must be greater than or equal to 0",
        carg_required(intArg), NULL,
        carg_mutually_exclusive(boolArg1, boolArg3), "-b and --xarg cannot be toggled at the same time",
        carg_mutually_required(boolArg1, boolArg2), "Boolean 1 requires boolean 2 to be toggled",
        positionalArgValue > 0, "First argument must be positive"
    );

     // Testing arguments
    printf("Basic arguments - positional arg count: %d, positionalArg: %d, positionalArg2: %d, positionalStringArg: %s, "
           "intArg: %d[%d], keywordIntArg: %d[%d], stringArg: %s[%d], multiStringArg: %s[%d], float: %f[%d], bool1: %d[%d], bool2: %d[%d], "
           "bool3: %d[%d]\n",
        cargDefaultContext -> cargPositionalArgCount, positionalArgValue, positionalArg2Value,
        positionalStringArgValue, intArgValue, intArg -> argvIndexFound,
        keywordIntArgValue, keywordIntArg -> argvIndexFound,
        stringArgValue, stringArg -> argvIndexFound, multiStringArgValue,
        multiStringArg -> argvIndexFound, floatArgValue, floatArg -> argvIndexFound,
        boolArg1Value, boolArg1 -> argvIndexFound, boolArg2Value, boolArg2 -> argvIndexFound,
        boolArg3Value, boolArg3 -> argvIndexFound);
    printf("\nNested arguments - nestedArg1: %d, nestedArg2: %d, nestedArg3: %d, nestedArg4: %d, nestedArg5: %d, nestedArg6: %d, "
           "nestedArg7: %d nestedArg8: %d, nestedArg20: %s, nestedArg21: %d, nestedArg22: %d\n",
        nestedArg1Value, nestedArg2Value, nestedArg3Value, nestedArg4Value, nestedArg5Value, nestedArg6Value, nestedArg7Value, 
        nestedArg8Value, nestedArg20Value, nestedArg21Value, nestedArg22Value);
    // CARG_PRINT_NON_STRING_ITEM(multiIntArg, int); // Uncomment these to see how the argument printing macros work.
    // CARG_PRINT_STRING_ITEM(stringArg);
    // CARG_PRINT_NON_STRING_MULTI_ITEM(multiIntArg, int);
    // CARG_PRINT_STRING_MULTI_ITEM(multiStringArg);
    carg_terminate();
    return 0;
}
