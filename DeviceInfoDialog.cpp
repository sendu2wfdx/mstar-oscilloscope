// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Mstar Semiconductor, Inc.

#include "DeviceInfoDialog.h"
#include "SignatureDialog.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QHBoxLayout>

DeviceInfoDialog::DeviceInfoDialog(QWidget *parent)
    : QDialog(parent), m_hdev(nullptr)
{
    setWindowTitle(tr("设备详情"));
    setFixedSize(400, 480);

    m_labelState = new QLabel;
    m_labelVersion = new QLabel;
    m_labelBuild = new QLabel;
    //m_labelType = new QLabel;
    //m_labelRevision = new QLabel;
    m_labelIp = new QLabel;
    m_labelMac = new QLabel;
    m_labelPort = new QLabel;
    m_labelSpeed = new QLabel;
    m_labelClientNum = new QLabel;
    m_labelCrypto = new QLabel;
    m_btnSignature = new QPushButton(tr("加载签名"));
    m_btnSignature->setVisible(false);
    connect(m_btnSignature, &QPushButton::clicked, this, &DeviceInfoDialog::onSignatureClicked);

    // 设备ID（可复制）
    m_editDeviceId = new QLineEdit;
    m_editDeviceId->setReadOnly(true);

    // 采集信息标签
    m_labelDeviceModel = new QLabel;
    m_labelCollectVersion = new QLabel;
    m_labelSampleFreq = new QLabel;
    m_labelDataBufLen = new QLabel;
    m_labelChannelNum = new QLabel;
    m_labelAdBits = new QLabel;
    m_labelAdChipNum = new QLabel;
    m_labelRangeArryNum = new QLabel;

    auto *formLayout = new QFormLayout;
    formLayout->addRow(tr("功能状态:"), m_labelState);
    formLayout->addRow(tr("协议版本:"), m_labelVersion);
    formLayout->addRow(tr("模块开发时间:"), m_labelBuild);
    //formLayout->addRow(tr("主版本号:"), m_labelType);
    //formLayout->addRow(tr("次版本号:"), m_labelRevision);
    formLayout->addRow(tr("IP 地址:"), m_labelIp);
    formLayout->addRow(tr("MAC 地址:"), m_labelMac);
    formLayout->addRow(tr("UDP 端口:"), m_labelPort);
    formLayout->addRow(tr("网口速率:"), m_labelSpeed);
    formLayout->addRow(tr("主控端数量:"), m_labelClientNum);

    // 加密状态行：标签 + 按钮
    auto *cryptoLayout = new QHBoxLayout;
    cryptoLayout->addWidget(m_labelCrypto);
    cryptoLayout->addWidget(m_btnSignature);
    cryptoLayout->addStretch();
    formLayout->addRow(tr("加密状态:"), cryptoLayout);

    // 设备ID
    formLayout->addRow(tr("设备 ID:"), m_editDeviceId);

    // 采集信息分组
    formLayout->addRow(tr(""), new QLabel("<b>" + tr("--- 采集信息 ---") + "</b>"));
    formLayout->addRow(tr("设备型号:"), m_labelDeviceModel);
    formLayout->addRow(tr("采集版本:"), m_labelCollectVersion);
    formLayout->addRow(tr("最高采样率:"), m_labelSampleFreq);
    formLayout->addRow(tr("缓存大小:"), m_labelDataBufLen);
    formLayout->addRow(tr("通道数:"), m_labelChannelNum);
    formLayout->addRow(tr("AD 精度:"), m_labelAdBits);
    formLayout->addRow(tr("AD 芯片数:"), m_labelAdChipNum);
    formLayout->addRow(tr("量程数组大小:"), m_labelRangeArryNum);

    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(formLayout);
    mainLayout->addStretch();
    mainLayout->addWidget(buttonBox);
}

void DeviceInfoDialog::setDeviceInfo(const EthBridgeInfo &info)
{
    // 功能状态：解析 state 位定义
    QStringList stateDesc;
    // AXI Full 总线状态
    QStringList axiFullDesc;
    if (info.state & BRIDGE_INFORE_STATE_AXIFULL_WEN) axiFullDesc << tr("写");
    if (info.state & BRIDGE_INFORE_STATE_AXIFULL_REN) axiFullDesc << tr("读");
    if (axiFullDesc.isEmpty()) axiFullDesc << tr("未使能");
    int addrWidth = (info.state & BRIDGE_INFORE_STATE_AXIFULL_ADDWIDTH) ? 64 : 32;
    int datWidth = (info.state & BRIDGE_INFORE_STATE_AXIFULL_DATWIDTH)>>3;
    int dataBits[] = {32, 64, 128, 256};
    stateDesc << tr("AXI Full 总线: %1, %2位地址, %3位数据")
                     .arg(axiFullDesc.join("/"))
                     .arg(addrWidth)
                     .arg(dataBits[datWidth]);
    m_labelState->setText(stateDesc.join(", "));

    // 加密状态
    if (info.state & BRIDGE_INFORE_STATE_CRYPTO_EN) {
        m_labelCrypto->setText(tr("未验证"));
        m_btnSignature->setVisible(true);
    } else {
        m_labelCrypto->setText(tr("已验证"));
        m_btnSignature->setVisible(false);
    }

    // 协议版本
    m_labelVersion->setText(QString("V%1.%2").arg(info.type).arg(info.revision));

    // 链接库更新时间：高4位月份，低12位年份（+2025）
    int month = (info.build) & 0xF;
    int year = ((info.build>>4) & 0xFFF) + 2025;
    m_labelBuild->setText(tr("%1年%2月").arg(year).arg(month));

    //m_labelType->setText(QString::number(info.type));
    //m_labelRevision->setText(QString::number(info.revision));
    m_labelIp->setText(QString(info.ip));
    m_labelMac->setText(QString(info.mac));
    m_labelPort->setText(QString::number(info.port));

    // 网口速率
    QString speedStr;
    switch (info.speed) {
    case BRIDGE_ETH_SPEED_10M:   speedStr = "10 Mbps"; break;
    case BRIDGE_ETH_SPEED_100M:  speedStr = "100 Mbps"; break;
    case BRIDGE_ETH_SPEED_1000M: speedStr = "1000 Mbps"; break;
    default:                     speedStr = tr("未知"); break;
    }
    m_labelSpeed->setText(speedStr);

    m_labelClientNum->setText(QString::number(info.clientnum));
}

void DeviceInfoDialog::setCollectInfo(const collectinforst &info)
{
    m_labelDeviceModel->setText(QString(info.devmodel));
    m_labelCollectVersion->setText(QString("V%1.%2").arg(info.main_version).arg(info.minor_version));
    m_labelSampleFreq->setText(QString("%1 Hz").arg(info.samplefreq_max));
    m_labelDataBufLen->setText(QString("%1 MB").arg(info.databuflen / (1024 * 1024)));
    m_labelChannelNum->setText(QString::number(info.nchannel));
    m_labelAdBits->setText(tr("%1 位").arg(info.nbits));
    m_labelAdChipNum->setText(QString::number(info.nadchip));
    m_labelRangeArryNum->setText(QString::number(info.rangearry_num));
}

void DeviceInfoDialog::setDeviceId(const QString &id)
{
    m_editDeviceId->setText(id);
}

void DeviceInfoDialog::setDeviceHandle(MHANDLE hdev)
{
    m_hdev = hdev;
}

void DeviceInfoDialog::onSignatureClicked()
{
    if (!m_hdev) return;
    SignatureDialog dlg(m_hdev, this);
    dlg.exec();
}
