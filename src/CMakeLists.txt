## Settings ##

## Paths ##

set( incl ${PROJECT_SOURCE_DIR}/include )
set( h_dir ${incl}/mkvwriter )


## What to build ##

set( sources
    MatroskaMuxer.cpp
    MatroskaSimpleTags.cpp
)

set( headers
    ${h_dir}/MatroskaMuxer.h
    ${h_dir}/MatroskaSimpleTags.h
)

add_library( mkvwriter SHARED ${sources} )

target_link_libraries( mkvwriter ${EBML_LIBRARY} ${Matroska_LIBRARY} )


## How to build it ##

include_directories(
    ${incl}
    ${EBML_INCLUDE_DIRS}
)


## Where to install it ##

install(
    TARGETS mkvwriter
    DESTINATION lib )

install(
    FILES ${headers}
    DESTINATION include/mkvwriter )

