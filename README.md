# cargs - C Argument Parsing Library

## How to Use This Library
Before using this library, ensure your `main()` function accepts an `argc` and `argv` as parameters.
This library parses those parameters using string formatters and sets variables to their values accordingly.

Because this library uses string formatters, you should only use types that support `scanf()` formatting.
For example, there is no formatter for an array of 100 pointers to functions that accept integers and return a character.
Nor should you expect data remotely resembling that from a user.

This library is a header-only library. Everything is in the `args.h` header, so to add this library into your project,
put it in your environment's include path and include the header accordingly:

```
#include "args.h"
```

To set up the library to use your argument vector, call `libcargInit()` with the argument count and vector.

At the end of your usage of this library, be sure to call `libcargTerminate()` to clean up any heap-allocated memory and
prevent memory leaks.

### Setting a Usage Message
It's likely the first thing you will want to do is declare a usage message for your program.
The `setUsageMessage()` function allows you to do this; it sets a usage message that caps out at 1023 characters.
It accepts a string with formatters in it. This might look something like:

```
setUsageMessage("USAGE: %s -n Arg1 -t Arg2", cargBasename(argv[0]));
```

The `usage()` function will print out this message and terminate the program.

### Initializing Arguments
As for arguments, they are stored in structs and variables of their respective type.
Structs serve to keep track of whether an argument has been specified by the user or not.
To simplify the declaration of arguments, `argInit()` and `basicArgInit()` were created.
Simply call `argInit()` or `basicArgInit()` with both a variable name to declare, type information, a default value, a bitmask representing toggled flags, and a usage string.
To write explicitly that a variable should not have a default value, use the `NO_DEFAULT_VALUE` macro in place of a default value.
This macro is only for readability, and it will zero-initialize the variable you create.
To declare that an argument has no usage string associated with it, use the `NO_USAGE_STRING` macro.
To enforce that an argument must be given a value, see `argAssert()` below.

`argInit()`, `basicArgInit()` and `heapArgInit()` will create a struct from the variable name entered into them.
Keep in mind the resulting struct uses a void pointer to reference the variable where the argument data will be stored.
To access the data the user entered, add "Value" to the end of the struct's name.
For arguments initialized with `heapArgInit()`, the "Value" variable contains a pointer to the heap-allocated memory.
This pointer will be freed automatically via `libcargTerminate()`.
For example, when declaring an int argument via:

```
basicArgInit(int, intArg, 1, NO_FLAGS, NO_USAGE_STRING);
```

- `intArg` is a struct.
- `intArg` contains a void pointer to `intArgValue`, an int representing whether the argument has been set by the user, and a flags bitmask.
- `intArgValue` is an int where the value of the argument is stored. 
- If the arg was declared as a char instead, the type of `intArgValue` would be char.

### Setting Argument Values from The Argument Vector
The value of an argument should only be accessed after setting the variables' values in accordance with user input.
Keep in mind variables must be initialized via `argInit()`, `basicArgInit()`, `pointerArgInit()` or `heapArgInit()` before being set.
To set argument values from the user, use the `setFlagsFromNamedArgs()` function. This function accepts the `argc` and `argv` parameters and a string formatter for arguments.
`setFlagsFromNamedArgs()` accepts both flag parameters and string formatters associated with them.
That formatter might look like: `"-v:%s"` followed by a string argument struct.
This would mean that the flag -v should be followed by a string:

```
example.exe -v <INSERT_STRING_HERE>
```

Below is an example of the initializer and setter in conjunction.
Keeping everything in mind, the char variable would need to be initialized first:

```
basicArgInit(char, myString, 'c', NO_FLAGS, NO_USAGE_STRING);
```

All arguments should be initialized before setting them, so let's add an int argument also:

```
basicArgInit(int, myInt, 0, NO_FLAGS, NO_USAGE_STRING);
```

Then, these values can be set using the `setFlagsFromNamedArgs()` function:

```
setFlagsFromNamedArgs("-v:%s -i:%d", myString, myInt);
```

The %s formatter corresponds to the argument struct `myString` and the %d formatter corresponds to the struct `myInt`.

If you need to have spaces in your string, be sure to use the `%[^\n]` formatter instead of `%s`.
String arguments with spaces need to be delimited with double quotes.

Assuming the resulting program is run with the following arguments:

```
example.exe -v yes -i 5
```

`myStringValue` will contain the string "yes" and `myIntValue` will contain the number 5.

We also declared default values for those, so if we instead ran:

```
example.exe
```

`myStringValue` would contain the string "default" and `myIntValue` would contain the number 5.

### Argument Assertions
You may want to make an argument required or limit which values the user can set it to, especially one that is initialized with NO_DEFAULT_VALUE.

`argAssert()` is designed for this; it accepts the number of argument assertions as an argument. All assertions after that are two arguments each.

The first argument should be the condition that must be met. This can be any expression which can be evaluated as a zero versus nonzero value.
If this condition is that the argument is required, use `requiredArgument()` with the corresponding argument struct.
If two arguments should not be set at the same time, use `mutuallyExclusive()` with the corresponding structs.
Lastly, `mutuallyRequired()` should be used to assert that two arguments must be set simultaneously when used.

The second argument should be the message to display if the condition is not met. Set this to `USAGE_MESSAGE` to use the usage message instead.

For example:

```
argAssert(3, 
        myIntValue > -1, "Int 1 must not be negative",
        requiredArgument(myInt), USAGE_MESSAGE,
        requiredArgument(myString), USAGE_MESSAGE
);
```

will print `"Int 1 must not be negative"`if a value less than or equal to -1 is given.
The usage message will show if arguments `myInt` or `myString` are not given values by the user.

### Arguments That Override Program Control Flow
To specify arguments that make the program do something entirely different, primarily running a single function and then terminating, call the `argumentOverrideCallbacks()` function. This function accepts the argument count, argument vector, flags, and function pointers associated with them.

For example, to declare arguments for a help-displaying function and another random helper function:

```
argumentOverrideCallbacks("-h -r", &help, &randomHelperFunction);
```

This function should be called before any other arguments are set.

### Arguments Without Flags
Another feature this library supports is positional arguments. Positional arguments are passed in to the program without a flag.
These arguments should always come before named arguments to prevent argument ambiguity.

To use positional arguments alongside named arguments, initialize both first:

```
basicArgInit(int, positionalArg, 0, POSITIONAL_ARG, NO_USAGE_STRING);
basicArgInit(char, namedArg, 'a', NO_FLAGS, NO_USAGE_STRING);
```

Then, set the values for positional arguments first:

```
setFlagsFromPositionalArgs("%d", &positionalArg);
```

Lastly, set the values for named arguments:

```
setFlagsFromNamedArgs("-n:%d", &namedArg);
```

Keep in mind that positional arguments are required regardless if they are enforced with an assertion or not.
Furthermore, they are assigned to argument variables based on their order. Make sure they line up correctly when you set their values!
Positional arguments can be used in assertions the same way as named arguments.

## Function Implementations

### Generic Functions

#### libcargInit()
Pass `argc` and `argv` into this to initialize the library to use the command line arguments passed in to your program.

#### contains()
The `contains()` function will return the pointer where a substring starts in another string. If the substring is not 
a part of the greater string, it returns `NULL`.

#### charInString()
The `charInString()` function returns the index of the first occurrence of a character in a string. If the character is 
not in the string, it returns -1.

#### cargBasename()
The `cargBasename()` function is similar to the function often included in `libgen.h` in POSIX systems; considering this 
header is not officially supported on other compilers, a basic implementation of `cargBasename` is included in this 
argument library.
A basename function takes a full file path string and returns the very last item in the file tree, or more specifically 
the substring following all forward or backward slashes. For example, `cargBasename("C:\Users\User1\test.exe")` will return 
`test.exe`. This is useful for truncating the name of your program as it appears in the argument vector.

#### usage()
This simply prints out the usage message and terminates the program.

#### setUsageMessage()
This function-style macro accepts a string formatter and variadic arguments. It uses both of those pieces of information 
to manually generate and set a usage message for your program.

#### setUsageFunc()
This function will set a function to use for showing the usage message. If you want a custom usage message, it is 
recommended to use setUsageMessage() instead, though this function can be used if special functionality is required for 
your usage messages.

#### usageMessageAutoGenerate()
This function will automatically generate a usage message based on initialized arguments. As such, it should be called 
after all arguments have been initialized, and it should not be called alongside `setUsageMessage()`. Furthermore, the
usage message should be set before running any assertions.

This function works by combining the usage string set via `argInit()` variadic arguments with the auto-generated type 
string of the argument, which may be something like `<int>` or `<char *>`. It displays positional arguments first, then 
boolean arguments, nested argument roots, then named arguments. Keep in mind that *only* nested argument roots will be 
shown in the usage message to avoid clutter; non-root nested arguments are not shown in the usage message.

For example, calling `basicArgInit()` as follows:

```
basicArgInit(int, intArg, NO_DEFAULT_VALUE, NO_FLAGS, "-n", NO_USAGE_STRING);
```

Will add `-n <int>` to the usage message.

#### printOutStringArgument() and printOutNonStringArgument()
These macros both print out an individual argument, though the handling of the values in each argument is different
for string versus non-string printed types. Use the respective macro to fetch data about an argument of any type.
Both macros accept an argument struct pointer to print from.
Note that printing functions are type-agnostic when printing pointer values because the pointer type is grabbed from the 
formatter; therefore, `printOutStringArgument()` requires no type to be passed in.
However, `printOutNonStringArgument()` does require a provided type.
For arguments which are stored as pointers, the indirection is handled internally; only the type which will be printed 
out should be passed in to these macros (i.e. `int *` arguments should be passed to the macros as `int` types).

For example:

```
printOutNonStringArgument(&intArg, int);
printOutStringArgument(&stringArg);
```

would be the conventional way to call these macros.

#### printOutStringMultiArgument() and printOutNonStringMultiArgument()
These are the respective macro variants for printing out multi-argument versions of the above macros.
Similar to above, `printOutStringMultiArgument()` does not accept a type as a parameter. Furthermore, pointer indirection
likewise applies here.

These macros may be used as shown below:

```
printOutNonStringMultiArgument(&multiIntArg, int);
printOutStringMultiArgument(&multiStringArg);
```

#### libcargTerminate()
This function will clean up heap allocations this library uses to parse and set arguments, particularly for automatic 
usage message generation. This should be called after all arguments have been set and after any usage message can be 
generated or printed, or at the end of the program's runtime. Therefore, it should be called after assertions are made.

### Argument Initialization

- As a short side note, all initialized arguments accept a variadic argument for a usage string. It is most helpful to 
put the expected flag both here and in the setter functions. This argument is not required, however. 

#### argInit()
This function-style macro initializes an argument via a variable where the result goes and a struct which contains a 
void pointer to that variable. It accepts split type information, a variable name, a default value, flags, and a usage string where the expected flag might go.
For example, to initialize a simple character argument, the following might be used:

```
arginit(char, charArg, NONE, NO_DEFAULT_VALUE, NO_FLAGS, NO_USAGE_STRING);
```

To initialize an array of 100 characters, that would look like:

```
arginit(char, charArrayArg, [100], NO_DEFAULT_VALUE, NO_FLAGS, NO_USAGE_STRING);
```

These arguments could later be accessed with `charArgValue` and `charArrayArgValue` respectively.
For use in argument setting functions, however, `charArg` and `charArrayArg` should be used.

#### basicArgInit()
This macro is a wrapper for `argInit()` which only specifies basic type information; arrays and function pointers cannot 
be declared with this macro. To declare the char argument like in `argInit()`, but with a default value of 2,
do the following:

```
basicArginit(char, charArg, 2, NO_FLAGS, NO_USAGE_STRING);
```

#### heapArgInit()
This function-style macro will heap-allocate a variable for which an argument's value will be copied into. It takes the 
same arguments as `argInit()`, except it has no default value argument and a memory allocation size must be given as the 
last argument to the macro.

#### pointerArgInit()
This macro will create an argument which contains a pointer which does not need to be automatically heap-allocated; the 
level of indirection on the value pointer element in the argument struct created from this macro and the `heapArgInit()` 
macro is one less than in the other argument initializers.

#### adjustArgumentCursor()
This function can change what variable an argument struct points to. This is useful if you would like to rename a 
variable, use a global variable, or do some pointer abstractions.

### Argument Setting

#### setFlagsFromNamedArgs()
This function is variadic; it accepts the argument count, the argument vector, a string formatter, and a sequence of
arguments which are the addresses of argument structs. This data is used to set arguments based on what is passed from 
the command line. The argument structs should correspond to flags in the string formatter. Each argument should be a 
flag plus a colon plus the corresponding string formatter. Each argument should also be separated by spaces. For 
example:

```
setFlagsFromNamedArgs("-n:%d -t:%10s -b:bool", &intArg, &stringArg, &boolArg)
```

- Will use the `-n` flag plus a digit value to set the `intArg` argument and subsequently the `intArgValue` variable.
- Will use the `-t` flag plus a string value to set the `stringArg` argument and subsequently the `stringArgValue` variable.
- Will use the `-b` flag to toggle the boolean `boolArg` argument and subsequently the `boolArgValue` variable.

When passed in via the command line, the flag and its value may be separated by a space or an `=`, like in keyword argument
syntax: `-n=5`. Using this syntax with a boolean argument will toggle the boolean while discarding the value provided.

#### setFlagsFromPositionalArgs()
This function works similarly to `setFlagsFromNamedArgs()`, but it has a few key differences:
- Arguments are "positional", meaning they are defined based on their order and not any flags.
- The string formatter passed in should not have any flags in it as a result.

For example:

```
setFlagsFromPositionalArgs("%d %d %20s", &positionalArg, &positionalArg2, &positionalStringArg);
```

 - Will use a digit formatter to set `positionalArg` to the value passed in as the very first command line argument.
 - Will use a digit formatter to set `positionalArg2` to the value passed in as the second command line argument.
 - Will use a string formatter to set `positionalStringArg` to the value passed in as the third command line argument.

#### setFlagsFromGroupedBooleanArgs()
This function takes in a string, which should be a prefix plus a series of characters, each one representing a boolean
flag in order. The order of the characters in the string match up to the boolean argument structs passed in to this 
function. This function gives similar functionality to `getopt()` on UNIX systems because it allows for different
permutations of flags like `-bc`, `-cb`, and `-b -c` from the command line.

Keep in mind when using this function that it will try to match any arguments which begin with the prefix, '-' in this
example, that have characters which match those in the boolean grouped flags string. Fortunately, argument vectors which 
have been grabbed by named arguments will be marked as such and skipped by this function. Therefore, this function must
be called after all other non-default arguments are initialized.

#### setDefaultFlagsFromEnv()
This function takes in a string with flags and formatters in it, like the other setters in this library. This function
expects the flags to contain the name of an environment variable, which it will then fetch and `sscanf()` into an 
argument value using the corresponding formatter. The environment variable is only copied into the argument if the 
argument is not set; if setting an argument value to an environment variable unconditionally is not desirable, be sure 
to call this function after any other setter functions.

```
setDefaultFlagsFromEnv("PATH:%s", &string1);
```
-   This will copy the value contained in the `$PATH` variable into the string `string1`.

### Argument Nesting

Argument nesting can be a little tricky, and to use it, create a few boolean arguments using something like `argInit()`.
At least one of these booleans will serve as a root node, meaning the node where a tree of boolean options starts.

For the sake of clarification, nested arguments are meant to give options conditional to other options passed in. For
example:

```
mycommand push now
```

might be configured via a nested argument. The push option could be stored as a root nested node that has now nested
inside of it. In this case, the keyword `now` would only have meaning if `push` is supplied alongside it.

#### nestedBooleanArgumentInit()
This function will initialize a boolean argument as a nested argument root node. This function takes the argument, a 
string, and new flags; the string is the argument the program will search for to match and toggle that boolean argument.

#### nestBooleanArgument()
This function will nest a boolean argument in another nested argument, whether it is a root node or not. As of now, each 
individual nested boolean argument can only directly nest 256 other arguments. Since arguments will always end up nested
inside a root node, you can use curly braces to make the nesting a little easier to read. On a basic level, it looks
like:

```
nestedArgumentInit(&arg1, "arg", NO_FLAGS); { // Initialize arg1.
    nestArgument(&arg1, &arg2, "thing2"); // Nest arg2 in arg1 with the string representing being "thing2".
    nestArgument(&arg1, &arg3, "thing3"); // Nest arg3 in arg1 with the string being "thing3".
}
```

This might be uses as such:

```
program.exe arg thing2 thing3
```

which will set all three booleans, provided that arg and thing2 are passed alongside thing3.

#### nestedArgumentInit() and nestArgument()
These are the non-boolean versions of the nested argument functions. They accept the same parameters as their boolean
counterparts, and they also require a `scanf()` formatter after their nested name, as such:

```
nestedArgumentInit(&thing20, "thing20", NO_FLAGS, "%99s"); { // Initialize thing20 as a root with %99s formatter.
    nestArgument(&thing20, &thing21, "thing21", "%d"); { // Nest thing21 in thing20 with %d formatter
        nestArgument(&thing21, &thing22, "thing22", "%d"); // Nest thing22 in thing21 with %d formatter
    }
}
```

#### setFlagsFromNestedArgs()
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
- So will this.

```
program.exe flag1
```
- This will only toggle `flag1`.

```
program.exe flag2
```
- This will toggle nothing.

### Control Flow Interrupts

#### argumentOverrideCallbacks()
This function accepts the argument count and vector, plus a series of variadic arguments, mainly a formatter and 
function pointers which accept no arguments and return nothing. This function type is referred to as `voidFuncPtr` in
the `args.h` header. It will set flags where, when passed by the user, will run a specific function and terminate the 
program, essentially overriding the control flow of the program. This function should be called before any setter 
functions.

For example:

```
argumentOverrideCallbacks("-h -h2", &help, &help2);
```

 - Means the `-h` flag will run the `help()` function and then terminate the program.
 - Means the `-h2` flag will run the `help2()` function and then terminate the program.

#### argAssert()
This function accepts variadic arguments for assertions plus assertion messages. `argAssert()` keeps track of this via 
an assertion count, which is the first argument this function accepts. Each assertion should test some value
for an argument, like `intArgValue > 0` for example. Assertion messages are what gets printed when the assertion fails.

An assertion might look like:

```
argAssert(1, intArgValue > 0, "Int argument must be positive");
```

You can otherwise specify `NULL` to print out the usage message instead. The `USAGE_MESSAGE` macro expands to NULL and 
can also be used; it exists for readability purposes.

#### libcargValidate()
This function will do a final pass to ensure every argument in the argument vector has been used for something. If it
encounters a redundant or unused argument, it will show the argument in an error message and terminate the program. Of 
course, this function should be called after any argument setters.

## Assertion Macros

### requiredArgument()
This is a function-style macro which accepts an argument struct as an argument. This is an assertion which will fail if 
an argument is not given a value by the user. This should be used in `argAssert()`:

```
argAssert(1, requiredArgument(charArg), "Char argument is required");
```

### mutuallyExclusive()
This is another assertion macro. This one accepts two argument structs, and the assertion will fail if both arguments 
have been provided a value by the user. In other words, this assertion forces the user to pick at most one of two 
arguments.

## Initializer Flags

### NO_FLAGS
This is a macro which expands to `0`. This is a semantic choice to show in `argInit()` that an argument has no custom 
flags.

### POSITIONAL_ARG
This macro should be passed in to the flags section of `argInit()` to specify an argument should be 
given a value without any sort of flag preceding it.

### BOOLEAN_ARG
When specifying an argument should simply be a flag which toggles some variable on or off, two things must be done. The 
argument must be initialized as a boolean, which is what this flag is for. Pass this flag into `argInit()` and later 
reference the generated argument in `setFlagsFromNamedArgs()` with `bool` to create a boolean argument. Boolean arguments
are always flags, and they can therefore never appear as a positional argument.

### HEAP_ALLOCATED
This is a flag for declaring an argument as heap-allocated. In practice, this should never be used. This library 
automatically handles setting this flag when using the `heapArgInit()` function-style macro. Setting this manually may
cause the library to free a pointer which has not been heap-allocated. Doing so may result in segmentation faults/crashes.

### ENFORCE_NESTING_ORDER
This is a flag for declaring the root of a nested argument as not order-agnostic; this flag should be set in
`nestedArgumentInit()`, and the library will do runtime checks in `setFlagsFromNestedArgs()` to ensure arguments nested
within other arguments come after their parent arguments in the argument vector.

### ENFORCE_STRICT_NESTING_ORDER
This flag declares that nested arguments should be passed all in sequence, without any arguments in-between.

### MULTI_ARG
This flag should be used to declare that an argument should be a linked list of values from which to add on to each time
an argument is found. When an argument is toggled with this flag, it is allowed to be repeated in the argument vector.

### NO_DEFAULT_VALUE
This macro expands to `{0}`, and any argument initialized with it in `argInit()` will be zero-initialized. To enforce 
this argument should have no default value, or in other words must be given a value by the user, use `argAssert()` in 
combination with this.

### NO_USAGE_STRING
This macro expands to `""`, or the empty string. Its purpose is in `argInit()` to declare that an argument has no 
usage string associated with it.

### NONE
This macro expands to nothing, and its purpose is for declaring empty type information in `argInit()` or `heapArgInit()`.
For example, declaring a char in `argInit()` is as follows:

```
argInit(char, charArg, NONE, NO_DEFAULT_VALUE, NO_FLAGS, NO_USAGE_STRING);
```

A char variable has no type information on the right side of it, so right-side type information should be omitted. This 
macro does precisely that.

