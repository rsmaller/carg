//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//  SECTION: Includes and Type Definitions
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

#define CARG_MAX_FORMATTER_SIZE ((size_t)128U)
#define CARG_USAGE_STRING_SIZE ((size_t)2048U)

#if defined(_MSC_VER) && !defined(CARG_STATIC)
    #define CARG_EXPORT __declspec(dllexport)
    #define CARG_IMPORT __declspec(dllimport)
#else
    #define CARG_EXPORT
    #define CARG_IMPORT
#endif

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

struct CargContext;

typedef void (*CargCallbackFunc)(void);
typedef void (*CargUsageFunc)   (const struct CargContext *);

typedef struct CargContext {
    int              cargArgCount;
    int              cargPositionalArgCount;
    int *            internal_cargSetArgs;
    char **          internal_cargArgVector;
    CargArgArray     internal_cargAllArgs;
    char *           internal_cargUsageString;
    char *           internal_cargUsageStringCursor;
    char *           internal_cargUsageStringEnd;
    uint64_t         internal_cargInternalFlags;
    CargUsageFunc    internal_carg_usage_ptr;
} CargContext;

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//  SECTION: Global Variables and Definitions
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#if !defined(CARG_INCLUDE) && !defined(CARG_IMPL)
    CARG_IMPORT CargContext *cargDefaultContext;
    #define CARG_INCLUDE
#endif

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//  SECTION: Flags and Initializer Macros
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//  Flag Construction.
#define CARG_FLAG_BIT(bit) ((uint64_t)(1ULL << bit))

//  Generic Flags.
#define CARG_ITEM_NO_FLAGS                     (0ULL)

//  Internal Argument Flags [Reserved bit indices 0 - 31].
#define CARG_ITEM_NESTED                       CARG_FLAG_BIT(0)
#define CARG_ITEM_NESTED_ROOT                  CARG_FLAG_BIT(1)

//  User-Facing Argument Flags [Reserved bit indices 32 - 63].
#define CARG_ITEM_POSITIONAL                   CARG_FLAG_BIT(32)
#define CARG_ITEM_BOOLEAN                      CARG_FLAG_BIT(33)
#define CARG_ITEM_ENFORCE_NESTING_ORDER        CARG_FLAG_BIT(34)
#define CARG_ITEM_ENFORCE_STRICT_NESTING_ORDER CARG_FLAG_BIT(35)
#define CARG_ITEM_MULTI                        CARG_FLAG_BIT(36)
#define CARG_ITEM_HEAP_ALLOCATED               CARG_FLAG_BIT(37)

//  Global State Flags for internal_cargInternalFlags.
#define CARG_INITIALIZED            CARG_FLAG_BIT(0)
#define CARG_NAMED_ARGS_SET         CARG_FLAG_BIT(1)
#define CARG_POSITIONAL_ARGS_SET    CARG_FLAG_BIT(2)
#define CARG_GROUPED_ARGS_SET       CARG_FLAG_BIT(3)
#define CARG_NESTED_ARGS_SET        CARG_FLAG_BIT(4)
#define CARG_OVERRIDE_CALLBACKS_SET CARG_FLAG_BIT(5)
#define CARG_ASSERTIONS_SET         CARG_FLAG_BIT(6)
#define CARG_USAGE_MESSAGE_SET      CARG_FLAG_BIT(7)

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//  SECTION: User-Facing Function Prototypes
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

CARG_EXPORT void carg_init     (int argc, char **argv);
CARG_EXPORT void carg_validate (void);
CARG_EXPORT void carg_terminate(void);

CARG_EXPORT void carg_init_ts     (CargContext **cargLocalContext, int argc, char **argv);
CARG_EXPORT void carg_validate_ts (const CargContext *cargLocalContext);
CARG_EXPORT void carg_terminate_ts(const CargContext *cargLocalContext);

CARG_EXPORT void carg_validate_context(const CargContext *cargLocalContext);

CARG_EXPORT void  carg_print_container_data (const CargArgContainer *container);
CARG_EXPORT void *carg_fetch_multi_arg_entry(const CargArgContainer *container, int index);

CARG_EXPORT const char *carg_basename              (const char *container);
CARG_EXPORT const char *carg_string_contains_substr(const char *container, const char *substr);
CARG_EXPORT int         carg_string_contains_char  (const char *container, char subchar);

CARG_EXPORT void carg_set_usage_message    (const char *format, ...);
CARG_EXPORT void carg_override_callbacks   (const char *format, ...);
CARG_EXPORT void carg_arg_assert           (int assertionCount, ...);
CARG_EXPORT void carg_set_usage_function   (CargUsageFunc usageFunc);
CARG_EXPORT void carg_usage_message_autogen(void);
CARG_EXPORT void carg_usage                (void);

CARG_EXPORT void carg_set_usage_message_ts    (CargContext *cargLocalContext, const char *format, ...);
CARG_EXPORT void carg_override_callbacks_ts   (CargContext *cargLocalContext, const char *format, ...);
CARG_EXPORT void carg_arg_assert_ts           (CargContext *cargLocalContext, int assertionCount, ...);
CARG_EXPORT void carg_set_usage_function_ts   (CargContext *cargLocalContext, CargUsageFunc usageFunc);
CARG_EXPORT void carg_usage_message_autogen_ts(CargContext *cargLocalContext);
CARG_EXPORT void carg_usage_ts                (const CargContext *cargLocalContext);

CARG_EXPORT void carg_set_usage_message_tsv (CargContext *cargLocalContext, const char *format, va_list args);
CARG_EXPORT void carg_override_callbacks_tsv(CargContext *cargLocalContext, const char *format, va_list args);
CARG_EXPORT void carg_arg_assert_tsv        (CargContext *cargLocalContext, int assertionCount, va_list args);

CARG_EXPORT bool carg_required          (const CargArgContainer *arg);
CARG_EXPORT bool carg_mutually_exclusive(const CargArgContainer *arg1, const CargArgContainer *arg2);
CARG_EXPORT bool carg_mutually_required (const CargArgContainer *arg1, const CargArgContainer *arg2);

CARG_EXPORT void carg_heap_default_value(const CargArgContainer *heapArg, const void *val, size_t bytes);

CARG_EXPORT CargArgContainer *carg_arg_create                     (void *argVarPtr, size_t varSize, uint64_t flagsArg, const char usageStringArg[]);
CARG_EXPORT CargArgContainer *carg_nested_container_create        (CargArgContainer *arg, const char nestedArgString[], uint64_t flagsArg, const char *format);
CARG_EXPORT CargArgContainer *carg_nested_boolean_container_create(CargArgContainer *arg, const char nestedArgString[], uint64_t flagsArg);
CARG_EXPORT CargArgContainer *carg_nest_container                 (CargArgContainer *nestIn, CargArgContainer *argToNest, const char nestedArgString[], const char *format);
CARG_EXPORT CargArgContainer *carg_nest_boolean_container         (CargArgContainer *nestIn, CargArgContainer *argToNest, const char nestedArgString[]);
CARG_EXPORT CargArgContainer *carg_arg_create_ts                  (CargContext *cargLocalContext, void *argVarPtr, size_t varSize, uint64_t flagsArg, const char usageStringArg[]);

CARG_EXPORT void carg_set_named_args          (const char *format, ...);
CARG_EXPORT void carg_set_positional_args     (const char *format, ...);
CARG_EXPORT void carg_set_grouped_boolean_args(const char *format, ...);
CARG_EXPORT void carg_set_env_defaults        (const char *format, ...);
CARG_EXPORT void carg_set_nested_args         (int nestedArgumentCount, ...);

CARG_EXPORT void carg_set_named_args_ts          (CargContext *cargLocalContext, const char *format, ...);
CARG_EXPORT void carg_set_positional_args_ts     (CargContext *cargLocalContext, const char *format, ...);
CARG_EXPORT void carg_set_grouped_boolean_args_ts(CargContext *cargLocalContext, const char *format, ...);
CARG_EXPORT void carg_set_nested_args_ts         (CargContext *cargLocalContext, int nestedArgumentCount, ...);
CARG_EXPORT void carg_set_env_defaults_ts        (const CargContext *cargLocalContext, const char *format, ...);

CARG_EXPORT void carg_set_named_args_tsv          (CargContext *cargLocalContext, const char *format, va_list args);
CARG_EXPORT void carg_set_positional_args_tsv     (CargContext *cargLocalContext, const char *format, va_list args);
CARG_EXPORT void carg_set_grouped_boolean_args_tsv(CargContext *cargLocalContext, const char *format, va_list args);
CARG_EXPORT void carg_set_nested_args_tsv         (CargContext *cargLocalContext, int nestedArgumentCount, va_list args);
CARG_EXPORT void carg_set_env_defaults_tsv        (const CargContext *cargLocalContext, const char *format, va_list args);

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//  SECTION: Internal Function Prototypes
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool internal_carg_has_flag   (uint64_t  item, uint64_t flag);
static void internal_carg_set_flag   (uint64_t *item, uint64_t flag);
static void internal_carg_clear_flag (uint64_t *item, uint64_t flag);
static void internal_carg_toggle_flag(uint64_t *item, uint64_t flag);

static int internal_carg_test_printf   (const char *formatter, ...);
static int internal_carg_test_vsnprintf(const char *formatter, va_list args);

static int   internal_carg_secure_sprintf_concat (char *startPointer, char *endPointer, char **cursor, const char *formatter, ...);
static int   internal_carg_secure_vsprintf_concat(char *startPointer, char *endPointer, char **cursor, const char *formatter, va_list argsToCopy);
static char *internal_carg_strdup                (const char *str);
static char *internal_carg_strtok_reentrant      (char *str, const char *delim, char **savePtr);

static void internal_carg_flag_conditional_ts(const CargContext *cargLocalContext, uint64_t flag, bool truthiness, const char *errorMessage);
static void internal_carg_error              (const char *formatter, ...);
static void internal_carg_heap_check         (const void *ptr);
static void internal_carg_free_nullify       (const void *ptr);

static int internal_carg_cmp_flag(const char *argument, const char *parameter);
static int internal_carg_is_flag (const char *formatter, const char *toCheck);

static void internal_carg_usage_default_ts(const CargContext *cargLocalContext);

static void internal_carg_print_positional_usage_buffer_ts    (CargContext *cargLocalContext);
static void internal_carg_print_non_positional_usage_buffer_ts(CargContext *cargLocalContext);

static bool internal_carg_adjust_multi_arg_setter_ts(const CargContext *cargLocalContext, CargArgContainer *currentArg, void **varDataPtr);
static bool internal_carg_adjust_named_assign_ts    (const CargContext *cargLocalContext, int argIndex, const char *formatToken, const char *flagToken, const char **argToFormat, char *argumentFlagToCompare);

static void internal_carg_validate_formatter_extended(const char *formatToken);
static void internal_carg_validate_formatter         (const char *formatToken);
static void internal_carg_validate_flag              (const char *flagToken);

static void internal_carg_reference_named_arg_formatter_ts          (const CargContext *cargLocalContext, int argIndex, const char *format, va_list args);
static void internal_carg_reference_positional_arg_formatter_ts     (const CargContext *cargLocalContext, CargArgContainer *currentArg, int i, void **formatToTokenizeAllocation, char **formatToTokenize, char **tokenSavePointer, void **varDataPtr);
static void internal_carg_reference_grouped_boolean_arg_formatter_ts(const CargContext *cargLocalContext, int i, size_t j, const char *noPrefixFormat, bool **varDataPtr, va_list args);

static void internal_carg_set_named_arg_ts   (const CargContext *cargLocalContext, CargArgContainer *currentArg, void **varDataPtr, int argIndex, const char *formatToken, const char *argToFormat, char **formatToTokenize, char **argumentFlagToCompare);
static int  internal_carg_set_nested_arg_ts  (const CargContext *cargLocalContext, CargArgContainer *currentArg);
static void internal_carg_set_env_defaults_ts(const CargContext *cargLocalContext, char **stringToTokenize, char **tokenSavePointer, void **stringAllocation, va_list args);

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//  SECTION: Platform Compatibility Enforcement
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//  Provide C standard errors for those using outdated standards.
//  None of the functions will have bodies if this condition is met.
#if (__STDC_VERSION__ < 199901L || !defined(__STDC_VERSION__)) && !defined(__cplusplus) && !defined(_MSC_VER)
    #error args.h is only supported on the C99 standard and above.
#else

#endif

#ifdef __cplusplus
}
#endif
