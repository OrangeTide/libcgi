# used by things that link to libcgi

ifndef TARGET_OS
TARGET_OS:=$(shell uname -s)
endif

ifeq (${TARGET_OS}, CYGWIN_NT-5.1)
TARGET_OS:=MINGW32_NT-5.1
CFLAGS+=-mno-cygwin
endif

ifeq (${TARGET_OS}, Darwin)
## OS X 
else
ifeq (${TARGET_OS}, MINGW32_NT-5.1)
## Windows build
else
## GLX build
endif
endif

LIBCGI_CFLAGS:=$(CFLAGS)
LIBCGI_CPPFLAGS+=-I$(LIBCGI_DIR)
export LIBCGI_CPPFLAGS
export LIBCGI_CFLAGS

ifdef LIBCGI_DIR

$(LIBCGI_DIR)/libcgi.a :
	$(MAKE) -C $(LIBCGI_DIR) libcgi.a

clean ::
	$(MAKE) -C $(LIBCGI_DIR) clean

endif
