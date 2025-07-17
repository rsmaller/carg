# carg - C Argument Parsing Library

## How to Use This Library
Before using this library, ensure your `main()` function accepts an `argc` and `argv` as parameters.
This library parses those parameters using string formatters and sets variables to their values accordingly.

Because this library uses string formatters, you should only use types that support `scanf()` formatting.
For example, there is no formatter for an array of 100 pointers to functions that accept integers and return a character.
Nor should you expect data remotely resembling that from a user.

This library is a header-only library. Everything is in the `carg.h` header, so to add this library into your project,
put it in your environment's include path and include the header accordingly:

```
#include "carg.h"
```

Make sure to put the `carg_impl.h` implementation header in your project directory as well.

If for any reason you would like to create your own implementation for this library, define `CARGS_CUSTOM_IMPL`
before including the `carg.h` header.

To set up the library to use your argument vector, call `carg_init()` with the argument count and vector.

At the end of your usage of this library, be sure to call `carg_terminate()` to clean up any heap-allocated memory and
prevent memory leaks.

### Setting a Usage Message
It's likely the first thing you will want to do is declare a usage message for your program.
The `carg_set_usage_message()` function allows you to do this; it sets a usage message that caps out at 1023 characters.
It accepts a string with formatters in it. This might look something like:

```
carg_set_usage_message("USAGE: %s -n Arg1 -t Arg2", carg_basename(argv[0]));
```

The `usage()` function will print out this message and terminate the program.

### Initializing Arguments
As for arguments, they are stored in structs and variables of their respective type.
Structs serve to keep track of whether an argument has been specified by the user or not.
Declaring an argument is relatively simple; first, create a variable where the argument will be stored:

```
int myArg1 = 5;
```

In this case, the default value of the argument will be 5.
Then, an argument can be initialized from this variable. Use the `carg_arg_create()` function to create an argument with
the address of the variable created above:

```
CargArgContainer *myArg1Container = carg_arg_create(&intArgValue, sizeof(int), NO_FLAGS, "-n <number>");
```

Note that the `carg_arg_create()` function returns a pointer to an argument container, which is initialized using the 
address of the variable used, the size of the variable, a flags option, and a usage string.

### Setting Argument Values from The Argument Vector
The value of an argument should only be accessed after setting the variables' values in accordance with user input.
Keep in mind variables must be initialized via the `carg_arg_create()` function before being set.
To set argument values from the user, use the `carg_set_named_args()` function. This function accepts the `argc` and `argv` parameters and a string formatter for arguments.
`carg_set_named_args()` accepts both flag parameters and string formatters associated with them.
That formatter might look like: `"-v:%s"` followed by a string argument struct.
This would mean that the flag -v should be followed by a string:

```
example.exe -v <INSERT_STRING_HERE>
```

Below is an example of the initializer and setter in conjunction.
Keeping everything in mind, the char variable would need to be initialized first:

```
char myChar = 'c';
CargArgContainer *myCharContainer = carg_arg_create(&myChar, sizeof(char), NO_FLAGS, NO_USAGE_STRING)
```

All arguments should be initialized before setting them, so let's add an int argument also:

```
int myInt = 0;
CargArgContainer *myIntContainer = carg_arg_create(&myInt, sizeof(int), NO_FLAGS, NO_USAGE_STRING);
```

Then, these argument containers can be set using the `carg_set_named_args()` function:

```
carg_set_named_args("-v:%c -i:%d", myCharContainer, myIntContainer);
```

The `%c` formatter corresponds to the argument struct `myCharContainer` and the `%d` formatter corresponds to the struct `myIntContainer`.

If you need to have spaces in a string, be sure to use the `%[^\n]` formatter instead of `%s`.
String arguments with spaces on the command line need to be delimited with double quotes regardless.

Assuming the resulting program is run with the following arguments:

```
example.exe -v y -i 5
```

`myChar` will contain the character `'y'` and `myInt` will contain the number 5.

We also declared default values for those, so if we instead ran:

```
example.exe
```

`myChar` would contain the character `'c'` and `myInt` would contain the number 0.

### Argument Assertions
You may want to make an argument required or limit which values the user can set it to, especially one that is initialized with NO_DEFAULT_VALUE.

`carg_arg_assert()` is designed for this; it accepts the number of argument assertions as an argument. All assertions after that are two arguments each.

The first argument should be the condition that must be met. This can be any expression which can be evaluated as a zero versus nonzero value.
If this condition is that the argument is required, use `REQUIRED_ARGUMENT()` with the corresponding argument struct.
If two arguments should not be set at the same time, use `MUTUALLY_EXCLUSIVE()` with the corresponding structs.
Lastly, `MUTUALLY_REQUIRED()` should be used to assert that two arguments must be set simultaneously when used.

The second argument should be the message to display if the condition is not met. Set this to `USAGE_MESSAGE` to use the usage message instead.

For example:

```
carg_arg_assert(3, 
        myInt > -1, "Int 1 must not be negative",
        REQUIRED_ARGUMENT(myInt), USAGE_MESSAGE,
        REQUIRED_ARGUMENT(myString), USAGE_MESSAGE
);
```

will print `"Int 1 must not be negative"`if a value less than or equal to -1 is given.
The usage message will show if arguments `myInt` or `myString` are not given values by the user.

### Arguments That Override Program Control Flow
To specify arguments that make the program do something entirely different, primarily running a single function and then terminating, call the `carg_override_callbacks()` function. This function accepts the argument count, argument vector, flags, and function pointers associated with them.

For example, to declare arguments for a help-displaying function and another random helper function:

```
carg_override_callbacks("-h -r", &help, &randomHelperFunction);
```

This function should be called before any other arguments are set.

### Arguments Without Flags
Another feature this library supports is positional arguments. Positional arguments are passed in to the program without a flag.
These arguments should always come before named arguments to prevent argument ambiguity.

To use positional arguments alongside named arguments, initialize both first:

```
int positionalArg = 0;
char namedArg = 'a';
CargArgContainer *positionalCargArgContainer = carg_arg_create(&positionalArg, sizeof(int), POSITIONAL_ARG, NO_USAGE_STRING);
CargArgContainer *namedCargArgContainer = carg_arg_create(&namedArg, sizeof(char), NO_FLAGS, NO_USAGE_STRING);
```

Then, set the values for positional arguments first:

```
carg_set_positional_args("%d", positionalArg);
```

Lastly, set the values for named arguments:

```
carg_set_named_args("-n:%d", namedArg);
```

Keep in mind that positional arguments are required regardless if they are enforced with an assertion or not.
Furthermore, they are assigned to argument variables based on their order. Make sure they line up correctly when you set their values!
Positional arguments can be used in assertions the same way as named arguments.

## Function Implementations

### Generic Functions

#### carg_init()
Pass `argc` and `argv` into this to initialize the library to use the command line arguments passed in to your program.

#### string_contains_substr()
The `string_contains_substr()` function will return the pointer where a substring starts in another string. If the substring is not 
a part of the greater string, it returns `NULL`.

#### string_contains_char()
The `string_contains_char()` function returns the index of the first occurrence of a character in a string. If the character is 
not in the string, it returns -1.

#### carg_basename()
The `carg_basename()` function is similar to the function often included in `libgen.h` in POSIX systems; considering this 
header is not officially supported on other compilers, a basic implementation of `carg_basename` is included in this 
argument library.
A basename function takes a full file path string and returns the very last item in the file tree, or more specifically 
the substring following all forward or backward slashes. For example, `carg_basename("C:\Users\User1\test.exe")` will return 
`test.exe`. This is useful for truncating the name of your program as it appears in the argument vector.

#### usage()
This simply prints out the usage message and terminates the program.

#### carg_set_usage_message()
This function-style macro accepts a string formatter and variadic arguments. It uses both of those pieces of information 
to manually generate and set a usage message for your program.

#### carg_set_usage_function()
This function will set a function to use for showing the usage message. If you want a custom usage message, it is 
recommended to use carg_set_usage_message() instead, though this function can be used if special functionality is required for 
your usage messages.

#### carg_usage_message_autogen()
This function will automatically generate a usage message based on initialized arguments. As such, it should be called 
after all arguments have been initialized, and it should not be called alongside `carg_set_usage_message()`. Furthermore, the
usage message should be set before running any assertions.

This function works by combining the usage strings set via `carg_arg_create()` with boolean flags and nested argument flags
to generate a usage message.

For example:

```
int arg1 = 0;
CargArgContainer *arg1Container = carg_arg_create(&arg1, sizeof(int), NO_FLAGS, "-n <number>");
```

Will add `-n <number>` to the usage string.

#### CARG_PRINT_STRING_ARG() and CARG_PRINT_NON_STRING_ARG()
These macros both print out an individual argument, though the handling of the values in each argument is different
for string versus non-string printed types. Use the respective macro to fetch data about an argument of any type.
Both macros accept an argument struct pointer to print from.
Note that printing functions are type-agnostic when printing pointer values because the pointer type is grabbed from the 
formatter; therefore, `CARG_PRINT_STRING_ARG()` requires no type to be passed in.
However, `CARG_PRINT_NON_STRING_ARG()` does require a provided type.
For arguments which are stored as pointers, the indirection is handled internally; only the type which will be printed 
out should be passed in to these macros (i.e. `int *` arguments should be passed to the macros as `int` types).

For example:

```
CARG_PRINT_NON_STRING_ARG(intArg, int);
CARG_PRINT_STRING_ARG(stringArg);
```

would be the conventional way to call these macros.

#### CARG_PRINT_STRING_MULTI_ARG() and CARG_PRINT_NON_STRING_MULTI_ARG()
These are the respective macro variants for printing out multi-argument versions of the above macros.
Similar to above, `CARG_PRINT_STRING_MULTI_ARG()` does not accept a type as a parameter. Furthermore, pointer indirection
likewise applies here.

These macros may be used as shown below:

```
CARG_PRINT_NON_STRING_MULTI_ARG(multiIntArg, int);
CARG_PRINT_STRING_MULTI_ARG(multiStringArg);
```

#### carg_terminate()
This function will clean up heap allocations this library uses to parse and set arguments, particularly for automatic 
usage message generation. This should be called after all arguments have been set and after any usage message can be 
generated or printed, or at the end of the program's runtime. Therefore, it should be called after assertions are made.

### Argument Initialization

#### carg_arg_create()
This function takes in the address of a variable which stores an argument's data, the memory capacity available for
storing data in that variable (which is usually the size of the variable unless it is heap-allocated), the flags for the
argument to create, and a usage string, which is semantic and can be anything.

### carg_heap_default_value()
This function will `memcpy()` a piece of data into a heap-allocated argument. Provide it with an argument container, a 
piece of memory to copy in, like the address of an integer for example, and the number of bytes to copy, which would 
most likely be 4 in this example.

### Argument Setting

#### carg_set_named_args()
This function is variadic; it accepts the argument count, the argument vector, a string formatter, and a sequence of
arguments which are the addresses of argument structs. This data is used to set arguments based on what is passed from 
the command line. The argument structs should correspond to flags in the string formatter. Each argument should be a 
flag plus a colon plus the corresponding string formatter. Each argument should also be separated by spaces. For 
example:

```
carg_set_named_args("-n:%d -t:%10s -b:bool", intArg, stringArg, boolArg)
```

- Will use the `-n` flag plus a digit value to set the `intArg` argument and subsequently the `intArgValue` variable.
- Will use the `-t` flag plus a string value to set the `stringArg` argument and subsequently the `stringArgValue` variable.
- Will use the `-b` flag to toggle the boolean `boolArg` argument and subsequently the `boolArgValue` variable.

When passed in via the command line, the flag and its value may be separated by a space or an `=`, like in keyword argument
syntax: `-n=5`. Using this syntax with a boolean argument will toggle the boolean while discarding the value provided.

#### carg_set_positional_args()
This function works similarly to `carg_set_named_args()`, but it has a few key differences:
- Arguments are "positional", meaning they are defined based on their order and not any flags.
- The string formatter passed in should not have any flags in it as a result.

For example:

```
carg_set_positional_args("%d %d %20s", positionalArg, positionalArg2, positionalStringArg);
```

 - Will use a digit formatter to set `positionalArg` to the value passed in as the very first command line argument.
 - Will use a digit formatter to set `positionalArg2` to the value passed in as the second command line argument.
 - Will use a string formatter to set `positionalStringArg` to the value passed in as the third command line argument.

#### carg_set_grouped_boolean_args()
This function takes in a string, which should be a prefix plus a series of characters, each one representing a boolean
flag in order. The order of the characters in the string match up to the boolean argument structs passed in to this 
function. This function gives similar functionality to `getopt()` on UNIX systems because it allows for different
permutations of flags like `-bc`, `-cb`, and `-b -c` from the command line.

Keep in mind when using this function that it will try to match any arguments which begin with the prefix, '-' in this
example, that have characters which match those in the boolean grouped flags string. Fortunately, argument vectors which 
have been grabbed by named arguments will be marked as such and skipped by this function. Therefore, this function must
be called after all other non-default arguments are initialized.

#### carg_set_env_defaults()
This function takes in a string with flags and formatters in it, like the other setters in this library. This function
expects the flags to contain the name of an environment variable, which it will then fetch and `sscanf()` into an 
argument value using the corresponding formatter. The environment variable is only copied into the argument if the 
argument is not set; if setting an argument value to an environment variable unconditionally is not desirable, be sure 
to call this function after any other setter functions.

```
carg_set_env_defaults("PATH:%s", string1);
```
-   This will copy the value contained in the `$PATH` variable into the string `string1`.

### Argument Nesting

Argument nesting can be a little tricky, so it is best to start out creating boolean arguments for nesting purposes.
At least one of these booleans will serve as a root node, meaning the node where a tree of boolean options starts.

For the sake of clarification, nested arguments are meant to give options conditional to other options passed in. For
example:

```
mycommand push now
```

might be configured via a nested argument. The push option could be stored as a root nested node that has now nested
inside of it. In this case, the keyword `now` would only have meaning if `push` is supplied alongside it.

#### carg_nested_boolean_container_create()
This function will initialize a boolean argument as a nested argument root node. This function takes the argument, a 
string, and new flags; the string is the argument the program will search for to match and toggle that boolean argument.

#### carg_nest_boolean_container()
This function will nest a boolean argument in another nested argument, whether it is a root node or not. As of now, each 
individual nested boolean argument can only directly nest 256 other arguments. Since arguments will always end up nested
inside a root node, you can use curly braces to make the nesting a little easier to read. On a basic level, it looks
like:

```
carg_nested_boolean_container_create(nestedArg3, "nestedArg3", ENFORCE_STRICT_NESTING_ORDER); {
        carg_nest_boolean_container(nestedArg3, nestedArg4, "nestedArg4");
        carg_nest_boolean_container(nestedArg3, nestedArg5, "nestedArg5"); {
            carg_nest_boolean_container(nestedArg5, nestedArg6, "nestedArg6"); {
                carg_nest_boolean_container(nestedArg6, nestedArg7, "nestedArg7");
                carg_nest_boolean_container(nestedArg6, nestedArg8, "nestedArg8");
            }
        }
    }
```

This might be used as such:

```
program.exe nestedArg3 nestedArg5 nestedArg6
```

#### carg_nested_container_create() and carg_nest_container()
These are the non-boolean versions of the nested argument functions. They accept the same parameters as their boolean
counterparts, and they also require a `scanf()` formatter after their nested name, as such:

```
carg_nested_boolean_container_create(nestedArg1, "nestedArg1", ENFORCE_NESTING_ORDER); {
        carg_nest_boolean_container(nestedArg1, nestedArg2, "nestedArg2");
        carg_nest_boolean_container(nestedArg1, nestedArg4, "nestedArg4");
        carg_nest_container(nestedArg1, nestedArg21, "nestedArg21", "%d"); {
            carg_nest_container(nestedArg21, nestedArg22, "nestedArg22", "%d");
        }
    }
```

#### carg_set_nested_args()
This function accepts a variable number of root nested nodes after an integer representing the number of root nested 
nodes passed into it. It will then look through the argument vector to initialize boolean flags in accordance with 
implemented nesting logic.

Note that nesting in this library is order-agnostic, which is a little different from how it is implemented in other
libraries. If multiple sibling nested arguments are supplied by the user, the index which they exist internally in the 
graph data structure is used for argument precedence.

Nested arguments in this library are meant to be simple, lightweight, and intuitive; while they are programmatically 
implemented as a graph, they can simply be imagined as booleans which rely on other booleans to be set, but in any 
order.

For example, a random flag titled `flag1` might have a flag nested in it named `flag2`. If `flag2` is supplied to the 
command line, it will only be toggled if `flag1` is also supplied on the command line. However, `flag1` does not need to 
come before `flag2`.

```
program.exe flag1 flag2
```
- This will set both `flag1` and `flag2`.

```
program.exe flag2 flag1
```
- So will this if nesting order is not enforced.

```
program.exe flag1
```
- This will only toggle `flag1`.

```
program.exe flag2
```
- This will return an error for an unknown argument. `flag2` means nothing without its parent, `flag1`.

### Control Flow Interrupts

#### carg_override_callbacks()
This function accepts the argument count and vector, plus a series of variadic arguments, mainly a formatter and 
function pointers which accept no arguments and return nothing. This function type is referred to as `CargCallbackFunc` in
the `carg.h` header. It will set flags where, when passed by the user, will run a specific function and terminate the 
program, essentially overriding the control flow of the program. This function should be called before any setter 
functions.

For example:

```
carg_override_callbacks("-h -h2", &help, &help2);
```

 - Means the `-h` flag will run the `help()` function and then terminate the program.
 - Means the `-h2` flag will run the `help2()` function and then terminate the program.

#### carg_arg_assert()
This function accepts variadic arguments for assertions plus assertion messages. `carg_arg_assert()` keeps track of this via 
an assertion count, which is the first argument this function accepts. Each assertion should test some value
for an argument, like `intArgValue > 0` for example. Assertion messages are what gets printed when the assertion fails.

An assertion might look like:

```
carg_arg_assert(1, intArgValue > 0, "Int argument must be positive");
```

You can otherwise specify `NULL` to print out the usage message instead. The `USAGE_MESSAGE` macro expands to NULL and 
can also be used; it exists for readability purposes.

#### carg_validate()
This function will do a final pass to ensure every argument in the argument vector has been used for something. If it
encounters a redundant or unused argument, it will show the argument in an error message and terminate the program. Of 
course, this function should be called after any argument setters. Furthermore, if `ENFORCE_NESTING_ORDER` is passed as
a flag to any nested argument, calling this function is necessary for enforcing the nesting order.

## Assertion Macros

### REQUIRED_ARGUMENT()
This is a function-style macro which accepts an argument struct as an argument. This is an assertion which will fail if 
an argument is not given a value by the user. This should be used in `carg_arg_assert()`:

```
carg_arg_assert(1, REQUIRED_ARGUMENT(charArg), "Char argument is required");
```

### MUTUALLY_EXCLUSIVE()
This is another assertion macro. This one accepts two argument structs, and the assertion will fail if both arguments 
have been provided a value by the user. In other words, this assertion forces the user to pick at most one of two 
arguments.

### MUTUALLY_REQUIRED()
Lastly, this assertion requires that two argument structs passed into it both have values set by the user. The assertion will fail otherwise.

## Initializer Flags

### NO_FLAGS
This is a macro which expands to `0`. This is a semantic choice to show in `carg_arg_create()` that an argument has no custom 
flags.

### POSITIONAL_ARG
This macro should be passed in to the flags section of `carg_arg_create()` to specify an argument should be 
given a value without any sort of flag preceding it.

### BOOLEAN_ARG
When specifying an argument should simply be a flag which toggles some variable on or off, two things must be done. The 
argument must be initialized as a boolean, which is what this flag is for. Pass this flag into `carg_arg_create()` and later 
reference the generated argument in `carg_set_named_args()` with `bool` to create a boolean argument. Boolean arguments
are always flags, and they can therefore never appear as a positional argument.

### HEAP_ALLOCATED
This is a flag for declaring an argument as heap-allocated. Doing so will cause this library to automatically free the
allocation when it terminates.

### ENFORCE_NESTING_ORDER
This is a flag for declaring the root of a nested argument as not order-agnostic; this flag should be set in
`carg_nested_container_create()`. The library will do runtime checks in `carg_validate()` to ensure arguments nested
within other arguments come after their parent arguments in the argument vector, so make sure to call this function to 
enable nesting order enforcement.

### ENFORCE_STRICT_NESTING_ORDER
This flag declares that nested arguments should be passed all in sequence, without any arguments in-between. Like
`ENFORCE_NESTING_ORDER`, this property is enforced in `carg_validate()`.

### MULTI_ARG
This flag should be used to declare that an argument should be a linked list of values from which to add on to each time
an argument is found. When an argument is toggled with this flag, it is allowed to be repeated in the argument vector.

To utilize the data stored in multi-argument vectors, the function `carg_fetch_multi_arg_entry()` exists. This function 
is a linked list iterator which returns a void pointer, meaning the pointer should be casted and dereferenced accordingly
when accessed. For example, 

```
int multi = *(int *)carg_fetch_multi_arg_entry(multiIntArg, 1);
```

Will set `multi` to the second integer passed as an argument to `multiIntArg`. If no second integer exists, this function
will throw an error.

### NO_DEFAULT_VALUE
This macro expands to `{0}` and should be used for argument variables which are not intended to have a default value. 
For example:

```
int noDefault = NO_DEFAULT_VALUE;
```

This macro is entirely semantic because it only zero-initializes whatever it is passed into. Please assert the respective argument as a required argument if it should not have a default value.

### NO_USAGE_STRING
This macro expands to `""`, or the empty string. Its purpose is in `carg_arg_create()` to declare that an argument has no 
usage string associated with it.

### NONE
This macro expands to nothing, and it exists to prevent macros from misbehaving.

