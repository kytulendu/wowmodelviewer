/*
	This file hosts our version numbers, and any other "static" variables that we might need to change between builds.
	Also hosts our OS-Dependent global variables.
*/
#ifndef WMV_VERSION_H
#define WMV_VERSION_H

#include <QObject>
#include <QString>

#define PROGRAMNAME QObject::tr("World of Warcraft Model Viewer")
#define MAJORVERSION QString("v0.8.0.0")
#define BUILDVERSION QString(" r611")

#if defined (_WINDOWS)
	#if defined (_WIN64)
		#define SYSTEMVERSION QObject::tr("Windows 64-bit")
	#elif defined (_WIN32)
		#define SYSTEMVERSION QObject::tr("Windows 32-bit")
	#else
		#error QObject::tr("Your Windows platform is not defined. Please specify either _WIN64 or _WIN32.")
	#endif
#elif defined (_MAC)
	#if  defined (_MAC_INTEL)
		#define SYSTEMVERSION QObject::tr("Macintosh Intel")
	#elif defined (_MAC_PPC)
		#define SYSTEMVERSION QObject::tr("Macintosh PowerPC")
	#else
		#error QObject::tr("Your Macintosh platform is not defined. Please specify either _MAC_INTEL or _MAC_PPC.")
	#endif
#elif defined (_LINUX)
	#if defined (_LINUX64)
		#define SYSTEMVERSION QObject::tr("Linux 64-bit")
	#elif defined (_LINUX32)
		#define SYSTEMVERSION QObject::tr("Linux 32-bit")
	#else
		#error QObject::tr("Your Linux platform is not defined. Please specify either _LINUX64 or _LINUX32.")
	#endif
#else
	#ifdef _DEBUG
		#error QObject::tr("You have not specified a valid Operating System for your Debug configuration.")
	#else
		#error QObject::tr("You have not specified a valid Operating System for your Release configuration.")
	#endif
#endif

#ifdef _DEBUG
	#define DEBUGVERSION QObject::tr(" Debug")
#else
	#define DEBUGVERSION ""
#endif

#endif