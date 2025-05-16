add_library(zlib STATIC IMPORTED GLOBAL)

set_target_properties(zlib PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/zlib/lib/libzlibstatic.a"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/zlib/include"
)
