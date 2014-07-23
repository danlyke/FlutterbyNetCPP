== FlutterbyNetCPP ==

a system for managing a mostly static personal web site.

Dan Lyke
danlyke@flutterby.com
http://www.flutterby.net/User:DanLyke

=== Design & Code Notes ===

I had a couple of goals with this system:

* A lot of working in a dynamically typed language (Perl) convinced me
  that I really wanted to be developing new personal code in a typed
  language.

* I had a Perl system that was built on top of the "Markdown"-ish
  (that wasn't yet a thing when I originally built it) parser that
  runs Flutterby.com. There were operations that weren't as fast as
  they should have been, and I knew that I some structural performance
  issues that it'd be nice to get fixed. I didn't realize how much
  just moving the same things into C++ would speed things up, but I
  was also (and still am) rebuilding too many files at a whack.

* I wanted to get comfortable with some of the C++11
  features. Hellooooo, "auto"!

There are vestiges of two early decisions still in this code:

When I started building this system I grabbed some code I'd written to
move code between Microsoft Managed .NET "C++" and regular C++, for a
code base that was originally C# (Porting a C# app to be
iOS/Windows/OSX), so there were some kludges for "property"
attributes, typing for "^" vs "*" pointer dereferences, strings,
dynamic arrays.

I also started doing a Lex/Flex based parser, and then realized that
for text processing just copying the regexes that the Perl system used
let me better control parsing element priority.

Todo

* The system needs more tests (the system always needs more tests).

* It'd be nice to split the library away from the application a little
  further. As I try to hone my C++ chops, I'm dropping back into Perl
  more often than I'd like because I can't just "-lfby", or use
  "sqlextractor" independently.

=== Philosophy & Submodules ===

==== ORM ===

fbydb & sqlextractor - 

==== Parsers ====

==== Mapping ====

OpenLayers

==== 

