#include "carg.h"

int main(const int argc, char **argv) {
    CargContext *threadSafeContext = NULL;
    carg_init_ts(&threadSafeContext, argc, argv);
    int arg1Value = 15;
    CargArgContainer *arg1 = carg_arg_create_ts(threadSafeContext, &arg1Value, sizeof(arg1Value), CARG_ITEM_NO_FLAGS, "-n <number>");
    carg_set_named_args_ts(threadSafeContext, "-n:%d", arg1);
    printf("arg1: %d\n", arg1Value);
    carg_terminate_ts(threadSafeContext);
}
