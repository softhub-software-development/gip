Clean Code License

The Clean Code License (CCL) is based on the concept of typographical licenses. In short, you, the licensee,
is granted the right to copy and modify the code, as long as your changes, and the code you copy in part or in full,
from this project to another project, adheres completely to the typographical rules implicitly given by the source
code from this project.

There are no explicit typographical rules specified by the project but the rules are intrinsically given
by how the code is formatted. The licensee looses all rights for reusing all or parts of the code, if he chooses
to typographically modify the code, adds code which does not adhere to the rules implicitly given by the source
code in this project or embeds code snippets from this project into code which does not follow the typographic
rules from this project.

Background: Various deviations from the typographic rules to lay out source code in different programming languages
are in use. Though some of them have their well founded origins, others evolved purely out of malice. We therefore
distinguish between Human Readable Code (HRC) and Machine Readable Code (MRC), sometimes also referred to as
Monkey Readable Code. As we are humans, we prefer HRC one should think. Knowing that, and taking advantage of
our human weaknesses, some programmers came up with totally screwed up coding conventions, which they sarcastically
refer to as clean code. Once such a screwed up coding convention can be implemented in a software development team,
it is easy to take over any project and kick out those programmers who do it right. The typographic license is
designed to prevent this from happening.

As there are different typographic rules for different programming languages, there may not be a set of rules applicable
to all of the source code. For example, the rules for formatting JavaScript code may be different to the way we format
C++ code. In general, there is a natural formatting for any programming language, most often enspired by the inventor
of the languange.

In special cases like Objective-C, the rules are, at least in this project, purposely overridden by typographic rules
sticking to what one learns, if one ever read a thing called "book". The inventor of Objective-C was certainly not
an idol in this sense, as he chose to consistently do the opposit of what one would expect somebody to do who ever
read a book and therby learned the implicit rules of typography.

Here are some examples how to spot monkey code (or MRC):

HRC:
<pre>
if (x > 0)
    some_function(x);
</pre>

Some variations of MRC:
<pre>
if ( x > 0)    // see the monkey? one nonsense blank
    some_function(x);
or
if ( x > 0 )   // see the monkeys? even two nonsense blanks
    some_function( x );
</pre>

HRC:
<pre>
int some_function(int x)
{
    // do something usefull and return
    return 0;
}
</pre>

Example for MRC:
<pre>
int some_function(int x)
{
    // do something usefull and return
    return( 0 );   // what the fuck is this supposed to mean?
}
</pre>

HRC:
<pre>
if (x > 0) {
    do_a();
    do_b();
}
</pre>

Example for MRC:
<pre>
if (x > 0)
{           // yeah, you did Pascal before, right?
    do_a();
    do_b();
}
</pre>
