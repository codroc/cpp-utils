cmake_minimum_required (VERSION 2.8)

project (cpp-utils)

include_directories (cct itc ds log config .)

aux_source_directory (cct CCT_SRCS)
aux_source_directory (itc ITC_SRCS)
aux_source_directory (ds DS_SRCS)
aux_source_directory (log LOG_SRCS)
aux_source_directory (config CONFIG_SRCS)
aux_source_directory (. TIME_SRCS)

file (GLOB_RECURSE HEADERS *h)

execute_process(COMMAND python3-config --ldflags --cflags OUTPUT_VARIABLE include_options)
string (REPLACE "\n" " " include_options ${include_options})

set (CMAKE_CXX_FLAGS "${include_options} -fPIC")

add_library (cpputils ${CCT_SRCS} ${ITC_SRCS} ${DS_SRCS} ${LOG_SRCS} ${CONFIG_SRCS} ${TIME_SRCS})

install (TARGETS cpputils DESTINATION /usr/local/bin)
install (FILES ${HEADERS} DESTINATION /usr/local/include)

