// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Mstar Semiconductor, Inc.

#include "MainWindow.h"
#include "ConnectDialog.h"
#include "NetworkSettingsDialog.h"
#include "DeviceInfoDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QStatusBar>
#include <QFileDialog>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QRegularExpression>
#include <QIcon>
#include <QLabel>
#include <QDesktopServices>
#include <QUrl>
#include <QApplication>
#include <QTemporaryFile>
#include <QTextDocument>
#include <algorithm>
#include "Translation.h"

// 前向声明
static QString markdownToHtml(const QString &md);
static QString formatInline(const QString &text);

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Mstar Oscilloscope");
    setWindowIcon(QIcon(":/image/logo.png"));
    resize(1000, 700);

    // --- 顶部工具栏 (按钮带图标) ---
    auto *topWidget = new QWidget;
    auto *topLayout = new QHBoxLayout(topWidget);
    topLayout->setContentsMargins(0, 0, 0, 0);

    m_btnAddDevice = new QPushButton(style()->standardIcon(QStyle::SP_DialogApplyButton), T("添加设备"));
    m_btnDisconnect = new QPushButton(style()->standardIcon(QStyle::SP_DialogCloseButton), T("断开连接"));
    m_btnDisconnect->setEnabled(false);
    topLayout->addWidget(m_btnAddDevice);
    topLayout->addWidget(m_btnDisconnect);
    topLayout->addSpacing(20);

    m_btnOpenFile = new QPushButton(style()->standardIcon(QStyle::SP_DialogOpenButton), T("打开数据文件"));
    topLayout->addWidget(m_btnOpenFile);
    m_btnAbout = new QPushButton(style()->standardIcon(QStyle::SP_DialogHelpButton), T("软件信息"));
    topLayout->addWidget(m_btnAbout);
    m_btnDllHelp = new QPushButton(style()->standardIcon(QStyle::SP_FileDialogDetailedView), T("DLL接口帮助"));
    topLayout->addWidget(m_btnDllHelp);
    topLayout->addSpacing(20);

    // 语言切换按钮
    m_btnLang = new QPushButton(Translation::instance().isEnglish() ? "中文" : "English");
    m_btnLang->setToolTip(T("切换语言 / Switch Language"));
    m_btnLang->setFixedWidth(60);
    topLayout->addWidget(m_btnLang);
    topLayout->addStretch();

    // --- 中部区域: 设备列表(大图标) + 折叠按钮 + 设备面板栈 ---
    m_deviceList = new QListWidget;
    m_deviceList->setViewMode(QListWidget::IconMode);
    m_deviceList->setIconSize(QSize(48, 48));
    m_deviceList->setSpacing(12);
    m_deviceList->setMinimumWidth(100);
    m_deviceList->setMaximumWidth(120);
    m_deviceList->setMovement(QListWidget::Static);
    m_deviceList->setFlow(QListWidget::TopToBottom);
    m_deviceList->setWrapping(false);
    m_deviceList->setResizeMode(QListWidget::Adjust);
    m_deviceList->setContextMenuPolicy(Qt::CustomContextMenu);
    m_deviceList->setSelectionMode(QAbstractItemView::SingleSelection);

    m_btnToggleList = new QPushButton("\xe2\x97\x80");  // ◀ 左箭头
    m_btnToggleList->setFixedSize(20, 60);
    m_btnToggleList->setToolTip(T("隐藏/显示设备列表"));

    m_stack = new QStackedWidget;

    auto *centerLayout = new QHBoxLayout;
    centerLayout->setContentsMargins(0, 0, 0, 0);
    centerLayout->addWidget(m_deviceList);
    centerLayout->addWidget(m_btnToggleList);
    centerLayout->addWidget(m_stack, 1);

    // --- 主布局 ---
    auto *central = new QWidget;
    auto *mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(4, 0, 4, 4);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(topWidget);
    mainLayout->addLayout(centerLayout, 1);
    setCentralWidget(central);
    statusBar()->showMessage("DLL Version: " + QString::number(get_dllapp_version()));

    // --- Signals ---
    connect(m_btnAddDevice, &QPushButton::clicked, this, &MainWindow::onAddDevice);
    connect(m_btnDisconnect, &QPushButton::clicked, this, &MainWindow::onDisconnect);
    connect(m_btnOpenFile, &QPushButton::clicked, this, &MainWindow::onOpenFile);
    connect(m_btnAbout, &QPushButton::clicked, this, &MainWindow::onAbout);
    connect(m_btnDllHelp, &QPushButton::clicked, this, &MainWindow::onDllHelp);
    connect(m_btnLang, &QPushButton::clicked, this, &MainWindow::onToggleLanguage);
    connect(m_deviceList, &QListWidget::itemClicked, this, &MainWindow::onDeviceListClicked);
    connect(m_deviceList, &QListWidget::customContextMenuRequested, this, &MainWindow::onDeviceListContextMenu);
    connect(m_btnToggleList, &QPushButton::clicked, this, &MainWindow::onToggleDeviceList);
}

MainWindow::~MainWindow()
{
    // 断开所有设备
    for (int i = m_panels.size() - 1; i >= 0; --i) {
        DevicePanel *devPanel = qobject_cast<DevicePanel*>(m_panels[i]);
        if (devPanel) {
            devPanel->disconnectDevice();
        }
        delete m_panels[i];
    }
    m_panels.clear();
}

void MainWindow::onAddDevice()
{
    ConnectDialog dlg(this);
    // 收集已连接设备的 IP 列表，防止重复添加
    QStringList existingIps;
    for (auto *widget : m_panels) {
        DevicePanel *devPanel = qobject_cast<DevicePanel*>(widget);
        if (devPanel) {
            existingIps << devPanel->deviceIp();
        }
    }
    dlg.setExistingDevices(existingIps);

    if (dlg.exec() != QDialog::Accepted) return;

    QString ip = dlg.selectedIp();
    if (ip.isEmpty()) return;

    std::string ipStr = ip.toStdString();
    int ret = mstar_ethbridge_open(ipStr.c_str());
    if (ret <= 0) {
        QMessageBox::warning(this, T("错误"), T("Open bridge failed!"));
        return;
    }

    MHANDLE hdev = get_device_ip(ipStr.c_str());
    if (!hdev) {
        QMessageBox::warning(this, T("错误"), T("No device found!"));
        return;
    }

    EthBridgeInfo devinfo;
    get_devinfor(hdev, &devinfo);
    int netSpeedMbps = devinfo.speed;

    int ret2 = collect_device_init(hdev);
    if (ret2 <= 0) {
        QMessageBox::warning(this, T("错误"), T("Device init failed!"));
        device_handle_release(hdev);
        return;
    }

    // 验证设备签名
    bool signatureValid = verify(hdev);
    if (!signatureValid) {
        QMessageBox::warning(this, T("设备签名验证失败"),
            T("设备未通过签名验证，剩余可用时间 3 小时。\n"
               "3 小时后设备将不执行任何总线读写命令（网络通信仍正常）。\n\n"
               "请联系杭州幻智星科技有限公司申请合法签名。\n"
               "邮箱：yupc123@qq.com\n"
               "电话：13867435883"));
    }

    collectinforst info;
    ret2 = get_collect_information(hdev, &info);
    if (ret2 <= 0) {
        QMessageBox::warning(this, T("错误"), T("Get collect info failed!"));
        device_handle_release(hdev);
        return;
    }

    // 创建设备面板
    auto *panel = new DevicePanel(ip, hdev, info, netSpeedMbps, this);
    panel->setDeviceMac(devinfo.mac);
    
    // MAC 设置后加载参数配置
    panel->loadDeviceProfile();

    // 设置设备信息标签
    panel->setDeviceInfoText(
        QString("Device: %1(%2) Port:%3  DLL Ver: %4")
            .arg(devinfo.mac).arg(devinfo.ip)
            .arg(devinfo.port)
            .arg(get_dllapp_version()));

    // 添加到栈和面板列表
    int stackIdx = m_stack->addWidget(panel);
    m_panels.append(panel);

    // 添加到设备列表（大图标模式）
    auto *listItem = new QListWidgetItem;
    QString displayName = panel->deviceModel();
    if (!signatureValid) displayName += T("\n(未验证)");
    listItem->setText(displayName);
    listItem->setIcon(QIcon(":/image/MScollect_icon.png"));
    listItem->setToolTip(T("型号: %1\nIP: %2\nMAC: %3\n主/从: %4\n签名: %5")
        .arg(panel->deviceModel())
        .arg(devinfo.ip)
        .arg(devinfo.mac)
        .arg(panel->masterIp().isEmpty() ? T("主设备") : T("从设备"))
        .arg(signatureValid ? T("已验证") : T("未验证")));
    listItem->setData(Qt::UserRole, stackIdx);
    m_deviceList->addItem(listItem);

    // 自动选中新设备
    m_deviceList->setCurrentItem(listItem);
    m_stack->setCurrentIndex(stackIdx);
    m_btnDisconnect->setEnabled(true);
}

void MainWindow::onDisconnect()
{
    QListWidgetItem *item = m_deviceList->currentItem();
    if (!item) return;
    int stackIdx = item->data(Qt::UserRole).toInt();
    if (stackIdx < 0 || stackIdx >= m_panels.size()) return;
    removePanel(stackIdx);
}

void MainWindow::removePanel(int index)
{
    if (index < 0 || index >= m_panels.size()) return;
    QWidget *widget = m_panels[index];
    
    // 判断面板类型
    DevicePanel *devPanel = qobject_cast<DevicePanel*>(widget);
    FileViewerPanel *filePanel = qobject_cast<FileViewerPanel*>(widget);
    
    QString ip;
    if (devPanel) {
        ip = devPanel->deviceIp();
        // 级联断开从设备（仅对真实设备）
        QVector<int> slaveIndices;
        for (int i = 0; i < m_panels.size(); ++i) {
            if (i != index) {
                DevicePanel *other = qobject_cast<DevicePanel*>(m_panels[i]);
                if (other && other->masterIp() == ip)
                    slaveIndices.append(i);
            }
        }
        // 先断从（倒序避免索引偏移）
        std::sort(slaveIndices.begin(), slaveIndices.end(), std::greater<int>());
        for (int si : slaveIndices)
            removePanel(si);
    }

    // 重新计算当前 index（可能因从设备移除而偏移）
    index = m_panels.indexOf(widget);
    if (index < 0) return;

    // 从列表中移除（通过 stackIdx 查找对应的 listItem）
    for (int i = 0; i < m_deviceList->count(); ++i) {
        QListWidgetItem *item = m_deviceList->item(i);
        if (item->data(Qt::UserRole).toInt() == index) {
            delete m_deviceList->takeItem(i);
            break;
        }
    }

    // 从栈和面板列表移除
    m_stack->removeWidget(widget);
    m_panels.remove(index);
    
    // 仅真实设备需要调用 disconnectDevice
    if (devPanel) {
        devPanel->disconnectDevice();
    }
    delete widget;

    // 重建所有列表项的索引
    for (int i = 0; i < m_deviceList->count(); ++i) {
        QListWidgetItem *item = m_deviceList->item(i);
        int oldIdx = item->data(Qt::UserRole).toInt();
        if (oldIdx > index) item->setData(Qt::UserRole, oldIdx - 1);
    }

    // 切换到其他设备（如有）
    if (m_panels.isEmpty()) {
        m_btnDisconnect->setEnabled(false);
    } else {
        m_deviceList->setCurrentRow(0);
        m_stack->setCurrentIndex(0);
    }
}

QWidget *MainWindow::panelFromItem(QListWidgetItem *item)
{
    if (!item) return nullptr;
    int idx = item->data(Qt::UserRole).toInt();
    if (idx >= 0 && idx < m_panels.size()) return m_panels[idx];
    return nullptr;
}

DevicePanel *MainWindow::devicePanelFromItem(QListWidgetItem *item)
{
    QWidget *w = panelFromItem(item);
    return qobject_cast<DevicePanel*>(w);
}

FileViewerPanel *MainWindow::filePanelFromItem(QListWidgetItem *item)
{
    QWidget *w = panelFromItem(item);
    return qobject_cast<FileViewerPanel*>(w);
}

void MainWindow::onDeviceListClicked(QListWidgetItem *item)
{
    QWidget *widget = panelFromItem(item);
    if (!widget) return;
    int idx = m_panels.indexOf(widget);
    if (idx >= 0) m_stack->setCurrentIndex(idx);
}

void MainWindow::onDeviceListContextMenu(const QPoint &pos)
{
    QListWidgetItem *item = m_deviceList->itemAt(pos);
    if (!item) return;
    
    DevicePanel *devPanel = devicePanelFromItem(item);
    FileViewerPanel *filePanel = filePanelFromItem(item);
    
    QMenu menu;
    
    if (devPanel) {
        // 真实设备的完整菜单
        menu.addAction(T("瞬时采集"), devPanel, &DevicePanel::onInstantCollect);
        QAction *actSingle = menu.addAction(T("单次触发"), devPanel, &DevicePanel::onSingleTriggerCollect);
        QAction *actMulti  = menu.addAction(T("连续触发"), devPanel, &DevicePanel::onMultiTriggerCollect);
        bool hasTrig = devPanel->hasTrigCapability();
        actSingle->setEnabled(hasTrig);
        actMulti->setEnabled(hasTrig);
        menu.addAction(T("连续采集"), devPanel, &DevicePanel::onContinuousCollect);
        QAction *actStop = menu.addAction(T("停止"), devPanel, &DevicePanel::onStopCollect);
        actStop->setEnabled(devPanel->isCollecting());
        menu.addSeparator();
        menu.addAction(T("保存数据"), devPanel, &DevicePanel::onSaveData);
        menu.addSeparator();
        menu.addAction(T("参数设置"), devPanel, &DevicePanel::onParamSettings);
        menu.addAction(T("网络设置"), this, &MainWindow::onNetworkSettings);
        menu.addAction(T("设备详情"), this, &MainWindow::onDeviceInfo);
        menu.addSeparator();
        menu.addAction(T("断开连接"), this, &MainWindow::onDisconnect);
    } else if (filePanel) {
        // 文件查看器的简化菜单
        menu.addAction(T("关闭文件"), this, &MainWindow::onDisconnect);
    } else {
        return;
    }

    menu.exec(m_deviceList->viewport()->mapToGlobal(pos));
}

void MainWindow::onToggleDeviceList()
{
    bool visible = m_deviceList->isVisible();
    m_deviceList->setVisible(!visible);
    m_btnToggleList->setText(visible ? "\xe2\x96\xb6" : "\xe2\x97\x80");  // ▶ / ◀
}

void MainWindow::onOpenFile()
{
    QString filePath = QFileDialog::getOpenFileName(this, T("打开数据文件"),
        QDir::homePath(), T("CSV 文件 (*.csv)"));
    if (filePath.isEmpty()) return;

    // 创建文件查看器面板（虚拟设备，无采集功能）
    auto *panel = new FileViewerPanel(filePath, this);
    if (!panel->loadCsvFile(filePath)) {
        delete panel;
        QMessageBox::warning(this, T("错误"), T("无法加载文件: %1").arg(filePath));
        return;
    }

    // 添加到栈和面板列表
    int stackIdx = m_stack->addWidget(panel);
    m_panels.append(panel);

    // 添加到设备列表（大图标模式）
    auto *listItem = new QListWidgetItem;
    // 显示名：去掉扩展名，过长则截断
    QString displayName = QFileInfo(filePath).baseName();
    if (displayName.length() > 12) displayName = displayName.left(10) + "..";
    listItem->setText(displayName);
    listItem->setIcon(QIcon(":/image/MSOscilloscope_icon.png"));
    listItem->setToolTip(T("文件: %1\n采样率: %2 Hz")
        .arg(panel->filePath())
        .arg("--"));  // 采样率信息在 panel 内部
    listItem->setData(Qt::UserRole, stackIdx);
    m_deviceList->addItem(listItem);

    // 选中新面板
    m_deviceList->setCurrentItem(listItem);
    m_stack->setCurrentIndex(stackIdx);
    m_btnDisconnect->setEnabled(true);
}

void MainWindow::onNetworkSettings()
{
    QListWidgetItem *item = m_deviceList->currentItem();
    if (!item) return;
    DevicePanel *devPanel = devicePanelFromItem(item);
    if (!devPanel) return;

    NetworkSettingsDialog dlg(this);
    dlg.setMacAddress(devPanel->deviceMac());
    dlg.setIpAddress(devPanel->deviceIp());
    if (dlg.exec() != QDialog::Accepted) return;

    QString newMac = dlg.macAddress().trimmed();
    QString newIp = dlg.ipAddress().trimmed();
    bool macChanged = (newMac != devPanel->deviceMac());
    bool ipChanged = (newIp != devPanel->deviceIp());
    if (!macChanged && !ipChanged) return;

    // 验证 MAC 地址格式：XX:XX:XX:XX:XX:XX
    if (macChanged) {
        QRegularExpression macRegex("^([0-9A-Fa-f]{2}:){5}[0-9A-Fa-f]{2}$");
        if (!macRegex.match(newMac).hasMatch()) {
            QMessageBox::warning(this, T("格式错误"),
                T("MAC 地址格式不正确，应为 XX:XX:XX:XX:XX:XX\n例如: 00:1A:2B:3C:4D:5E"));
            return;
        }
    }

    // 验证 IP 地址格式
    if (ipChanged) {
        QRegularExpression ipRegex("^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
        if (!ipRegex.match(newIp).hasMatch()) {
            QMessageBox::warning(this, T("格式错误"),
                T("IP 地址格式不正确，应为 xxx.xxx.xxx.xxx\n例如: 192.168.0.100"));
            return;
        }
    }

    QString confirmMsg = T("确认要修改以下网络参数？\n");
    if (macChanged) confirmMsg += T("MAC 地址: %1 → %2\n").arg(devPanel->deviceMac(), newMac);
    if (ipChanged) confirmMsg += T("IP 地址: %1 → %2\n").arg(devPanel->deviceIp(), newIp);
    confirmMsg += T("\n修改后需断电重启设备才能生效。");
    if (QMessageBox::question(this, T("确认修改"), confirmMsg,
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes) return;

    MHANDLE hdev = devPanel->hdev();
    if (!hdev) return;

    if (macChanged) {
        int ret = set_device_mac(hdev, newMac.toStdString().c_str());
        if (ret <= 0) {
            QMessageBox::warning(this, T("错误"),
                T("修改 MAC 地址失败（返回 %1）。").arg(ret));
        }
    }
    if (ipChanged) {
        int ret = set_device_ip(hdev, newIp.toStdString().c_str());
        if (ret <= 0) {
            QMessageBox::warning(this, T("错误"),
                T("修改 IP 地址失败（返回 %1）。").arg(ret));
        }
    }
    QMessageBox::information(this, T("提示"),
        T("MAC地址、IP地址修改成功后，要断电重启设备，新地址才能生效。"));

    auto ret = QMessageBox::question(this, T("断开连接"),
        T("是否立即断开当前设备连接？\n断电重启后可手动重新添加设备。"),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    if (ret == QMessageBox::Yes) {
        onDisconnect();
    }
}

void MainWindow::onDeviceInfo()
{
    QListWidgetItem *item = m_deviceList->currentItem();
    if (!item) return;
    DevicePanel *devPanel = devicePanelFromItem(item);
    if (!devPanel) return;

    MHANDLE hdev = devPanel->hdev();
    if (!hdev) return;

    EthBridgeInfo info;
    get_devinfor(hdev, &info);

    collectinforst collectInfo;
    get_collect_information(hdev, &collectInfo);

    const char *devId = get_device_id(hdev);

    DeviceInfoDialog dlg(this);
    dlg.setDeviceHandle(hdev);
    dlg.setDeviceInfo(info);
    dlg.setCollectInfo(collectInfo);
    if (devId) dlg.setDeviceId(QString(devId));
    dlg.exec();
}

void MainWindow::onAbout()
{
    QMessageBox msgBox(this);
    msgBox.setWindowTitle(T("软件信息"));
    msgBox.setTextFormat(Qt::RichText);
    msgBox.setText(
        T("<h2>Mstar Oscilloscope</h2>"
           "<p>版本: V%1</p>"
           "<p>多设备示波器采集软件</p>"
           "<p>支持多设备并行采集，主从设备树形管理</p>"
           "<p>Copyright (C) 2026 Mstar Semiconductor, Inc.</p>"
           "<p>Email：yupc123@qq.com</p>")
            .arg(get_dllapp_version()));
    msgBox.setIconPixmap(QPixmap(":/image/logo.png").scaled(128, 128,
        Qt::KeepAspectRatio, Qt::SmoothTransformation));
    msgBox.exec();
}

void MainWindow::onDllHelp()
{
    // 查找帮助文件：优先在 exe 同级目录找，其次在源码 include 目录找
    QString helpFile;
    QStringList searchPaths = {
        QApplication::applicationDirPath(),
        QApplication::applicationDirPath() + "/../include",
        QApplication::applicationDirPath() + "/../../include",
    };
    for (const QString &dir : searchPaths) {
        QString candidate = dir + "/MStarEthernetBridge_API_Help.md";
        if (QFile::exists(candidate)) {
            helpFile = candidate;
            break;
        }
    }

    if (helpFile.isEmpty()) {
        QMessageBox::warning(this, T("提示"),
            T("未找到 DLL 接口帮助文件。\n"
               "请确认 MStarEthernetBridge_API_Help.md 存在于程序目录中。"));
        return;
    }

    // 读取 Markdown 文件并转换为 HTML
    QFile mdFile(helpFile);
    if (!mdFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, T("错误"), T("无法读取帮助文件。"));
        return;
    }
    QString mdContent = QString::fromUtf8(mdFile.readAll());
    mdFile.close();

    // 简单 Markdown -> HTML 转换
    QString html = markdownToHtml(mdContent);

    // 写入临时 HTML 文件并用浏览器打开
    QString tempPath = QDir::tempPath() + "/MStarEthernetBridge_API_Help.html";
    QFile htmlFile(tempPath);
    if (htmlFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        htmlFile.write(html.toUtf8());
        htmlFile.close();
        QDesktopServices::openUrl(QUrl::fromLocalFile(tempPath));
    }
}

void MainWindow::onToggleLanguage()
{
    auto &trans = Translation::instance();
    QString newLang = trans.isEnglish() ? "zh" : "en";
    trans.setLanguage(newLang);

    // 更新按钮文本
    m_btnLang->setText(trans.isEnglish() ? "中文" : "English");

    // 提示重启生效
    QMessageBox::information(this, T("提示"),
        T("语言已切换，部分界面需要重启后生效。"));
}

// 简易 Markdown -> HTML 转换（支持标题、表格、代码块、粗体、列表、分隔线）
static QString markdownToHtml(const QString &md)
{
    QStringList lines = md.split('\n');
    QString html;
    html += "<!DOCTYPE html><html><head><meta charset=\"utf-8\">"
            "<style>"
            "body{font-family:'Microsoft YaHei','Segoe UI',sans-serif;max-width:960px;margin:0 auto;padding:20px;background:#fafafa;color:#333;}"
            "h1{color:#1a5276;border-bottom:2px solid #1a5276;padding-bottom:8px;}"
            "h2{color:#2471a3;border-bottom:1px solid #ddd;padding-bottom:5px;margin-top:30px;}"
            "h3{color:#2e86c1;margin-top:20px;}"
            "h4{color:#3498db;}"
            "table{border-collapse:collapse;width:100%;margin:12px 0;}"
            "th,td{border:1px solid #ccc;padding:6px 10px;text-align:left;}"
            "th{background:#eaf2f8;font-weight:bold;}"
            "tr:nth-child(even){background:#f8f9fa;}"
            "code{background:#f0f0f0;padding:2px 5px;border-radius:3px;font-family:Consolas,'Courier New',monospace;font-size:0.9em;}"
            "pre{background:#2d2d2d;color:#f8f8f2;padding:14px;border-radius:6px;overflow-x:auto;font-size:0.88em;line-height:1.5;}"
            "pre code{background:none;color:inherit;padding:0;}"
            "blockquote{border-left:4px solid #3498db;margin:10px 0;padding:8px 16px;background:#eef7ff;color:#555;}"
            "hr{border:none;border-top:1px solid #ddd;margin:20px 0;}"
            "a{color:#2980b9;text-decoration:none;}"
            "a:hover{text-decoration:underline;}"
            ".note{background:#fff3cd;border-left:4px solid #ffc107;padding:8px 14px;margin:10px 0;border-radius:3px;}"
            "</style></head><body>\n";

    bool inCodeBlock = false;
    bool inTable = false;
    bool tableHeaderDone = false;

    for (int i = 0; i < lines.size(); ++i) {
        QString line = lines[i];

        // 代码块 ```
        if (line.trimmed().startsWith("```")) {
            if (!inCodeBlock) {
                if (inTable) { html += "</tbody></table>\n"; inTable = false; }
                html += "<pre><code>";
                inCodeBlock = true;
            } else {
                html += "</code></pre>\n";
                inCodeBlock = false;
            }
            continue;
        }
        if (inCodeBlock) {
            html += line.toHtmlEscaped() + "\n";
            continue;
        }

        // 空行
        if (line.trimmed().isEmpty()) {
            if (inTable) { html += "</tbody></table>\n"; inTable = false; tableHeaderDone = false; }
            continue;
        }

        // 水平线 ---
        if (QRegularExpression("^-{3,}$").match(line.trimmed()).hasMatch()) {
            if (inTable) { html += "</tbody></table>\n"; inTable = false; tableHeaderDone = false; }
            html += "<hr>\n";
            continue;
        }

        // 标题 # ## ### ####
        QRegularExpression headingRe("^(#{1,4})\\s+(.+)$");
        auto headingMatch = headingRe.match(line);
        if (headingMatch.hasMatch()) {
            if (inTable) { html += "</tbody></table>\n"; inTable = false; tableHeaderDone = false; }
            int level = headingMatch.captured(1).length();
            QString text = formatInline(headingMatch.captured(2));
            html += QString("<h%1>%2</h%1>\n").arg(level).arg(text);
            continue;
        }

        // 表格行 |
        if (line.trimmed().startsWith("|") && line.trimmed().endsWith("|")) {
            // 检查是否是分隔行 |---|---|
            if (QRegularExpression("^\\|[\\s\\-:|]+\\|$").match(line.trimmed()).hasMatch()) {
                tableHeaderDone = true;
                continue;
            }
            if (!inTable) {
                html += "<table>\n<thead>\n";
                inTable = true;
                tableHeaderDone = false;
            }
            QStringList cells = line.trimmed().mid(1, line.trimmed().length() - 2).split("|");
            if (!tableHeaderDone) {
                html += "<tr>";
                for (const QString &cell : cells)
                    html += "<th>" + formatInline(cell.trimmed()) + "</th>";
                html += "</tr>\n</thead>\n<tbody>\n";
            } else {
                html += "<tr>";
                for (const QString &cell : cells)
                    html += "<td>" + formatInline(cell.trimmed()) + "</td>";
                html += "</tr>\n";
            }
            continue;
        }

        // 普通段落
        if (inTable) { html += "</tbody></table>\n"; inTable = false; tableHeaderDone = false; }
        html += "<p>" + formatInline(line) + "</p>\n";
    }

    if (inTable) html += "</tbody></table>\n";
    if (inCodeBlock) html += "</code></pre>\n";

    html += "</body></html>";
    return html;
}

// 处理行内格式：粗体、行内代码、链接
static QString formatInline(const QString &text)
{
    QString s = text.toHtmlEscaped();
    // 行内代码 `code`
    s.replace(QRegularExpression("`([^`]+)`"), "<code>\\1</code>");
    // 粗体 **text**
    s.replace(QRegularExpression("\\*\\*([^*]+)\\*\\*"), "<strong>\\1</strong>");
    // 斜体 *text*
    s.replace(QRegularExpression("\\*([^*]+)\\*"), "<em>\\1</em>");
    // 链接 [text](url)
    s.replace(QRegularExpression("\\[([^\\]]+)\\]\\(([^)]+)\\)"), "<a href=\"\\2\">\\1</a>");
    return s;
}
