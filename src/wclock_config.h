#ifndef G_J6V2KW6SHY7JYVPORA1KOO8VYBF6P
#define G_J6V2KW6SHY7JYVPORA1KOO8VYBF6P
/*#define WCLOCK_SHARED*/
#ifdef WCLOCK_SHARED
# if defined _WIN32 || defined __CYGWIN__
#  ifdef WCLOCK_BUILD
#   define WCLOCK_EXTERN __declspec(dllexport)
#  else
#   define WCLOCK_EXTERN __declspec(dllimport)
#  endif
# elif __GNUC__ >= 4
#  define WCLOCK_EXTERN __attribute__ ((visibility ("default")))
# else
#  define WCLOCK_EXTERN
# endif
#else
# define WCLOCK_EXTERN
#endif
#endif
