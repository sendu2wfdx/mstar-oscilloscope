// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Mstar Semiconductor, Inc.

#include <QApplication>
#include <QMessageBox>
#include <windows.h>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    // 单实例检测：防止上次崩溃后进程残留占用 DLL 资源
    HANDLE hMutex = CreateMutexW(NULL, TRUE, L"MstarOscilloscope_SingleInstance");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        QMessageBox::warning(nullptr, "提示",
            "已有 MstarOscilloscope 实例在运行（可能是上次崩溃后残留）。\n\n"
            "请先关闭已有实例后再重新启动。\n"
            "可在任务管理器中结束 MstarOscilloscope 进程。");
        CloseHandle(hMutex);
        return 1;
    }

    QApplication app(argc, argv);

    MainWindow w;
    w.show();

    int ret = app.exec();

    CloseHandle(hMutex);
    return ret;
}
