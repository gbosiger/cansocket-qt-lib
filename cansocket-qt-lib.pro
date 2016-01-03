lessThan(QT_MAJOR_VERSION, 5) {
    message("Cannot build CanSocket-Qt-lib sources with Qt version $${QT_VERSION}.")
    error("Use at least Qt 5.0.0.")
}

requires(linux)

load(qt_parts)

