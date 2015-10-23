< <a href='http://code.google.com/p/wowmodelviewer/wiki/Home'>Home</a> ><
<a href='http://code.google.com/p/wowmodelviewer/wiki/ToDo'>To Do</a> ><
<a href='http://code.google.com/p/wowmodelviewer/wiki/Exporting'>Model Exporting</a> ><
<a href='http://wowmodelviewer.googlecode.com/svn/trunk/doc/buildnotes.txt'>Build Notes</a> ><
<a href='http://wowmodelviewer.googlecode.com/svn/trunk/doc/ChangeLog.txt'>Changelog</a> >

![http://i43.tinypic.com/25zh65e.jpg](http://i43.tinypic.com/25zh65e.jpg)
> _To build the v0.8.0.0 WoW Model Viewer on Windows, please view_<a href='http://code.google.com/p/wowmodelviewer/wiki/v0800_WindowsCompilation'>this page</a>._# Introduction #
Some of the work is already done for users of Visual Studio 2008 & 2010. Check [Uncommon Events](WindowsCompilation#Uncommon_Events.md) if you are using Visual Studio 2005 or another compiler._

# Common Events #
Everyone needs to do the following:
## Download and install wxWidgets 2.8.11 ##
  1. Get wxMSW - installer for Windows (`wxMSW-2.8.11-Setup.exe`) from [the wxWidgets Website](http://www.wxwidgets.org/downloads/).
  1. Install `wxMSW-2.8.11-Setup.exe` to `C:\wxWidgets-2.8.11\`
  1. Use Visual Studio 2008 to open `wxWidgets-2.8.11\build\msw\wx.dsw` and build a Debug & Release version.
## 64-Bit ##
If you are building a 64-bit version of WoW Model Viewer, you also need to do the following:
  1. Add a 64-bit Configuration. (May not be needed if you get wxWidgets 2.9.0, but that version has not yet been tested.)
  1. Build 64-bit Release & Debug versions.

# WoWModelViewer #
If you need a Subversion program, I recommend [TortoiseSVN](http://tortoisesvn.net/) for Windows users, or you can try one of the ones [listed here](http://subversion.apache.org/packages.html).
  1. Get wowmodelviewer from [here](http://code.google.com/p/wowmodelviewer/source/checkout). Please refer to your Subversion software's documentation on how to checkout (download) the source code listed on the checkout page.
  1. Use Visual Studio 2008 to open `wowmodelviewer-read-only\src\wowmodelview_VC90.sln` or Visual Studio 2010 to open `wowmodelviewer-read-only\src\wowmodelview_VC100.sln`
  1. If you are not using one of the provided Visual Studio sln files, you will need to open `wowmodelviewer-read-only\src\stormlib` and compile that first.
  1. Build the WMV project.

Congratulations! You should have successfully compiled WMV's source code!

# Uncommon Events #
If you are building on Visual Studio 2005, or you are not using Visual Studio, you will also need to do the following:
## Build Glew Libraries ##
  1. Navigate to `wowmodelviewer-read-only\src\glew\build\vc6`
  1. Convert `glew.dsw` to your project type.
  1. Compile Release and Debug versions.
## 64-Bit ##
  1. Navigate to `wowmodelviewer-read-only\src\glew\build\vc6`
  1. Convert `glew.dsw` to your project type.
  1. Build 64-bit compilations, if nessicary.
  1. Compile 64-bit Release & Debug versions.

## Build CxImage Libraries ##
  1. Get `cximage600_full.7z` from http://www.xdp.it/cximage.htm
  1. Extract the files to `C:\cximage600_full` and open/convert `CxImgLib.dsw`. Build a release and a debug version of the `CXImageCrtdll` project.
  1. Copy `bin\cximagecrt32.dll` & `bin\cximagecrt32d.dll` to `wowmodelviewer-read-only\bin_support\(OS)\32bit`.
  1. Copy `CxImage\CxImageDLL\Debug\cximagecrt32d.lib` & `CxImage\CxImageDLL\Release\cximagecrt32.lib` to `wowmodelviewer-read-only\src\lib\(OS)\32bit`.
## 64-Bit ##
  1. Add a configuration for 64-bits to all the projects within the CxImgLib solution. (This will take some time.) I recommend sending the binary information to a new directory, specifically for 64bits. (bin64, Release64 & Debug64 for example.)
  1. Compile 64-bit Release & Debug versions.
  1. Copy `bin64\cximagecrt64.dll` & `bin64\cximagecrt64d.dll` to `wowmodelviewer-read-only\bin_support\(OS)\64bit`.
  1. Copy `CxImage\CxImageDLL\Debug64\cximagecrt64d.lib` & `CxImage\CxImageDLL\Release64\cximagecrt64.lib` to `wowmodelviewer-read-only\src\lib\(OS)\64bit`.