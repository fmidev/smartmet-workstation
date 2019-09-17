cmake_minimum_required(VERSION 3.14.4)

project(Dialogs)

set(CMAKE_MFC_FLAG 2)

file(GLOB CPP_FILES *.cpp)
file(GLOB H_FILES *.h)
file(GLOB RES_FILES *.rc res/*)

add_library(
    ${PROJECT_NAME} STATIC
    ${CPP_FILES}
    ${H_FILES}
    ${RES_FILES}
    )

target_link_libraries(
    ${PROJECT_NAME}
    SmartTools
    ToolboxDep
    Win32Utils
    MfcUtils
    GridCtrl
    Newbase
    SmetUtils
    Bzip2Util
    Processes
    StationViews
    ModifyEditedData
    GridCtrl
    CaseStudyLib
    SmetData
    FilterDataViews
    Toolbox
    Textgen
    CtrlViews
    HakeMessage
    Q2ClientLib
	AddParamsToViews
	ApplicationDataBase
    CtrlViewUtils
    )

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}
    PUBLIC ${SmartTools_INCLUDE_DIRS}
    PUBLIC ${ToolboxDep_INCLUDE_DIRS}
    PUBLIC ${Win32Utils_INCLUDE_DIRS}
    PUBLIC ${MfcuUtils_INCLUDE_DIRS}
    PUBLIC ${GridCtrl_INCLUDE_DIRS}
    PUBLIC ${Newbase_INCLUDE_DIRS}
    PUBLIC ${SmetUtils_INCLUDE_DIRS}
    PUBLIC ${Bzip2Util_INCLUDE_DIRS}
    PUBLIC ${Processes_INCLUDE_DIRS}
    PUBLIC ${StationViews_INCLUDE_DIRS}
    PUBLIC ${ModifyEditedData_INCLUDE_DIRS}
    PUBLIC ${GridCtrl_INCLUDE_DIRS}
    PUBLIC ${CaseStudyLib_INCLUDE_DIRS}
    PUBLIC ${SmetData_INCLUDE_DIRS}
    PUBLIC ${FilterDataViews_INCLUDE_DIRS}
    PUBLIC ${Toolbox_INCLUDE_DIRS}
    PUBLIC ${Textgen_INCLUDE_DIRS}
    PUBLIC ${CtrlViews_INCLUDE_DIRS}
    PUBLIC ${HakeMessage_INCLUDE_DIRS}
    PUBLIC ${Q2ClientLib_INCLUDE_DIRS}
    PUBLIC ${AddParamsToViews_INCLUDE_DIRS}
    PUBLIC ${ApplicationDataBase_INCLUDE_DIRS}
    PUBLIC ${CtrlViewUtils_INCLUDE_DIRS}
    PUBLIC ${BOOST_INCLUDE_DIR}
    )

set(${PROJECT_NAME}_RES_FILE
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.dir/$(Configuration)/SmartMetDialogs.res
    CACHE INTERNAL "${PROJECT_NAME}: Generated .res file" FORCE
    )

set(
    ${PROJECT_NAME}_INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}
    ${SmartTools_INCLUDE_DIRS}
    ${ToolboxDep_INCLUDE_DIRS}
    ${Win32Utils_INCLUDE_DIRS}
    ${MfcuUtils_INCLUDE_DIRS}
    ${GridCtrl_INCLUDE_DIRS}
    ${Newbase_INCLUDE_DIRS}
    ${SmetUtils_INCLUDE_DIRS}
    ${Bzip2Util_INCLUDE_DIRS}
    ${Processes_INCLUDE_DIRS}
    ${StationViews_INCLUDE_DIRS}
    ${ModifyEditedData_INCLUDE_DIRS}
    ${GridCtrl_INCLUDE_DIRS}
    ${CaseStudyLib_INCLUDE_DIRS}
    ${SmetData_INCLUDE_DIRS}
    ${FilterDataViews_INCLUDE_DIRS}
    ${Toolbox_INCLUDE_DIRS}
    ${Textgen_INCLUDE_DIRS}
    ${CtrlViews_INCLUDE_DIRS}
    ${HakeMessage_INCLUDE_DIRS}
    ${Q2ClientLib_INCLUDE_DIRS}
	${AddParamsToViews_INCLUDE_DIRS}
    ${ApplicationDataBase_INCLUDE_DIRS}
    ${CtrlViewUtils_INCLUDE_DIRS}
    ${BOOST_INCLUDE_DIR}
    CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE
    )
