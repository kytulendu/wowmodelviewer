< <a href='http://code.google.com/p/wowmodelviewer/wiki/Home'>Home</a> ><
<a href='http://code.google.com/p/wowmodelviewer/wiki/ToDo'>To Do</a> ><
<a href='http://code.google.com/p/wowmodelviewer/wiki/Exporting'>Model Exporting</a> ><
<a href='http://wowmodelviewer.googlecode.com/svn/trunk/doc/buildnotes.txt'>Build Notes</a> ><
<a href='http://wowmodelviewer.googlecode.com/svn/trunk/doc/ChangeLog.txt'>Changelog</a> >

![http://i43.tinypic.com/25zh65e.jpg](http://i43.tinypic.com/25zh65e.jpg)
# Introduction #

```
# apt-get install cmake
# apt-get install gcc
# apt-get install g++
# apt-get install make
# apt-get install libwxbase2.8-dev
# apt-get install libwxgtk2.8-dev
# apt-get install libjpeg62-dev
# apt-get install libpng12-dev
# apt-get install libglew1.5-dev
# apt-get install libbz2-dev
# apt-get install subversion
or in one line
$ sudo apt-get install cmake gcc g++ make libwxbase2.8-dev libwxgtk2.8-dev libjpeg62-dev libpng12-dev libglew1.5-dev libbz2-dev subversion

$ svn checkout http://wowmodelviewer.googlecode.com/svn/trunk/ wowmodelviewer
$ cd wowmodelviewer/src
$ cmake .
-- The C compiler identification is GNU
-- The CXX compiler identification is GNU
-- Check for working C compiler: /usr/bin/gcc
-- Check for working C compiler: /usr/bin/gcc -- works
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Check for working CXX compiler: /usr/bin/c++
-- Check for working CXX compiler: /usr/bin/c++ -- works
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Found wxWidgets: TRUE
-- Found BZip2: /usr/lib/libbz2.so
-- Looking for BZ2_bzCompressInit in /usr/lib/libbz2.so
-- Looking for BZ2_bzCompressInit in /usr/lib/libbz2.so - found
-- Found ZLIB: /usr/lib/libz.so
-- Found JPEG: /usr/lib/libjpeg.so
-- Found PNG: /usr/lib/libpng.so
-- Configuring done
-- Generating done
-- Build files have been written to: /home/xp/wowmodelviewer/src
$ make
-- Configuring done
-- Generating done
-- Build files have been written to: /home/xp/wowmodelviewer/src
[ 43%] Built target cximage
[ 52%] Built target mpq

Linking CXX executable wowmodelviewer
[100%] Built target wowmodelviewer
```