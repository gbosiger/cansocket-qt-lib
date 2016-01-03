TEMPLATE = subdirs
SUBDIRS = canframe cmake

!contains(QT_CONFIG, private_tests): SUBDIRS -= \
	canframedata
