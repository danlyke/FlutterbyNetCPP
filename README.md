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

## Todo

* The system needs more tests (the system always needs more tests).

### Mapping

OpenLayers talking to Google Maps and a locally hosted Open Street Map
tile cache.

To be done here is a little more on pulling some combination of status
updates and image metadata into maps.

