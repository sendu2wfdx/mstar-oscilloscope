// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Mstar Semiconductor, Inc.

#ifndef CONNECTDIALOG_H
#define CONNECTDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QStringList>

/**
 * @brief 启动连接对话框
 *
 * 程序启动时弹出，提供两种方式连接设备：
 * 1. 手动输入设备 IP 地址
 * 2. 自动扫描网络设备，从列表中选择
 *
 * 已添加的设备不会在扫描列表中显示，防止重复添加。
 */
class ConnectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConnectDialog(QWidget *parent = nullptr);

    /// 设置已连接设备的 IP 列表，扫描时会过滤这些设备
    void setExistingDevices(const QStringList &ips);

    /// 返回用户选定的 IP 地址，取消时返回空字符串
    QString selectedIp() const;

private slots:
    void onScan();
    void onDeviceClicked(QListWidgetItem *item);

protected:
    void accept() override;

private:
    QLineEdit *m_editIp;
    QPushButton *m_btnScan;
    QListWidget *m_listDevices;
    QStringList m_existingDevices;  // 已连接设备的 IP 列表
};

#endif // CONNECTDIALOG_H
