## What to build ##

set( sources main.cpp )

add_executable( mjpgmuxer EXCLUDE_FROM_ALL ${sources} )

target_link_libraries( mjpgmuxer mkvwriter )


## How to build it ##

include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${EBML_INCLUDE_DIRS}
    ${Matroska_INCLUDE_DIRS}
)

