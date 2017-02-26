add_subdirectory( mjpgmuxer )

add_custom_target( examples )
add_dependencies( examples mjpgmuxer )
