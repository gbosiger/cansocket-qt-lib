#ifndef CANSOCKETGLOBAL_H
#define CANSOCKETGLOBAL_H

#include <QtCore/qstring.h>
#include <QtCore/qglobal.h>

#ifndef QT_STATIC
#  if defined(BUILD_CANSOCKET_LIB)
#    define CANSOCKET_EXPORT Q_DECL_EXPORT
#  else
#    define CANSOCKET_EXPORT Q_DECL_IMPORT
#  endif
#else
#  define CANSOCKET_EXPORT
#endif

#endif // CANSOCKETGLOBAL_H
