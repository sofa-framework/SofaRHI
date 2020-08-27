cmake_minimum_required(VERSION 3.1)
project(SofaRHI VERSION 0.0 LANGUAGES CXX)

set(SOFARHI_SRC_DIR src/SofaRHI)

set(SOURCE_FILES
    ${SOFARHI_SRC_DIR}/gui/RHIViewer.cpp
    # ${SOFARHI_SRC_DIR}/gui/RHIPickHandler.cpp
    ${SOFARHI_SRC_DIR}/gui/RHIBackend.cpp
    # ${SOFARHI_SRC_DIR}/RHIObject.cpp
    ${SOFARHI_SRC_DIR}/RHIUtils.cpp
    ${SOFARHI_SRC_DIR}/RHIModel.cpp
    ${SOFARHI_SRC_DIR}/DrawToolRHI.cpp
    # ${SOFARHI_SRC_DIR}/RHIVisualManagerLoop.cpp
    # ${SOFARHI_SRC_DIR}/RHIVisualVisitor.cpp
    # ${SOFARHI_SRC_DIR}/RHIVisualParams.cpp
    ${SOFARHI_SRC_DIR}/initSofaRHI.cpp
)

set(HEADER_FILES
    ${SOFARHI_SRC_DIR}/config.h.in
    ${SOFARHI_SRC_DIR}/gui/RHIViewer.h
    # ${SOFARHI_SRC_DIR}/gui/RHIPickHandler.h
    ${SOFARHI_SRC_DIR}/gui/RHIBackend.h
    # ${SOFARHI_SRC_DIR}/RHIObject.h
    ${SOFARHI_SRC_DIR}/RHIUtils.h
    ${SOFARHI_SRC_DIR}/RHIVisualModel.h
    ${SOFARHI_SRC_DIR}/RHIModel.h
    ${SOFARHI_SRC_DIR}/DrawToolRHI.h
    # ${SOFARHI_SRC_DIR}/RHIVisualManagerLoop.h
    # ${SOFARHI_SRC_DIR}/RHIVisualVisitor.h
    ${SOFARHI_SRC_DIR}/RHIVisualParams.h
)

set(QT_RESOURCE_FILES
    ${SOFARHI_SRC_DIR}/rhi/qtresources.qrc
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5 COMPONENTS Core Widgets REQUIRED PATHS "${SOFA-EXTERNAL_QT5_PATH}")
find_package(Qt5Gui CONFIG REQUIRED Private PATHS "${SOFA-EXTERNAL_QT5_PATH}")

find_package(SofaBase REQUIRED)
find_package(SofaGui REQUIRED)

add_library(${PROJECT_NAME} SHARED ${HEADER_FILES}  ${SOURCE_FILES}  ${QT_RESOURCE_FILES})


target_link_libraries(${PROJECT_NAME} SofaCore SofaBaseVisual SofaGuiCommon SofaGuiMain SofaGuiQt)
target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Gui Qt5::GuiPrivate Qt5::Widgets )

target_compile_definitions(${PROJECT_NAME} PRIVATE "-DSOFA_BUILD_SOFARHI")

# TEMP
message("###### TO REMOVE LATER: ")
if(MSVC)
    foreach (CONFIG Debug Release RelWithDebInfo MinSizeRel) 
        set(plugin_config_file "${CMAKE_BINARY_DIR}/bin/${CONFIG}/plugin_list.conf.default")
        FILE(APPEND ${plugin_config_file} "\n${PROJECT_NAME} NO_VERSION\n")
        message("Adding SofaRHI to runSofa plugin list at ${plugin_config_file}")
    endforeach()
else()
    set(plugin_config_file "${CMAKE_BINARY_DIR}/lib/plugin_list.conf.default")
    FILE(APPEND ${plugin_config_file} "\n${PROJECT_NAME} NO_VERSION")
    message("Adding SofaRHI to runSofa plugin list at ${plugin_config_file}")
endif()
message("######")

## Install rules for the library; CMake package configurations files
sofa_create_package_with_targets(
    PACKAGE_NAME ${PROJECT_NAME}
    PACKAGE_VERSION ${PROJECT_VERSION}
    TARGETS ${PROJECT_NAME} AUTO_SET_TARGET_PROPERTIES
    INCLUDE_SOURCE_DIR "src"
    INCLUDE_INSTALL_DIR "${PROJECT_NAME}"
    RELOCATABLE "plugins"
    )