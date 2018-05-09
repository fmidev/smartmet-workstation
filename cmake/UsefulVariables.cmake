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

set(
    BOOST_VERSION
    1_61_0
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
    ${CRASHRPT_BASE_DIR}/lib/Release/CrashRpt1402.lib
    CACHE INTERNAL "" FORCE
    )

set(
    ICU_VERSION
    55_1
    CACHE INTERNAL "" FORCE
    )

set(
    ICU_BASE_DIR
    ${LIBS_3RD}/icu_${ICU_VERSION}
    CACHE INTERNAL "" FORCE
    )

set(
    ICU_LIB_DIR
    ${ICU_BASE_DIR}/lib
    CACHE INTERNAL "" FORCE
    )

set(
    ICU_DEBUG_DLL_DIR
    ${ICU_BASE_DIR}/bin/Debug
    CACHE INTERNAL "" FORCE
    )

set(
    ICU_OPTIMIZED_DLL_DIR
    ${ICU_BASE_DIR}/bin/Release
    CACHE INTERNAL "" FORCE
    )

set(
    ICU_DEBUG_LIB_DT
    ${ICU_LIB_DIR}/icudtd.lib 
    CACHE INTERNAL "" FORCE
    )

set(
    ICU_DEBUG_LIB_IN
    ${ICU_LIB_DIR}/icuind.lib 
    CACHE INTERNAL "" FORCE
    )

set(
    ICU_DEBUG_LIB_UC
    ${ICU_LIB_DIR}/icuucd.lib 
    CACHE INTERNAL "" FORCE
    )

set(
    ICU_OPTIMIZED_LIB_DT
    ${ICU_LIB_DIR}/icudt.lib 
    CACHE INTERNAL "" FORCE
    )

set(
    ICU_OPTIMIZED_LIB_IN
    ${ICU_LIB_DIR}/icuin.lib 
    CACHE INTERNAL "" FORCE
    )

set(
    ICU_OPTIMIZED_LIB_UC
    ${ICU_LIB_DIR}/icuuc.lib 
    CACHE INTERNAL "" FORCE
    )

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
    ${CPPRESTSDK_LIB_DIR}/cpprest140_2_9.lib
    CACHE INTERNAL "" FORCE
    )

set(
    CPPRESTSDK_DEBUG_LIB
    ${CPPRESTSDK_LIB_DIR}/cpprest140d_2_9.lib
    CACHE INTERNAL "" FORCE
    )
endif()
