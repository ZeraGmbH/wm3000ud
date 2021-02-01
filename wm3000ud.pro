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
	gaussmatrix.h \
        justdatabase.h \
    wmjustdatabase.h \
    wmjustdatav215.h \
    justdatav215.h

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
	gaussmatrix.cpp \
    justdatabase.cpp \
    wmjustdatav215.cpp \
    justdatav215.cpp

unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
}

target.path = /usr/bin
INSTALLS += target





#The following line was inserted by qt3to4
QT += xml  qt3support 
