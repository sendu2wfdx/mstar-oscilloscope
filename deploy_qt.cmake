# deploy_qt.cmake - 构建时由 cmake -P 调用，仅 Release 配置时运行 windeployqt
# 调用方式: cmake -DWINDEPLOYQT=... -DTARGET_EXE=... -DBUILD_CONFIG=... -P deploy_qt.cmake

if(BUILD_CONFIG STREQUAL "Release")
    message(STATUS "Running windeployqt for ${TARGET_EXE}")
    execute_process(
        COMMAND "${WINDEPLOYQT}" --no-translations "${TARGET_EXE}"
        RESULT_VARIABLE result
    )
    if(NOT result EQUAL 0)
        message(WARNING "windeployqt failed with exit code: ${result}")
    endif()
else()
    message(STATUS "Skipping windeployqt (${BUILD_CONFIG} build)")
endif()
