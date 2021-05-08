#save oggplayer to output file.
SET(EXECUTABLE_OUTPUT_PATH  ${PROJECT_SOURCE_DIR}/output)


#include
SET(PUBLIC_INCLUDE_PATH
${PROJECT_SOURCE_DIR}/include
/usr/include/gstreamer-1.0
/usr/include/glib-2.0
/usr/lib/x86_64-linux-gnu/glib-2.0/include
)

INCLUDE_DIRECTORIES(${PUBLIC_INCLUDE_PATH})

LINK_DIRECTORIES(/usr/lib/x86_64-linux-gnu/)

#link so
SET(PUBLIC_SO libgstreamer-1.0.so libgobject-2.0.so.0 libglib-2.0.so.0)

#set src_list
SET(SRC_LIST  main.c player.c)

#exec
ADD_EXECUTABLE(oggplayer ${SRC_LIST})
TARGET_LINK_LIBRARIES(oggplayer ${PUBLIC_SO})

