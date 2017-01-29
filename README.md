# MKV Writer Library #

A simple library for writing MKV files.


## Background ##

This is a fork of https://github.com/Matroska-Org/mkvwriter.git, which had no
commits since being transitioned from another repo.  It seemed to be a MKV
writer plugin for the Helix media player (originally developed by
RealNetworks).  After seeing a post about it, in the MKV mailing list archives,
I've simply used it as a starting point for writing MKV files in my own apps.

I've tried to expunge everything not relevant to my purposes, in the interest
of simplicity, portability, and comprehensibility.  To that end, I've replaced
the old buildsystem with CMake.  I've only built it for Linux & Android NDK,
though I welcome contributions from anyone interested in using it on other
platforms.

## License ##

Thanks to Steve, John, and Jory, this library is now licensed under the MIT
license.  See LICENSE.txt, for details.


## Releases ##

### v0.1.0 - 2017-01-16 ###

Initial release.

* Added CMake buildsystem.
* Fixed Linux build errors vs. libmatroska v1.4.4
* Removed everything extraneous
* Added this README.md
* Added `mkvwriter` namespace
* Removed `using namespace` from public headers
* Restructured sources into src/ and include/
* Added simple example program that combines multiple JPEG files into a .MKV


### v0.1.1 - 2017-01-29 ###

Re-licensed under MIT license, with permission from all copyright holders.


## To Do ##

* Had to disable m_AllCues->AddBlockGroup() in MatroskaMuxer.cpp, as this
  function was deprecated.  Need to find out with what it should be replaced.
  See commit 29d289d999f16972406a7f79f1d266d686d181d7.
* Improve API stability, by converting interface classes into abstract base
  classes.
* Improve conformance with later versions of the MKV specification.

