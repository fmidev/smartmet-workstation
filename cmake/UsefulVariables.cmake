set(
    ROOT 
    ${CMAKE_SOURCE_DIR}
    CACHE INTERNAL "" FORCE
    )

set(
    SRC
    ${ROOT}/src
    CACHE INTERNAL "" FORCE
    )

set(
    LIBS_FMI
    ${ROOT}/libs/fmi
    CACHE INTERNAL "" FORCE
    )

set(
    LIBS_3RD
    ${ROOT}/libs/3rd
    CACHE INTERNAL "" FORCE
    )

# **** Boost library section ****
set(
    BOOST_VERSION
    1_70_0
    CACHE INTERNAL "" FORCE
    )

set(
    BOOST_BASE_DIR
    ${LIBS_3RD}/boost_${BOOST_VERSION}
    CACHE INTERNAL "" FORCE
    )

set(
    BOOST_INCLUDE_DIR
    ${BOOST_BASE_DIR}
    CACHE INTERNAL "" FORCE
    )

set(
    BOOST_LIB_DIR
    ${BOOST_BASE_DIR}/lib
    CACHE INTERNAL "" FORCE
    )

# **** ToolkitPro library section ****
set(
    TOOLKITPRO_VERSION
    17_1_0
    CACHE INTERNAL "" FORCE
    )

set(
    TOOLKITPRO_BASE_DIR
    ${LIBS_3RD}/xtremetoolkitpro_${TOOLKITPRO_VERSION}
    CACHE INTERNAL "" FORCE
    )

set(
    TOOLKITPRO_INCLUDE_DIR
    ${TOOLKITPRO_BASE_DIR}/inc
    CACHE INTERNAL "" FORCE
    )

set(
    TOOLKITPRO_LIB_DIR
    ${TOOLKITPRO_BASE_DIR}/lib
    CACHE INTERNAL "" FORCE
    )

# **** Uniras library section ****
set(
    UNIRAS_VERSION
    7v5
    CACHE INTERNAL "" FORCE
    )

set(
    UNIRAS_BASE_DIR
    ${LIBS_3RD}/uniras_${UNIRAS_VERSION}
    CACHE INTERNAL "" FORCE
    )

set(
    UNIRAS_INCLUDE_DIR
    ${UNIRAS_BASE_DIR}/inc
    CACHE INTERNAL "" FORCE
    )

set(
    UNIRAS_LIB_DIR
    ${UNIRAS_BASE_DIR}/lib
    CACHE INTERNAL "" FORCE
    )

set(
    UNIRAS_LIB
    ${UNIRAS_BASE_DIR}/lib/Release/agx.lib
    CACHE INTERNAL "" FORCE
    )

# **** CrashRpt library section ****
set(
    CRASHRPT_VERSION
    1_4_2_r1609
    CACHE INTERNAL "" FORCE
    )

set(
    CRASHRPT_BASE_DIR
    ${LIBS_3RD}/crashrpt_${CRASHRPT_VERSION}
    CACHE INTERNAL "" FORCE
    )

set(
    CRASHRPT_INCLUDE_DIR
    ${CRASHRPT_BASE_DIR}/inc
    CACHE INTERNAL "" FORCE
    )

set(
    CRASHRPT_LIB_DIR
    ${CRASHRPT_BASE_DIR}/lib
    CACHE INTERNAL "" FORCE
    )

set(
    CRASHRPT_LIB
    ${CRASHRPT_LIB_DIR}/Release/CrashRpt1402.lib
    CACHE INTERNAL "" FORCE
    )

# **** Fmt library section ****
set(
    FMT_VERSION
    7_0_3
    CACHE INTERNAL "" FORCE
    )

set(
    FMT_BASE_DIR
    ${LIBS_3RD}/fmt_${FMT_VERSION}
    CACHE INTERNAL "" FORCE
    )

set(
    FMT_INCLUDE_DIR
    ${FMT_BASE_DIR}/inc
    CACHE INTERNAL "" FORCE
    )

set(
    FMT_LIB_DIR
    ${FMT_BASE_DIR}/lib
    CACHE INTERNAL "" FORCE
    )

set(
    FMT_LIB_DEBUG
    ${FMT_LIB_DIR}/fmtd.lib
    CACHE INTERNAL "" FORCE
    )

set(
    FMT_LIB_RELEASE
    ${FMT_LIB_DIR}/fmt.lib
    CACHE INTERNAL "" FORCE
    )

set(
    FMT_TARGET_LINK_LIBRARIES
    debug ${FMT_LIB_DEBUG}
    optimized ${FMT_LIB_RELEASE}
    )

# **** Gdal library section ****
set(
    GDAL_VERSION
    3_5_2
    CACHE INTERNAL "" FORCE
    )

set(
    GDAL_BASE_DIR
    ${LIBS_3RD}/gdal-${GDAL_VERSION}
    CACHE INTERNAL "" FORCE
    )

set(
    GDAL_INCLUDE_DIR
    ${GDAL_BASE_DIR}/include
    CACHE INTERNAL "" FORCE
    )

set(
    GDAL_LIB_DIR
    ${GDAL_BASE_DIR}/lib
    CACHE INTERNAL "" FORCE
    )

set(
    GDAL_BIN_DIR
    ${GDAL_BASE_DIR}/bin
    CACHE INTERNAL "" FORCE
    )

set(
    GDAL_LIBS
    gdal_i.lib proj.lib geos.lib
    CACHE INTERNAL "" FORCE
    )


# **** CppRestSdk library section ****
if(NOT DISABLE_CPPRESTSDK)	
set(
    CPPRESTSDK_BASE_DIR
    ${LIBS_3RD}/cpprestsdk
    CACHE INTERNAL "" FORCE
    )

set(
    CPPRESTSDK_INCLUDE_DIR
    ${CPPRESTSDK_BASE_DIR}/inc
    CACHE INTERNAL "" FORCE
    )

set(
    CPPRESTSDK_LIB_DIR
    ${CPPRESTSDK_BASE_DIR}/lib
    CACHE INTERNAL "" FORCE
    )

set(
    CPPRESTSDK_DEBUG_DLL_DIR
    ${CPPRESTSDK_BASE_DIR}/bin/Debug
    CACHE INTERNAL "" FORCE
    )

set(
    CPPRESTSDK_OPTIMIZED_DLL_DIR
    ${CPPRESTSDK_BASE_DIR}/bin/Release
    CACHE INTERNAL "" FORCE
    )

set(
    CPPRESTSDK_OPTIMIZED_LIB
    ${CPPRESTSDK_LIB_DIR}/cpprest142_2_10.lib
    CACHE INTERNAL "" FORCE
    )

set(
    CPPRESTSDK_DEBUG_LIB
    ${CPPRESTSDK_LIB_DIR}/cpprest142_2_10d.lib
    CACHE INTERNAL "" FORCE
    )
endif()
