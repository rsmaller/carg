//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//  SECTION: Includes and Type Definitions
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>     // NOLINT
#include <stdint.h>     // NOLINT
#include <stdbool.h>    // NOLINT
#include <stdarg.h>     // NOLINT

#define CARG_MAX_FORMATTER_SIZE 128

typedef struct CargMultiArgContainer {
        struct CargMultiArgContainer *next;
        void                         *value;
} CargMultiArgContainer;

typedef struct CargArgContainer {
    CargMultiArgContainer     valueContainer;
    int                       multiArgIndex;
    size_t                    valueSize;
    bool                      hasValue;
    int                       argvIndexFound;
    uint64_t                  flags;
    const char * const        usageString;
    char                      formatterUsed[CARG_MAX_FORMATTER_SIZE];
    const char               *nestedArgString;
    int                       nestedArgFillIndex;
    size_t                    nestedArgArraySize;
    struct CargArgContainer  *parentArg;
    struct CargArgContainer **nestedArgs;
} CargArgContainer;

typedef struct CargArgArray {
    size_t             size;
    int                fillIndex;
    CargArgContainer **array;
} CargArgArray;

typedef void (*CargCallbackFunc)(void);

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//  SECTION: Global Variables and Definitions
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

extern int              cargArgCount;
extern int              cargPositionalArgCount;

extern int *            internal_cargSetArgs;
extern char **          internal_cargArgVector;
extern CargArgArray     internal_cargAllArgs;
extern char             internal_cargUsageString[];
extern char *           internal_cargUsageStringCursor; // Pointer to the byte after the farthest one written to.
extern char * const     internal_cargUsageStringEnd;    // Pointer to the final byte of the usage string buffer.
extern uint64_t         internal_cargInternalFlags;
extern CargCallbackFunc internal_carg_usage_ptr;

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//  SECTION: Flags, Flag Checkers, and Initializer Macros
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//  Getters and Setters.
#define HAS_FLAG(item, flag)    ((item) & (flag))
#define SET_FLAG(item, flag)    ((item) |= (flag))
#define CLEAR_FLAG(item, flag)  ((item) &= ~(flag))
#define TOGGLE_FLAG(item, flag) ((item) ^= (flag))

//  Generic Flags.
#define CARG_ITEM_NO_FLAGS                     (0ULL)

//  Internal Argument Flags.
#define CARG_ITEM_NESTED                       (1ULL<<1ULL)
#define CARG_ITEM_NESTED_ROOT                  (1ULL<<2ULL)

//  User-Facing Argument Flags.
#define CARG_ITEM_POSITIONAL                   (1ULL<<32ULL)
#define CARG_ITEM_BOOLEAN                      (1ULL<<33ULL)
#define CARG_ITEM_ENFORCE_NESTING_ORDER        (1ULL<<34ULL)
#define CARG_ITEM_ENFORCE_STRICT_NESTING_ORDER (1ULL<<35ULL)
#define CARG_ITEM_MULTI                        (1ULL<<36ULL)
#define CARG_ITEM_HEAP_ALLOCATED               (1ULL<<37ULL)

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//  SECTION: Internal Macros
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//  Global State Flags for internal_cargInternalFlags.
#define CARG_INITIALIZED            (1ULL<<0ULL)
#define CARG_NAMED_ARGS_SET         (1ULL<<1ULL)
#define CARG_POSITIONAL_ARGS_SET    (1ULL<<2ULL)
#define CARG_GROUPED_ARGS_SET       (1ULL<<3ULL)
#define CARG_NESTED_ARGS_SET        (1ULL<<4ULL)
#define CARG_OVERRIDE_CALLBACKS_SET (1ULL<<5ULL)
#define CARG_ASSERTIONS_SET         (1ULL<<6ULL)
#define CARG_USAGE_MESSAGE_SET      (1ULL<<7ULL)

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//  SECTION: User-Facing Function Prototypes
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void carg_init     (int argc, char **argv);
void carg_validate (void);
void carg_terminate(void);

void  carg_print_container_data (const CargArgContainer *container);
void *carg_fetch_multi_arg_entry(const CargArgContainer *container, int index);

char *      carg_string_contains_substr(char *container, const char * substr);
int         carg_string_contains_char  (const char * container, char subchar);
const char *carg_basename              (const char * pathStart);

void carg_set_usage_message    (const char * format, ...);
void carg_set_usage_function   (CargCallbackFunc usageFunc);
void carg_override_callbacks   (const char * format, ...);
void carg_arg_assert           (int assertionCount, ...);
void carg_usage                (void);
void carg_usage_message_autogen(void);

bool carg_required          (const CargArgContainer *arg);
bool carg_mutually_exclusive(const CargArgContainer *arg1, const CargArgContainer *arg2);
bool carg_mutually_required (const CargArgContainer *arg1, const CargArgContainer *arg2);

CargArgContainer *carg_arg_create                     (void *argVarPtr, size_t varSize, uint64_t flagsArg, const char usageStringArg[]);
CargArgContainer *carg_nested_container_create        (CargArgContainer *arg, const char nestedArgString[], uint64_t flagsArg, const char * format);
CargArgContainer *carg_nested_boolean_container_create(CargArgContainer *arg, const char nestedArgString[], uint64_t flagsArg);
CargArgContainer *carg_nest_container                 (CargArgContainer *nestIn, CargArgContainer *argToNest, const char nestedArgString[], const char * format);
CargArgContainer *carg_nest_boolean_container         (CargArgContainer *nestIn, CargArgContainer *argToNest, const char nestedArgString[]);
void              carg_heap_default_value             (const CargArgContainer *heapArg, const void *val, size_t bytes);

void carg_set_named_args          (const char * format, ...);
void carg_set_positional_args     (const char * format, ...);
void carg_set_grouped_boolean_args(const char * format, ...);
void carg_set_env_defaults        (const char * format, ...);
void carg_set_nested_args         (int nestedArgumentCount, ...);

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//  SECTION: Internal Function Prototypes
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

int   internal_carg_test_printf           (char *formatter, ...);
int   internal_carg_test_vsnprintf        (const char *formatter, va_list args);
int   internal_carg_secure_sprintf_concat (char *startPointer, char *endPointer, char **cursor, const char *formatter, ...);
int   internal_carg_secure_vsprintf_concat(char *startPointer, char *endPointer, char **cursor, const char *formatter, va_list argsToCopy);
char *internal_carg_strdup                (const char *str);

void internal_carg_flag_conditional(uint64_t flag, bool truthiness, const char *errorMessage);
void internal_carg_error           (const char *formatter, ...);
void internal_carg_heap_check      (void *ptr);
void internal_carg_free_nullify    (void *ptr);

char *internal_carg_strtok_string_init    (const char * str);
void  internal_carg_strtok_register_string(char *str);
int   internal_carg_cmp_flag              (const char *argument, const char *parameter);
int   internal_carg_is_flag               (const char *formatter, const char *toCheck);

void internal_carg_usage_default(void);

void internal_carg_print_positional_usage_buffer(void);
void internal_carg_print_non_positional_usage_buffer(void);

bool internal_carg_adjust_multi_arg_setter(CargArgContainer *currentArg, void **varDataPtr);
bool internal_carg_adjust_named_assign    (int argIndex, const char *formatToken, const char *flagToken, const char **argToFormat, char *argumentFlagToCompare);

void internal_carg_validate_formatter_extended(const char *formatToken);
void internal_carg_validate_formatter         (const char *formatToken);
void internal_carg_validate_flag              (const char *flagToken);

void internal_carg_reference_named_arg_formatter          (int argIndex, const char *format, va_list args);
void internal_carg_reference_positional_arg_formatter     (CargArgContainer *currentArg, int i, void **formatToTokenizeAllocation, char **formatToTokenize, char **tokenSavePointer, void **varDataPtr);
void internal_carg_reference_grouped_boolean_arg_formatter(int i, size_t j, const char *noPrefixFormat, bool **varDataPtr, va_list args);

void internal_carg_set_named_arg   (CargArgContainer *currentArg, void **varDataPtr, int argIndex, const char *formatToken, const char *argToFormat, char **formatToTokenize, char **argumentFlagToCompare);
int  internal_carg_set_nested_arg  (CargArgContainer *currentArg);
void internal_carg_set_env_defaults(char **stringToTokenize, char **tokenSavePointer, void **stringAllocation, va_list args);

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//  SECTION: Platform Compatibility Enforcement
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//  Provide C standard errors for those using outdated standards.
//  None of the functions will have bodies if this condition is met.
#if (__STDC_VERSION__ < 199901L || !defined(__STDC_VERSION__)) && !defined(__cplusplus) && !defined(_MSC_VER)
    #error args.h is only supported on the C99 standard and above.
#else

#ifndef CARG_CUSTOM_IMPL
    #include "carg_impl.h" // NOLINT
#endif

#endif

#ifdef __cplusplus
}
#endif
