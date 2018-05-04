include("${CMAKE_CURRENT_LIST_DIR}/scripts/helpers.cmake")

set(INSTALL_DIR ${CMAKE_SOURCE_DIR}/libs/3rd)
set(SERVER "http://devinfra.fmi.fi:8008")

function(download_and_install PACKAGE_NAME)
    download_and_unpack("${SERVER}/${PACKAGE_NAME}.tar.gz" ${INSTALL_DIR})
endfunction(download_and_install)

if (NOT EXISTS ${INSTALL_DIR}/boost_1_61_0 )
download_and_install("boost_1_61_0")
download_and_install("cpprestsdk")
download_and_install("crashrpt_1_4_2_r1609")
download_and_install("icu_55_1")
download_and_install("uniras_7v5")
download_and_install("xtremetoolkitpro_17_1_0")
clean_cache()
endif()
