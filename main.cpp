// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Mstar Semiconductor, Inc.

#include <QApplication>
#include <QMessageBox>
#include <QTranslator>
#include <QLocale>
#include <windows.h>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    // 单实例检测：防止上次崩溃后进程残留占用 DLL 资源
    HANDLE hMutex = CreateMutexW(NULL, TRUE, L"MstarOscilloscope_SingleInstance");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        QMessageBox::warning(nullptr, "MstarOscilloscope",
            "Another instance of MstarOscilloscope is already running.\n\n"
            "Please close the existing instance before starting a new one.\n"
            "You can terminate the MstarOscilloscope process in Task Manager.");
        CloseHandle(hMutex);
        return 1;
    }

    QApplication app(argc, argv);

    // 加载翻译文件（根据系统语言自动选择中文/英文）
    QTranslator translator;
    const QString locale = QLocale::system().name();  // e.g. "zh_CN", "en_US"
    if (translator.load("mstar_" + locale, ":/i18n")) {
        app.installTranslator(&translator);
    } else if (translator.load("mstar_en", ":/i18n")) {
        // 回退到英文
        app.installTranslator(&translator);
    }

    MainWindow w;
    w.show();

    int ret = app.exec();

    CloseHandle(hMutex);
    return ret;
}
