### CMakeLists.txt ---
## 
## Author: Angelos Mantzaflaris 
## Copyright (C) 2016 - RICAM-Linz.
######################################################################

## CoDiPack extension
project(gsCoDiPackExtension)

# Apply same configuration as G+Smo
include(gsConfig)

gismo_fetch_directory(CoDiPack
    URL https://github.com/SciCompKL/CoDiPack/archive/numericExtension.zip
    DESTINATION    external
)

set(CODIPACK_INCLUDE_DIR ${gismo_SOURCE_DIR}/external/CoDiPack/include CACHE INTERNAL "")
include_directories(${CODIPACK_INCLUDE_DIR})

install(DIRECTORY ${CODIPACK_INCLUDE_DIR}
        DESTINATION include/codi/ OPTIONAL)
