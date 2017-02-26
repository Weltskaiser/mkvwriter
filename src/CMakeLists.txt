## Settings ##

## Paths ##

set( incl ${PROJECT_SOURCE_DIR}/include )
set( h_dir ${incl}/mkvwriter )


## What to build ##

set( sources
    MatroskaMuxer.cpp
    MatroskaSimpleTags.cpp
)

file( GLOB headers
    LIST_DIRECTORIES false
    ${h_dir}/*
)

add_library( mkvwriter ${sources} )

target_link_libraries( mkvwriter ${EBML_LIBRARY} ${Matroska_LIBRARY} )


## How to build it ##

include_directories(
    ${incl}
    ${EBML_INCLUDE_DIRS}
    ${Matroska_INCLUDE_DIRS}
)


## Where to install it ##

install(
    TARGETS mkvwriter
    DESTINATION lib )

install(
    FILES ${headers}
    DESTINATION include/mkvwriter )

