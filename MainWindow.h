// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Mstar Semiconductor, Inc.

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QListWidget>
#include <QStackedWidget>
#include <QVector>
#include "DevicePanel.h"
#include "FileViewerPanel.h"
#include "MstarEthBridge.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onAddDevice();
    void onDisconnect();
    void onDeviceListClicked(QListWidgetItem *item);
    void onDeviceListContextMenu(const QPoint &pos);
    void onToggleDeviceList();
    void onOpenFile();
    void onNetworkSettings();
    void onDeviceInfo();
    void onAbout();
    void onDllHelp();
    void onToggleLanguage();

private:
    void removePanel(int index);  // 移除指定索引的 panel（含级联断从）
    DevicePanel *devicePanelFromItem(QListWidgetItem *item);
    FileViewerPanel *filePanelFromItem(QListWidgetItem *item);
    QWidget *panelFromItem(QListWidgetItem *item);  // 通用版本

    // UI
    QPushButton *m_btnAddDevice;
    QPushButton *m_btnDisconnect;
    QPushButton *m_btnOpenFile;
    QPushButton *m_btnAbout;
    QPushButton *m_btnDllHelp;
    QPushButton *m_btnLang;
    QListWidget *m_deviceList;
    QStackedWidget *m_stack;
    QPushButton *m_btnToggleList;

    // 设备面板（支持 DevicePanel 和 FileViewerPanel）
    QVector<QWidget*> m_panels;
};

#endif // MAINWINDOW_H
