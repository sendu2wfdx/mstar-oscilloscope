// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Mstar Semiconductor, Inc.

#include "SignatureDialog.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QRegularExpression>
#include "Translation.h"

SignatureDialog::SignatureDialog(MHANDLE hdev, QWidget *parent)
    : QDialog(parent), m_hdev(hdev)
{
    setWindowTitle(T("签名验证"));
    setFixedSize(450, 350);

    // 设备ID（只读，可复制）
    m_editDeviceId = new QLineEdit;
    m_editDeviceId->setReadOnly(true);

    // 签名输入框（多行）
    m_editSignature = new QTextEdit;
    m_editSignature->setPlaceholderText(T("请输入签名信息"));
    m_editSignature->setFixedHeight(
        m_editSignature->fontMetrics().lineSpacing() * 4 + 10);

    // 当前签名显示（多行只读）
    m_editCurrentSigner = new QTextEdit;
    m_editCurrentSigner->setReadOnly(true);
    m_editCurrentSigner->setFixedHeight(
        m_editCurrentSigner->fontMetrics().lineSpacing() * 4 + 10);

    // 状态标签
    m_labelStatus = new QLabel;
    m_labelStatus->setStyleSheet("color: gray;");

    auto *formLayout = new QFormLayout;
    formLayout->addRow(T("设备 ID:"), m_editDeviceId);
    formLayout->addRow(T("当前签名:"), m_editCurrentSigner);
    formLayout->addRow(T("新签名:"), m_editSignature);
    formLayout->addRow(m_labelStatus);

    // 按钮
    auto *btnLoad = new QPushButton(T("加载签名"));
    auto *btnLoadFile = new QPushButton(T("从文件加载"));
    auto *btnVerify = new QPushButton(T("验证签名"));
    auto *btnSave = new QPushButton(T("保存到设备"));
    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);

    connect(btnLoad, &QPushButton::clicked, this, &SignatureDialog::onLoadSignature);
    connect(btnLoadFile, &QPushButton::clicked, this, &SignatureDialog::onLoadFromFile);
    connect(btnVerify, &QPushButton::clicked, this, &SignatureDialog::onVerifySignature);
    connect(btnSave, &QPushButton::clicked, this, &SignatureDialog::onSaveSignature);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *btnLayout = new QHBoxLayout;
    btnLayout->addWidget(btnLoad);
    btnLayout->addWidget(btnLoadFile);
    btnLayout->addWidget(btnVerify);
    btnLayout->addWidget(btnSave);
    btnLayout->addStretch();

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(formLayout);
    mainLayout->addLayout(btnLayout);
    mainLayout->addWidget(buttonBox);

    // 初始化显示
    const char *devId = get_device_id(m_hdev);
    if (devId) {
        m_editDeviceId->setText(QString(devId));
    }

    const char *signer = get_verifysigner(m_hdev);
    if (signer) {
        m_editCurrentSigner->setPlainText(QString(signer));
    } else {
        m_editCurrentSigner->setPlainText(T("(无)"));
    }
}

void SignatureDialog::onLoadSignature()
{
    QString signature = m_editSignature->toPlainText().trimmed();
    if (signature.isEmpty()) {
        QMessageBox::warning(this, T("提示"), T("请输入签名信息"));
        return;
    }

    MBOOL ret = load_verifysigner(m_hdev, signature.toStdString().c_str());
    if (ret) {
        m_labelStatus->setText(T("签名加载成功"));
        m_labelStatus->setStyleSheet("color: green;");

        // 更新当前签名显示
        const char *signer = get_verifysigner(m_hdev);
        if (signer) {
            m_editCurrentSigner->setPlainText(QString(signer));
        }
    } else {
        m_labelStatus->setText(T("签名加载失败"));
        m_labelStatus->setStyleSheet("color: red;");
    }
}

void SignatureDialog::onVerifySignature()
{
    MBOOL ret = verify(m_hdev);
    if (ret) {
        m_labelStatus->setText(T("签名验证通过"));
        m_labelStatus->setStyleSheet("color: green;");
        QMessageBox::information(this, T("验证结果"), T("签名验证成功，设备功能正常。"));
    } else {
        m_labelStatus->setText(T("签名验证失败"));
        m_labelStatus->setStyleSheet("color: red;");
        QMessageBox::warning(this, T("验证结果"),
            T("签名验证失败，设备功能可能受到限制。\n"
               "请确认签名信息是否正确。"));
    }
}

void SignatureDialog::onSaveSignature()
{
    // 先检查是否已加载签名
    const char *signer = get_verifysigner(m_hdev);
    if (!signer) {
        QMessageBox::warning(this, T("提示"), T("请先加载签名"));
        return;
    }

    int ret = save_verifysigner(m_hdev);
    if (ret > 0) {
        m_labelStatus->setText(T("签名已保存到设备"));
        m_labelStatus->setStyleSheet("color: green;");
        QMessageBox::information(this, T("提示"),
            T("签名已成功保存到设备，重启后仍然有效。"));
    } else {
        m_labelStatus->setText(T("保存失败"));
        m_labelStatus->setStyleSheet("color: red;");
        QMessageBox::warning(this, T("错误"), T("签名保存失败（返回 %1）").arg(ret));
    }
}

void SignatureDialog::onLoadFromFile()
{
    QString filePath = QFileDialog::getOpenFileName(this,
        T("打开签名文件"), QString(),
        T("签名文件 (*.mstar);;所有文件 (*)"));

    if (filePath.isEmpty()) return;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, T("错误"), T("无法打开文件"));
        return;
    }

    QString currentId = m_editDeviceId->text();
    QTextStream in(&file);
    QString foundSignature;
    bool found = false;

    // 遍历所有行，查找匹配的设备ID
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        // 解析格式: MAC地址 空格 ID号 空格 签名信息
        QStringList parts = line.split(QRegularExpression("\\s+"));
        if (parts.size() < 3) continue;

        QString fileId = parts[1];
        if (fileId == currentId) {
            foundSignature = parts.mid(2).join(" ");  // 签名可能包含空格
            found = true;
            break;
        }
    }
    file.close();

    if (!found) {
        QMessageBox::warning(this, T("未找到"),
            T("文件中没有找到与当前设备ID匹配的签名。\n"
               "当前设备ID: %1").arg(currentId));
        return;
    }

    // 填充签名到输入框
    m_editSignature->setPlainText(foundSignature);
    m_labelStatus->setText(T("已从文件加载签名，请点击“加载签名”按钮"));
    m_labelStatus->setStyleSheet("color: blue;");
}
