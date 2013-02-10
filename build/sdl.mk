ifeq ($(TARGET),ANDROID)
# Android must use OpenGL
ENABLE_SDL = n
# UNIX/Linux defaults to OpenGL, but can use SDL_gfx instead
else ifeq ($(HAVE_WIN32),y)
# Windows defaults to GDI
ENABLE_SDL ?= n
else
# everything else defaults to SDL
ENABLE_SDL ?= y
endif

ifeq ($(ENABLE_SDL),y)

SDL_PKG = sdl

ifeq ($(LIBPNG),n)
SDL_PKG += SDL_image
endif

ifeq ($(FREETYPE),n)
SDL_PKG += SDL_ttf
endif

ifeq ($(OPENGL),n)
SDL_PKG += SDL_gfx
endif

$(eval $(call pkg-config-library,SDL,$(SDL_PKG)))

SDL_CPPFLAGS += -DENABLE_SDL

ifeq ($(TARGET_IS_DARWIN),y)
# { TARGET_IS_DARWIN
# the pkg-config file on MacPorts is broken, we must filter out the
# -lSDL flag manually
SDL_LDLIBS := $(filter-out -l%,$(SDL_LDLIBS))
SDL_LDADD = /opt/local/lib/libSDL_ttf.a /opt/local/lib/libfreetype.a
SDL_LDADD += /opt/local/lib/libbz2.a /opt/local/lib/libz.a
SDL_LDADD += /opt/local/lib/libfreetype.a
SDL_LDADD += /opt/local/lib/libxcb.a /opt/local/lib/libXau.a /opt/local/lib/libXdmcp.a
endif
endif
