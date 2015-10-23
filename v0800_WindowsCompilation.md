< <a href='http://code.google.com/p/wowmodelviewer/wiki/Home'>Home</a> ><
<a href='http://code.google.com/p/wowmodelviewer/wiki/ToDo'>To Do</a> ><
<a href='http://code.google.com/p/wowmodelviewer/wiki/Exporting'>Model Exporting</a> ><
<a href='http://wowmodelviewer.googlecode.com/svn/trunk/doc/buildnotes.txt'>Build Notes</a> ><
<a href='http://wowmodelviewer.googlecode.com/svn/trunk/doc/ChangeLog.txt'>Changelog</a> >

![http://i43.tinypic.com/25zh65e.jpg](http://i43.tinypic.com/25zh65e.jpg)
> _To build the v0.7.0.1 WoW Model Viewer on Windows, please view_<a href='http://code.google.com/p/wowmodelviewer/wiki/WindowsCompilation'>this page</a>._# Introduction #
This document will cover how to build WoW Model Viewer version 0.8.0.0 on a Windows System._

# Software #
First, we need some software.
  * [Qt Application Framework](http://qt.nokia.com/) - This is the primary framework, or resource, of v0.8.0.0. Download the latest version of Qt. (4.7.3, as of this writing)

Qt comes with [several compilers pre-built](http://doc.qt.nokia.com/4.7-snapshot/platform-notes.html), so it will work with a lot of systems out-of-the-box. Some systems, however, (Visual Studio 2010, for one) are not yet available, and must be compiled by users before Qt can be used by those IDEs. **Not everyone has to download these files.**
  * [Qt SDK](http://qt.nokia.com/downloads/downloads#qt-lib) - This will let you build a QMake project so you can use a compiler that is not natively supported.
  * [Jom](ftp://ftp.qt.nokia.com/jom/) - Jom is nmake replacement for Qt compilation on multi-core CPU. Its parameter `-j N` allows you to specify the number of parallel processes for compilation. Number of physical CPU cores is good choice for N. For these instructions, we'll asssume you have 2 cores.

### Visual Studio ###
If you are using Visual Studio, there are one or two additional things you should download.
  * [Qt Visual Studio Add-In](http://qt.nokia.com/downloads/downloads#qt-other) - This makes working with Qt a breeze in Visual Studio 2005, 2008 or 2010.

# Building Qt #
If your compiler isn't supported by Qt out-of-the-box, then you will need to compile a compatible Qt Framework.

## 64-Bit Edition ##
In order to build a 64-bit WoW Model Viewer, you **MUST** compile a 64-bit Qt framework. Simply follow the steps below for building Qt with the following exceptions:
  * Instead of `msvc2010-32`, use `msvc2010-64`.
  * Use the 64-bit command prompt to compile Qt.
  * Do **NOT** change the `QMAKESPEC` to something like `win64-msvc2010`. The `win32-msvc2010` works perfectly.

## Visual Studio 2010 ##
  1. Extract the Qt 4.7.3 Source Code from either the `.zip` or `.tar.gz` file you downloaded.
  1. Copy the contents of the folder `qt-everywhere-opensource-src-4.7.3` to the directory where you intend to install Qt. In our case this is `D:\QtSDK\Desktop\Qt\4.7.3\msvc2010-32`.
  1. Extract jom to the `D:\QtSDK\Desktop\Qt\4.7.3\jom` folder.
  1. You need to add the following [environmental variables to your operating system](http://www.java.com/en/download/help/path.xml):
```
 QTDIR=D:\QtSDK\Desktop\Qt\4.7.3\msvc2010-32
 QMAKESPEC=win32-msvc2010
```
  1. You can change `QMAKESPEC` to any compiler listed in `D:\QtSDK\Desktop\Qt\4.7.3\msvc2010-32\mkspecs`.
  1. Update Window's `PATH` variable to include `%QTDIR%\bin`.
  1. Start Visual Studio 2010 Command Prompt, usually found in Start > Programs > Microsoft Visual Studio 2010 > Visual Studio Tools > Visual Studio Command Prompt.
  1. Run following commands in it:
```
 cd D:\QtSDK\Desktop\Qt\4.7.3\msvc2010-32
 configure -debug-and-release -opensource -platform win32-msvc2010
 ..\jom\jom.exe -j 2
```
  1. Make sure `-j 2` specifies the number of CPU cores in your current system. IE: Dual-Core systems have 2 and Hex-Core systems have 6.
  1. Allow Jom to run. Depending on your system, this may take a few hours.
  1. Install Qt Visual Studio Add-in.
  1. Load Visual Studio 2010.
  1. To integrate our recently compiled Qt, go to Qt > Qt Options. Press the Add button.
  1. Press the `...` button and navigate to `D:\QtSDK\Desktop\Qt\4.7.3\msvc2010-32`
  1. Change the version name to `4.7.3-msvc2010-32`. Press `OK`, then press `OK` again.


# Building WoWModelViewer #
If you need a Subversion program, I recommend [TortoiseSVN](http://tortoisesvn.net/) for Windows users, or you can try one of the ones [listed here](http://subversion.apache.org/packages.html).
  1. Checkout (download) `http://wowmodelviewer.googlecode.com/svn/branches/wmv0800` as outlined [here](http://code.google.com/p/wowmodelviewer/source/checkout). Please refer to your Subversion software's documentation on how to checkout  the source code listed on the checkout page.
  1. Navigate to `wowmodelviewer-read-only\Source\StormLib` and open `StormLib_VC90.sln` if you're using Visual Studio 2008, or `StormLib_VC100.sln` if you're using Visual Studio 2010.
  1. Compile Debug and Release configurations for 32-bit. If you're planning to build a 64-bit version of WMV, build the 64-bit configurations too.
  1. Open `wowmodelviewer-read-only\Source\WMV_VC90.sln` for Visual Studio 2008, or `wowmodelviewer-read-only\Source\WMV_VC100.sln` for Visual Studio 2010.
  1. Build the WMV project.

Congratulations! You should have successfully compiled WoW Model Viewer v0.8.0.0's source code!