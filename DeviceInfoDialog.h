// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Mstar Semiconductor, Inc.

#ifndef DEVICEINFODIALOG_H
#define DEVICEINFODIALOG_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include "MstarEthBridge.h"
#include "MstarCollect.h"

// 设备详情对话框：显示 EthBridgeInfo 结构体信息
class DeviceInfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DeviceInfoDialog(QWidget *parent = nullptr);

    void setDeviceInfo(const EthBridgeInfo &info);
    void setCollectInfo(const collectinforst &info);
    void setDeviceId(const QString &id);
    void setDeviceHandle(MHANDLE hdev);

private slots:
    void onSignatureClicked();

private:
    QLabel *m_labelState;
    QLabel *m_labelVersion;
    QLabel *m_labelBuild;
    QLabel *m_labelType;
    QLabel *m_labelRevision;
    QLabel *m_labelIp;
    QLabel *m_labelMac;
    QLabel *m_labelPort;
    QLabel *m_labelSpeed;
    QLabel *m_labelClientNum;
    QLabel *m_labelCrypto;
    QPushButton *m_btnSignature;
    MHANDLE m_hdev;

    // 设备ID（可复制）
    QLineEdit *m_editDeviceId;

    // 采集信息
    QLabel *m_labelDeviceModel;
    QLabel *m_labelCollectVersion;
    QLabel *m_labelSampleFreq;
    QLabel *m_labelDataBufLen;
    QLabel *m_labelChannelNum;
    QLabel *m_labelAdBits;
    QLabel *m_labelAdChipNum;
    QLabel *m_labelRangeArryNum;
};

#endif // DEVICEINFODIALOG_H
