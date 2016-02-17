# The tab cookbook #

This is an example list of useful `tab` programs.

## Working with lines and words:

#### Count the number of lines in a file:
    :::bash
    count(@)

#### Output the longest line in a file:
    :::bash
    max([ count(@), @ ])~1

#### Output the three longest lines in a file:
    :::bash
    [ @~1 : sort([ count(@), @ ])[-3,-1] ]

#### Output the most common English word in a file:
    :::bash
    freq={ @ -> sum.1 : :[grep(@,"[A-Za-z0-9]+")] }, second.max.flip.freq

#### Output the counts of top ten most common bytes in a file:
    :::bash
    freq={ @ -> sum.1 : :[bytes.@] }, [ string.array.@~1, @~0 : (sort.flip.freq)[-10,-1] ]

#### The histogram of English word frequencies by word lengths:
    :::bash
    freq={ count.@ -> sum.1 : :[grep(@,"[A-Za-z0-9]+")] }, sort.freq

#### Same histogram, but also with the words themselves:
    :::bash
    freq={ count.@ -> sum.1, {tolower.@} : :[grep(@,"[A-Za-z0-9]+")] },
    def f first.@,
    def s second.@,
    sort.[ f.@, f.s.@, join(f.s.s.@, ",") : freq]

#### All English words following the word 'the':
    :::bash
    ?[ (tolower.first.@) == "the", second.@ : pairs( :[grep(@,"[A-Za-z0-9]+")] ) ]

## Sampling data randomly from files:

#### Output four random lines from a file:
    :::bash
    sample(4, @)

#### Check if there is an empty line among four random lines from a file:
    :::bash
    sum([ count(@) == 0 : sample(4, @) ]) > 0

#### Sample four lines from `file.txt` one hundred times and output the histogram of number of empty lines:
    :::bash
    def empties sum([ count(@) == 0 : sample(4, @) ]),
    { empties.open."file.txt" -> sum.1 : count(100) }

## Basic math and loops:

#### A sine and cose table:
    :::bash
    [ sin.@, cos.@ : count(0.0, 2*pi(), pi()/8) ]

## Numeric data aggregation:

For the next few examples let's use an input file that looks something like this:

    1948	12	11	24
    1948	12	12	19
    1948	12	13	-74
    1948	12	14	-56

The first three fields are the year, month and day. The fourth field is the daily max temperature in units of 0.1 degrees Celcius.

#### The average (mean) of the temperature, aggregated by year:
    :::bash
    { x=cut(@,"\t"), x~0 -> avg.real.x~3 }

#### As above, except also with the median:
    :::bash
    data={ x=cut(@,"\t"), v=real.x~3, x~0 -> avg.v, sort.v }, [ @~0, @~1~0, @~1~1~0.5 : data ]

#### A histogram of the temperature, grouped by buckets of ten degrees:
    :::bash
    sort.{ x=cut(@,"\t"), 100*int(real(x~3)/100) -> sum.1 }

#### Check if the distribution of temperature matches a (sampled) normal distribution:
    :::bash
    temps=[. real.cut(@,"\t",3) .],
    u=mean.temps, s=stddev.temps, a=min.temps, b=max.temps,
    sort.{ bucket(@~0, a, b, 5) -> sum.@~1, sum.@~2 :
           :seq([ @, 1, 0 : temps ], [ normal(u,s), 0, 1 : count.count.temps ]) }

#### Find temperature outliers (more than 3 standard deviations away from mean) and group them by year:
    :::bash
    temps=[. x=cut(@,"\t"), uint.x~0, real.x~3 .],
    sd=stddev.[@~1 : temps],
    { @~0 -> sum.1 : ?[@~1 > 3*sd, @ : temps] }

#### Find years with spotty temperature records -- where the number of measurements taken is more than one standard deviation away from the average year:
    :::bash
    y_n={ uint.cut(@,"\t",0) -> sum.1 },
    u=mean.second.y_n, sd=stddev.second.y_n,
    ?[ abs(@~1 - u) > sd, @~0 : y_n ]

#### Group temperatures by year and month, and find the mean and median for September 1998:
    :::bash
    t={ x=cut(@,"\t"), uint.x~1 -> map(uint.x~0, sort.real.x~3) }~9~1998, mean.t, t~0.5

#### Same as above, but compare it to the mean and median across all Septembers:
    :::bash
    t={ x=cut(@,"\t"), uint.x~1 -> map(uint.x~0, sort.real.x~3) }~9,
    mean.t~1998, t~1998~0.5,
    mean.flatten.second.t, (sort.flatten.second.t)~0.5

#### Average September temperatures by year:
    :::bash
    sort.map.?[ x=cut(@,"\t"), (uint.x~1) == 9, uint.x~0, avg.real.x~3 ]
: pipe the output to `gnuplot -p -e "plot '-' with lines"` to see a graph

#### Moving average of September temperatures, over 10 previous years:
    :::bash
    t=sort.map.?[ x=cut(@,"\t"), (uint.x~1) == 9, uint.x~0, avg.real.x~3 ],
    def to_seq [@:@],
    sort.{ @~(-1)~0 -> avg.second.to_seq.@ : ngrams(to_seq.t, 10) }

## Working with ad-hoc text formats:

#### Convert MySQL output to a machine-readable one:
    :::bash
    [ join(recut(@, " *\\| *")[1,-2], "\t") : skip(grepif(@, "^\\| "), 1) ]

#### Parse a log file where every line is a JSON object:
    :::bash
    regex = '"([^"]+)" *: *([0-9.]+)|"([^"]+)"',
    def split ?[ count.@, @ : grep(@, regex) ],
    [ map(stripe(pairs.split.@, 2))~"response" ]
: You will need to add extra backslashes if you want to enter that regex via a shell command line.

#### Given a file with a URL on each line, find the hostnames:
    :::bash
    { grep(@, "//([^/]*)/")~0 -> sum.1 }

#### Parse the GET parameters in a file of URLs:
    :::bash
    regex = "[?&]([^&]+)=([^&]+)";
    def urlgetparams map.stripe(pairs.[@ : grep(@, regex) ], 2);
    [ urlgetparams.@ ]

#### Same as above, but find the most popular GET parameter:
    :::bash
    regex = "[?&]([^&]+)=([^&]+)";
    def urlgetparams map.stripe(pairs.[@ : grep(@, regex) ], 2);
    second.max.flip.{ @ -> sum.1 : :[ first.urlgetparams.@ ] }

## Working with multi-line data:

#### Double-space a file:
    ::bash
    [ seq(@, "") ]

#### Glue every pair of lines in a file together:
    ::bash
    [ join(head(@, 2), "\t") : explode.@ ]

#### Count the number of lines in every paragraph in a file:
    ::bash
    [ count.while.[ @ != "", @ ] : explode.@ ]

#### Remove duplicate lines, like the Unix tool `uniq`:
    ::bash
    x=pairs.@, head=take.x, x=glue(head, x), glue(first.head, ?[ @~0 != @~1, @~1 : x ])

## Recursive functions

#### Compute the factorial
    :::bash
    def fac << @~0 * @~1 : 1, count.@ >>, fac.12

