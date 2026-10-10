// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Mstar Semiconductor, Inc.

#include "ConnectDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QMessageBox>
#include <QTimer>
#include "MstarEthBridge.h"
#include "Translation.h"

ConnectDialog::ConnectDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(T("Connect Device"));
    setMinimumWidth(380);

    auto *layout = new QVBoxLayout(this);

    // --- IP input ---
    auto *ipGroup = new QGroupBox(T("Device IP"));
    auto *ipLayout = new QHBoxLayout(ipGroup);
    m_editIp = new QLineEdit("192.168.0.100");
    ipLayout->addWidget(new QLabel("IP:"));
    ipLayout->addWidget(m_editIp);
    layout->addWidget(ipGroup);

    // --- Scan ---
    auto *scanGroup = new QGroupBox(T("Auto Scan"));
    auto *scanLayout = new QVBoxLayout(scanGroup);
    auto *btnLayout = new QHBoxLayout;
    m_btnScan = new QPushButton(T("Scan Devices"));
    btnLayout->addWidget(m_btnScan);
    btnLayout->addStretch();
    scanLayout->addLayout(btnLayout);

    m_listDevices = new QListWidget;
    m_listDevices->setMaximumHeight(150);
    scanLayout->addWidget(m_listDevices);
    layout->addWidget(scanGroup);

    // --- OK / Cancel ---
    auto *btnBox = new QHBoxLayout;
    auto *btnOk = new QPushButton(T("Connect"));
    auto *btnCancel = new QPushButton(T("Cancel"));
    btnBox->addStretch();
    btnBox->addWidget(btnOk);
    btnBox->addWidget(btnCancel);
    layout->addLayout(btnBox);

    // --- Signals ---
    connect(m_btnScan, &QPushButton::clicked, this, &ConnectDialog::onScan);
    connect(m_listDevices, &QListWidget::itemClicked, this, &ConnectDialog::onDeviceClicked);
    connect(btnOk, &QPushButton::clicked, this, &QDialog::accept);
    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
}

QString ConnectDialog::selectedIp() const
{
    return m_editIp->text().trimmed();
}

void ConnectDialog::setExistingDevices(const QStringList &ips)
{
    m_existingDevices = ips;
}

void ConnectDialog::onScan()
{
    m_listDevices->clear();
    m_btnScan->setEnabled(false);

    scan_device();
    //mstar_ethbridge_open(NULL);

    // 延迟 100ms 后读取设备列表，给广播响应留出时间
    QTimer::singleShot(3000, this, [this]() {
        int count = get_udpclient_size();

        if (count <= 0) {
            m_listDevices->addItem(T("No device found"));
            m_btnScan->setEnabled(true);
            return;
        }

        for (int i = 0; i < count; ++i) {
            MHANDLE hdev = get_device(i);
            if (hdev) {
                EthBridgeInfo info;
                get_devinfor(hdev, &info);
                // 过滤已连接的设备，防止重复添加
                if (m_existingDevices.contains(info.ip)) {
                    continue;
                }
                QString text = QString("%1  (%2)  Port:%3")
                    .arg(info.ip).arg(info.mac).arg(info.port);
                m_listDevices->addItem(text);
            }
        }

        if (m_listDevices->count() == 0) {
            m_listDevices->addItem(T("No new device found (all connected)"));
        }

        m_btnScan->setEnabled(true);
    });
}

void ConnectDialog::onDeviceClicked(QListWidgetItem *item)
{
    QString text = item->text();
    // Extract IP: first token before spaces
    QString ip = text.section("  ", 0, 0).trimmed();
    m_editIp->setText(ip);
}

void ConnectDialog::accept()
{
    QString ip = m_editIp->text().trimmed();
    if (ip.isEmpty()) {
        QMessageBox::warning(this, T("Error"), T("Please enter device IP address!"));
        return;
    }
    // 检查是否已连接
    if (m_existingDevices.contains(ip)) {
        QMessageBox::warning(this, T("提示"),
            T("设备 %1 已连接，请勿重复添加").arg(ip));
        return;
    }
    QDialog::accept();
}
