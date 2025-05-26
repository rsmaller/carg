# cargs - C Argument Parsing Library

## How to Use This Library
Before using this library, ensure your `main()` function accepts an argc and argv as parameters.
This library parses those parameters using string formatters and sets variables to their values accordingly.

It's likely the first thing you will want to do is declare a usage message for your program.
The `setUsageMessage()` function allows you to do this; it sets a usage message that caps out at 1023 characters.
It accepts a string with formatters in it. This might look something like:

```
setUsageMessage("USAGE: %s -n Arg1 -t Arg2", basename(argv[0]));
```

The `usage()` function will print out this message and terminate the program.

As for arguments, they are stored in structs and variables of their respective type.
Structs serve to keep track of whether an argument has been specified by the user or not.
To simplify the declaration of arguments, `argInit()` and basicArgInit() were created.
Simply call `argInit()` or `basicArgInit()` with both a variable name to declare, type information, a default value, and a bitmask representing toggled flags.
To write explicitly that a variable should not have a default value, use the `NO_DEFAULT_VALUE` macro in place of a default value.

`argInit()` and `basicArgInit()` will create a struct from the variable name entered into them.
Keep in mind the resulting struct does not contain type information about the variable you declare a container for the argument.
To access the data the user entered, add "Value" to the end of the struct's name.
For example, when declaring an int argument via:

```
basicArgInit(int, intArg, 1, NO_FLAGS)
```

- `intArg` is a struct.
- `intArg` contains a void pointer to `intArgValue`, an int representing whether the argument has been set by the user, and a flags bitmask.
- `intArgValue` is an int where the value of the argument is stored. 
- If the arg was declared as a char instead, the type of `intArgValue` would be char.

The value of an argument should only be accessed after setting the variables' values in accordance with user input.
Keep in mind variables must be initialized via `argInit()` or `basicArgInit()` before being set.
To set argument values from the user, use the `setFlagsFromNamedArgs()` function. This function accepts the argc and argv parameters and a string formatter for arguments.
`setFlagsFromNamedArgs()` accepts both flag parameters and string formatters associated with them.
That formatter might look like: `"-v:%s"` followed by a string argument struct.
This would mean that the flag -v should be followed by a string:

```
example.exe -v <INSERT_STRING_HERE>
```

Below is an example of the initializer and setter in conjunction.
Keeping everything in mind, the string variable would need to be initialized first:

```
basicArgInit(char *, myString, "default", NO_FLAGS);
```

All arguments should be initialized before setting them, so let's add an int argument also:

```
basicArgInit(int, myInt, 0, NO_FLAGS);
```

Then, these values can be set using the `setFlagsFromNamedArgs()` function:

```
setFlagsFromNamedArgs(argc, argv, "-v:%s -i:%d", myString, myInt);
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

If this is all the functionality you need, you're done!

However, you may want to make an argument required or limit which values the user can set it to, especially one that is initialized with NO_DEFAULT_VALUE.

`argAssert()` is designed for this; it accepts the number of argument assertions as an argument. All assertions after that are two arguments each.

The first argument should be the condition that must be met. This can be any expression which can be evaluated as a zero versus nonzero value.
If this condition is that the argument is required, use `REQUIRED_ARGUMENT()` with the corresponding argument struct.

If two arguments should not be set at the same time, use `MUTUALLY_EXCLUSIVE()` with the corresponding structs.
The second argument should be the message to display if the condition is not met. Set this to `USAGE_MESSAGE` to use the usage message instead.

For example:

```
argAssert(3, 
        myIntValue > -1, "Int 1 must not be negative",
        REQUIRED_ARGUMENT(myInt), USAGE_MESSAGE,
        REQUIRED_ARGUMENT(myString), USAGE_MESSAGE
);
```

will print `"Int 1 must not be negative"`if a value less than or equal to -1 is given.
The usage message will show if arguments `myInt` or `myString` are not given values by the user.

To specify arguments that make the program do something entirely different, primarily running a single function and then terminating, call the `argumentOverrideCallbacks()` function. This function accepts the argument count, argument vector, flags, and function pointers associated with them.

For example, to declare arguments for a help-displaying function and another random helper function:

```
argumentOverrideCallbacks(argc, argv, "-h -r", &help, &randomHelperFunction);
```

Another feature this library supports is nameless arguments. Nameless arguments are passed in to the program without a flag.
These arguments should always come before named arguments to prevent argument ambiguity.

To use nameless arguments alongside named arguments, initialize both first:

```
basicArgInit(int, namelessArg, 0, NAMELESS_ARG);
basicArgInit(char, namedArg, 'a', NO_FLAGS);
```

Then, set the values for nameless arguments first:

```
setFlagsFromNamelessArgs(argc, argv, "%d", &namelessArg);
```

Lastly, set the values for named arguments:

```
setFlagsFromNamedArgs(argc, argv, "-n:%d", &namedArg);
```

Keep in mind that nameless arguments are required regardless if they are enforced with an assertion or not.
Furthermore, they are assigned to argument variables based on their order. Make sure they line up correctly when you set their values!
Nameless arguments can be used in assertions the same way as named arguments.
