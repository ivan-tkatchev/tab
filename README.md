# Introduction #

This is a tutorial and reference for `tab`, a modern text processing language that's similar to awk in spirit. (But not similar in design philosophy, implementation or syntax.)

Highlights:

* Designed for concise one-liner aggregation and manupulation of tabular text data.
* Makes no compromises on performance; aims to be no slower than traditional old-school UNIX shell utilities whenever possible.
* Feature-rich enough to support even very complex queries. (Also includes a complete set of mathematical operations.)
* Statically typed, type-inferred, declarative.
* Portable: requires only a standards-compliant C++11 compiler and nothing else.

(Also see ['Comparison'](#markdown-header-comparison) below.)

Skip to:

* [Tutorial](#markdown-header-language-tutorial)
* [Grammar reference](#markdown-header-grammar)
* [Builtin functions reference](#markdown-header-builtin-functions)

## Compiling and installing ##

Type `make`. Currently the `Makefile` requires a recent gcc compiler. (Tested with gcc 4.9)

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

* **Int**, a signed integer. (Equivalent to a `long` in C.)
* **UInt**, an unsigned integer. (Equivalent to an `unsigned long` in C.)
* **Real**, a floating-point number. (Equivalent to a `double` in C.)
* **String**, a string, stored as a byte array.

There are also four structured types:

* **Tuple**, a sequence of several values of (possibly) different types. The number of values and their types cannot change at runtime.
* **Array**, an array of values. Elements can be added and removed at runtime, but the type of all of the values is the same and cannot change.
* **Map**, a hash map (associative array) from values to values. Like with the array, elements can be added and removed, but the type of keys and values cannot change.
* **Sequence**, a.k.a. "lazy list" or "generator". A sequence doesn't store any values, but will generate a new element in the sequence each time is asked to. As with arrays, all generated elements are of the same time.

Structures can be composed together in complex ways. So, for example, you cannot mix integers and strings in an array, but you can store pairs of strings and integers. (A pair is a tuple of two elements.)

When output, each element of an array, map or sequence is output on its own line, even when nested inside some other structure. The elements of a tuple are printed separated by a tab character, `\t`.

(So, for example, a printed sequence of arrays of strings looks exactly the same as a sequence of strings.)

### Control structures ###

`tab` has no loops or conditional "if" statements; the input expression is evaluated, and the resulting value is printed on standard output.

Instead of loops you'd use sequences and comprehensions.

The input is fed a file stream, usually the standard input. A file stream in `tab` is represented as a sequence of strings, where each string is a line (separated by `\n`) in the file.

Built-in functions in `tab` are polymorphic, meaning that a function with the same name will act differently when input arguments of different types.

You can enable a verbose debug mode to output the precise derivations of types in the input expression:

* `-v` will output the resulting type of the whole input expression
* `-vv` will output the resulting type along the the generated virtual machine instruction codes and their types
* `-vvv` will output the parse tree along with the generated code and resulting type.

### Examples ###

An introduction to `tab` in 10 easy steps:

###### 1.

    $ ./tab '@'

This command is equivalent to `cat`. `@` is a variable holding the top-level input, which is the stdin as a sequence of strings. Printing a sequence means printing each element in the sequence; thus, the effect of this whole expression is to read stdin line-by-line and output each line on stdout.

###### 2.

    $ ./tab 'sin(pi()/2)'
    1

    $ ./tab 'cos(1)**2+sin(1)**2'
    1

`tab` can also be used as a desktop calculator. `pi()` is a function that returns the value of *pi*, `cos()` and `sin()` are the familiar trigonometric functions. The usual mathematical infix operators are supported; `**` is the exponentiation oprator.

###### 3.

    $ ./tab 'count(@)'

This command is equivalent to `wc -l`. `count()` is a function that will count the number of elements in a sequence, array or map. Each element in `@` (the stdin) is a line, thus counting elements in `@` means counting lines in stdin.

###### 4.

    $ ./tab '[ grep(@,"[a-zA-Z]+") ]'

This command is equivalent to `egrep -o "[a-zA-Z]+"`. `grep()` is a function that takes two strings, where the second argument is a regular expression, and outputs an array of strings -- the array of any found matches.

`[...]` is the syntax for *sequence comprehensions* -- transformers that apply an expression to all elements of a sequence; the result of a sequence comprehension is also a sequence.

The general syntax for sequence comprehensions is this: `[ <element> : <input> ]`. Here `<input>` is evaluated (once), converted to a sequence, and each element of that sequence becomes the input to the epxression `<element>`. The result is a sequence of `<element>`. (Or, in other words, a sequence of transformed elements from `<input>`.)

If the `: <input>` part is omitted, then `: @` is automatically implied instead.

Each time `<element>` is evaluated, its argument (an individual element in `<input>`) is passed via a variable that is also called `@`.

Thus: the expressions `@`, `[@]` and `[@ : @]` are all equivalent; they all return the input sequence of lines from stdin unchanged.

The variables defined in `<element>` (on the left side of `:`) are *scoped*: you can read from variables defined in a higher-level scope, but any variable writes will not be visible outside of the `[ ... ]` brackets.

###### 5.

    $ ./tab 'zip(count(), @)'

This command is equivalent to `nl -ba -w1`; that is, it outputs stdin with a line number prefixed to each line.

`zip()` is a function that accepts one or more sequences and returns a sequence that returns a tuple with an element from each input sequence. (The returned sequence stops when any of the input sequences stop.)

`count()` when called without arguments will return an infinite sequence of successive numbers, starting with `1`.

###### 6.

    $ ./tab 'count(:[ grep(@,"\\S+") ])'

This command is equivalent to `wc -w`: it prints the number of words in stdin. `[ grep(@,"\\S+") ]` is an expression we have seen earlier -- it returns a sequence of array of regex matches.

`:` here is *not* part of a comprehension, it is a special `flatten` operator: given a sequence of sequences, it will return a "flattened" sequence of elements in all the interior sequences.

If given a sequence of arrays, maps or atomic values then this operator will automatically convert the interior structures into equivalent sequences.

Thus, the result of `:[ grep(@,"\\S+") ]` is a sequence of strings, regex matches from stdin, ignoring line breaks. Counting elements in this sequence will count the number of matches of `\S+` in stdin.

**Note:** the unary prefix `:` operator is just straightforward syntactic sugar for the `flatten()` builtin function.

###### 7.

    $ ./tab '{ @ : :[ grep(@,"\\S+") ] }'

This command will output an unsorted list of unique words in stdin.

The `{ @ : ... }` is the syntax for *map comprehensions*. The full form of map comprehensions looks like this: `{ <key> -> <value> : <input> }`. Like with sequence comprehensions, `<input>` will be evaluated, each element will be used to construct `<key>` and `<value>`, and the key-value pairs will be stored in the resulting map.

If `-> <value>` is omitted, then `-> 1` will be automatically implied. If `: <input>` is omitted, then `: @` will be automatically implied.

The result of this command will be a map where each word in stdin is mapped to an integer value of one.

(Note: you can use whitespace creatively to make this command prettier, `{ @ :: [ grep(@,"\\S+") ] }`

You can also wrap the expression in `count(...)` if you just want the number of unique words in stdin.

###### 8.

    $ ?[ grepif(@,"this"), @ ]

This command is equivalent to `grep`; it will output all lines from stdin having the string `"this"`.

`grepif()` is a lighter version of `grep()`: given a string and a regular expression it will return an integer: `1` if the regex is found in the string and `0` if it not. (You could use `count(grep(@,"this"))` instead, but `grepif` is obviously shorter and quicker.)

`grepif(@,"this"),@` is a tuple of two elements: `1` or `0` if the line has `"this"` as a substring as the first element, and the whole line itself as the second element.

**Note**: tuples in `tab` are *not* surrounded by brackets. It is also impossible to create nested tuples literally. (Though they can exist as a result of a function call, and there is a built-in function called `tuple` for doing just that.)

To write a tuple, simply list its elements separated by commas.

`?` is the *filter* operator: it accepts a sequence of tuples, where the first element of each tuple must be an integer. The output is also a sequence: if a tuple of the input sequence has `0` as the first element, then it is skipped in the output sequence; if the first element of the input tuple is any other value, then it is removed, and the rest of the input tuple is output.

(So, for example: `?[1,@ : x]` is equivalent to the original sequence `x`.)

**Note**: the `?` operator is straightforward syntactic sugar for the `filter()` function.

###### 9.

    $ ./tab '{ @[0] % 2 -> sum(count(@[1])) : zip(count(), @) }'

This command will output the number of bytes on even lines versus the number of bytes on odd lines in stdin.

`{ ... : zip(count(), @) }` is, as before, a map comprehension, with a sequence of pairs (line number, line) as the input.

`@[0] % 2` is the key in the map: we use the indexing operator `[]` to select the first element from the input pair, which is the line number. `%` is the mathematical modulo operator (like in C); line number modulo 2 gives us `0` for even line numbers and `1` for odd line numbers.

`sum(count(@[1]))` is the mapped value in the map. As before, indexing the input pair with `1` gives us the second element, which is the contents of the line from stdin; `count()`, when applied to a string, gives us the length of the string in bytes. 

`sum()` is a little tricker: when applied to a number, it returns the input argument, but marks it with a special tag that causes the map comprehension to add together values marked with `sum()` when groupped together as part of the map's value.

(So, for example, using `sum(1)` on the right side of `->` in a map comprehension will count the number of occurences of whatever is on the left side of `->`.)

###### 10.

    $ ./tab 'z={ tolower(@) -> sum(1) :: [grep(@,"[a-zA-Z]+")] }, sort([ @[1], @[0] : z ])[-5,-1]'

This command will tally a count for each word (first lowercased) in a file, sort by word frequency, and output the top 5 most frequent words.

The `z=` here is an example of *variable assignment*. Here the variable `z` will be assigned a map of unique words with their frequencies. (See example 7; `z` here is the same, except that each word is lowercased and a word count is tallied.)

Variable assignments do not produce a type and do not evaluate to a value; whatever is between the `=` and the `,` (the map comprehension in this case) will not be output.

Moving on: `sort()` is a function that accepts an array, map or sequence and returns its elements in an array, sorted lexicographically. Here we reverse the keys and values in the map `z` by wrapping it in a sequence, so that the resulting array is sorted by word frequency, not by word.

`[-5,-1]` is the *indexing* operator, which accesses elements in a tuple, array or map. The logic and arguments of this operator differ depending on what type is being indexed:

* Tuples can only be indexed with literal iteger values. (Not variables or results of a computation.)
* Maps can be indexed by the key, returning the corresponding value; if the key is not in the map, an error will be signalled.
* Arrays indexes are more complex, they can be indexed by:
    * 0-based integers. (0 being the first element in an array.)
    * Negative indexes, where -1 is the last element in the array, -2 is second-to-last, etc.
    * Real-valued indexes; in this case 0.0 is interpreted as the first element in the array and 1.0 as the last. (So 0.5 would be the middle element in the array.)
    * Splices, which are two comma-separated indexes. In this case a sub-array will be returned, beginning with element referenced by the first index and ending with the element referenced by the last.

In this case a splice of five elements is returned -- the last five elements in the array returned by `sort()`

###### Bonus track

    $ ./tab -f req.log '
    > x=[ uint(cut(@,"|",7)) ],
    > x={ 1 -> avg(@), stdev(@), sort(@) : x}[1],
    > avg=x[0], stdev=x[1],
    > tabulate(tuple("mean/median", avg, x[2][0.5]),
    >   tuple("68-percentile", avg + stdev, x[2][0.68]),
    >   tuple("95-percentile", avg + 2*stdev, x[2][0.95]),
    >   tuple("99-percentile", avg + 3*stdev, x[2][0.99]))'
    mean/median     1764.54 1728
    68-percentile   1933.15 1840
    95-percentile   2101.75 1992
    99-percentile   2270.35 2419

    $ ./tab -f weird.csv '{ @[0] -> avg(@[1]), avg(@[2]), sum(1) : [ z=cut(replace(@,",","."),";"), real(z[2]) < 65.0, uint(z[1]), real(z[3]) : array(@)[-16,-3] ] }'



## Comparison ##

A short, hands-on comparison of `tab` with equivalent shell and Python scripts.

The input file is around 100000 lines of web server logs, and we want to find out the number of requests for each URL path.

Here is a solution using standard shell utilities:

    $ cat req.log | cut -d' ' -f3 | cut -d'?' -f1 | sort | uniq -c

Running time: around 2.7 seconds on my particular (slow) laptop.

Here is an equivalent Python script:

    import sys
    
    d = {}
    for l in sys.stdin:
        x = l.split(' ')[2].split('?')[0]
        d[x] = d.get(x,0) + 1
    
    for k,v in d.iteritems():
        print k,v

Running time: around 3.1 seconds.

Here is the solution using `tab`:

    $ ./tab -f req.log '{cut(cut(@," ",2),"?",0) -> sum(1)}'

Running time: around 0.9 seconds.

Not only is `tab` faster in this case, it is also (in my opinion) more concise and idiomatic.

## Reference ##

### Grammar ###

```python

expr := atomic_or_assignment ("," atomic_or_assignment)*

atomic_or_assignment := assignment | atomic

assignment := var "=" atomic

atomic := e_eq

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
         "~" e_not

e_flat := e_idx |
          ":" e_flat |
          "?" e_flat

e_idx := e |
         e ("[" expr "]")*

e := literal | funcall | var | array | map | seq | paren

literal := int | uint | real | string

funcall := var "(" expr ")"

array := "[." expr (":" expr)? ".]"

map := "{" expr ("->" expr)? (":" expr)? "}"

seq := "[" expr (":" expr)? "]"

paren := "(" expr ")"

var := "@" | [a-zA-Z][a-zA-Z0-9_]*

int := ("-")? [0-9]+

uint := [0-9]+ "u"

real := [-+]? [0-9]+ "." [0-9]+ |
        [-+]? [0-9]+ "e" [-+]? [0-9]+

string := '"' chars '"' |
          "'" chars "'"

chars := ("\t" | "\n" | "\r" | "\e" | "\\" | any)*

```

### Builtin functions ###

Listed alphabetically.

`array`
: Stores a sequence or map into an array. See also `sort` for a version of this function with sorting.  
Usage:  
`array Map * -> Arr *`  
`array Seq * -> Arr *`  
`array UInt|Int|Real|String|Tuple -> Arr UInt|Int|Real|String|Tuple` -- **Note:** this version of this function will return an array with one element, marked so that storing it as a value in an existing key of a map will produce an unsorted array of all such values, listed in order of insertion into the map. 

`avg`
: Synonym for `mean`.

`cat`
: Concatenates strings.  
Usage:  
`cat String... -> String`. At least one string argument is required.

`cos`
: The cosine function.  
Usage:  
`cos UInt|Int|Real -> Real`

`count`
: Counts the number of elements.  
Usage:  
`count None -> Seq UInt` -- returns an infinite sequence that counts from 1 to infinity.  
`count String -> UInt` -- returns the number of bytes in the string.  
`count Seq * -> UInt` -- returns the number of elements in the sequence. (*Warning*: counting the number of elements will consume the sequence!)  
`count Map * -> UInt` -- returns the number of keys in the map.  
`count Arr * -> UInt` -- returns the number of elements in the array.

`cut`
: Splits a string using a delimiter.  
Usage:  
`cut String String -> Arr String` -- returns an array of strings, such that the first argument is split using the second argument as a delimiter.  
`cut String String UInt|Int -> String` -- calling `cut(a,b,n)` is equivalent to `cut(a,b)[n]`, except much faster.

`e`
: Returns the number *e*.  
Usage:  
`e None -> Real`.

`exp`
: The exponentiation function. Calling `exp(a)` is equivalent to `e()**a`.  
Usage:  
`exp UInt|Int|Real -> Real`

`filter`
: Filters a sequence by returning an equivalent sequence but with certain elements removed. The input sequence must be a tuple where the first element is an integer; elements where this first elelemt is equal to 0 will be removed from the output sequence.  
Usage:  
`filter Seq (UInt|Int *...) -> Seq (*...)`

`flatten`
: Flattens a sequence of sequences, a sequence of arrays or a sequence of maps into a sequence of values.  
Usage:  
`flatten Seq (Seq *) -> Seq *`  
`flatten Seq (Arr *) -> Seq *`  
`flatten Seq (Map *) -> Seq *`  
`flatten Seq * -> Seq *` -- sequences that are already flat will be returned unchanged. (Though at a performance cost.)

`grep`
: Finds regular expression matches in a string. The first argument is the string to match in, the second argument is the regular expression. Matches are returned in an array of strings. Regular expressions use ECMAScript syntax.  
Usage:  
`grep String String -> Arr String`

`grepif`
: Returns 1 if a regular expression has matches in a string, 0 otherwise. Calling `grepif(a,b)` is equivalent to `count(grep(a,b)) != 0u`, except much faster.  
Usage:  
`grepif String String -> UInt`. 

`head`
: Accepts a sequence and returns an equivalent sequence that is truncated to be no longer than N elements. See also: `skip`.  
Usage:  
`head (Seq *) UInt|Int -> Seq *`

`if`
: Choose between alternatives. If the first integer argument is not 0, then the second argument is returned; otherwise, the third argument is returned. The second and third arguments must have the same type.
*Note*: this is not a true conditional control structure, since all three arguments are always evaluated.  
Usage:  
`if UInt|Int * * -> *`  

`index`
: Select elements from arrays, maps or tuples. Indexing a non-existent element will cause an error.  
Usage:  
`index (Arr *) UInt` -- returns element from the array, using a 0-based index.  
`index (Arr *) Int` -- negative indexes select elements from the end of the array, such that -1 is the last element, -2 is second-to-last, etc.  
`index (Arr *) Real` -- returns an element such that 0.0 is the first element of the array and 1.0 is the last.  
`index (Map *) *` -- returns the element stored in the map with the given key.  
`index (Tuple *) UInt`, `index (Tuple *) Int` -- returns an element from a tuple.  
`index (Arr *) UInt|Int|Real UInt|Int|Real` -- returns a sub-array from an array; the start and end elements of the sub-array are indexed as with the two-argument version of `index`.

`int`
: Converts an unsigned integer, floating-point value or string into a signed integer.  
Usage:  
`int UInt -> Int`  
`int Real -> Int`  
`int String -> Int`

`join`
: Concatenates the elements in a string array using a delimiter.  
Usage:  
`join (Arr String) String -> String`

`log`
: The natural logarithm function.  
Usage:  
`log UInt|Int|Real -> Real`

`max`
: Finds the maximum element in a sequence or array. See also: `min`.  
Usage:  
`max Arr * -> *`  
`max Seq * -> *`  
`max UInt|Int|Real -> UInt|Int|Real` -- **Note:** this version of this function will mark the return value to calculate the max when stored as a value into an existing key of a map.

`mean`
: Calculates the mean (arithmetic average) of a sequence or array of numbers. See also: `var` and `stdev`.  
Usage:  
`mean Arr UInt|Int|Real -> Real`  
`mean Seq UInt|Int|Real -> Real`  
`mean UInt|Int|Real -> Real` -- **Note:** this version of this function will mark the returned value to calculate the mean when stored as a value into an existing key of a map.

`min`
: Finds the minimum element in a sequence or array. See also: `max`.  
Usage:  
`min Arr * -> *`  
`min Seq * -> *`  
`min UInt|Int|Real -> UInt|Int|Real` -- **Note:** this version of this function will mark the return value to calculate the min when stored as a value into an existing key of a map.

`pi`
: Return the number *pi*.  
Usage:  
`pi None -> Real`

`real`
: Converts an unsigned integer, signed integer or string into a floating-point value.  
Usage:  
`real UInt -> Real`  
`real Int -> Real`  
`real String -> Real`

`replace`
: Search-and-replace in a string with regexes. The first argument is the string to search, the second argument is the regex, and the third argument is the replacement string. Regex and replacement string use ECMAScript syntax.  
Usage:  
`replace String String String -> String`

`round`
: Rounds a floating-point number to the nearest integer.  
Usage:  
`round Real -> Real`

`sin`
: The sine function.  
Usage:  
`sin UInt|Int|Real -> Real`

`skip`
: Accepts a sequence and returns an equivalent sequence where the fist N elements are ignored. See also: `head`.  
Usage:  
`skip (Seq *) UInt|Int -> Seq *`

`sort`
: Sorts a sequence, array or map lexicographically. The result is stored into an array if the input is a map or a sequence. See also `array` a version of this function without sorting.  
Usage:  
`sort Arr * -> Arr *`  
`sort Map * -> Arr *`  
`sort Seq * -> Arr *`  
`sort UInt|Int|Real|String|Tuple -> Arr UInt|Int|Real|String|Tuple` -- **Note:** this version of this function will return an array with one element, marked so that storing it as a value in an existing key of a map will produce a sorted array of all such values. 

`sqrt`
: The square root function.  
Usage:  
`sqrt UInt|Int|Real -> Real`

`stddev`
: Synonym for `stdev`.

`stdev`
: Calculates the sample standard deviation, defined as the square root of the variance. This function is completely analogous to `var`, with the difference that the square root of the result is taken. See also: `mean`.  
Usage:  
`stdev Arr UInt|Int|Real -> Real`  
`stdev Seq UInt|Int|Real -> Real`  
`stdev UInt|Int|Real -> Real` -- **Note:** this version of this function will mark the returned value to calculate the standard deviation when stored as a value into an existing key of a map.

`string`
: Converts an unsigned integer, signed integer or floating-point number to a string.  
Usage:  
`string UInt -> String`  
`string Int -> String`  
`string Real -> String`

`sum`
: Computes a sum of the elements of a sequence or array.  
Usage:  
`sum Arr UInt|Int|Real -> UInt|Int|Real`  
`sum Seq UInt|Int|Real -> UInt|Int|Real`  
`sum UInt|Int|Real -> UInt|Int|Real` -- **Note:** this version of this function will mark the value to be aggregated as a sum when stored as a value into an existing key of a map.

`tan`
: The tangent function.  
Usage:  
`tan UInt|Int|Real -> Real`

`tabulate`  
: Accepts two or more tuples of the same type and returns an array of those tuples. *Note*: this function is meant for pretty-printing results when there is only a few of them.  (A tuple of tuples will be printed on one line, while an array of tuples will print each tuple on its own line.)  
Usage:  
`tabulate (*...)... -> Arr (*...)`

`tolower`
: Converts to bytes of a string to lowercase. *Note:* only works on ASCII data, Unicode is not supported.  
Usage:  
`tolower String -> String`

`toupper`
: Converts to bytes of a string to uppercase. *Note:* only works on ASCII data, Unicode is not supported.  
Usage:  
`toupper String -> String`

`tuple`
: Returns its arguments as a tuple. Meant for grouping when defining tuples within tuples.  
Usage:  
`tuple (*...) -> (*...)`

`uint`
: Converts a signed integer, floating-point number or string to an unsigned integer.  
Usage:  
`uint UInt -> UInt`  
`uint Real -> UInt`  
`uint String -> UInt`

`var`
: Calculates the sample variance of a sequence of numbers. (Defined as the mean of squares minus the square of the mean.) See also: `mean` and `stdev`.  
Usage:  
`var Arr UInt|Int|Real -> Real`  
`var Seq UInt|Int|Real -> Real`  
`var UInt|Int|Real -> Real` -- **Note:** this version of this function will mark the returned value to calculate the variance when stored as a value into an existing key of a map.

`variance`
: Synonym for `var`.

`zip`
: Accepts two or more sequences and returns a sequence that returns a tuple of elements from each of the input sequences. The output sequence ends when any of the input sequences end.  
Usage:  
`zip (Seq *)... -> Seq (*...)`

### Aggregators ###

Aggregators are functions like any other; they accept a value and return a value, though usually the result is not useful as such. What's important is that aggregators have a side effect: the returned value is (invisibly) marked such that it will combine in special ways when it ends up keyed in a map that already stores another element at this key.

Aggregation is performed efficiently: no unnecessary temporary data structures are created and no unnecessary bookkeeping calculations are performed.

Here is a list of aggregators and their effects, sorted alphabetically:

`array`
: Accepts a value, returns an array of one element with that value. When combined together, these arrays will merge together, with the resulting elements appearing according to insertion order. (Last inserted elements coming last in the array.) See also: `sort`.

`avg`
: Accepts a numeric value, returns a floating-point number. When combined together, the arithmetic mean of the numbers will be computed.

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

`var`
: Accepts a numeric value, returns a floating-point number. When combined together, the sample variance is computed, defined as the mean of squares minus the square of the mean.

`variance`
: Synonymous with `var`.

