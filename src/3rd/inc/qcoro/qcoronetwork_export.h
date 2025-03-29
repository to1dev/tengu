
#ifndef QCORONETWORK_EXPORT_H
#define QCORONETWORK_EXPORT_H

#ifdef QCORONETWORK_STATIC_DEFINE
#  define QCORONETWORK_EXPORT
#  define QCORONETWORK_NO_EXPORT
#else
#  ifndef QCORONETWORK_EXPORT
#    ifdef QCoro5Network_EXPORTS
        /* We are building this library */
#      define QCORONETWORK_EXPORT 
#    else
        /* We are using this library */
#      define QCORONETWORK_EXPORT 
#    endif
#  endif

#  ifndef QCORONETWORK_NO_EXPORT
#    define QCORONETWORK_NO_EXPORT 
#  endif
#endif

#ifndef QCORONETWORK_DEPRECATED
#  define QCORONETWORK_DEPRECATED __declspec(deprecated)
#endif

#ifndef QCORONETWORK_DEPRECATED_EXPORT
#  define QCORONETWORK_DEPRECATED_EXPORT QCORONETWORK_EXPORT QCORONETWORK_DEPRECATED
#endif

#ifndef QCORONETWORK_DEPRECATED_NO_EXPORT
#  define QCORONETWORK_DEPRECATED_NO_EXPORT QCORONETWORK_NO_EXPORT QCORONETWORK_DEPRECATED
#endif

/* NOLINTNEXTLINE(readability-avoid-unconditional-preprocessor-if) */
#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef QCORONETWORK_NO_DEPRECATED
#    define QCORONETWORK_NO_DEPRECATED
#  endif
#endif

#endif /* QCORONETWORK_EXPORT_H */
