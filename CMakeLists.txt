cmake_minimum_required(VERSION 3.16)

project(MstarOscilloscope VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Qt6
find_package(Qt6 REQUIRED COMPONENTS Widgets PrintSupport)

# DLL 路径配置（开源用户需指定 MStarEthernetBridge DLL 的位置）
# 可通过 cmake -DDLL_INCLUDE_DIR=... -DDLL_PROJECT_OUT=... 覆盖
set(DLL_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dll/include" CACHE PATH "Path to DLL header files")
set(DLL_PROJECT_OUT "${CMAKE_CURRENT_SOURCE_DIR}/dll/lib" CACHE PATH "Path to DLL library files")

add_executable(MstarOscilloscope WIN32
    main.cpp
    MainWindow.h
    MainWindow.cpp
    DevicePanel.h
    DevicePanel.cpp
    FileViewerPanel.h
    FileViewerPanel.cpp
    ConnectDialog.h
    ConnectDialog.cpp
    NetworkSettingsDialog.h
    NetworkSettingsDialog.cpp
    DeviceInfoDialog.h
    DeviceInfoDialog.cpp
    SignatureDialog.h
    SignatureDialog.cpp
    CollectParamDialog.h
    CollectParamDialog.cpp
    DeviceProfileManager.h
    DeviceProfileManager.cpp
    DataFileWriter.h
    DataFileWriter.cpp
    WindowIndicator.h
    WindowIndicator.cpp
    qcustomplot.h
    qcustomplot.cpp
    resources.qrc
    app.rc
)

target_include_directories(MstarOscilloscope PRIVATE
    ${DLL_INCLUDE_DIR}
)

target_link_directories(MstarOscilloscope PRIVATE
    ${DLL_PROJECT_OUT}
)

target_link_libraries(MstarOscilloscope PRIVATE
    Qt6::Widgets
    Qt6::PrintSupport
    MStarEthernetBridge
)

# MSVC: treat source files as UTF-8
if(MSVC)
    target_compile_options(MstarOscilloscope PRIVATE /utf-8)
endif()

# 每次构建自动从 DLL 工程目录拷贝最新 DLL，无需手动拷贝
add_custom_command(TARGET MstarOscilloscope POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${DLL_PROJECT_OUT}/MStarEthernetBridge.dll"
        "$<TARGET_FILE_DIR:MstarOscilloscope>/MStarEthernetBridge.dll"
)

# 自动拷贝 DLL 接口帮助文件到输出目录
add_custom_command(TARGET MstarOscilloscope POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${DLL_INCLUDE_DIR}/MStarEthernetBridge_API_Help.md"
        "$<TARGET_FILE_DIR:MstarOscilloscope>/MStarEthernetBridge_API_Help.md"
)

# Release 构建后自动运行 windeployqt 收集 Qt 依赖 DLL
find_program(WINDEPLOYQT_EXECUTABLE
    NAMES windeployqt6 windeployqt
    HINTS "${Qt6_DIR}/../../../bin"
)

if(WINDEPLOYQT_EXECUTABLE)
    message(STATUS "windeployqt found: ${WINDEPLOYQT_EXECUTABLE}")
    # 仅 Release 配置时通过 deploy_qt.cmake 脚本运行 windeployqt
    add_custom_command(TARGET MstarOscilloscope POST_BUILD
        COMMAND ${CMAKE_COMMAND}
            -DWINDEPLOYQT=${WINDEPLOYQT_EXECUTABLE}
            -DTARGET_EXE=$<TARGET_FILE:MstarOscilloscope>
            -DBUILD_CONFIG=$<CONFIG>
            -P ${CMAKE_CURRENT_SOURCE_DIR}/deploy_qt.cmake
        COMMENT "Deploying Qt dependencies with windeployqt (Release only)..."
    )
else()
    message(WARNING "windeployqt not found, automatic Qt deployment disabled")
endif()
