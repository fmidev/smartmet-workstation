cmake_minimum_required(VERSION 3.7.1)
project(SmartMet)

set(CMAKE_MFC_FLAG 2)

# Configure the template file
SET(USER_FILE ${PROJECT_NAME}.vcxproj.user)
SET(OUTPUT_PATH ${PROJECT_BINARY_DIR}/${USER_FILE})
SET(COMMANDLINE_ARGUMENTS "-d -p ../../../smartmet-workstation-conf-fmi/control/smartmet_sap_scand.conf")
SET(WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
CONFIGURE_FILE(UserTemplate.user ${USER_FILE} @ONLY)

add_definitions(
    -DFMI_DISABLE_MFC_FEATURE_PACK
    )

set(LINK_DIRECTORIES_LIST 
    ${BOOST_LIB_DIR}
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    )

if(NOT DISABLE_EXTREME_TOOLKITPRO)
	set(LINK_DIRECTORIES_LIST ${LINK_DIRECTORIES_LIST} ${TOOLKITPRO_LIB_DIR})
endif()
	
link_directories(${LINK_DIRECTORIES_LIST})

file( GLOB CPP_FILES *.cpp )
file( GLOB H_FILES *.h )

if(NOT DISABLE_CPPRESTSDK)
file( GLOB RES_FILES *.rc res/* ${TOOLKITPRO_INCLUDE_DIR}/*.rc)
else()
file( GLOB RES_FILES *.rc res/*)
endif()

file( GLOB REG_FILES *.reg )
file( GLOB IDL_FILES *.idl )

add_executable( 
    ${PROJECT_NAME} WIN32
    ${CPP_FILES} 
    ${H_FILES} 
    ${RES_FILES} 
    ${REG_FILES} 
    ${IDL_FILES} 
    )

IF(MSVC)
    SET_TARGET_PROPERTIES(
        ${PROJECT_NAME}
        PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/MAP"
        )
ENDIF()

set(TARGET_LINK_LIBRARIES_LIST 
    ${PROJECT_NAME}
    ToolboxDep
    GeneralDoc
    Dialogs
    SmartTools
    ModifyEditedData
    SmetUtils
    Newbase
    Threads
    HakeMessage
	CapData
    AtlUtils
    MfcUtils
    SmetData
    StationViews
    Toolbox
    Processes
    AutoComplete
    optimized ${CRASHRPT_LIB}
    debug ${ICU_DEBUG_LIB_DT}
    debug ${ICU_DEBUG_LIB_IN}
    debug ${ICU_DEBUG_LIB_UC}
    optimized ${ICU_OPTIMIZED_LIB_DT}
    optimized ${ICU_OPTIMIZED_LIB_IN}
    optimized ${ICU_OPTIMIZED_LIB_UC}
    )

if(NOT DISABLE_UNIRAS_TOOLMASTER)
	set(TARGET_LINK_LIBRARIES_LIST ${TARGET_LINK_LIBRARIES_LIST} ${UNIRAS_LIB})
endif()

target_link_libraries(${TARGET_LINK_LIBRARIES_LIST})

target_include_directories( 
    ${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}
    PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/$(IntDir)
    PUBLIC ${ToolboxDep_INCLUDE_DIRS}
    PUBLIC ${GeneralDoc_INCLUDE_DIRS}
    PUBLIC ${Dialogs_INCLUDE_DIRS}
    PUBLIC ${SmartTools_INCLUDE_DIRS}
    PUBLIC ${ModifyEditedData_INCLUDE_DIRS}
    PUBLIC ${SmetUtils_INCLUDE_DIRS}
    PUBLIC ${Newbase_INCLUDE_DIRS}
    PUBLIC ${Threads_INCLUDE_DIRS}
    PUBLIC ${HakeMessage_INCLUDE_DIRS}
	PUBLIC ${CapData_INCLUDE_DIRS}
    PUBLIC ${AtlUtils_INCLUDE_DIRS}
    PUBLIC ${MfcUtils_INCLUDE_DIRS}
    PUBLIC ${SmetData_INCLUDE_DIRS}
    PUBLIC ${StationViews_INCLUDE_DIRS}
    PUBLIC ${Toolbox_INCLUDE_DIRS}
    PUBLIC ${Processes_INCLUDE_DIRS}
    PUBLIC ${AutoComplete_INCLUDE_DIRS}
    PUBLIC ${BOOST_INCLUDE_DIR}
    PUBLIC ${CRASHRPT_INCLUDE_DIR}
    PUBLIC ${ICU_INCLUDE_DIR}
    )

# OBS! you have to give all the linker flags in one go so you have to put them all in one variable (here its
# the SMARTMET_RES_DEPENDENCIES) and give that for PROPERTIES LINK_FLAGS with set_target_properties -function.
set(
    SMARTMET_RES_DEPENDENCIES " /ignore:4099 ${Dialogs_RES_FILE} ${MfcUtils_RES_FILE} ${AtlUtils_RES_FILE} ${ToolboxDep_RES_FILE} ${Threads_RES_FILE}"
    )

set_target_properties( 
    ${PROJECT_NAME}
    PROPERTIES LINK_FLAGS ${SMARTMET_RES_DEPENDENCIES}
    )

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD 
    COMMAND ${CMAKE_COMMAND} -E copy_directory 
    ${ICU_OPTIMIZED_DLL_DIR}
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/RelWithDebInfo
    VERBATIM
    )

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD 
    COMMAND ${CMAKE_COMMAND} -E copy_directory 
    ${ICU_OPTIMIZED_DLL_DIR}
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release
    VERBATIM
    )

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD 
    COMMAND ${CMAKE_COMMAND} -E copy_directory 
    ${ICU_DEBUG_DLL_DIR}
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug
    VERBATIM
    )

if(NOT DISABLE_CPPRESTSDK)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD 
    COMMAND ${CMAKE_COMMAND} -E copy_directory 
    ${CPPRESTSDK_OPTIMIZED_DLL_DIR}
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/RelWithDebInfo
    VERBATIM
    )

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD 
    COMMAND ${CMAKE_COMMAND} -E copy_directory 
    ${CPPRESTSDK_OPTIMIZED_DLL_DIR}
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release
    VERBATIM
    )

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD 
    COMMAND ${CMAKE_COMMAND} -E copy_directory 
    ${CPPRESTSDK_DEBUG_DLL_DIR}
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug
    VERBATIM
    )
endif()

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${CRASHRPT_BASE_DIR}/bin/CrashRpt1402.dll
    ${CRASHRPT_BASE_DIR}/bin/CrashSender1402.exe
    ${CRASHRPT_BASE_DIR}/bin/crashrpt_lang.ini
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$(Configuration)
    )
