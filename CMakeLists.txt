cmake_minimum_required( VERSION 3.1 )

project( mkvwriter
    VERSION 0.0.0.1 )


## Options ##


## External Dependencies ##

set( CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules/" ${CMAKE_MODULE_PATH} )

find_package( EBML REQUIRED )
if( NOT EBML_FOUND )
    message( FATAL_ERROR "Required package not found: libebml" )
endif()

find_package( Matroska REQUIRED )
if( NOT Matroska_FOUND )
    message( FATAL_ERROR "Required package not found: libmatroska" )
endif()



## Subdirectories ##

add_subdirectory( src )
#add_subdirectory( tools )

