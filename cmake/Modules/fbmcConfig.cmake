INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(PC_FBMC fbmc)

FIND_PATH(
    FBMC_INCLUDE_DIRS
    NAMES fbmc/api.h
    HINTS $ENV{FBMC_DIR}/include
        ${PC_FBMC_INCLUDEDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/include
          /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    FBMC_LIBRARIES
    NAMES gnuradio-fbmc
    HINTS $ENV{FBMC_DIR}/lib
        ${PC_FBMC_LIBDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/lib
          ${CMAKE_INSTALL_PREFIX}/lib64
          /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(FBMC DEFAULT_MSG FBMC_LIBRARIES FBMC_INCLUDE_DIRS)
MARK_AS_ADVANCED(FBMC_LIBRARIES FBMC_INCLUDE_DIRS)

