# Introduction #

This is a tutorial and reference for `tab`, a modern text processing language that's similar to awk in spirit. (But not similar in design philosophy, implementation or syntax.)

Highlights:

* Designed for concise one-liner aggregation and manupulation of tabular text data.
* Makes no compromises on performance; aims to be no slower than traditional old-school UNIX shell utilities whenever possible.
* Feature-rich enough to support even very complex queries. (Also includes a complete set of mathematical operations.)
* Statically typed, type-inferred, declarative.
* Portable: requires only a standards-compliant C++11 compiler and nothing else.

## Compiling and installing ##

Type `make`. Currently the `Makefile` requires a recent `gcc` compiler. (Tested with gcc 4.9)

Copy the resulting binary of `tab` somewhere in your path.

## Usage ##

The default is to read from standard input:

    $ cat myfile | tab <expression>

The result will be written to standard output.

You can also use the `-f` flag to read from a named file:

    $ tab -f myfile <expression>

## Language tutorial ##

### Basic types ###

`tab` is a statically-typed language. However, you will not need to declare any types, the appropriate type information will be deduced automatically, and any errors will be reported before execution.

There are four basic atomic types:

* `Int`, a signed integer. (Equivalent to a `long` in `C`.)
* `UInt`, an unsigned integer. (Equivalent to an `unsigned long` in `C`.)
* `Real`, a floating-point number. (Equivalent to a `double` in `C`.)
* `String`, a string, stored as a byte array.

There are also four structured types:

* `Tuple`, a sequence of several values of (possibly) different types. The number of values and their types cannot change at runtime.
* `Array`, an array of values. Elements can be added and removed at runtime, but the type of all of the values is the same and cannot change.
* `Map`, a hash map (associative array) from values to values. Like with the array, elements can be added and removed, but the type of keys and values cannot change.
* `Sequence`, a.k.a. "lazy list" or "generator". A sequence doesn't store any values, but will generate a new element in the sequence each time is asked to. As with arrays, all generated elements are of the same time.

Types can be composed together in complex ways. Thus, you cannot mix integers and strings in an array, but you can store pairs of strings and integers. (A pair is a tuple of two elements.)

When output, each element of an array, map or sequence is output on its own line, even when nested inside some other structure. The elements of a tuple are printed separated by a tab character, `\\t`.

(So, for example, a printed sequence of arrays of strings looks exactly the same as a sequence of strings.)

### Control structures ###

`tab` has no loops or conditional "if" statements; the input expression is evaluated, and the resulting value is printed on standard output.

Instead of loops you'd use sequences and comprehensions.

The input is fed a file stream, usually the standard input. A file stream in `tab` is represented as a sequence of strings, where each string is a line (separated by `\\n`) in the file.

Built-in functions in `tab` are polymorphic, meaning that a function with the same name will act differently when input arguments of different types.

You can enable a verbose debug mode to output the precise derivations of types in the input expression:
* `-v` will output the resulting type of the whole input expression
* `-vv` will output the resulting type along the the generated virtual machine instruction codes and their types
* `-vvv` will output the parse tree along with the generated code and resulting type.

### Examples ###

    $ ./tab '@'

This command is equivalent to `cat`. `@` is a variable holding the top-level input value, which is the stdin as a sequence of strings. Printing a sequence means printing each element in the sequence; thus, the effect of this whole expression is to read stdin line-by-line and output each line on stdout.

    $ ./tab 'sin(pi()/2)'
    1

    $ ./tab 'cos(1)**2+sin(1)**2'
    1

`tab` can also be used as a desktop calculator. `pi()` is a function that returns the value of *pi*, `cos()` and `sin()` are the familiar trigonometric functions. The usual mathematical infix operators are supported; `**` is the exponentiation oprator.

    $ ./tab 'count(@)'

This command is equivalent to `wc -l`. `count()` is a function that will count the number of elements in a sequence, array or map. Each element in `@` (the stdin) is a line, thus counting elements in `@` means counting lines in stdin.

    $ ./tab '[grep(@,"[a-zA-Z]+")]'

This command is equivalent to `egrep -o "[a-zA-Z]+"`. `grep()` is a function that takes two strings, where the second argument is a regular expression, and outputs an array of strings. The regular expression is searched in the first string argument, and an array of any found matches is returned.

`[...]` is the syntax for *sequence comprehensions* -- transformers that apply an expression to all elements of a sequence; the result of a sequence comprehension is also a sequence.

The general syntax for sequence comprehensions is this: `[ <element> : <input> ]`. Here `<input>` is evaluated (once), converted to a sequence, and each element of that sequence becomes the input to the epxression `<element>`. The result is a sequence of `<element>`. (Or, in other words, a sequence of transformed elements in `<input>`.)

If the `: <input>` part is omitted, then `: @` is automatically implied instead.

Each time `<element>` is evaluated, its argument (an individual element in `<input>`) is also stored in a variable called `@`.

Thus: the expressions `@`, `[@]` and `[@ : @]` are all equivalent; they all return the input sequence of lines from stdin unchanged.

The variables defined in `<element>` (on the left side of `:`) are *scoped*: you can read from variables defined in a higher-level scope, but any variable writes will not be visible outside of the `[ ... ]` brackets.

    $ ./tab 'zip(count(),@)'

This command is equivalent to `nl -ba -w1`; that is, it outputs stdin with a line number prefixed to each line.

`zip()` is a function that accepts one or more sequences and returns a sequence that returns a tuple with an element from each input sequence. (The returned sequence stops when any of the input sequences stop.)

`count()` when called without arguments will return an infinite sequence of successive numbers, starting with `1`.

    $ ./tab 'count(:[ grep(@,"\\S+") ])'

This command is equivalent to `wc -w`: it prints the number of words in stdin. `[ grep(@,"\\S+") ]` is an expression we have seen earlier -- it returns a sequence of array of regex matches.

`:` here is *not* part of a comprehension, it is a special `flatten` operator: given a sequence of sequences, it will return a "flattened" sequence of elements in all the interior sequences.

If given a sequence of arrays, maps or atomic values then this operator will automatically convert the interior values into equivalent sequences.

Thus, the result of `:[ grep(@,"\\S+") ]` is a sequence of strings, ignoring line breaks. Counting elements in this sequence will count the number of matches of `\\S+` in stdin.

**Note:** the unary prefix `:` operator is just straightforward syntactic sugar for the `flatten()` builtin function.

    $ ./tab '{ @ : :[ grep(@,"\\S+") ] }`

This command will output an unsorted list of unique words in stdin.

The `{ @ : ... }` is the syntax for *map comprehensions*. The full form of map comprehensions looks like this: `{ <key> -> <value> : <input> }`. Like with sequence comprehensions, `<input>` will be evaluated, each element will be used to construct `<key>` and `<value>`, and the key-value pairs will be stored in the resulting map.

If `-> <value>` is omitted, then `-> 1` will be automatically implied. If `: <input>` is omitted, then `: @` will be automatically implied.

The result of this command will be a map where each word in stdin is mapped to an integer value of one.

(Note: you can use whitespace creatively to make this command prettier, `{ @ :: [ grep(@,"\\S+") ] }`

You can also wrap the expression in `count(...)` if you just want the number of unique words in stdin.

