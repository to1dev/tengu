
#ifndef QCOROCORE_EXPORT_H
#define QCOROCORE_EXPORT_H

#ifdef QCOROCORE_STATIC_DEFINE
#  define QCOROCORE_EXPORT
#  define QCOROCORE_NO_EXPORT
#else
#  ifndef QCOROCORE_EXPORT
#    ifdef QCoro5Core_EXPORTS
        /* We are building this library */
#      define QCOROCORE_EXPORT 
#    else
        /* We are using this library */
#      define QCOROCORE_EXPORT 
#    endif
#  endif

#  ifndef QCOROCORE_NO_EXPORT
#    define QCOROCORE_NO_EXPORT 
#  endif
#endif

#ifndef QCOROCORE_DEPRECATED
#  define QCOROCORE_DEPRECATED __declspec(deprecated)
#endif

#ifndef QCOROCORE_DEPRECATED_EXPORT
#  define QCOROCORE_DEPRECATED_EXPORT QCOROCORE_EXPORT QCOROCORE_DEPRECATED
#endif

#ifndef QCOROCORE_DEPRECATED_NO_EXPORT
#  define QCOROCORE_DEPRECATED_NO_EXPORT QCOROCORE_NO_EXPORT QCOROCORE_DEPRECATED
#endif

/* NOLINTNEXTLINE(readability-avoid-unconditional-preprocessor-if) */
#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef QCOROCORE_NO_DEPRECATED
#    define QCOROCORE_NO_DEPRECATED
#  endif
#endif

#endif /* QCOROCORE_EXPORT_H */
