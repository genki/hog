/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.ac by autoheader.  */

/* Define if building universal (internal helper macro) */
/* #undef AC_APPLE_UNIVERSAL_BUILD */

/* "specified configure options" */
#define CONFIGURE_OPTIONS " '--prefix=/usr' '--without-onigmo' '--without-pcre' '--disable-maintainer-mode' '--disable-dependency-tracking' '--disable-groonga-httpd' '--disable-documenta' '--disable-zeromq' 'CFLAGS=-g -O2 -fPIE -fstack-protector-strong -Wformat -Werror=format-security' 'LDFLAGS=-Wl,-Bsymbolic-functions -fPIE -pie -Wl,-z,relro -Wl,-z,now' 'CPPFLAGS=-Wdate-time -D_FORTIFY_SOURCE=2' 'CXXFLAGS=-g -O2 -fPIE -fstack-protector-strong -Wformat -Werror=format-security'"

/* Default command line option configuration file. */
#define GRN_CONFIG_PATH "/usr/etc/groonga/groonga.conf"

/* "specified default DB key management algorithm" */
#define GRN_DEFAULT_DB_KEY "auto"

/* "specified default encoding" */
#define GRN_DEFAULT_ENCODING "utf8"

/* "specified match escalation threshold" */
#define GRN_DEFAULT_MATCH_ESCALATION_THRESHOLD 0

/* lock timeout */
#define GRN_LOCK_TIMEOUT 900000

/* lock wait time in nanosecond */
#define GRN_LOCK_WAIT_TIME_NANOSECOND 1000000

/* "plugin suffix" */
#define GRN_PLUGIN_SUFFIX ".so"

/* "The relative synonyms file for TSV query expander" */
#define GRN_QUERY_EXPANDER_TSV_RELATIVE_SYNONYMS_FILE "etc/groonga/synonyms.tsv"

/* "The default synonyms file for TSV query expander" */
#define GRN_QUERY_EXPANDER_TSV_SYNONYMS_FILE "/usr/etc/groonga/synonyms.tsv"

/* stack size */
#define GRN_STACK_SIZE 1024

/* groonga version */
#define GRN_VERSION "6.1.4-172-g459cb94"

/* Define to 1 if benchamrk is available */
/* #undef GRN_WITH_BENCHMARK */

/* Define to 1 if you use Cutter */
/* #undef GRN_WITH_CUTTER */

/* use KyTea */
/* #undef GRN_WITH_KYTEA */

/* Use libedit with multibyte support. */
/* #undef GRN_WITH_LIBEDIT */

/* Define to 1 if libevent is available. */
/* #undef GRN_WITH_LIBEVENT */

/* use libstemmer */
/* #undef GRN_WITH_LIBSTEMMER */

/* Support data compression by LZ4. */
/* #undef GRN_WITH_LZ4 */

/* use MeCab */
/* #undef GRN_WITH_MECAB */

/* Define to 1 if MessagePack is available. */
#define GRN_WITH_MESSAGE_PACK 1

/* Define to 1 if mruby is enabled. */
/* #undef GRN_WITH_MRUBY */

/* compile with nfkc.c */
#define GRN_WITH_NFKC 1

/* Use Onigmo. */
/* #undef GRN_WITH_ONIGMO */

/* Define to 1 if ZeroMQ is available. */
/* #undef GRN_WITH_ZEROMQ */

/* Support data compression by zlib. */
/* #undef GRN_WITH_ZLIB */

/* Support data compression by Zstandard. */
/* #undef GRN_WITH_ZSTD */

/* Define to 1 if you have the `backtrace' function. */
/* #undef HAVE_BACKTRACE */

/* use clock_gettime */
#define HAVE_CLOCK_GETTIME 1

/* Define to 1 if you have the <dirent.h> header file. */
#define HAVE_DIRENT_H 1

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Define to 1 if you have the <errno.h> header file. */
#define HAVE_ERRNO_H 1

/* Define to 1 if you have the <execinfo.h> header file. */
/* #undef HAVE_EXECINFO_H */

/* use fpclassify with _ISOC99_SOURCE */
#define HAVE_FPCLASSIFY 1

/* Define to 1 if you have the `gmtime_r' function. */
#define HAVE_GMTIME_R 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the <libstemmer.h> header file. */
/* #undef HAVE_LIBSTEMMER_H */

/* Define to 1 if you have the <linux/futex.h> header file. */
/* #undef HAVE_LINUX_FUTEX_H */

/* Define to 1 if you have the `localtime_r' function. */
#define HAVE_LOCALTIME_R 1

/* Define to 1 if MeCab has the type `mecab_dictionary_info_t'. */
/* #undef HAVE_MECAB_DICTIONARY_INFO_T */

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the `mkstemp' function. */
#define HAVE_MKSTEMP 1

/* Define to 1 if you have the <netdb.h> header file. */
#define HAVE_NETDB_H 1

/* Define to 1 if you have the `pthread_condattr_setpshared' function. */
#define HAVE_PTHREAD_CONDATTR_SETPSHARED 1

/* Define to 1 if you have the <pthread.h> header file. */
#define HAVE_PTHREAD_H 1

/* Define to 1 if you have the `pthread_mutexattr_setpshared' function. */
#define HAVE_PTHREAD_MUTEXATTR_SETPSHARED 1

/* Define to 1 if you have the <signal.h> header file. */
#define HAVE_SIGNAL_H 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the `strcasecmp' function. */
#define HAVE_STRCASECMP 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the `strncasecmp' function. */
#define HAVE_STRNCASECMP 1

/* Define to 1 if you have the `strtoull' function. */
#define HAVE_STRTOULL 1

/* Define to 1 if you have the <sys/mman.h> header file. */
#define HAVE_SYS_MMAN_H 1

/* Define to 1 if you have the <sys/param.h> header file. */
#define HAVE_SYS_PARAM_H 1

/* Define to 1 if you have the <sys/resource.h> header file. */
#define HAVE_SYS_RESOURCE_H 1

/* Define to 1 if you have the <sys/select.h> header file. */
/* #undef HAVE_SYS_SELECT_H */

/* Define to 1 if you have the <sys/socket.h> header file. */
#define HAVE_SYS_SOCKET_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/syscall.h> header file. */
/* #undef HAVE_SYS_SYSCALL_H */

/* Define to 1 if you have the <sys/sysctl.h> header file. */
/* #undef HAVE_SYS_SYSCTL_H */

/* Define to 1 if you have the <sys/time.h> header file. */
#define HAVE_SYS_TIME_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <sys/wait.h> header file. */
#define HAVE_SYS_WAIT_H 1

/* Define to 1 if you have the <time.h> header file. */
#define HAVE_TIME_H 1

/* Define to 1 if you have the <ucontext.h> header file. */
#define HAVE_UCONTEXT_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define to 1 if you have the `_gmtime64_s' function. */
/* #undef HAVE__GMTIME64_S */

/* Define to 1 if you have the `_localtime64_s' function. */
/* #undef HAVE__LOCALTIME64_S */

/* Define to 1 if you have the `_strtoui64' function. */
/* #undef HAVE__STRTOUI64 */

/* host CPU */
#define HOST_CPU "x86_64"

/* host OS */
#define HOST_OS "linux-gnu"

/* Define to the sub-directory where libtool stores uninstalled libraries. */
#define LT_OBJDIR ".libs/"

/* Name of package */
#define PACKAGE "groonga"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "groonga@razil.jp"

/* Label of package */
#define PACKAGE_LABEL "Groonga"

/* Define to the full name of this package. */
#define PACKAGE_NAME "groonga"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "groonga 7.0.1"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "groonga"

/* Define to the home page for this package. */
#define PACKAGE_URL ""

/* Define to the version of this package. */
#define PACKAGE_VERSION "7.0.1"

/* The size of `off_t', as computed by sizeof. */
#define SIZEOF_OFF_T 8

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Define to 1 if you enable dynamic malloc change */
/* #undef USE_DYNAMIC_MALLOC_CHANGE */

/* use epoll */
#define USE_EPOLL 1

/* alloc_count with atomic */
#define USE_EXACT_ALLOC_COUNT 1

/* use fmalloc */
/* #undef USE_FAIL_MALLOC */

/* use futex */
/* #undef USE_FUTEX */

/* use kqueue */
/* #undef USE_KQUEUE */

/* use MAP_HUGETLB */
/* #undef USE_MAP_HUGETLB */

/* Define to 1 if you enable debuging memory management */
/* #undef USE_MEMORY_DEBUG */

/* use MSG_MORE */
/* #undef USE_MSG_MORE */

/* use MSG_NOSIGNAL */
/* #undef USE_MSG_NOSIGNAL */

/* use poll */
/* #undef USE_POLL */

/* use abort */
/* #undef USE_QUERY_ABORT */

/* use select */
/* #undef USE_SELECT */

/* Version number of package */
#define VERSION "7.0.1"

/* Define WORDS_BIGENDIAN to 1 if your processor stores words with the most
   significant byte first (like Motorola and SPARC, unlike Intel). */
#if defined AC_APPLE_UNIVERSAL_BUILD
# if defined __BIG_ENDIAN__
#  define WORDS_BIGENDIAN 1
# endif
#else
# ifndef WORDS_BIGENDIAN
/* #  undef WORDS_BIGENDIAN */
# endif
#endif

/* Enable large inode numbers on Mac OS X 10.5.  */
#ifndef _DARWIN_USE_64_BIT_INODE
# define _DARWIN_USE_64_BIT_INODE 1
#endif

/* Number of bits in a file offset, on hosts where this is settable. */
/* #undef _FILE_OFFSET_BITS */

/* Define to 1 if you use GCC. */
#define _GNU_SOURCE 1

/* Define to 1 for fpclassify */
/* #undef _ISOC99_SOURCE */

/* Define for large files, on AIX-style hosts. */
/* #undef _LARGE_FILES */

/* Define to 1 if you are on NetBSD. */
/* #undef _NETBSD_SOURCE */

/* Define to 1 for msghdr.msg_control if you are on Solaris. */
/* #undef _XPG4_2 */

/* Define to 1 for singal.h with _XPG4_2 if you are on Solaris. */
/* #undef __EXTENSIONS__ */

/* Define to `long int' if <sys/types.h> does not define. */
/* #undef off_t */

/* Define to `unsigned int' if <sys/types.h> does not define. */
/* #undef size_t */
