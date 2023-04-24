## Process this file with automake to produce Makefile.in



AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)."\" \
	-DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\" \
	$(GTKMM_CFLAGS) \
	$(GTHREAD_CFLAGS) \
	$(KEYBINDER_CFLAGS) \
	$(ALSA_CFLAGS)

AM_CFLAGS= \
	-Wall

AM_CXXFLAGS =\
	-Wall

bin_PROGRAMS = mixa

mixa_SOURCES = \
	resources.c \
	MixaApp.cpp \
	MixaApp.hpp \
	MixaAppWindow.cpp \
	MixaAppWindow.hpp \
	CardFrame.cpp \
	CardFrame.hpp \
	AlsaCard.cpp \
	AlsaCard.hpp \
	AlsaMixer.cpp \
	AlsaMixer.hpp \
	AlsaElement.cpp \
	AlsaElement.hpp \
	AlsaControl.cpp \
	AlsaControl.hpp \
	AlsaEvents.cpp \
	AlsaEvents.hpp \
	ModeComponent.cpp \
	ModeComponent.hpp \
	ControlComponents.cpp \
	ControlComponents.hpp \
	ElementComponent.cpp \
	ElementComponent.hpp


if KEYBINDER
mixa_SOURCES += \
	Keybind.cpp \
	Keybind.hpp 
endif

mixa_LDFLAGS =

mixa_LDADD = $(MIXA_LIBS) \
	$(GLIBMM_LIBS) \
	$(GTKMM_LIBS) \
	$(GTHREAD_LIBS) \
	$(KEYBINDER_LIBS) \
	$(ALSA_LIBS)


# Remove ui directory on uninstall
uninstall-local:
	-rm -r $(pkgdatadir)
