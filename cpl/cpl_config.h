/* Define if you don't have vprintf but do have _doprnt.  */
/* #undef HAVE_DOPRNT */

/* Define if you have the vprintf function.  */
#define HAVE_VPRINTF 1

/* Define if you have the ANSI C header files.  */
#define STDC_HEADERS 1

/* Define if you have the <fcntl.h> header file.  */
#define HAVE_FCNTL_H 1

/* Define if you have the <unistd.h> header file.  */
#ifndef WIN32
#  define HAVE_UNISTD_H 1
#endif

#ifdef FME_BIG_ENDIAN
#  define WORDS_BIGENDIAN
#endif

#define CPL_DISABLE_DLL
