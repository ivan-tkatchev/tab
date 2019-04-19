# Introduction #

This is a tutorial and reference for [tab](http://tkatchev.bitbucket.io/tab/), a kind of programming language/shell calculator.

Why another programming language? Because `tab` is a special programming language unlike any other:

* It's statically-typed and type-infered.
* It also infers memory consumption and guarantees O(n) memory use.
* It is designed for concise one-liner computations right in the shell prompt.
* It features both a mathematics library and a set of data slicing and aggregation primitives.
* It is faster than all other interpreted languages with a similar scope. (Perl, Python, awk, ...)
* It is not Turing-complete. (But can compute virtually anything nonetheless.)
* It is self-contained: distributed as a single statically linked binary and nothing else.
* It has no platform dependencies.

You can think of `tab` as a kind of general-purpose query language for text files.

(Also see ['Comparison'](#markdown-header-comparison) below, and a cookbook of [examples](http://tkatchev.bitbucket.io/tab/examples.html).)

Skip to:

* [Tutorial](#markdown-header-language-tutorial)
* Reference
    * [Grammar](#markdown-header-grammar)
    * [Semantics](#markdown-header-semantics)
    * [Builtin functions](#markdown-header-builtin-functions)
    * [Aggregators](#markdown-header-aggregators)
* Advanced features:
    * [Error handling](#markdown-header-error-handling)
    * [Recursion](#markdown-header-recursion)
    * [Multi-core](#markdown-header-multi-core)

## Compiling and installing ##

Type `make`. Requires a modern C++ compiler. Recent versions of gcc (4.9 and up) and clang will work.

Copy the resulting binary of `tab` somewhere in your path.

If you want to use a compiler other than gcc, e.g., clang, then type this:

    :::bash
    $ CXX=clang++ make

## Usage ##

The default is to read from standard input:

    :::bash
    $ cat mydata | tab <expression>...

The result will be written to standard output.

You can also use the `-i` flag to read from a named file:

    :::bash
    $ tab -i mydata <expression>...

If your `<expression>` is too long, you can pass it in via a file, with the `-f` flag:

    :::bash
    $ tab -f mycode <expression>...

(In this case, the contents of `mycode` will be appended to `<expression>`, separated with a comma.)

Run `tab -h` to see the rest of the supported command-line parameters. The binary comes with built-in documentation; use `-h` to read a complete language reference right in your shell prompt.

## Language tutorial ##

### Basic types ###

`tab` is a statically-typed language. However, you will not need to declare any types, the appropriate type information will be deduced automatically, and any errors will be reported before execution.

There are four basic atomic types:

* **Int**, a signed integer. (Equivalent to a `long` in C.)
* **UInt**, an unsigned integer. (Equivalent to an `unsigned long` in C.)
* **Real**, a floating-point number. (Equivalent to a `double` in C.)
* **String**, a string, stored as a byte array.

There are also four structured types:

* **Tuple**, a sequence of several values of (possibly) different types. The number of values and their types cannot change at runtime.
* **Array**, an array of values. Elements can be added and removed at runtime, but the type of all of the values is the same and cannot change.
* **Map**, a hash map (associative array) from values to values. Like with the array, elements can be added and removed, but the type of keys and values cannot change.
* **Sequence**, a.k.a. "lazy list" or "generator". A sequence doesn't store any values, but will generate a new element in the sequence each time is asked to. As with arrays, all generated elements are of the same type.

Structures can be composed together in complex ways. So, for example, you cannot mix integers and strings in an array, but you can store pairs of strings and integers. (A pair is a tuple of two elements.)

When outputing, each element of an array, map or sequence is printed on its own line, even when nested inside some other structure. The elements of a tuple are printed separated by a tab character, `\t`.

(So, for example, a printed sequence of arrays of strings looks exactly the same as a sequence of strings.)

Maps, by default, store values in an unspecified order. Use the `-s` command-line parameter to force a strict ordering on map keys.

### Atomic types ###

The default number type in `tab` is the unsigned integer. A plain sequence of digits will be interpreted as a `UInt`. When you need an explicitly signed `Int`, put an `s`, `i` or `l` suffix onto the digits; for example, `1996l`. All three suffixes are equivalent, they are syntactic sugar.

Floating-point number literals can be entered using a `.` or using scientific notation; for example, `3.` or `3e0`.

String literals are delimited with single or double quotes. Both are equivalent. (Again, syntactic sugar.) A limited set of escape characters are supported within strings: `\t`, `\n`, `\r`, `\e`, `\\`, `\'`, `\"`.

### Control structures ###

`tab` has no loops or conditional "if" statements; the input expression is evaluated, and the resulting value is printed on standard output.

Instead of loops you'd use sequences and comprehensions.

The input is a file stream, usually the standard input. A file stream in `tab` is represented as a sequence of strings, each string being a line from the file. (Lines are assumed be be separated by `\n`.)

Built-in functions in `tab` are polymorphic, meaning that a function with the same name will act differently with input arguments of different types.

You can enable a verbose debug mode to output the precise derivations of types in the input expression:

* `-v` will output the resulting type of the whole input expression
* `-vv` will output the resulting type along the the generated virtual machine instruction codes and their types
* `-vvv` will output the parse tree along with the generated code and resulting type.

### Examples ###

An introduction to `tab` in 10 easy steps.

###### 1.

    :::bash
    $ ./tab '@'

This command is equivalent to `cat`. `@` is a variable holding the top-level input, which is the stdin as a sequence of strings. Printing a sequence means printing each element in the sequence; thus, the effect of this whole expression is to read stdin line-by-line and output each line on stdout.

###### 2.

    :::bash
    $ ./tab 'sin(pi()/2)'
    1
    
    $ ./tab 'cos(1)**2+sin(1)**2'
    1

`tab` can also be used as a desktop calculator. `pi()` is a function that returns the value of *pi*, `cos()` and `sin()` are the familiar trigonometric functions. The usual mathematical infix operators are supported; `**` is the exponentiation oprator.

###### 3.

    :::bash
    $ ./tab 'count(@)'

This command is equivalent to `wc -l`. `count()` is a function that will count the number of elements in a sequence, array or map. Each element in `@` (the stdin) is a line, thus counting elements in `@` means counting lines in stdin.

###### 4.

    :::bash
    $ ./tab '[ grep(@,"[a-zA-Z]+") ]'

This command is equivalent to `egrep -o "[a-zA-Z]+"`. `grep()` is a function that takes two strings, where the second argument is a regular expression, and outputs an array of strings -- the array of any found matches.

`[...]` is the syntax for *sequence comprehensions* -- transformers that apply an expression to all elements of a sequence; the result of a sequence comprehension is also a sequence.

The general syntax for sequence comprehensions is this: `[ <element> : <input> ]`. Here `<input>` is evaluated (once), converted to a sequence, and each element of that sequence becomes the input to the epxression `<element>`. The result is a sequence of `<element>`. (Or, in other words, a sequence of transformed elements from `<input>`.)

If the `: <input>` part is omitted, then `: @` is automatically implied instead.

Each time `<element>` is evaluated, its argument (an individual element in `<input>`) is passed via a variable that is also called `@`.

Thus: the expressions `@`, `[@]` and `[@ : @]` are all equivalent; they all return the input sequence of lines from stdin unchanged.

The variables defined in `<element>` (on the left side of `:`) are *scoped*: you can read from variables defined in a higher-level scope, but any variable writes will not be visible outside of the `[ ... ]` brackets.

###### 5.

    :::bash
    $ ./tab 'zip(count(), @)'

This command is equivalent to `nl -ba -w1`; that is, it outputs stdin with a line number prefixed to each line.

`zip()` is a function that accepts two or more sequences and returns one sequence of tuples of elements from each input sequence. (The returned sequence stops when any of the input sequences stop.)

`count()` when called without arguments will return an infinite sequence of successive numbers, starting with `1`.

###### 6.

    :::bash
    $ ./tab 'count(:[ grep(@,"\\S+") ])'

This command is equivalent to `wc -w`: it prints the number of words in stdin. `[ grep(@,"\\S+") ]` is an expression we have seen earlier -- it returns a sequence of arrays of regex matches.

`:` here is *not* part of a comprehension, it is a special `flatten` operator: given a sequence of sequences, it will return a "flattened" sequence of elements in all the interior sequences.

If given a sequence of arrays, maps or atomic values then this operator will automatically convert the interior structures into equivalent sequences.

Thus, the result of `:[ grep(@,"\\S+") ]` is a sequence of strings, regex matches from stdin, ignoring line breaks. Counting elements in this sequence will count the number of matches of `\S+` in stdin.

**Note:** the unary prefix `:` operator is just straightforward syntactic sugar for the `flatten()` builtin function.

###### 7.

    :::bash
    $ ./tab '{ @ : :[ grep(@,"\\S+") ] }'

This command will output an unsorted list of unique words in stdin.

The `{ @ : ... }` is the syntax for *map comprehensions*. The full form of map comprehensions looks like this: `{ <key> -> <value> : <input> }`. Like with sequence comprehensions, `<input>` will be evaluated, each element will be used to construct `<key>` and `<value>`, and the key-value pairs will be stored in the resulting map.

If `-> <value>` is omitted, then `-> 1` will be automatically implied. If `: <input>` is omitted, then `: @` will be automatically implied.

The result of this command will be a map where each word in stdin is mapped to an integer value of one.

(Note: you can use whitespace creatively to make this command prettier, `{ @ :: [ grep(@,"\\S+") ] }`

You can also wrap the expression in `count(...)` if you just want the number of unique words in stdin.

###### 8.

    :::bash
    $ ./tab '?[ grepif(@,"this"), @ ]'

This command is equivalent to `grep`; it will output all lines from stdin having the string `"this"`.

`grepif()` is a lighter version of `grep()`: given a string and a regular expression it will return an integer: `1` if the regex is found in the string and `0` if it not. (You could use `count(grep(@,"this"))` instead, but `grepif` is obviously shorter and quicker.)

`grepif(@,"this"), @` is a tuple of two elements: the first element is `1` or `0` depending on if the line has `"this"` as a substring, and the second element is the whole line itself.

**Note**: tuples in `tab` are *not* surrounded by parentheses. There is no syntax for creating nested tuples literally. (Though they can exist as a result of a function call, and there is a built-in function called `tuple` for doing just that.)

To write a tuple, simply list its elements separated by commas.

`?` is the *filter* operator: it accepts a sequence of tuples, where the first element of each tuple must be an integer. The output is also a sequence: if a tuple of the input sequence has `0` as the first element, then it is skipped in the output sequence; if the first element of the input tuple is any other value, then it is removed, and the rest of the input tuple is output.

(So, for example: `?[1,@ : x]` is equivalent to the original sequence `x`.)

**Note**: the `?` operator is straightforward syntactic sugar for the `filter()` function.

**Note**: the `?[ grepif(@,b), @ : a ]` expression has a shortcut convenience function, written simply as `grepif(a, b)`. Thus, one could have simply run `./tab 'grepif(@,"this")'` instead.

###### 9.

    :::bash
    $ ./tab '{ @[0] % 2 -> sum(count(@[1])) : zip(count(), @) }'

This command will output the number of bytes on even lines versus the number of bytes on odd lines in stdin.

`{ ... : zip(count(), @) }` is, as before, a map comprehension, with a sequence of pairs (line number, line) as the input.

`@[0] % 2` is the key in the map: we use the indexing operator `[]` to select the first element from the input pair, which is the line number. `%` is the mathematical modulo operator (like in C); line number modulo 2 gives us `0` for even line numbers and `1` for odd line numbers.

`sum(count(@[1]))` is the mapped value in the map. As before, indexing the input pair with `1` gives us the second element, which is the contents of the line from stdin; `count()`, when applied to a string, gives us the length of the string in bytes. 

`sum()` is a little tricker: when applied to a number, it returns the input argument, but marks it with a special tag that causes the map comprehension to add together values marked with `sum()` when groupped together as part of the map's value.

(So, for example, using `sum(1)` on the right side of `->` in a map comprehension will count the number of occurences of whatever is on the left side of `->`.)

###### 10.

    :::bash
    $ ./tab 'z={ tolower(@) -> sum(1) :: [grep(@,"[a-zA-Z]+")] }, sort([ @~1, @~0 : z ])[-5,-1]'

This command will tally a count for each word (first lowercased) in a file, sort by word frequency, and output the top 5 most frequent words.

The `z=` here is an example of *variable assignment*. Here the variable `z` will be assigned a map of unique words with their frequencies. (See example 7; `z` here is the same, except that each word is lowercased and a word count is tallied.)

Variable assignments do not produce a type and do not evaluate to a value; whatever is between the `=` and the `,` (the map comprehension in this case) will not be output.

Moving on: `sort()` is a function that accepts an array, map or sequence and returns its elements in an array, sorted lexicographically. Here we reverse the keys and values in the map `z` by wrapping it in a sequence, so that the resulting array is sorted by word frequency, not by word.

`@~0` is syntactic sugar that is completely equivalent to `@[0]`.

`[-5,-1]` is the *indexing* operator, which accesses elements in a tuple, array or map. The logic and arguments of this operator differ depending on what type is being indexed:

* Tuples can only be indexed with literal integer values. (Not variables or results of a computation.)
* Maps can be indexed by the key, returning the corresponding value; if the key is not in the map, an error will be signalled. (There is a corresponding `get` function that returns a default value instead signalling an error.)
* Arrays indexes are more complex, they can be indexed by:
    * 0-based integers. (0 being the first element in an array.)
    * Negative indexes, where -1 is the last element in the array, -2 is second-to-last, etc.
    * Real-valued indexes; in this case 0.0 is interpreted as the first element in the array and 1.0 as the last. (So 0.5 would be the middle element in the array.)
    * Splices, which are two comma-separated indexes. In this case a sub-array will be returned, beginning with element referenced by the first index and ending with the element referenced by the last. (The last element is also part of the range, unlike in Python and C++.)
* Strings can be spliced as if they were byte arrays; substrings will returned.

In this case a sub-array of five elements is returned -- the last five elements in the array returned by `sort()`

**Note**: the `[...]` indexing operator is straightforward syntactic sugar for the `index()` function.

**Note**: the `~` indexing operator is equivalent to `[...]`. It's syntactic sugar to make chained indexes more palatable: `a~0~1` is equivalent to `a[0][1]`. (The `~` will only work for single-element indexes, not splices.)

###### Bonus track

    :::bash
    $ ./tab -i req.log '
     def stats tuple(avg.@, stdev.@, max.@, min.@, sort.@),
     def uniq { 1 -> stats(@) }[1],
     x=[ uint.cut(@,"|",3) ],
     x=uniq(x),
     avg=x[0], stdev=x[1], max=x[2], min=x[3], q=x[4],
     tabulate(tuple("mean/median", avg, q[0.5]),
              tuple("68-percentile", avg + stdev, q[0.68]),
              tuple("95-percentile", avg + 2*stdev, q[0.95]),
              tuple("99-percentile", avg + 3*stdev, q[0.99]),
              tuple("min and max", real(min), max))'
    mean/median     1764.54 1728
    68-percentile   1933.15 1840
    95-percentile   2101.75 1992
    99-percentile   2270.35 2419
    min and max     0       2508

Here we run a crude test for the normal distribution in the response lengths (in bytes) in a webserver log. (The distrubution of lengths doesn't look to be normally-distributed.)

**Note**: The `f.x` notation is an alternative syntax for calling functions with only one argument; `f.x` is completely equivalent to `f(x)`. (Likewise, `g.f.x` is equivalent to `g(f(x))`.)

**Note**: The `def` keyword is for defining user-defined functions. User-defined functions in `tab` are polymorphic and bound at call time; they act like templates that are inlined when called. The names of user-defined functions have lexical scope, like variables. (However, they are stored in a separate namespace; you cannot assign a function to a variable.)

You can use parentheses to delimit code blocks in function definitions. For example:

    :::bash
    def square_of_square ( def square @*@; square(@)*square(@) );
    square_of_square(4)

**Note**: The semicolon is an equivalent way of writing the comma, because multi-line code looks better with semicolons.

Let's check the distribution visually, with a histogram: (The first column is a size in bytes, the second column is the number of log lines; for example, there were 227 log lines with a response size between 1504.8 and 1755.6 bytes.)

    :::bash
    $ ./tab -i req.log 'hist([. uint.cut(@,"|",3) .], 10)'
    250.8   23
    501.6   0
    752.4   1
    1003.2  0
    1254    0
    1504.8  227
    1755.6  28027
    2006.4  19986
    2257.2  490
    2508    1792

## Comparison ##

A short, hands-on comparison of `tab` with equivalent shell and Python scripts.

The input file is around 100000 lines of web server logs, and we want to find out the number of requests for each URL path.

Here is a solution using standard shell utilities:

    :::bash
    $ cat req.log | cut -d' ' -f3 | cut -d'?' -f1 | sort | uniq -c

Running time: around 2.7 seconds on my particular (slow) laptop.

Here is an equivalent Python script:

    :::python
    import sys
    
    d = {}
    for l in sys.stdin:
        x = l.split(' ')[2].split('?')[0]
        d[x] = d.get(x,0) + 1
    
    for k,v in d.iteritems():
        print k,v

Running time: around 3.1 seconds.

Perl:

    :::perl
    my %counts;
    for my $line (<>) {
        my $path = (split /\?/, (split / /, $line)[2])[0];
        $counts{$path}++
    }
    
    for my $path (keys %counts) {
        my $count = $counts{$path};
        print("$count $path\n");
    }

Running time: around 4.1 seconds.

A resonably simple solution using `awk`:

    ::awk
    $ awk -F" " '{ split($3,x,"?"); paths[x[1]]++; } END { for (path in paths) { print paths[path], path }}'

Running time: around 2.1 seconds.

Here is the solution using `tab`:

    :::bash
    $ ./tab -i req.log '{cut(cut(@," ",2),"?",0) -> sum(1)}'

Running time: around 0.9 seconds.

Not only is `tab` faster in this case, it is also (in my opinion) more concise and idiomatic.

## Reference ##

### Grammar ###

```bash

expr := atomic_or_assignment (("," | ";") atomic_or_assignment)*

atomic_or_assignment := assignment | define | atomic

assignment := var "=" atomic

define := def_fun | def_struct

def_fun := "def" var (atomic | "(" expr ")")

def_struct := "def" "[" var atomic? ("," var atomic?)+ "]"

atomic := e_andor (".." e_andor)*

e_andor := e_eq |
           e_eq "&&" e_eq |
           e_eq "||" e_eq

e_eq := e_bit |
        e_bit "==" e_bit |
        e_bit "!=" e_bit |
        e_bit "<"  e_bit |
        e_bit ">"  e_bit |
        e_bit "<=" e_bit |
        e_bit ">=" e_bit

e_bit := e_add |
         e_add "&" e_add |
         e_add "|" e_add |
         e_add "^" e_add

e_add := e_mul |
         e_mul "+" e_mul |
         e_mul "-" e_mul

e_mul := e_exp |
         e_exp "*" e_exp |
         e_exp "/" e_exp |
         e_exp "%" e_exp

e_exp := e_not |
         e_not "**" e_not

e_not := e_flat |
         "!" e_not

e_flat := e_idx |
          ":" e_flat |
          "?" e_flat

e_idx := e |
         e ("[" expr "]")* |
         e ("~" e)*

e := literal | funcall | var | array | map | seq | paren

literal := real | int | uint | string

funcall := funcall_paren | funcall_dot

funcall_paren := var "(" expr ")"

funcall_dot := var "." e_bit

array := "[." "try"? expr (":" expr)? ".]"

map := "{" "try"? expr ("->" expr)? (":" expr)? "}"

seq := "[" "try"? expr (":" expr)? "]"

paren := "(" atomic ")"

var := "@" | [a-zA-Z][a-zA-Z0-9_]*

digits := [0-9]+

int := "-" digits+ | digits ("i" | "s" | "l")

uint := digits "u"? | ("0x" | "0X") [0-9a-fA-F]+

real := [-+]? digits ("." [0-9]*)? ([eE] [-+]? digits)?

string := '"' chars '"' |
          "'" chars "'"

chars := ("\t" | "\n" | "\r" | "\e" | "\\" | any)*

```

### Semantics ###

##### Expressions

An expression is either an atomic value, an assignment or definition. Assignments and definitions do not produce a value and return nothing.

Expressions separated by `,` or `;` are a tuple. A tuple is itself an expression and a value.

Note: tuples cannot be surrounded by parentheses; if you need to nest tuples, use the builtin function named `tuple`.

This expression produces the tuple `(0, 1)`:

    :::tab
    0, a = 1, def b @; b(a)

##### Variables

Variables are single-assignment: you cannot change the value of an existing variable.

Assigning to a variable with a name that already exists will create a new variable; the old variable will become unreachable.

This is a legal expression that returns `2`:

    :::tab
    a = 1, a = a + 1, a

This is also a legal expression, and will return a sequence of ten numbers `2`:

    :::tab
    a = 1, [ a = a + 1, a : count.10 ]

##### Defining functions

Functions can be defined with the `def` keyword. All function calls are always inlined, and recursive function calls are impossible.

There are three forms for `def`:

* `def f expr`: defines the functon `f`, and `expr` is an atomic value.
* `def f (expr)`: same, but `expr` can be a tuple, including nested definitions and assignments.
* `def [f expr, g expr, ...]`: defines two or more functions, an equivalent shortcut for `def f (@=@[0], expr), def g (@=@[1], expr), ...`. This form is intented to make it easy to give human-readable names to tuple elements. The `expr` is an atomic value and can be omitted -- the simplest form is `def [f,g,...]`.

##### Calling functions

There are two function call syntaxes: `f(a, b, ...)` and `f.a`. Both are equivalent, except that the first form allows calling a function with a tuple argument.

Note, however, that the `.` has low precedence! Thus, this code `f.a & b` is equivalent to `f(a & 1)`!

(See table below.)

##### Operators

In order of precedence, from highest to lowest:

Operator | Meaning
---------|--------
`a~b` `a[b]` | Indexing arrays, maps and tuples. See the `index` function. Use `~` with atomic values, while `[]` can accept tuples.
`:a`  `?a` | Syntactic sugar for the functions `flatten` and `filter`, respectively.
`!a`   | Bitwise NOT.
`a**b` | Exponentiation.
`a*b`  `a/b`  `a%b` | Multiplication, division, modulo.
`a+b`  `a-b` | Addition and subtraction.
`a&b`  `aǀb`  `a^b` | Binary AND, OR and XOR.
`f.a`               | Function call. Operators above this line are assumed to be part of expression `a`.
`a==b` `a!=b` `a<b`  `a>b`  `a<=b`  `a>=b` | Comparision.
`a&&b` `aǀǀb` | Equivalent to `&` and `ǀ` except with a different precedence.
`a .. b`     | Pipe operator. Equivalent to `@=a, b`.

Note that arithmetic operators will silently promote the type of the the result as needed. (Subtracting integers always results in a signed integer, adding a real results in a real, etc.)

Also note that function calls will _not_ promote numeric types as needed! If a function requires a signed integer, then passing in an unsigned is an error.

The `&&` and `||` operators are there because otherwise an expression like `a == b & c == d` is parsed as `a == (b & c) == d` and results in a syntax error.

The "pipe operator" `..` is syntactic sugar meant to make composing code blocks easier. (See the section below about 'magic variables'.)
The following two snippets are equivalent:

    :::tab
    sample(3, :[ seq.@ : head(cut(@,"\t"), 1000)])


    :::tab
    cut(@,"\t") .. head(@, 1000) .. :[ seq.@ ] .. sample(3, @)

(The code snippet selects 3 random values from the first 1000 lines of a tab-separated file.)

##### Literals

Syntax for literal number and string values:

Type | Syntax
-----|-------
`UInt` | `1234` or `1234u` or `0x4D2`. Numbers are unsigned by default. Hexadecimal notation is supported for unsigned numbers.
`Int`  |    `-1234` or `1234i` or `1234s` or `1234l`. Numbers must be explicitly marked as signed; `i`, `s` and `l` are all equivalent syntactic sugar.
`Real` |    `+10.50` or `1.` or `4.4e-10`. Scientific notation and trailing dot are supported.
`String` |   `'chars'` or `"chars"`. Supported escape sequences: `\t` `\n` `\r` `\e` `\\`.

##### Magic variables

The magic variable `@` is used by the language to denote the input value in generator expressions and function definitions.

Note that in all other respects this variable acts like a normal variable.

##### Generator expressions

Type | Syntax
-----|-------
`Seq` | `[ elt : input ]`
`Arr` | `[. elt : input .]`
`Map` | `{ key -> value : input }`

The `: input` part can be omitted, in which case `: @` will be silently assumed. For maps the `-> key` can also be omitted, in which case `-> 1` will be assumed.

The right-hand argument `input` will be converted to a sequence of values automatically. If it is a single value, then a sequence of one element will be assumed.

The keyword `try` can be inserted after the opening bracket; fatal errors while generating elements will then be silently swallowed. (See [error handling](#markdown-header-error-handling).)

See also [recursion](#markdown-header-recursion) for a generator expression for complex single values.

The left- and right-hand sides can include assigment and definition statements. Anything defined or assigned in a generator expression is limited in scope only to this generator expression.

Thus, this code

    :::tab
    [ a=@, @ ], a

Will result in an 'undefined variable' error.

### Builtin functions ###

Listed alphabetically.

`abs`
: Computes absolute value.  
Usage:  
`abs Int -> Int`  
`abs Real -> Real`

`and`
: Returns 1 if all the arguments are not 0, returns 0 otherwise. Equivalent to `a & b & c ...`.  See also `or`.  
Usage:  
`and (Integer, Integer...) -> UInt`

`array`
: Stores a sequence or map or atomic value into an array. See also `sort` for a version of this function with sorting. See also: `iarray`.  
Usage:  
`array Map[a,b] -> Arr[(a,b)]`  
`array Seq[a] -> Arr[a]`  
`array Number|String|Tuple -> Arr[Number|String|Tuple]` -- returns an array with one element.  
**Note:** when arrays are used as values in a map, they will concatenate. (See [aggregators](#markdown-header-aggregators) below for details.)

`avg`
: Synonym for `mean`.

`box`
: Remembers a value. Returns a 'box', which is a tuple of one remembered value. Stores the second argument in the box if the box is empty. If the box is not empty and the first argument is not zero, then replaces the value in the box with the second argument.  
Usage:  
`box UInt, a -> (a,)`

`bucket`
: Return a bucket key. `bucket(x, a, b, n)` will split the interval `[a, b]` into `n` equal sub-intervals and return `x` rounded down to the nearest sub-interval lower bound. Useful for making histograms. See also: `hist`.  
Usage:  
`bucket Number, Number, Number, UInt -> Number` -- the first three arguments must be the same numeric type.

`bytes`
: Accepts a string and returns an array of integers representing the bytes in the string. _Warning_: this function is not Unicode-aware and assumes the string is an ASCII bytestream.  
Usage:  
`bytes String -> Arr[UInt]`

`case`
: A switch/case function. The first argument is compared to every argument at position `n+1`, and if they compare equal, the argument at position `n+2` is returned. If none match equal, then the last argument is returned. See also: `if`.  
Example: `[ case(int.@; 1,'a'; 2,'b'; 'c') : count(4) ]` returns `a b c c`.  
Usage:  
`case a,a,b,...,b -> b`

`cat`
: Concatenates strings.  
Usage:  
`cat String,... -> String`. At least one string argument is required.

`ceil`
: Rounds a floating-point number to the smallest integer that is greater than the input value.  
Usage:  
`ceil Real -> Real`

`cos`
: The cosine function.  
Usage:  
`cos Number -> Real`

`count`
: Counts the number of elements.  
Usage:  
`count None -> Seq[UInt]` -- returns an infinite sequence that counts from 1 to infinity.  
`count UInt -> Seq[UInt]` -- returns a sequence that counts from 1 to the supplied argument.  
`count Number, Number, Number` -- returns a sequence of numbers from `a` to `b` with increment `c`. All three arguments must be the same numeric type.  
`count String -> UInt` -- returns the number of bytes in the string.  
`count Seq[a] -> UInt` -- returns the number of elements in the sequence. (*Warning*: counting the number of elements will consume the sequence!)  
`count Map[a] -> UInt` -- returns the number of keys in the map.  
`count Arr[a] -> UInt` -- returns the number of elements in the array.

`cut`
: Splits a string using a delimiter. See also `recut` for splitting with a regular expression.  
Usage:  
`cut String, String -> Arr[String]` -- returns an array of strings, such that the first argument is split using the second argument as a delimiter.  
`cut String, String, Integer -> String` -- calling `cut(a,b,n)` is equivalent to `cut(a,b)[n]`, except much faster.  
`cut Seq[String], String -> Seq[Arr[String]]` -- equivalent to `[ cut(@,delim) : seq ]`.

`date`
: Converts a UNIX timestamp to a textual representation of a UTC date.  
Usage:  
`date Int -> String` -- returns a UTC date in the `"YYYY-MM-DD"` format.

`datetime`
: Converts a UNIX timestamp to a textual representation of a UTC date and time.  
Usage:  
`datetime Int -> String` -- returns a UTC date and time in the `"YYYY-MM-DD HH:MM:SS"` format.

`e`
: Returns the number *e*.  
Usage:  
`e None -> Real`

`eq`
: Checks values for equality. If the first argument is equal to any of the other arguments, returns 1. Otherwise returns 0.  
Usage:  
`eq a,a,... -> UInt`

`exp`
: The exponentiation function. Calling `exp(a)` is equivalent to `e()**a`.  
Usage:  
`exp Number -> Real`

`explode`
: Makes a sequence of sequences from a plain sequence: given an input sequence, returns that sequence for every element in it. Equivalent to `x=@, [ glue(@, x) ]`.  
Usage:  
`explode Seq[a] -> Seq[Seq[a]]`

`file`
: Opens a file and returns the lines in the file as a sequence of strings. (This allows a `tab` expression to process several files instead of just one.)  
Usage:  
`file String -> Seq[String]`

`filter`
: Filters a sequence by returning an equivalent sequence but with certain elements removed. The input sequence must be a tuple where the first element is an integer; elements where this first elelemt is equal to 0 will be removed from the output sequence. See also: `while`.  
Usage:  
`filter Seq[(Integer,a...) -> Seq[(a...)]`

`find`
: Finds a substring match in a string. The first argument is the string to search in, the second argument is the substring. Returns an array of one element containing the substring if found, and an empty array otherwise. See also: `grep`, `grepif`, `findif` for the rationale.  
Usage:  
`find String, String -> Arr[String]`

`findif`
: Filter strings that contain a substring. See also: `grep`, `grepif`, `find`.  
Usage:  
`findif String, String -> UInt` -- returns 1 if the first argument contains the second argument as a substring, 0 otherwise. Equivalent to `count(find(a,b)) != 0u`, except much faster.  
`findif Seq[String], String -> Seq[String]` -- returns a sequence of only those strings that have a substring match. Equivalent to `?[ findif(@,b), @ : a ]`.

`first`
: Return the first element in a pair, map or sequence or pairs. See also: `second`.  
Usage:  
`first a,b -> a`  
`first Map[a,b] -> Seq[a]`  
`first Seq[(a,b)] -> Seq[a]`

`flatten`
: Flattens a sequence of sequences, a sequence of arrays or a sequence of maps into a sequence of values.  
Usage:  
`flatten Seq[ Seq[a] ] -> Seq[a]`  
`flatten Seq[ Arr[a] ] -> Seq[a]`  
`flatten Seq[ Map[a,b] ] -> Seq[(a,b)]`  
`flatten Seq[a] -> Seq[a]` -- sequences that are already flat will be returned unchanged. (Though at a performance cost.)

`flip`
: Given a sequence of pairs or a map, returns a sequence where the pair elements are swapped.  
Usage:  
`flip Seq[(a,b)] -> Seq[(b,a)]`  
`flip Map[a,b] -> Seq[(b,a)]`

`floor`
: Rounds a floating-point number to the greatest integer that is less than the input value.  
Usage:  
`floor Real -> Real`

`get`
: Accesses map or array elements (like `index`), but returns a default value if the key is not found in the map or if the index is out of bounds. (Unlike `index` which throws an exception.)  
Usage:  
`get Map[a,b], a, b -> b` -- returns the element stored in the map with the given key, or the third argument if the key is not found.  
`get Arr[a], UInt, a -> a` -- returns the element at the given index, or the third argument if the index is out of bounds.

`glue`
: Adds an element to the head of a sequence. `glue(1, seq(2, 3))` is equivalent to `seq(1, 2, 3)`. See also: `take`, `peek`.  
Usage:  
`glue a, Seq[a] -> Seq[a]`

`gmtime`
: Converts a UNIX timestamp to a UTC date and time.  
Usage:  
`gmtime Int -> Int, Int, Int, Int, Int, Int` -- returns year, month, day, hour, minute, second.

`grep`
: Finds regular expression matches in a string. The first argument is the string to match in, the second argument is the regular expression. Matches are returned in an array of strings. Regular expressions use ECMAScript syntax. See also: `grepif`, `find`, `findif`.  
Usage:  
`grep String, String -> Arr[String]`

`grepif`
: Filter strings according to a regular expression. See also: `grep`, `find`, `findif`.  
Usage:  
`grepif String, String -> UInt` -- returns 1 if a regular expression has matches in a string, 0 otherwise. Equivalent to `count(grep(a,b)) != 0u`, except much faster.  
`grepif Seq[String], String -> Seq[String]` -- returns a sequence of only those strings that have regular expression matches. Equivalent to `?[ grepif(@,b), @ : a ]`.

`has`
: Checks for existence in a map or array.  
Usage:  
`has Map[a,b], a -> UInt` -- returns 1 if a key exists in the map, 0 otherwise. The first argument is the map, the second argument is the key to check.  
`has Arr[a], a -> UInt` -- returns 1 if a value is in the array, 0 otherwise. The first argument is the array, the second argument is the value. Equivalent to `has(map.zip(seq.a, count()), b)`.

`hash`
: Hashes a value to an unsigned integer. The FNV hash function (32 or 64 bit depending on CPU architecture) is used.  
Usage:  
`hash a -> UInt`

`head`
: Accepts a sequence or array and returns an equivalent sequence that is truncated to be no longer than N elements. See also: `skip`, `stripe`.  
Usage:  
`head Seq[a], UInt -> Seq[a]`  
`head Arr[a], UInt -> Seq[a]`

`hex`
: Marks the given unsigned integer such that it is output in hexadecimal.  
Usage:  
`hex UInt -> UInt`

`hist`
: Accepts an array of numbers and a bucket count and returns an array of tuples representing a histogram of the values in the array. (The interval between the maximum and minimum value is split into N equal sub-intervals, and a number of values that falls into each sub-interval is tallied.) The return value is an array of pairs: (sub-interval lower bound, number of elements). See also: `bucket`.  
: Usage:  
`hist Arr[Number], UInt -> Arr[(Real,UInt)]`  

`iarray`
: Exactly equivalent to `array`, except when printing the elements will be separated with a `;` instead of a newline.  
Usage:  
`iarray Map[a,b] -> Arr[(a,b)]`  
`iarray Seq[a] -> Arr[a]`  
`iarray Number|String|Tuple -> Arr[Number|String|Tuple]`  
`iarray Arr[a] -> Arr[a]`

`if`
: Choose between alternatives. If the first integer argument is not 0, then the second argument is returned; otherwise, the third argument is returned. The second and third arguments must have the same type.
*Note*: this is not a true conditional control structure, since all three arguments are always evaluated.  
Usage:  
`if Integer, a, a -> a`  

`index`
: Select elements from arrays, maps or tuples. Indexing a non-existent element will cause an error.  
Usage:  
`index Arr[a], UInt -> a` -- returns element from the array, using a 0-based index.  
`index Arr[a], Int -> a` -- negative indexes select elements from the end of the array, such that -1 is the last element, -2 is second-to-last, etc.  
`index Arr[a], Real -> a` -- returns an element such that 0.0 is the first element of the array and 1.0 is the last.  
`index Map[a,b], a -> b` -- returns the element stored in the map with the given key. It is an error if the key is not found; see `get` for a version that returns a default value instead.  
`index (a,b,...), UInt` -- returns an element from a tuple.  
`index Arr[a], Number, Number -> Arr[a]` -- returns a sub-array from an array, _including_ the end element.
`index String, Integer, Integer -> String` -- returns a substring from a string, as with the array slicing above. _Note:_ string indexes refer to _bytes_, `tab` is not Unicode-aware.

`int`
: Converts an unsigned integer, floating-point value or string into a signed integer.  
Usage:  
`int UInt -> Int`  
`int Real -> Int`  
`int String -> Int`  
`int String, Integer -> Int` -- tries to convert the string to an integer; if the conversion fails, returns the second argument instead.

`join`
: Concatenates the elements in a string array or sequence using a delimiter.  
Usage:  
`join Arr[String], String -> String`  
`join Seq[String], String -> String`  
`join String, Arr[String], String, String -> String` -- adds a prefix and suffix as well. Equivalent to `cat(p, join(a, d), s)`.  
`join String, Seq[String], String, String -> String`

`lines`
: Returns its arguments as a tuple, except that each element will be printed on its own line. See also: `tuple`.  
Usage:  
`lines (a,b,...) -> (a,b,...)`

`log`
: The natural logarithm function.  
Usage:  
`log Number -> Real`

`lsh`
: Bit shift left; like the C `<<` operator. (See also `rsh`.)  
Usage:  
`lsh Int, Integer -> Int`  
`lsh UInt, Integer -> UInt`

`map`
: Stores a sequence of pairs or a single pair into a map.  
Usage:  
`map Seq[(a,b)] -> Map[a,b]`  
`map (a,b) -> Map[a,b]` -- returns a map with one element.  
**Note:** when maps are used as values in other maps, they will merge. (See [aggregators](#markdown-header-aggregators) below for details.)

`max`
: Finds the maximum element in a sequence or array. See also: `min`.  
Usage:  
`max Arr[a] -> a`  
`max Seq[a] -> a`  
`max Number -> Number` -- **Note:** this version of this function will mark the return value to calculate the max when stored as a value into an existing key of a map.

`mean`
: Calculates the mean (arithmetic average) of a sequence or array of numbers. See also: `var` and `stdev`.  
Usage:  
`mean Arr[Number] -> Real`  
`mean Seq[Number] -> Real`  
`mean Number -> Real` -- **Note:** this version of this function will mark the returned value to calculate the mean when stored as a value into an existing key of a map.

`merge`
: Aggregates a sequence of values. `merge(a)` is equivalent to `{ 1 -> @ : a }~1`, except faster. See also [aggregators](#markdown-header-aggregators).  
Usage:  
`merge Seq[a] -> a`

`min`
: Finds the minimum element in a sequence or array. See also: `max`.  
Usage:  
`min Arr[a] -> a`  
`min Seq[a] -> a`  
`min Number -> Number` -- **Note:** this version of this function will mark the return value to calculate the min when stored as a value into an existing key of a map.

`ngrams`
: Similar to `pairs` and `triplets`, except returns a sequence of arrays of length N instead of tuples.  
Usage:  
`ngrams Seq[a], UInt -> Seq[Arr[a]]`

`normal`
: Returns random numbers from the normal (gaussian) distribution. (See also: `rand`, `sample`.)  
Usage:  
`normal None -> Real` -- returns a random number with mean `0` and standard deviation `1`.  
`normal Real, Real -> Real` -- same, but with mean and standard deviation of `a` and `b`.

`now`
: Returns the current UNIX timestamp.  
Usage:  
`now None -> Int`

`open`
: Same as `file`.

`or`
: Returns 0 if all the arguments are 0, returns 1 otherwise. Equivalent to `a | b | c ...`.  See also `and`.  
Usage:  
`or (Integer, Integer...) -> UInt`

`pairs`
: Given a sequence, return a sequence of pairs of the previous sequence element and the current sequence element. Example: given `[ 1, 2, 3, 4 ]` will return `[ (1, 2), (2, 3), (3, 4) ]`. (See also: `triplets` and `ngrams`.)  
Usage:  
`pairs Seq[a] -> Seq[(a,a)]`

`peek`
: Given a sequence, return a pair of its first element and the sequence itself with the first element reattached. Equivalent to `h=take.@, h, glue(h, @)`. See also: `take`, `glue`.  
Usage:  
`peek Seq[a] -> (a, Seq[a])`

`pi`
: Return the number *pi*.  
Usage:  
`pi None -> Real`

`rand`
: Returns random numbers from the uniform distribution. (See also: `normal`, `sample`.)  
Usage:  
`rand None -> Real` -- returns a random real number from the range `[0, 1)`.  
`rand Real, Real -> Real` -- same, but with the range `[a, b)`.  
`rand UInt, UInt -> UInt`  
`rand Int, Int -> Int` -- returns a random number from the integer range `[a, b]`.

`real`
: Converts an unsigned integer, signed integer or string into a floating-point value.  
Usage:  
`real UInt -> Real`  
`real Int -> Real`  
`real String -> Real`  
`real String, Real -> Real` -- tries to convert the string to a floating-point value; if the conversion fails, returns the second argument instead.

`recut`
: Splits a string using a regular expression. See also `cut` for splitting with a byte string.  
`recut String, String -> Arr[String]` -- returns an array of strings, such that the first argument is split using the second argument as a regular expression delimiter.  
`recut String, String, UInt -> String` -- calling `recut(a,b,n)` is equivalent to `recut(a,b)[n]`, except faster.  
`recut Seq[String], String -> Seq[Arr[String]]` -- equivalent to `[ recut(@,delim) : seq ]`.

`replace`
: Search-and-replace in a string with regexes. The first argument is the string to search, the second argument is the regex, and the third argument is the replacement string. Regex and replacement string use ECMAScript syntax.  
Usage:  
`replace String, String, String -> String`

`resplit`
: A synonym for `recut`.

`reverse`
: Reverses the elements in an array.  
Usage:  
`reverse Arr[a] -> Arr[a]`

`round`
: Rounds a floating-point number to the nearest integer.  
Usage:  
`round Real -> Real`

`rsh`
: Bit shift right; like the C `>>` operator. (See also `lsh`.)  
Usage:  
`rsh Int, Integer -> Int`  
`rsh UInt, Integer -> UInt`

`sample`
: Sample from a sequence of atomic values, without replacement. (See also: `rand`, `normal`.)  
Usage:  
`sample UInt, Seq[a] -> Arr[a]` -- the first argument is the sample size.

`second`
: Return the second element in a pair, map or sequence or pairs. See also: `first`.  
Usage:  
`second a,b -> b`  
`second Map[a,b] -> Seq[b]`  
`second Seq[(a,b)] -> Seq[b]`

`seq`
: Accepts two or more values of the same type and returns a sequence of those values. (A synonym for `tabulate`.)  
If one argument is passed, then it is equivalent to `[@ : arg]`.  
Usage:  
`seq (a,...),... -> Seq[a]`  
`seq Arr[a] -> Seq[a]`  
`seq Map[a,b] -> Seq[(a,b)]`  
`seq a -> Seq[a]`

`sin`
: The sine function.  
Usage:  
`sin Number -> Real`

`skip`
: Accepts a sequence or array and returns an equivalent sequence where the first N elements are ignored. See also: `head`, `stripe`.  
Usage:  
`skip Seq[a], UInt -> Seq[a]`  
`skip Arr[a], UInt -> Seq[a]`

`sort`
: Sorts a sequence, array or map lexicographically. The result is stored into an array if the input is a map or a sequence. See also `array` a version of this function without sorting.  
Usage:  
`sort Arr[a] -> Arr[a]`  
`sort Map[a,b] -> Arr[(a,b)]`  
`sort Seq[a] -> Arr[a]`  
`sort Number|String|Tuple -> Arr[Number|String|Tuple]` -- **Note:** this version of this function will return an array with one element, marked so that storing it as a value in an existing key of a map will produce a sorted array of all such values. 

`split`
: A synonym for `cut`.

`sqrt`
: The square root function.  
Usage:  
`sqrt Number -> Real`

`stddev`
: Synonym for `stdev`.

`stdev`
: Calculates the sample standard deviation, defined as the square root of the variance. This function is completely analogous to `var`, with the difference that the square root of the result is taken. See also: `mean`.  
Usage:  
`stdev Arr[Number] -> Real`  
`stdev Seq[Number] -> Real`  
`stdev Number -> Real` -- **Note:** this version of this function will mark the returned value to calculate the standard deviation when stored as a value into an existing key of a map.

`string`
: Converts an unsigned integer, signed integer, floating-point number or a byte array to a string.  
Usage:  
`string UInt -> String`  
`string Int -> String`  
`string Real -> String`  
`string Arr[UInt] -> String` -- **Note:** here it is assumed that the array will hold byte (0-255) values. Passing in something else is an error. This function is not Unicode-aware.

`stripe`
: Accepts a sequence or array and returns an equivalent sequence except with only every Nth element. See also: `head`, `skip`.  
Usage:  
`stripe Seq[a], UInt -> Seq[a]`  
`stripe Arr[a], UInt -> Seq[a]`

`sum`
: Computes a sum of the elements of a sequence or array.  
Usage:  
`sum Arr[Number] -> Number`  
`sum Seq[Number] -> Number`  
`sum Number -> Number` -- **Note:** this version of this function will mark the value to be aggregated as a sum when stored as a value into an existing key of a map.

`take`
: Returns the first element in a sequence. Equivalent to `array(head(@, 1))[0]`. See also: `peek`, `glue`.  
Usage:  
`take Seq[a] -> a` -- gives an error on empty sequence.  
`take Seq[a], a -> a` -- returns the second argument on empty sequence.

`tan`
: The tangent function.  
Usage:  
`tan Number -> Real`

`tabulate`
: A synonym for `seq`.

`time`
: Converts a UNIX timestamp to a textual representation of a UTC time.  
Usage:  
`time Int -> String` -- returns a UTC time in the `"HH:MM:SS"` format.

`tolower`
: Converts to bytes of a string to lowercase. *Note:* only works on ASCII data, Unicode is not supported.  
Usage:  
`tolower String -> String`

`toupper`
: Converts to bytes of a string to uppercase. *Note:* only works on ASCII data, Unicode is not supported.  
Usage:  
`toupper String -> String`

`triplets`
: Similar to `pairs`, except returns triplets of before-previous, previous and current elements. (See also: `pairs` and `ngrams`.)  
Usage:  
`triplets Seq[a] -> Seq[(a,a,a)]`

`tuple`
: Returns its arguments as a tuple. Meant for grouping when defining tuples within tuples. See also: `lines`.  
Usage:  
`tuple (a,b,...) -> (a,b,...)`

`uint`
: Converts a signed integer, floating-point number or string to an unsigned integer.  
Usage:  
`uint Int -> UInt`  
`uint Real -> UInt`  
`uint String -> UInt`  
`uint String, Integer -> UInt` -- tries to convert the string to an unsigned integer; if the conversion fails, returns the second argument instead.

`uniques`
: Returns an aggregator for counting the number of unique values. Hashes of all values are stored, so the result is exact as long as there are no hash collisions. Memory usage is proportional to the count of unique items. See also `uniques_estimate`.  
Usage:  
`uniques a -> UInt`

`uniques_estimate`
: Returns an aggregator for estimating the number of unique values. A [statistical estimator](http://en.wikipedia.org/wiki/HyperLogLog) is used instead of exact counts; memory usage is constant. Note: the estimator works better with larger counts of unique values. See also `uniques`.  
Usage:  
`uniques_estimate a -> UInt`

`url_getparam`
: Splits a string with URL query-string parameters into keys and values. Values will be automatically percent-decoded.  
Usage:  
`url_getparam String, String -> String` -- calling `url_getparam(url, key)` will return the first value in `url` for `key`. Example: `url_getparam("http://www.google.com?q=Hello%20World", "q")` will return `"Hello World"`.  
`url_getparam String -> Seq[(String,String)]` -- returns a sequence of all key/value pairs in the url. Example: `url_getparam."&one=1&two=2"` will return a value equivalent to `seq(tuple("one","1"), tuple("two","2"))`.

`var`
: Calculates the sample variance of a sequence of numbers. (Defined as the mean of squares minus the square of the mean.) See also: `mean` and `stdev`.  
Usage:  
`var Arr[Number] -> Real`  
`var Seq[Number] -> Real`  
`var Number -> Real` -- **Note:** this version of this function will mark the returned value to calculate the variance when stored as a value into an existing key of a map.

`variance`
: Synonym for `var`.

`while`
: Similar to `filter`, but stops the output sequence once the first filtered element is reached. See: `filter`.  
Usage:  
`while Seq[(Integer,a...)] -> Seq[(a...)]`

`zip`
: Accepts two or more sequences (or arrays) and returns a sequence that returns a tuple of elements from each of the input sequences. The output sequence ends when any of the input sequences end.  
Usage:  
`zip Seq[a], Seq[b],... -> Seq[(a,b,...)]`  
`zip Arr[a], Arr[b],... -> Seq[(a,b,...)]`

### Aggregators ###

Aggregators are functions like any other; they accept a value and return a value, though usually the result is not useful as such. What's important is that aggregators have a side effect: the returned value is (invisibly) marked such that it will combine in special ways when it ends up keyed in a map that already stores another element at this key.

Aggregation is performed efficiently: no unnecessary temporary data structures are created and no unnecessary bookkeeping calculations are performed.

Here is a list of aggregators and their effects, sorted alphabetically:

`array`, `[. .]`
: Arrays are implicit aggregators. When combined together under one key of a map, arrays will concatenate, with the resulting elements appearing according to insertion order. (Last inserted elements coming last in the array.) See also: `sort`.

`avg`
: Accepts a numeric value, returns a floating-point number. When combined together, the arithmetic mean of the numbers will be computed.

`iarray`
: Like `array` except all elements are printed on one line.

`map`, `{ }`
: Maps are implicit aggregators. When a value of a map is another map, those maps will merge when aggregated under one key. (See below for an example.)

`max`
: Accepts a numeric value, returns a value of the same type. When combined together, the maximum value is computed.

`mean`
: Synonymous with `avg`.

`min`
: Accepts a numeric value, returns a value of the same type. When combined together, the minimum value is computed.

`sort`
: Like `array`, except that the resulting elements will be sorted in ascending order.

`stddev`
: Synonymous with `stddev`.

`stdev`
: Accepts a numeric value, returns a floating-point number. When combined together, the sample standard deviation is computed, defined as the square root of the variance. See also: `var`.

`sum`
: Accepts a numeric value, returns a value of the same type. When combined together, the sum of the values is computed.

`uniques`
: Accepts any value and returns a `UInt`-valued aggregator that counts the number of unique values when combined. *Note:* hashes of values are stored, so the result is exact as long as there are no hash collisions. Memory usage is proportional to the count of unique values.

`uniques_estimate`
: Like `uniques`, except that a [statistical estimator](http://en.wikipedia.org/wiki/HyperLogLog) is used instead. The result is not exact but the estimator uses constant memory. *Note:* the estimator works better with larger counts of unique values.

`var`
: Accepts a numeric value, returns a floating-point number. When combined together, the sample variance is computed, defined as the mean of squares minus the square of the mean.

`variance`
: Synonymous with `var`.

An explanation of how arrays and maps are aggregated implicitly:

    { @~0 -> map(@~1, sum.1) : pairs(@) }

This program will produce the intuitively obvious result -- a map of maps where the leaf values are frequency counts.
This works as expected because maps-inside-maps will automatically aggregate.

Similarly for arrays:

    { month(@) -> array(day_values(@)) : data }

Arrays under a map key will concatenate, and such a program will produce the expected result -- an array of all day values for each month.

### Error handling ###

Sequence, map and array comprehensions allow a special syntax for handling exceptions thrown while evaluating generator expressions.

Simply put the special token `try` after the `[`, `{` or `[.` opening parenthesis to silently ignore errors instead of aborting evaluation.

For example:

    [ try uint.@ ]

will ignore any lines on the standard input that can't be parsed as a number.

    first.{ try cut(@, " ", 1) }

will output the second word from each line, and ignore all lines that don't contain a space character.

### Recursion ###

`tab` supports a limited kind of tail recursion for special cases when a simple step-by-step application of operations will not work.

Consider the example of computing the factorial: given a sequence of integers, compute its product.

In `tab` the factorial function looks like this:

    def fac << @~0 * @~1 : 1, count.@ >>

The `<< ... : ... >>` takes an expression on the left-hand side and a pair of value and sequence on the right-hand side.

An expression that looks like `<< f(@~0, @~1) : a, seq(b, c, d) >>` will be unrolled to be equivalent to this:

    f(f(f(a, b), c), d)
    
The left-hand side will be evaluated repeatedly, with an argument that is a pair of values. The first element of the pair is the previous evaluation result, and the second element is the next element in the input sequence. The right-hand side is also a pair, with the first element a starting value and the second element the input sequence.

For example: calling `fac.3` from the above example results in evaluating `(((1 * 1) * 1) * 2) * 3`.

Note that the type of the result and the type of the sequence elements can be different. This will calculate the 11th Fibonacci number:

    << a=@~0~0, b=@~0~1, tuple(b, a + b) : tuple(0, 1), count.10 >>~1

### Multi-core ###

`tab` can take advantage of multi-core systems by evaluating expressions using multiple threads.

Use the `-t` command-line option to enable multithreaded evaluation.

Parallel evaluation is not quite automatic: `tab` uses a simple scatter/gather evaluation model. N parallel threads will evaluate a 'scatter' expression, generating N independent sequences. A separate 'gather' thread will then read sequentially from all N sequences and aggregate them into a single result stream.

The syntax for parallel evaluation looks like this:

    :::bash
    $ tab -tN scatter --> gather

The `-->` is a special token that separates 'scatter' and 'gather' expressions. 

Examples:

###### 1.

    :[ grep(@, '[0-9]{4}') ]

A simple expression that will search for all four-digit numbers. 

**Note:** if there is no `-->` token in the epxression, then a default `--> @` will be automatically appended.

In this case no result aggregation is done, all parallel threads will simply print what they found to standard output.

###### 2.

    count.flatten.[ grep(@, '[0-9]{4}') ] --> sum.@

Same as the previous example, except that we want to count the numbers we found, instead of outputting them.
The aggregating 'gather' expression will compute the sum of the counts found by all of the 'scatter' counting threads.

**Note:** the 'scatter' threads will read from the input stream atomically; there is no danger of an input line being read twice.

(A reminder that the `:` operator is equivalent to the `flatten()` function.)

###### 3.

    { @ ::[ grep(@, '[0-9]{4}') ] } --> count.map.@

Here we count the unique numbers found. The 'scatter' threads will aggregate a subset of the input into a map with a four-digit number as the key.
The 'gather' thread will aggregate each of the 'scattered' maps into one final map, and output the count of its keys.

**Note:** the output of each 'scatter' thread will be a _sequence_. When a map or array is the result, it will be automatically turned into a sequence by an automatic application of `seq()`. (Same as with the right-hand side expression in a `[ ... : ... ]` or `{ ... : ... }` generator.)

The input type of the 'gather' thread is `Seq[(String, Int)]`.
