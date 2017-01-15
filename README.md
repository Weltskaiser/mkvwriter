# MKV Writer Library #

## Background ##

This is a fork of https://github.com/mattgruenke/mkvwriter.git, which had no
commits since being transitioned from another repo.  It seemed to be a MKV
writer plugin for the Helix media player (originally developed by
RealNetworks).  After seeing a post about it, in the MKV mailing list archives,
I've simply used it as a starting point for writing MKV files in my own apps.

I've tried to expunge everything not relevant to my purposes, in the interest
of simplicity, portability, and comprehensibility.  To that end, I've replaced
the old buildsystem with CMake.  I've only built it for Linux & Android NDK,
though I welcome contributions from anyone interested in using it on other
platforms.


## Releases ##

### v0.1 ###

Initial release.

* Added CMake buildsystem.
* Fixed build errors vs. libmatroska v1.4.4
* Removed everything extraneous
* Added this README.md

