include_directories(${XCSOAR_SRC})
add_definitions(-DHAVE_POSIX) # For POSIX only
add_definitions(-DZLIB_CONST) # Make some of zlib const.
add_definitions(-DUSE_FREETYPE)

set(OUTPUT_DATA ${OUTPUT_BASE}/data)

# These are for the 'UNIX' target
set(GLX false)
set(OPENGL true)
set(FREETYPE true)
set(HAVE_POSIX true)
set(EGL false)
set(ENABLE_SDL false)
set(ENABLE_ALSA true)
set(VFB false)
set(USE_FB false)
set(USE_SDL true)
set(HAVE_WIN32 false)
set(HAVE_CE false)
set(USE_MEMORY_CANVAS ENABLE_SDL and not OPENGL)
set(GLES2 false)
set(GLSL ${GLES2})
set(HAVE_PCM_PLAYER true)

# TODO this needs to be moved downtree.
if(false)
add_custom_command(OUTPUT ${OUTPUT_INCLUDE}/resource.h
                   COMMAND cat ${XCSOAR_SRC}/Resources.hpp | ${TOOLS_DIR}/MakeResourceDefines > ${OUTPUT_INCLUDE}/resource.h
                   DEPENDS ${XCSOAR_SRC}/Resources.hpp
                   DEPENDS ${TOOLS_DIR}/MakeResourceDefines
                   VERBATIM
                   )

set(TARGET_CPPFLAGS -I${OUTPUT_INCLUDE} "-DHAVE_POSIX -DHAVE_STDINT_H -DHAVE_UNISTD_H -DHAVE_VASPRINTF -DRADIANS -DEYE_CANDY -DUSE_WGS84 -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC")
set(OPENGL_CPPFLAGS "-DENABLE_OPENGL -DGL_GLEXT_PROTOTYPES")
set(SCREEN_CPPFLAGS "-DUSE_FREETYPE -DENABLE_OPENGL -DGL_GLEXT_PROTOTYPES -DUSE_GLX -DUSE_X11 -DUSE_POLL_EVENT")
add_custom_command(OUTPUT ${OUTPUT_BASE}/XCSoar.rc
                   COMMAND cat ${XCSOAR}/Data/XCSoar.rc | ${CMAKE_C_COMPILER} -E -o ${OUTPUT_BASE}/XCSoar.rc -I${OUTPUT_INCLUDE} ${TARGET_CPPFLAGS} ${SCREEN_CPPFLAGS} -
                   DEPENDS ${XCSOAR}/Data/XCSoar.rc
                   DEPENDS ${OUTPUT_INCLUDE}/resource.h
                   VERBATIM)
add_custom_command(OUTPUT ${OUTPUT_INCLUDE}/resource_data.h
                   COMMAND perl ${TOOLS_DIR}/GenerateResources.pl < ${OUTPUT_BASE}/XCSoar.rc > ${OUTPUT_INCLUDE}/resource_data.h
                   DEPENDS ${OUTPUT_BASE}/XCSoar.rc
                   DEPENDS ${TOOLS_DIR}/GenerateResources.pl
                   VERBATIM)
set_property(SOURCE ${OUTPUT_INCLUDE}/resource_data.h
             PROPERTY GENERATED true)
add_custom_command(OUTPUT ${OUTPUT_BASE}/build/lib/resources.a
                   COMMAND perl ${TOOLS_DIR}/LinkResources.pl ${OUTPUT_BASE}/XCSoar.rc ${OUTPUT_BASE}/build/lib/resources.a as ${ASFLAGS} ar ${ARFLAGS}
                   DEPENDS ${OUTPUT_BASE}/XCSoar.rc
                   DEPENDS ${TOOLS_DIR}/LinkResources.pl
                   VERBATIM)
add_custom_target(resources ALL
                  DEPENDS ${OUTPUT_INCLUDE}/resource_data.h)
endif(false)

add_definitions(-DHAVE_POSIX
                -DHAVE_STDINT_H
                -DHAVE_UNISTD_H
                -DHAVE_VASPRINTF
                -DRADIANS
                -DEYE_CANDY
                -DUSE_WGS84
                -DUSE_FREETYPE
                -DENABLE_OPENGL
                -DENABLE_ALSA
                -DGL_GLEXT_PROTOTYPES
                -DUSE_GLX
                -DUSE_X11
                -DUSE_POLL_EVENT
                )
set(ASFLAGS "-arch i386")
set(ARFLAGS "-rcs")

include_directories(SYSTEM /usr/include/SDL)

set(MO_DIR ${OUTPUT_BASE}/po)
add_custom_target(make_output_po
                  COMMAND [ -d ${MO_DIR} ] || mkdir ${MO_DIR}
                 )
set(MO_SRCS
            ca
            he
            nl
            sr
            cs
            hr
            pl
            sv
            da
            hu
            pt
            tr
            de
            it
            pt_BR
            uk
            el
            ja
            ro
            vi
            es
            ko
            ru
            zh_CN
            fi
            lt
            sk
            fr
            nb
            sl
   )
foreach(mo IN LISTS MO_SRCS)
  set(fqs ${MO_DIR}/${mo}.mo)
  set(fqd ${OUTPUT_BASE}/include/${mo}.mo.c)
  add_custom_command(OUTPUT ${fqd}
                     COMMAND msgfmt --check -o ${fqs} ${XCSOAR}/po/${mo}.po
                     COMMAND perl -I${XCSOAR}/tools ${TOOLS_DIR}/BinToC.pl ${fqs} ${fqd}
                     DEPENDS make_output_po
                    )
  set(DATA_SRCS ${DATA_SRCS} ${fqd})
endforeach(mo)
set(DATA_RESOURCES 
                   COPYING
                   AUTHORS
   )
foreach(dr IN LISTS DATA_RESOURCES)
  set(fqs ${OUTPUT_DATA}/${dr}.gz)
  set(fqd ${OUTPUT_DATA}/${dr}.gz.c)
  add_custom_command(OUTPUT ${fqd}
                     COMMAND perl -I${XCSOAR}/tools ${TOOLS_DIR}/BinToC.pl ${fqs} ${fqd}
                    )
  set(DATA_SRCS ${DATA_SRCS} ${fqd})
endforeach(dr)
add_custom_command(OUTPUT ${OUTPUT_DATA}/egm96s.dem.c
                   COMMAND perl -I${XCSOAR}/tools ${TOOLS_DIR}/BinToC.pl ${XCSOAR}/Data/other/egm96s.dem ${OUTPUT_DATA}/egm96s.dem.c 
                  )
add_library(Data ${DATA_SRCS} ${OUTPUT_DATA}/egm96s.dem.c)
add_custom_target(lib_debug
                  COMMAND echo "PWD: `pwd`"
                 )

add_subdirectory(release)
add_subdirectory(debug)
add_subdirectory(sanitize)

# Patch panel
add_custom_target(lib-release  DEPENDS lib-linux-release)
add_custom_target(lib-debug    DEPENDS lib-linux-debug)
add_custom_target(lib-sanitize DEPENDS lib-linux-sanitize)
