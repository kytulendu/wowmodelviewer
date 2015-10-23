< <a href='http://code.google.com/p/wowmodelviewer/wiki/Home'>Home</a> ><
<a href='http://code.google.com/p/wowmodelviewer/wiki/ToDo'>To Do</a> ><
<a href='http://code.google.com/p/wowmodelviewer/wiki/Exporting'>Model Exporting</a> ><
<a href='http://wowmodelviewer.googlecode.com/svn/trunk/doc/buildnotes.txt'>Build Notes</a> ><
<a href='http://wowmodelviewer.googlecode.com/svn/trunk/doc/ChangeLog.txt'>Changelog</a> >

![http://i43.tinypic.com/25zh65e.jpg](http://i43.tinypic.com/25zh65e.jpg)
# Introduction #

This page describes how to compile WOWMV on your Mac. It is assumed that you have the [Apple Development Tools](http://developer.apple.com/tools/) already installed.

Furthermore, you will need to install some libraries like wxWidgets and Glew, which is easily done via [Fink](http://www.finkproject.org/). You might need to enable the [unstable tree](http://www.finkproject.org/faq/usage-fink.php#unstable) before installing any packages.

If you have OS X 10.6.x then you have to download the file by the name **fink-0.31.6.tar.gz**. Expand it, and open the folder and find the file by the name **bootstrap**. Open **bootstrap** and the application by the name Terminal will open for you. If you have OS X 10.5.x, you can actually download an installer by the name **Fink 0.9.0 Binary Installer (PowerPC)** or **Fink 0.9.0 Binary Installer (Intel)** – so those users get an easier method than OS X 10.6.x users really.

Type the command **fink selfupdate**. Chose the default answer for the question it will ask. If Fink is not responding to your commands by the way then type the command . **/sw/bin/init.sh** and Terminal will know that you are using Fink commands.

Now you have an updated Fink. You still have to tell Fink to enable the "unstable" tree which is off by default, or else the application will not work in the end. Type the command **fink configure**. Hit return for the default questions until you get to a particular question.

The "unstable" tree contains many packages not present in the "stable" tree and
often has newer versions of those that are present. All package updates are
tested in "unstable" before being approved for "stable", so "unstable" gets new
versions and bug-fixes sooner. However, that means packages in "unstable" can
change rapidly and are occasionally broken temporarily. In addition, packages
from the "unstable" tree are less likely to be available from bindist servers
immediately (if at all).

The "unstable" tree does not appear to be activated in your fink now. Do you
want to activate it? [y/N]

This time type Y for Yes before hitting the Return button instead of just hitting the Return button which has a default N for No. It will ask a few more questions which you just use the default answer by hitting the Return button.

Now you have the "unstable" tree turned on in Fink. Time to update Fink again. Type the command **fink selfupdate-rsync**. Eventually it will say that it has some new packages to install so just hit the Return button for Yes. Sit back for awhile as it downloads the newer update.

After that is done type the command **fink update-all**. Eventually it will say that it has some new packages to install so just hit the Return button for Yes. Sit back for awhile as it downloads the newest update.

Last but not least, the project will be generated through [CMake](http://www.cmake.org/cmake/resources/software.html), a meta build system that allows to create build files independent of the platform.

With these tools ready, let's install the necessary libraries first.

# Before we can start #

First of all, [checkout (download) the source code here](http://code.google.com/p/wowmodelviewer/source/checkout).

Open up your Terminal application and install wxWidgets, Glew, as well as the PNG and JPEG libraries. You can identify the exact string (in this example for PNG) by typing

```
$ fink apropos png
```

and then installing the library. Currently, you should be good to with

```
$ sudo su
$ fink install libpng3-shlibs
$ fink install libjpeg-shlibs
$ fink install wxmac28-shlibs
$ fink install glew-shlibs glew
```

If fink can't find one or more of the libraries listed above, you'll need to update fink. Running the following commands should help:

```
$ fink selfupdate
$ fink selfupdate-rsync
$ fink index -f
$ fink selfupdate
```

You should now be able to install the missing libraries.

After you're done, open the CMake application. Set the **Source** code directory to "src" in your checkout directory, and the **Build** directory to "build". Then proceed with **Configure**. If all fields contain a valid directory for libraries and headers, **Configure** again and then click **Generate**.

CMake for command line:

```
$ cd wowmodelviewer_check_out_dir/src
$ cmake .
```

You should now have an XCode project in your **Build** directory.

# Building the application #

What is left to do is to simply build the **wowmodelviewer** target. After that, your **Build** directory should contain Debug/Release/... with an app inside.

Cheers!