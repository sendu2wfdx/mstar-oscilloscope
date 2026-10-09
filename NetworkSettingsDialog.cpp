// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Mstar Semiconductor, Inc.

#include "NetworkSettingsDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>

NetworkSettingsDialog::NetworkSettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("网络设置"));
    setFixedSize(360, 180);

    m_editMac = new QLineEdit;
    m_editIp = new QLineEdit;

    auto *formLayout = new QFormLayout;
    formLayout->addRow(tr("MAC 地址:"), m_editMac);
    formLayout->addRow(tr("IP 地址:"), m_editIp);

    auto *warnLabel = new QLabel(tr("注意：MAC 地址在网络中是唯一的标识，修改后若与其他设备重复将导致通信异常，请谨慎操作。"));
    warnLabel->setStyleSheet("color: red;");
    warnLabel->setWordWrap(true);

    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(warnLabel);
    mainLayout->addStretch();
    mainLayout->addWidget(buttonBox);
}

void NetworkSettingsDialog::setMacAddress(const QString &mac)
{
    m_editMac->setText(mac);
}

void NetworkSettingsDialog::setIpAddress(const QString &ip)
{
    m_editIp->setText(ip);
}

QString NetworkSettingsDialog::macAddress() const
{
    return m_editMac->text();
}

QString NetworkSettingsDialog::ipAddress() const
{
    return m_editIp->text();
}
