TEMPLATE	= app
LANGUAGE	= C++

CONFIG	+= qt warn_on debug

HEADERS	+= scpi.h \
	parse.h \
	cmdinterpret.h \
	zhserver.h \
	wmjustdata.h \
	zeraglobal.h \
	i2ceeprom.h \
	wmscpi.h \
	wm3000ud.h \
	wmuglobal.h \
	i2cutils.h \
	crcutils.h \
	intelhexfileio.h \
	justdata.h \
	gaussmatrix.h

SOURCES	+= scpi.cpp \
	parse.cpp \
	main.cpp \
	cmds.cpp \
	cmdinterpret.cpp \
	zhserver.cpp \
	wmjustdata.cpp \
	i2ceeprom.cpp \
	wm3000ud.cpp \
	i2cutils.cpp \
	crcutils.cpp \
	intelhexfileio.cpp \
	justdata.cpp \
	gaussmatrix.cpp

unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
}


