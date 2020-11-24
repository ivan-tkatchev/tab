# The tab cookbook #

This is an example list of useful `tab` programs.

## Working with lines and words:

#### Count the number of lines in a file:
    :::tab
    count(@)

#### Output the longest line in a file:
    :::tab
    max([ count(@), @ ])~1

#### Output the three longest lines in a file:
    :::tab
    [ @~1 : sort([ count(@), @ ])[-3,-1] ]

#### Output the most common English word in a file:
    :::tab
    freq={ @ -> sum.1 : :[grep(@,"[A-Za-z0-9]+")] }, second.max.flip.freq

#### Output the counts of top ten most common bytes in a file:
    :::tab
    freq={ @ -> sum.1 : :[bytes.@] }, [ string.array.@~1, @~0 : (sort.flip.freq)[-10,-1] ]

#### The histogram of English word frequencies by word lengths:
    :::tab
    freq={ count.@ -> sum.1 : :[grep(@,"[A-Za-z0-9]+")] }, sort.freq

#### Same, but with syntactic sugar:
    :::tab
    :[grep(@,"[A-Za-z0-9]+")] .. { count.@ -> sum.1 } .. sort.@

#### Same histogram, but also with the words themselves:
    :::tab
    def f first.@,
    def s second.@,
    :[grep(@,"[A-Za-z0-9]+")] .. { count.@ -> sum.1, {tolower.@} } .. sort.[ f.@, f.s.@, join(f.s.s.@, ",")]

#### All English words following the word 'the':
    :::tab
    :[grep(@,"[A-Za-z0-9]+")] .. ?[ (tolower.first.@) == "the", second.@ : pairs.@ ]

#### Count the number of unique English words in a file:
    :::tab
    merge.flatten.[ [ uniques.tolower.@ : grep(@, "[A-Za-z0-9]+") ] ]

## Sampling data randomly from files:

#### Output four random lines from a file:
    :::tab
    sample(4, @)

#### Check if there is an empty line among four random lines from a file:
    :::tab
    sum([ count(@) == 0 : sample(4, @) ]) > 0

#### Sample four lines from `file.txt` one hundred times and output the histogram of number of empty lines:
    :::tab
    def empties sum.[ count(@) == 0 : sample(4, @) ],
    { empties.open."file.txt" -> sum.1 : count(100) }

## Basic math and loops:

#### A sine and cose table:
    :::tab
    [ sin.@, cos.@ : count(0.0, 2*pi(), pi()/8) ]

## Numeric data aggregation:

For the next few examples let's use an input file that looks something like this:

    1948	12	11	24
    1948	12	12	19
    1948	12	13	-74
    1948	12	14	-56

The first three fields are the year, month and day. The fourth field is the daily max temperature in units of 0.1 degrees Celcius.

#### The average (mean) of the temperature, aggregated by year:
    :::tab
    def [year, month, day, temp real.@], 
    { year.@ -> avg.temp.@ : cut(@,"\t") }

#### As above, except also with the median:
    :::tab
    data={ def [year, _, _, temp real.@], t=temp.@, year.@ -> avg.t, sort.t : cut(@,"\t") },
    [ @~0, @~1~0, @~1~1~0.5 : sort.data ]

#### A histogram of the temperature, grouped by buckets of ten degrees:
    :::tab
    sort.{ 100*int.(real.cut(@,"\t",3))/100 -> sum.1 }

#### Check if the distribution of temperature matches a (sampled) normal distribution:
    :::tab
    temps=[. real.cut(@,"\t",3) .],
    u=mean.temps, s=stddev.temps, a=min.temps, b=max.temps,
    sort.{ bucket(@~0, a, b, 5) -> sum.@~1, sum.@~2 :
           :seq([ @, 1, 0 : temps ], [ normal(u,s), 0, 1 : count.count.temps ]) }

#### Find temperature outliers (more than 3 standard deviations away from mean) and group them by year:
    :::tab
    def [ year uint.@, _, _, temp real.@ ],
    temps=[. uint.@~0, real.@~3 : cut(@,"\t") .],
    def [ year, t ],
    sd=stddev.[ t.@ : temps],
    { year.@ -> sum.1 : ?[t(@) > 3*sd, @ : temps] }

#### Find years with spotty temperature records -- where the number of measurements taken is more than one standard deviation away from the average year:
    :::tab
    y_n={ uint.cut(@,"\t",0) -> sum.1 },
    u=mean.second.y_n, sd=stddev.second.y_n,
    ?[ abs(@~1 - u) > sd, @~0 : y_n ]

#### Group temperatures by year and month, and find the mean and median for September 1998:
    :::tab
    t={ def [ year uint.@, month uint.@, day, temp real.@ ], month.@ -> map(year.@, sort.temp.@) : cut(@,"\t") }~9~1998,
    mean.t, t~0.5

#### Same as above, but compare it to the mean and median across all Septembers:
    :::tab
    t={ def [ year uint.@, month uint.@, day, temp real.@ ], month.@ -> map(year.@, sort.temp.@) : cut(@,"\t") }~9,
    mean.t~1998, t~1998~0.5,
    mean.flatten.second.t, (sort.flatten.second.t)~0.5

#### Average September temperatures by year:
    :::tab
    def [ year uint.@, month uint.@, day, temp real.@ ],
    sort.map.?[ (month.@) == 9, year.@, avg.temp.@ : cut(@,"\t") ]
: pipe the output to `gnuplot -p -e "plot '-' with lines"` to see a graph

#### Moving average of September temperatures, over 10 previous years:
    :::tab
    def [ year uint.@, month uint.@, day, temp real.@ ],
    t=sort.map.?[ (month.@) == 9, year.@, avg.temp.@ : cut(@,"\t") ],
    sort.{ @~(-1)~0 -> avg.second.seq.@ : ngrams(seq.t, 10) }

#### Calculate the average difference between this day's temperature and the temperature on the first of the month:
    :::tab
    def [ year uint.@, month uint.@, day, temp real.@ ],
    sort.{ t=(temp.@)/10, month.@ -> avg(t - box(day(@) == "1", t)~0) : cut(@,"\t") }

## Working with ad-hoc text formats:

#### Convert MySQL output to a machine-readable one:
    :::tab
    [ join(recut(@, " *\\| *")[1,-2], "\t") : skip(grepif(@, "^\\| "), 1) ]

#### Parse a log file where every line is a JSON object:
    :::tab
    regex = '"([^"]+)" *: *([0-9.]+)|"([^"]+)"',
    def split ?[ count.@, @ : grep(@, regex) ],
    [ map(stripe(pairs.split.@, 2))~"response" ]
: You will need to add extra backslashes if you want to enter that regex via a shell command line.

#### Given a file with a URL on each line, find the hostnames:
    :::tab
    { grep(@, "//([^/]*)/")~0 -> sum.1 }

#### Parse the GET parameters in a file of URLs:
    :::tab
    regex = "[?&]([^&]+)=([^&]+)";
    def urlgetparams map.stripe(pairs.[@ : grep(@, regex) ], 2);
    [ urlgetparams.@ ]

#### Same as above, but find the most popular GET parameter:
    :::tab
    regex = "[?&]([^&]+)=([^&]+)";
    def urlgetparams map.stripe(pairs.[@ : grep(@, regex) ], 2);
    :[ first.urlgetparams.@ ] .. second.max.flip.{ @ -> sum.1 }

#### Another implementation of same as above:
    :::tab
    def urlgetparams map.[ try take.@, take.@ : explode.seq.recut(@, "[?&=]")[1,-1] ];
    second.max.flip.{ @ -> sum.1 : :[ first.urlgetparams.@ ] }

#### There is built-in support for parsing GET parameters, so really you should be doing this instead:
    :::tab
    :[ first.url_getparam.@ ] .. second.max.flip.{ @ -> sum.1 }

## Working with multi-line data:

#### Double-space a file:
    :::tab
    [ seq(@, "") ]

#### Glue every pair of lines in a file together:
    :::tab
    [ join(head(@, 2), "\t") : explode.@ ]

#### Count the number of lines in every paragraph in a file:
    :::tab
    [ count.while.[ @ != "", @ ] : explode.@ ]

#### Remove duplicate lines, like the Unix tool `uniq`:
    :::tab
    x=peek.pairs.@, glue(first.first.x, ?[ @~0 != @~1, @~1 : second.x ])

## Recursive functions

#### Compute the factorial:
    :::tab
    def fac << @~0 * @~1 : 1, count.@ >>, fac.12

#### A 32-bit number with only the odd bits flipped:
    :::tab
    << lsh(@~0, 2) | 1 : 0, count.32 >>

#### The 11th Fibonacci number:
    :::tab
    << a=@~0~0, b=@~0~1, tuple(b, a + b) : tuple(0, 1), count.10 >>~1

#### Return the tail of a sequence, like the Unix tool `tail`:
    :::tab
    def tail << @~1 : peek.ngrams.@ >>, tail(@, 10)

#### Run the famous Rule 110 for 8 steps:
    :::tab
    def step [ t=string.@, case(t; 'XXX','.'; 'X..','.'; '...','.'; 'X') : ngrams(seq.bytes.@,3) ];
    def rule110 << join('.', step.@~0, '', '.') : @~0, count.@~1 >>;
    rule110('..........XX..........', 8)

