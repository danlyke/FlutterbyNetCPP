# FlutterbyNetCPP

a system for managing a mostly static personal web site.

Dan Lyke
danlyke@flutterby.com
http://www.flutterby.net/User:DanLyke

## Quickstart

Create a directory with text files with .wiki extensions in it. Format
is a combo of MediaWiki-ish (headers, lists, double brackets for
wiki-internal links, etc), with HTML.

In ~/.fby/config.ini put

```
input_directory = /path/to/your/.wiki/files
html_directory = /path/to/output/html
staging_directory = /a/staging/directory/for/intermediate/html
google_maps_api_key = whatitsays
```

Run "fby --doeverything", this will scan the HTML directory for
images, and generate the output HTML.

In my installation, I have a suid wrapper that lets other ancillary
apps do things like write images and files, sometimes from web
apps. There are also tags for dealing with maps (similar to the Google
Maps plugin for MediaWiki, except this does OpenLayers to Google Maps
and my own OSM cache), video, and inputs from a status update database
that I use to feed into Facebook and Twitter and the like.


## Design & Code Notes

I had a couple of goals with this system:

* A lot of working in a dynamically typed language (Perl) convinced me
  that I really wanted to be developing new personal code in a typed
  language.

* I had a Perl system that was built on top of the parser that
  runs Flutterby.com. There were operations that weren't as fast as
  they should have been, and I knew that I some structural performance
  issues that it'd be nice to get fixed. I didn't realize how much
  just moving the same things into C++ would speed things up, but I
  was also (and still am) rebuilding too many files at a whack.

* I wanted to get comfortable with some of the C++11
  features. Hellooooo, "auto" and lambdas!

There are vestiges of two early decisions still in this code:

When I started building this system I grabbed some code I'd written to
move code between Microsoft Managed .NET "C++" and regular C++, for a
code base that was originally C# (Porting a C# app to be
iOS/Windows/OSX), so there were some kludges for "property"
attributes, typing for "^" vs "*" pointer dereferences, strings,
dynamic arrays. I'm trying to rip that out and just go straight C++.

I also started doing a Lex/Flex based parser, and then realized that
for text processing just copying the regexes that the Perl system used
let me better control parsing element priority. But those vestiges
remain.

Todo

* The system needs more tests (the system always needs more tests).

* It'd be nice to split the library away from the application a little
  further. As I try to hone my C++ chops, I'm dropping back into Perl
  more often than I'd like because I can't just "-lfby", or use
  "sqlextractor" independently.

* Once the app is split from the library, there's a lot of test
  framework that needs to be built for both. As time allows.

## Philosophy & Submodules

### ORM

Object models are created in a mix of C++ and SQL.

`sqlextractor` parses `wikiobjects.h` into `sqldefinitions.{cpp,h}` and `sqldefinitions.sql`

The `fbydb` stuff provides a mapping that queries and saves those
objects, wrapped on top of SQLite and PostgreSQL.

### Parsers

One built on flex, one built on Perl style regexes.

### Mapping

OpenLayers talking to Google Maps and a locally hosted Open Street Map
tile cache.

To be done here is a little more on pulling some combination of status
updates and image metadata into maps.



