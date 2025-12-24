/*************************************************************************************************/
/**
	version.h
*/
/*************************************************************************************************/

#ifndef VERSION_H_
#define VERSION_H_

#define COPYRIGHT_YEAR 2025

// The version numbers
#define MAJOR_VERSION 1
#define MINOR_VERSION 11

// Something appended to the version, e.g. "rc2" for a release candidate
#define SPECIAL_VERSION ""

// Currently unused, though they are displayed in Explorer by some versions of Windows
#define RELEASE_NUMBER 0
#define BUILD_NUMBER 0

#define STRINGIZE(x) #x
#define EXPAND_TO_STRING(x) STRINGIZE(x)

#define VERSION EXPAND_TO_STRING(MAJOR_VERSION) "." EXPAND_TO_STRING(MINOR_VERSION) SPECIAL_VERSION

#endif // VERSION_H_
