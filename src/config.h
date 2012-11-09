/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.ac by autoheader.  */

/* Official build number as a number */
#define BUILDNR 0

/* Official build number as a VERSION suffix string, e.g. "+build123", or ""
   if this is not an official build. */
#define BUILDNR_SUFFIX ""

/* define if the Boost library is available */
#define HAVE_BOOST /**/

/* define if the Boost::Filesystem library is available */
#define HAVE_BOOST_FILESYSTEM /**/

/* define if the Boost::System library is available */
#define HAVE_BOOST_SYSTEM /**/

/* define if the Boost::Unit_Test_Framework library is available */
#define HAVE_BOOST_UNIT_TEST_FRAMEWORK /**/

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Define to 1 if you have the `fdatasync' function. */
#define HAVE_FDATASYNC 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have __malloc_hook, __realloc_hook, and __free_hook in
   <malloc.h>. */
#define HAVE_MALLOC_HOOKS 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Provide macro indicating the platform supports netlink */
#define HAVE_NETLINK 1

/* Provide macro indicating the platform has pcap */
#define HAVE_PCAP 1

/* Define to 1 if you have the `ppoll' function. */
#define HAVE_PPOLL 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define if thread-local storage with __thread is supported */
#define HAVE_TLS 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Defined if the module is enabled */
#define HAVE_coreapps 1

/* Defined if the module is enabled */
#define HAVE_netapps 1

/* Define to the sub-directory in which libtool stores uninstalled libraries.
   */
#define LT_OBJDIR ".libs/"

/* Name of package */
#define PACKAGE "nox"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "contact@noxrepo.org"

/* Define to the full name of this package. */
#define PACKAGE_NAME "nox"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "nox 0.9.0(zaku)~full~beta"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "nox"

/* Define to the home page for this package. */
#define PACKAGE_URL ""

/* Define to the version of this package. */
#define PACKAGE_VERSION "0.9.0(zaku)~full~beta"

/* Define to 'const' if SSL_CTX_new() takes a const argument */
#define SSL_CONST /**/

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Provide macro indicating that twisted python was enabled */
#define TWISTED_ENABLED 1

/* Provide macro indicating the preference to use ltdl */
/* #undef USE_LTDL */

/* Enable extensions on AIX 3, Interix.  */
#ifndef _ALL_SOURCE
# define _ALL_SOURCE 1
#endif
/* Enable GNU extensions on systems that have them.  */
#ifndef _GNU_SOURCE
# define _GNU_SOURCE 1
#endif
/* Enable threading extensions on Solaris.  */
#ifndef _POSIX_PTHREAD_SEMANTICS
# define _POSIX_PTHREAD_SEMANTICS 1
#endif
/* Enable extensions on HP NonStop.  */
#ifndef _TANDEM_SOURCE
# define _TANDEM_SOURCE 1
#endif
/* Enable general extensions on Solaris.  */
#ifndef __EXTENSIONS__
# define __EXTENSIONS__ 1
#endif


/* Version number of package */
#define VERSION "0.9.0(zaku)~full~beta"

/* Number of bits in a file offset, on hosts where this is settable. */
#define _FILE_OFFSET_BITS 64

/* Define for large files, on AIX-style hosts. */
/* #undef _LARGE_FILES */

/* Define to 1 if on MINIX. */
/* #undef _MINIX */

/* Define to 2 if the system does not provide POSIX.1 features except with
   this defined. */
/* #undef _POSIX_1_SOURCE */

/* Define to 1 if you need to in order for `stat' and other things to work. */
/* #undef _POSIX_SOURCE */

/* NOX version with build number, e.g. "1.2.3+build456". */
#define NOX_VERSION VERSION BUILDNR_SUFFIX
