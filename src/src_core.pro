# samplv1.pro
#
NAME = samplv1

TARGET = $${NAME}
TEMPLATE = lib
CONFIG += shared

include(src_core.pri)

HEADERS = \
	config.h \
	samplv1.h \
	samplv1_config.h \
	samplv1_filter.h \
	samplv1_formant.h \
	samplv1_resampler.h \
	samplv1_sample.h \
	samplv1_wave.h \
	samplv1_ramp.h \
	samplv1_list.h \
	samplv1_fx.h \
	samplv1_reverb.h \
	samplv1_param.h \
	samplv1_sched.h \
	samplv1_tuning.h \
	samplv1_programs.h \
	samplv1_controls.h

SOURCES = \
	samplv1.cpp \
	samplv1_config.cpp \
	samplv1_formant.cpp \
	samplv1_resampler.cpp \
	samplv1_sample.cpp \
	samplv1_wave.cpp \
	samplv1_param.cpp \
	samplv1_sched.cpp \
	samplv1_tuning.cpp \
	samplv1_programs.cpp \
	samplv1_controls.cpp


unix {

	OBJECTS_DIR = .obj_core
	MOC_DIR     = .moc_core
	UI_DIR      = .ui_core

	isEmpty(PREFIX) {
		PREFIX = /usr/local
	}

	isEmpty(LIBDIR) {
		TARGET_ARCH = $$system(uname -m)
		contains(TARGET_ARCH, x86_64) {
			LIBDIR = $${PREFIX}/lib64
		} else {
			LIBDIR = $${PREFIX}/lib
		}
	}

	INSTALLS += target

	target.path = $${LIBDIR}
}

QT -= gui
QT += xml
