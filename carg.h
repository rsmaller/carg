//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//  SECTION: Includes and Type Definitions
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once
#ifndef CARG_H
    #define CARG_H
#endif

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

#define maxFormatterSize 128

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
    char                      formatterUsed[maxFormatterSize];
    const char               *nestedArgString;
    int                       nestedArgFillIndex;
    size_t                    nestedArgArraySize;
    struct CargArgContainer  *parentArg;
    struct CargArgContainer **nestedArgs;
} CargArgContainer;

typedef struct argArray {
    size_t          size;
    int             fillIndex;
    CargArgContainer    **array;
} argArray;

typedef void (*CargCallbackFunc)(void);

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//  SECTION: Global Variables and Definitions
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

int argCount           = 1;
char **argVector       = NULL;
argArray allArgs       = {.size = 0, .fillIndex = -1, .array = NULL};
int *setArgs           = NULL;
int positionalArgCount = 0;

#define usageStringSize 2048U
char         usageString[usageStringSize] = "Please specify a usage message in your client code. You can do this via carg_set_usage_message() or carg_usage_message_autogen().";
char *       usageStringCursor            = usageString; // The default usage message should be immediately overwritten when a usage message setter is called.
char * const usageStringEnd               = usageString + usageStringSize - 1;

uint64_t cargInternalFlags = 0;

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//  SECTION: Flags, Flag Checkers, and Initializer Macros
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//  Generic Flags.
#define NO_FLAGS                (0ULL)  // Set no flags when creating an argument.
#define NO_DEFAULT_VALUE        {0}     // Set default argument to 0 in arg initializers, for readability purposes.
#define NO_USAGE_STRING         ""      // To declare in an argument initializer that no usage string should be present.
#define NONE                            // Empty and does nothing. For semantics in function-style macros like CARG_ARG_CREATE().

//  Argument Flags. (These should be set by the user.)
#define POSITIONAL_ARG                  (1ULL<<32ULL)
#define BOOLEAN_ARG                     (1ULL<<33ULL)
#define ENFORCE_NESTING_ORDER           (1ULL<<34ULL)
#define ENFORCE_STRICT_NESTING_ORDER    (1ULL<<35ULL)
#define MULTI_ARG                       (1ULL<<36ULL)
#define HEAP_ALLOCATED                  (1ULL<<37ULL)

//  Getters and Setters.
#define HAS_FLAG(item, flag)    (item & flag)
#define SET_FLAG(item, flag)    (item |= flag)
#define CLEAR_FLAG(item, flag)  (item &= ~flag)
#define TOGGLE_FLAG(item, flag) (item ^= flag)

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//  SECTION: Assertion Macros
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#define REQUIRED_ARGUMENT(varName)              (varName -> hasValue)
#define MUTUALLY_EXCLUSIVE(varName1, varName2)  (!(varName1 -> hasValue && varName2 -> hasValue))
#define MUTUALLY_REQUIRED(varName1, varName2)   (varName1 -> hasValue ? varName2 -> hasValue : 1)
#define USAGE_MESSAGE                           ((void *)0)

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//  SECTION: Internal Macros
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//  Function Initializer Flags for cargInternalFlags.
#define LIBCARG_INITIALIZED       (1ULL<<0ULL)
#define NAMED_ARGS_SET            (1ULL<<1ULL)
#define POSITIONAL_ARGS_SET       (1ULL<<2ULL)
#define GROUPED_ARGS_SET          (1ULL<<3ULL)
#define NESTED_ARGS_SET           (1ULL<<4ULL)
#define OVERRIDE_CALLBACKS_SET    (1ULL<<5ULL)
#define ASSERTIONS_SET            (1ULL<<6ULL)
#define USAGE_MESSAGE_SET         (1ULL<<7ULL)

//  Internal Argument Flags.
#define NESTED_ARG                (1ULL<<1ULL)
#define NESTED_ARG_ROOT           (1ULL<<2ULL)

//  Tokenization Macros
#define VA_ARG_1(arg1, ...)       arg1
#define VA_ARG_2(arg1, arg2, ...) arg2
#define EXPAND(x)                 x
#define EXPAND_R(x)               EXPAND(x)
#define TOKEN_TO_STRING(x)        #x

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//  SECTION: User-Facing Function Prototypes
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    
void          carg_init     (int argc, char **argv);
void          carg_validate (void);
void          carg_terminate(void);

void          carg_print_container_data (const CargArgContainer *container);
void         *carg_fetch_multi_arg_entry(const CargArgContainer *container, int index);

char         *carg_string_contains_substr(char *container, const char * const substr);
int           carg_string_contains_char  (const char * const container, char subchar);
const char   *carg_basename              (const char * const pathStart);
    
void          carg_set_usage_message    (const char * const format, ...);

void          carg_set_usage_function   (CargCallbackFunc usageFunc);
void          carg_override_callbacks   (const char * const format, ...);
void          carg_arg_assert           (const int assertionCount, ...);
void          usage                     (void);
void          carg_usage_message_autogen(void);

CargArgContainer *carg_arg_create                     (void *argVarPtr, size_t varSize, uint64_t flagsArg, const char usageStringArg[]);
CargArgContainer *carg_nested_container_create        (CargArgContainer *arg, const char nestedArgString[], const uint64_t flagsArg, const char * const format);
CargArgContainer *carg_nested_boolean_container_create(CargArgContainer *arg, const char nestedArgString[], const uint64_t flagsArg);
CargArgContainer *carg_nest_container                 (CargArgContainer *nestIn, CargArgContainer *argToNest, const char nestedArgString[], const char * const format);
CargArgContainer *carg_nest_boolean_container         (CargArgContainer *nestIn, CargArgContainer *argToNest, const char nestedArgString[]);
void              carg_heap_default_value             (const CargArgContainer *heapArg, const void *val, size_t bytes);

void          carg_set_named_args          (const char * const format, ...);
void          carg_set_positional_args     (const char * const format, ...);
void          carg_set_grouped_boolean_args(const char * const format, ...);
void          carg_set_env_defaults        (const char * const format, ...);
void          carg_set_nested_args         (const int nestedArgumentCount, ...);

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//  SECTION: Internal Function Prototypes
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    
int   test_printf           (char *formatter, ...);
int   test_vsnprintf        (const char *formatter, va_list args);
int   secure_sprintf_concat (char * const startPointer, char * const endPointer, const char **cursor, const char *formatter, ...);
int   secure_vsprintf_concat(char * const startPointer, char * const endPointer, const char **cursor, const char *formatter, va_list argsToCopy);
char *carg_strdup           (const char *str);

void  _carg_flag_conditional(uint64_t flag, bool truthiness, const char * const errorMessage);
void  _carg_error           (const char * const formatter, ...);
void  _carg_heap_check      (void *ptr);
void  _carg_free_nullify    (void *ptr);

char *_carg_strtok_string_init    (const char * const str);
void  _carg_strtok_register_string(char *str);
int   _carg_cmp_flag              (const char * const argument, const char * const parameter);
int   _carg_is_flag               (const char * const formatter, const char * const toCheck);

void             _carg_usage_default(void);
CargCallbackFunc _carg_usage_ptr = _carg_usage_default;
void             _carg_print_positional_usage_buffer(void);
void             _carg_print_non_positional_usage_buffer(void);

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