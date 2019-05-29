include("${CMAKE_CURRENT_LIST_DIR}/scripts/helpers.cmake")

#set(INSTALL_DIR ${CMAKE_SOURCE_DIR}/libs/3rd)
#set(SERVER "http://devinfra.fmi.fi:8008")

#function(download_and_install PACKAGE_NAME)
#    download_and_unpack("${SERVER}/${PACKAGE_NAME}.tar.gz" ${INSTALL_DIR})
#endfunction(download_and_install)

if (NOT EXISTS ${BOOST_BASE_DIR} )
	message( FATAL_ERROR "You need to get 3rd party lib binaries first installed, CMake will exit." )
endif()
