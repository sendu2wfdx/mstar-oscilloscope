// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Mstar Semiconductor, Inc.

#ifndef SIGNATUREDIALOG_H
#define SIGNATUREDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QLabel>
#include "MstarEthBridge.h"

// 签名加载对话框
class SignatureDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SignatureDialog(MHANDLE hdev, QWidget *parent = nullptr);

private slots:
    void onLoadSignature();
    void onSaveSignature();
    void onLoadFromFile();
    void onVerifySignature();

private:
    MHANDLE m_hdev;
    QLineEdit *m_editDeviceId;
    QTextEdit *m_editSignature;
    QLabel *m_labelStatus;
    QTextEdit *m_editCurrentSigner;
};

#endif // SIGNATUREDIALOG_H
