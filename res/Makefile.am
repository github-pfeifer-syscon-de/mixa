PKGCONFIG = pkgconf
GLIB_COMPILE_RESOURCES = $(shell $(PKGCONFIG) --variable=glib_compile_resources gio-2.0)

MIXA_RESOURCE_XML=mixa.gresource.xml
MIXA_RESOURCES=$(shell $(GLIB_COMPILE_RESOURCES) --sourcedir=. --generate-dependencies $(MIXA_RESOURCE_XML))
resources_c=../src/resources.c

all:$(resources_c)

$(resources_c): $(MIXA_RESOURCE_XML) $(MIXA_RESOURCES)
	$(GLIB_COMPILE_RESOURCES) --target=$@ --sourcedir=$(dir $<) --generate-source $<

# The desktop files
desktopdir = $(datadir)/applications
desktop_in_files=mixa.desktop.in
desktop_DATA=$(desktop_in_files:.desktop.in=.desktop)

# application icon
appicondir = $(datadir)/icons/hicolor/64x64/apps
appicon_DATA = mixa.png
appdir_icon=$(appicondir)/$(appicon_DATA)


EXTRA_DIST = $(MIXA_RESOURCES) $(MIXA_RESOURCE_XML)  $(appicon_DATA) $(desktop_in_files)

clean-local:
	-rm -rf $(resources_c)
	-rm -rf $(desktop_DATA)

%.desktop: %.desktop.in
	@regex_cmd@ -e "s,\@APP_ICON\@,$(appdir_icon),g" $? > $@
