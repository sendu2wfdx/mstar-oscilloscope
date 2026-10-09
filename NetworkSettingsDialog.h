// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Mstar Semiconductor, Inc.

#ifndef NETWORKSETTINGSDIALOG_H
#define NETWORKSETTINGSDIALOG_H

#include <QDialog>
#include <QLineEdit>

// 设备网络参数只读查看对话框
class NetworkSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NetworkSettingsDialog(QWidget *parent = nullptr);

    void setMacAddress(const QString &mac);
    void setIpAddress(const QString &ip);
    QString macAddress() const;
    QString ipAddress() const;

private:
    QLineEdit *m_editMac;
    QLineEdit *m_editIp;
};

#endif // NETWORKSETTINGSDIALOG_H
