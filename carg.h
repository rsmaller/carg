//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//  SECTION: Includes and Type Definitions
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

//  ReSharper disable CppNonInlineFunctionDefinitionInHeaderFile
#if defined(__INTELLISENSE__)
    #define true 1
    #define false 0
#endif

#ifdef _MSC_VER
    #define strtok_r strtok_s
    // sscanf() is required for this project.
    #define _CRT_SECURE_NO_WARNINGS
    // Some variadic macros in this library do not use their variadic arguments.
    #pragma warning(disable:4003)
#endif

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

typedef struct argArray {
    size_t             size;
    int                fillIndex;
    CargArgContainer **array;
} argArray;

typedef void (*CargCallbackFunc)(void);

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//  SECTION: Global Variables and Definitions
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

extern int              _cargArgCount;
extern int              _cargPositionalArgCount;
extern int *            _cargSetArgs;
extern char **          _cargArgVector;
extern argArray         _cargAllArgs;
extern char             _cargUsageString[];
extern char *           _cargUsageStringCursor; // Pointer to the byte after the farthest one written to.
extern char * const     _cargUsageStringEnd;    // Pointer to the final byte of the usage string buffer.
extern uint64_t         _cargInternalFlags;
extern CargCallbackFunc _carg_usage_ptr;

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//  SECTION: Flags, Flag Checkers, and Initializer Macros
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//  Generic Flags.
#define NO_FLAGS (0ULL)
    
//  Getters and Setters.
#define HAS_FLAG(item, flag)    ((item) & (flag))
#define SET_FLAG(item, flag)    ((item) |= (flag))
#define CLEAR_FLAG(item, flag)  ((item) &= ~(flag))
#define TOGGLE_FLAG(item, flag) ((item) ^= (flag))

//  Internal Argument Flags.
#define NESTED_ARG                   (1ULL<<1ULL)
#define NESTED_ARG_ROOT              (1ULL<<2ULL)

//  User-Facing Argument Flags.
#define POSITIONAL_ARG               (1ULL<<32ULL)
#define BOOLEAN_ARG                  (1ULL<<33ULL)
#define ENFORCE_NESTING_ORDER        (1ULL<<34ULL)
#define ENFORCE_STRICT_NESTING_ORDER (1ULL<<35ULL)
#define MULTI_ARG                    (1ULL<<36ULL)
#define HEAP_ALLOCATED               (1ULL<<37ULL)

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//  SECTION: Internal Macros
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//  Function Initializer Flags for _cargInternalFlags.
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

char *      carg_string_contains_substr(char *container, const char * const substr);
int         carg_string_contains_char  (const char * const container, char subchar);
const char *carg_basename              (const char * const pathStart);

void carg_set_usage_message    (const char * const format, ...);
void carg_set_usage_function   (CargCallbackFunc usageFunc);
void carg_override_callbacks   (const char * const format, ...);
void carg_arg_assert           (const int assertionCount, ...);
void carg_usage                (void);
void carg_usage_message_autogen(void);

bool carg_required          (const CargArgContainer *arg);
bool carg_mutually_exclusive(const CargArgContainer *arg1, const CargArgContainer *arg2);
bool carg_mutually_required (const CargArgContainer *arg1, const CargArgContainer *arg2);

CargArgContainer *carg_arg_create                     (void *argVarPtr, size_t varSize, uint64_t flagsArg, const char usageStringArg[]);
CargArgContainer *carg_nested_container_create        (CargArgContainer *arg, const char nestedArgString[], const uint64_t flagsArg, const char * const format);
CargArgContainer *carg_nested_boolean_container_create(CargArgContainer *arg, const char nestedArgString[], const uint64_t flagsArg);
CargArgContainer *carg_nest_container                 (CargArgContainer *nestIn, CargArgContainer *argToNest, const char nestedArgString[], const char * const format);
CargArgContainer *carg_nest_boolean_container         (CargArgContainer *nestIn, CargArgContainer *argToNest, const char nestedArgString[]);
void              carg_heap_default_value             (const CargArgContainer *heapArg, const void *val, size_t bytes);

void carg_set_named_args          (const char * const format, ...);
void carg_set_positional_args     (const char * const format, ...);
void carg_set_grouped_boolean_args(const char * const format, ...);
void carg_set_env_defaults        (const char * const format, ...);
void carg_set_nested_args         (const int nestedArgumentCount, ...);

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//  SECTION: Internal Function Prototypes
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    
int   _carg_test_printf           (char *formatter, ...);
int   _carg_test_vsnprintf        (const char *formatter, va_list args);
int   _carg_secure_sprintf_concat (char * const startPointer, char * const endPointer, const char **cursor, const char *formatter, ...);
int   _carg_secure_vsprintf_concat(char * const startPointer, char * const endPointer, const char **cursor, const char *formatter, va_list argsToCopy);
char *_carg_strdup                (const char *str);

void _carg_flag_conditional(uint64_t flag, bool truthiness, const char * const errorMessage);
void _carg_error           (const char * const formatter, ...);
void _carg_heap_check      (void *ptr);
void _carg_free_nullify    (void *ptr);

char *_carg_strtok_string_init    (const char * const str);
void  _carg_strtok_register_string(char *str);
int   _carg_cmp_flag              (const char * const argument, const char * const parameter);
int   _carg_is_flag               (const char * const formatter, const char * const toCheck);

void _carg_print_positional_usage_buffer(void);
void _carg_print_non_positional_usage_buffer(void);

bool _carg_adjust_multi_arg_setter(CargArgContainer *currentArg, void **varDataPtr);
bool _carg_adjust_named_assign    (const int argIndex, const char * const formatToken, const char * const flagToken, const char **argToFormat, char *argumentFlagToCompare);

void _carg_validate_formatter_extended(const char * const formatToken);
void _carg_validate_formatter         (const char * const formatToken);
void _carg_validate_flag              (const char * const flagToken);

void _carg_reference_named_arg_formatter          (const int argIndex, const char *format, va_list args);
void _carg_reference_positional_arg_formatter     (CargArgContainer *currentArg, const int i, void **formatToTokenizeAllocation, char **formatToTokenize, char **tokenSavePointer, void **varDataPtr);
void _carg_reference_grouped_boolean_arg_formatter(const int i, const size_t j, const char *noPrefixFormat, bool **varDataPtr, va_list args);

void _carg_set_named_arg_internal   (CargArgContainer *currentArg, void **varDataPtr, const int argIndex, const char *formatToken, const char *argToFormat, char **formatToTokenize, char **argumentFlagToCompare);
int  _carg_set_nested_arg_internal  (CargArgContainer *currentArg);
void _carg_set_env_defaults_internal(char **stringToTokenize, char **tokenSavePointer, void **stringAllocation, va_list args);

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

// Reset macro definitions to not interfere with other included libraries.
#ifdef _MSC_VER 
    #undef strtok_r
    #undef _CRT_SECURE_NO_WARNINGS
#endif

#endif // For C standard compatibility check.

#ifdef __cplusplus
}

// For C++ linking compatibility
#endif