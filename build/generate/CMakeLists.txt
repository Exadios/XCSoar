
add_custom_target(generate ALL
                  COMMAND [ -d ${OUTPUT_INCLUDE} ] || mkdir ${OUTPUT_INCLUDE}
                  DEPENDS ${OUTPUT_INCLUDE}/InputEvents_altair.cpp
                          ${OUTPUT_INCLUDE}/InputEvents_default.cpp
                          ${OUTPUT_INCLUDE}/MathTables.h
                          ${OUTPUT_INCLUDE}/InputEvents_Text2Event.cpp
                          ${OUTPUT_INCLUDE}/InputEvents_Text2GCE.cpp
                          ${OUTPUT_INCLUDE}/InputEvents_Text2GCE.cpp
                          ${OUTPUT_INCLUDE}/InputEvents_Text2NE.cpp
                          ${OUTPUT_INCLUDE}/Status_defaults.cpp
                          ${OUTPUT_INCLUDE}/resource.h
                          ${OUTPUT_INCLUDE}/resource_data.h
                  VERBATIM)
add_dependencies(generate tools)

add_custom_command(OUTPUT ${OUTPUT_INCLUDE}/MathTables.h
                   COMMAND [ -d ${OUTPUT_INCLUDE} ] || mkdir ${OUTPUT_INCLUDE}
                   COMMAND ${TOOLS_BIN}/generatesinetables > ${OUTPUT_INCLUDE}/MathTables.h || rm ${OUTPUT_INCLUDE}/MathTables.h
                   DEPENDS ${TOOLS_BIN}/generatesinetables)

add_custom_command(OUTPUT ${OUTPUT_INCLUDE}/InputEvents_altair.cpp
                   COMMAND [ -d ${OUTPUT_INCLUDE} ] || mkdir ${OUTPUT_INCLUDE}
                   COMMAND perl ${TOOLS_DIR}/xci2cpp.pl ${XCSOAR}/Data/Input/default.xci ${XCSOAR}/Data/Input/altair.xci > ${OUTPUT_INCLUDE}/InputEvents_altair.cpp
                   DEPENDS ${TOOLS_DIR}/xci2cpp.pl)

add_custom_command(OUTPUT ${OUTPUT_INCLUDE}/InputEvents_default.cpp
                   COMMAND [ -d ${OUTPUT_INCLUDE} ] || mkdir ${OUTPUT_INCLUDE}
                   COMMAND perl ${TOOLS_DIR}/xci2cpp.pl ${XCSOAR}/Data/Input/default.xci > ${OUTPUT_INCLUDE}/InputEvents_default.cpp
                   DEPENDS ${TOOLS_DIR}/xci2cpp.pl)

add_custom_command(OUTPUT ${OUTPUT_INCLUDE}/InputEvents_Text2Event.cpp
                   COMMAND [ -d ${OUTPUT_INCLUDE} ] || mkdir ${OUTPUT_INCLUDE}
                   COMMAND perl ${TOOLS_DIR}/Text2Event.pl ${XCSOAR_SRC}/Input/InputEvents.hpp > ${OUTPUT_INCLUDE}/InputEvents_Text2Event.cpp
                   DEPENDS ${TOOLS_DIR}/Text2Event.pl)

add_custom_command(OUTPUT ${OUTPUT_INCLUDE}/InputEvents_Text2GCE.cpp
                   COMMAND [ -d ${OUTPUT_INCLUDE} ] || mkdir ${OUTPUT_INCLUDE}
                   COMMAND perl ${TOOLS_DIR}/Text2GCE.pl ${XCSOAR_SRC}/Input/InputQueue.hpp > ${OUTPUT_INCLUDE}/InputEvents_Text2GCE.cpp
                   DEPENDS ${TOOLS_DIR}/Text2GCE.pl)

add_custom_command(OUTPUT ${OUTPUT_INCLUDE}/InputEvents_Text2NE.cpp
                   COMMAND [ -d ${OUTPUT_INCLUDE} ] || mkdir ${OUTPUT_INCLUDE}
                   COMMAND perl ${TOOLS_DIR}/Text2NE.pl ${XCSOAR_SRC}/Input/InputQueue.hpp > ${OUTPUT_INCLUDE}/InputEvents_Text2NE.cpp
                   DEPENDS  ${TOOLS_DIR}/Text2NE.pl)

add_custom_command(OUTPUT ${OUTPUT_INCLUDE}/Status_defaults.cpp
                   COMMAND [ -d ${OUTPUT_INCLUDE} ] || mkdir ${OUTPUT_INCLUDE}
                   COMMAND perl ${TOOLS_DIR}/xcs2cpp.pl ${XCSOAR}/Data/Status/default.xcs > ${OUTPUT_INCLUDE}/Status_defaults.cpp
                   DEPENDS ${TOOLS_DIR}/xcs2cpp.pl)

#add_custom_command(OUTPUT ${OUTPUT_INCLUDE}/resource.h
#                   COMMAND [ -d ${OUTPUT_INCLUDE} ] || mkdir ${OUTPUT_INCLUDE}
#                   COMMAND ${TOOLS_DIR}/GenerateOldResources.pl < src/Resources.hpp > ${OUTPUT_INCLUDE}/resource.h.tmp
#                   COMMAND mv ${OUTPUT_INCLUDE}/resource.h.tmp ${OUTPUT_INCLUDE}/resource.h
#                   DEPENDS ${TOOLS_DIR}/GenerateOldResources.pl)

add_custom_command(OUTPUT ${OUTPUT_INCLUDE}/XCSoar.rc
                   COMMAND [ -d ${OUTPUT_INCLUDE} ] || mkdir ${OUTPUT_INCLUDE}
                   COMMAND cat "Data/XCSoar.rc" | gcc -E -o  ${OUTPUT_INCLUDE}/XCSoar.rc -)

#add_custom_command(OUTPUT ${OUTPUT_INCLUDE}/resource_data.h
#                   COMMAND [ -d ${OUTPUT_INCLUDE} ] || mkdir ${OUTPUT_INCLUDE}
#                   COMMAND ${TOOLS_DIR}/GenerateResources.pl < ${OUTPUT_INCLUDE}/XCSoar.rc > ${OUTPUT_INCLUDE}/resource_data.h
#                   DEPENDS ${TOOLS_DIR}/GenerateOldResources.pl)
