// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Mstar Semiconductor, Inc.

#include "Translation.h"
#include <QLocale>
#include <QSettings>

Translation &Translation::instance()
{
    static Translation s;
    return s;
}

Translation::Translation()
{
    // 从 QSettings 读取上次选择的语言，默认跟随系统 locale
    QSettings settings("MstarOscilloscope", "MstarOscilloscope");
    m_lang = settings.value("language", "").toString();
    if (m_lang.isEmpty()) {
        m_lang = QLocale::system().name().startsWith("zh") ? "zh" : "en";
    }
    buildMap();
}

void Translation::setLanguage(const QString &lang)
{
    if (m_lang == lang) return;
    m_lang = lang;
    QSettings settings("MstarOscilloscope", "MstarOscilloscope");
    settings.setValue("language", lang);
    emit languageChanged();
}

QString Translation::t(const QString &source) const
{
    if (m_lang == "zh") return source;
    return m_map.value(source, source);  // 未找到翻译则返回原文
}

void Translation::buildMap()
{
    // ---- 通用 ----
    m_map["错误"] = "Error";
    m_map["提示"] = "Notice";
    m_map["确定"] = "OK";
    m_map["取消"] = "Cancel";
    m_map["关闭"] = "Close";
    m_map["停止"] = "Stop";
    m_map["Ready"] = "Ready";
    m_map["未知"] = "Unknown";
    m_map["已验证"] = "Verified";
    m_map["未验证"] = "Unverified";
    m_map["格式错误"] = "Format Error";

    // ---- MainWindow ----
    m_map["添加设备"] = "Add Device";
    m_map["断开连接"] = "Disconnect";
    m_map["打开数据文件"] = "Open Data File";
    m_map["软件信息"] = "About";
    m_map["DLL接口帮助"] = "DLL API Help";
    m_map["隐藏/显示设备列表"] = "Hide/Show device list";
    m_map["\n(未验证)"] = "\n(Unverified)";
    m_map["主设备"] = "Master";
    m_map["从设备"] = "Slave";
    m_map["瞬时采集"] = "Instant";
    m_map["单次触发"] = "Single Trigger";
    m_map["连续触发"] = "Multi Trigger";
    m_map["连续采集"] = "Continuous";
    m_map["保存数据"] = "Save Data";
    m_map["参数设置"] = "Parameters";
    m_map["网络设置"] = "Network Settings";
    m_map["设备详情"] = "Device Info";
    m_map["关闭文件"] = "Close File";
    m_map["CSV 文件 (*.csv)"] = "CSV Files (*.csv)";
    m_map["Open bridge failed!"] = "Open bridge failed!";
    m_map["No device found!"] = "No device found!";
    m_map["Device init failed!"] = "Device init failed!";
    m_map["Get collect info failed!"] = "Get collect info failed!";
    m_map["无法加载文件: %1"] = "Cannot load file: %1";
    m_map["设备签名验证失败"] = "Signature Verification Failed";
    m_map["设备未通过签名验证，剩余可用时间 3 小时。\n"
           "3 小时后设备将不执行任何总线读写命令（网络通信仍正常）。\n\n"
           "请联系杭州幻智星科技有限公司申请合法签名。\n"
           "邮箱：yupc123@qq.com\n"
           "电话：13867435883"] =
        "Device signature verification failed. 3 hours of use remaining.\n"
        "After 3 hours, the device will not execute any bus read/write commands (network communication still works).\n\n"
        "Contact Mstar Technology for a valid signature.\n"
        "Email: yupc123@qq.com\n"
        "Phone: 13867435883";
    m_map["型号: %1\nIP: %2\nMAC: %3\n主/从: %4\n签名: %5"] = "Model: %1\nIP: %2\nMAC: %3\nMaster/Slave: %4\nSignature: %5";
    m_map["文件: %1\n采样率: %2 Hz"] = "File: %1\nRate: %2 Hz";
    m_map["MAC 地址: %1 → %2\n"] = "MAC Address: %1 → %2\n";
    m_map["IP 地址: %1 → %2\n"] = "IP Address: %1 → %2\n";
    m_map["\n修改后需断电重启设备才能生效。"] = "\nPower cycle required for changes to take effect.";
    m_map["设备 %1 已连接，请勿重复添加"] = "Device %1 is already connected";
    m_map["MAC 地址格式不正确，应为 XX:XX:XX:XX:XX:XX\n例如: 00:1A:2B:3C:4D:5E"] =
        "Invalid MAC format. Expected: XX:XX:XX:XX:XX:XX\nExample: 00:1A:2B:3C:4D:5E";
    m_map["IP 地址格式不正确，应为 xxx.xxx.xxx.xxx\n例如: 192.168.0.100"] =
        "Invalid IP format. Expected: xxx.xxx.xxx.xxx\nExample: 192.168.0.100";
    m_map["确认要修改以下网络参数？\n"] = "Modify the following network parameters?\n";
    m_map["修改后需断电重启设备才能生效。"] = "Power cycle required for changes to take effect.";
    m_map["确认修改"] = "Confirm Changes";
    m_map["修改 MAC 地址失败（返回 %1）。"] = "Failed to set MAC address (returned %1).";
    m_map["修改 IP 地址失败（返回 %1）。"] = "Failed to set IP address (returned %1).";
    m_map["MAC地址、IP地址修改成功后，要断电重启设备，新地址才能生效。"] =
        "Power cycle the device for new MAC/IP to take effect.";
    m_map["是否立即断开当前设备连接？\n断电重启后可手动重新添加设备。"] =
        "Disconnect now?\nYou can re-add the device after power cycling.";
    m_map["未找到 DLL 接口帮助文件。\n请确认 MStarEthernetBridge_API_Help.md 存在于程序目录中。"] =
        "DLL API help file not found.\nPlease ensure MStarEthernetBridge_API_Help.md exists.";
    m_map["无法读取帮助文件。"] = "Cannot read help file.";

    // ---- DevicePanel ----
    m_map["Time (s)"] = "Time (s)";
    m_map["Voltage (V)"] = "Voltage (V)";
    m_map["指示器显示通道"] = "Indicator channel";
    m_map["波形显示"] = "Waveform Display";
    m_map["显示使能"] = "Display Enable";
    m_map["CH%1 波形偏移 (V)"] = "CH%1 offset (V)";
    m_map["状态：空闲（%1CH %2bit %3Msps）"] = "Status: Idle (%1CH %2bit %3Msps)";
    m_map["状态：%1 中..."] = "Status: %1 in progress...";
    m_map["状态：空闲（%1 采样点）"] = "Status: Idle (%1 samples)";
    m_map["状态：正在数据传输"] = "Status: Transferring data";
    m_map["状态：空闲（DMA 超时）"] = "Status: Idle (DMA timeout)";
    m_map["状态：空闲（读取失败）"] = "Status: Idle (Read failed)";
    m_map["状态：采集继续（已放弃保存）"] = "Status: Collecting (save aborted)";
    m_map["请先打开参数设置对话框配置主机缓存大小。"] = "Please open Parameters to configure host buffer.";
    m_map["确认开始连续采集？"] = "Start continuous acquisition?";
    m_map["同时保存数据到文件"] = "Also save data to file";
    m_map["带宽超限"] = "Bandwidth Exceeded";
    m_map["磁盘写入速度跟不上采集速度，数据正在丢失。\n"] = "Disk write cannot keep up with acquisition, data is being lost.\n";
    m_map["数据速率 %1 MB/s 超过网口有效带宽 %2 MB/s（40%利用率）。\n"] = "Data rate %1 MB/s exceeds effective network bandwidth %2 MB/s (40% utilization).\n";
    m_map["Collect start failed!"] = "Collect start failed!";
    m_map["保存速度不足"] = "Save Speed Insufficient";
    m_map["保存采集数据"] = "Save Acquisition Data";
    m_map["无法创建文件: %1"] = "Cannot create file: %1";
    m_map["数据已保存: %1 (%2 采样点)"] = "Data saved: %1 (%2 samples)";
    m_map["自动保存失败: 无法创建文件 %1"] = "Auto-save failed: %1";
    m_map["数据已自动保存: %1 (%2 采样点)"] = "Data auto-saved: %1 (%2 samples)";
    m_map["采集进行中，无法设置参数。请先停止采集。"] = "Acquisition in progress. Stop first.";

    // ---- CollectParamDialog ----
    m_map["采集参数设置"] = "Acquisition Parameters";
    m_map["基本参数"] = "Basic Parameters";
    m_map["采样频率:"] = "Sample Rate:";
    m_map["过采样:"] = "Oversample:";
    m_map["1倍（无过采样）"] = "1x (None)";
    m_map["2倍"] = "2x";
    m_map["4倍"] = "4x";
    m_map["8倍"] = "8x";
    m_map["16倍"] = "16x";
    m_map["32倍"] = "32x";
    m_map["64倍"] = "64x";
    m_map["128倍"] = "128x";
    m_map["256倍"] = "256x";
    m_map["设备缓存:"] = "Device Buffer:";
    m_map["预触发:"] = "Pre-trigger:";
    m_map["板间同步"] = "Board Sync";
    m_map["同步模式:"] = "Sync Mode:";
    m_map["无级联"] = "None";
    m_map["主设备（CLK输出）"] = "Master (CLK Out)";
    m_map["从设备（CLK输入）"] = "Slave (CLK In)";
    m_map["触发信号:"] = "Trigger Signal:";
    m_map["输出"] = "Output";
    m_map["输入"] = "Input";
    m_map["主设备IP:"] = "Master IP:";
    m_map["设备发送速度"] = "Device Send Speed";
    m_map["包间隔:"] = "Frame Delay:";
    m_map["注: 1000M网速时，20us大约降一半速度"] = "Note: At 1000M, 20us ~halves speed";
    m_map["数据包发送间隔时间（微秒），主动降低网口传输负载，可同一网口同时采集多台设备"] =
        "Frame delay in microseconds. Reduces network load, allows multiple devices on same network.";
    m_map["实时采集"] = "Real-time Capture";
    m_map["主机缓存:"] = "Host Buffer:";
    m_map["保存使能"] = "Save Enable";
    m_map["保存地址:"] = "Save Path:";
    m_map["选择保存目录"] = "Select save directory";
    m_map["浏览..."] = "Browse...";
    m_map["文件前缀:"] = "File Prefix:";
    m_map["通道设置"] = "Channel Settings";
    m_map["通道"] = "Channel";
    m_map["采集使能"] = "Collect En";
    m_map["触发使能"] = "Trigger En";
    m_map["触发模式"] = "Trigger Mode";
    m_map["触发值(V)"] = "Trig Value(V)";
    m_map["量程"] = "Range";
    m_map["带宽"] = "Bandwidth";
    m_map["偏移"] = "Offset";
    m_map["电平触发"] = "Level";
    m_map["上升沿触发"] = "Rising Edge";
    m_map["下降沿触发"] = "Falling Edge";
    m_map["高"] = "High";
    m_map["低"] = "Low";

    // ---- ConnectDialog ----
    m_map["Connect Device"] = "Connect Device";
    m_map["Device IP"] = "Device IP";
    m_map["Auto Scan"] = "Auto Scan";
    m_map["Scan Devices"] = "Scan Devices";
    m_map["Connect"] = "Connect";
    m_map["No device found"] = "No device found";
    m_map["No new device found (all connected)"] = "No new device found (all connected)";
    m_map["Please enter device IP address!"] = "Please enter device IP address!";

    // ---- DeviceInfoDialog ----
    m_map["设备详情"] = "Device Info";
    m_map["加载签名"] = "Load Signature";
    m_map["功能状态:"] = "Status:";
    m_map["协议版本:"] = "Protocol:";
    m_map["模块开发时间:"] = "Build Date:";
    m_map["IP 地址:"] = "IP Address:";
    m_map["MAC 地址:"] = "MAC Address:";
    m_map["UDP 端口:"] = "UDP Port:";
    m_map["网口速率:"] = "Link Speed:";
    m_map["主控端数量:"] = "Master Count:";
    m_map["加密状态:"] = "Crypto:";
    m_map["设备 ID:"] = "Device ID:";
    m_map["--- 采集信息 ---"] = "--- Acquisition Info ---";
    m_map["设备型号:"] = "Model:";
    m_map["采集版本:"] = "Collect Ver:";
    m_map["最高采样率:"] = "Max Rate:";
    m_map["缓存大小:"] = "Buffer:";
    m_map["通道数:"] = "Channels:";
    m_map["AD 精度:"] = "AD Resolution:";
    m_map["AD 芯片数:"] = "AD Chips:";
    m_map["量程数组大小:"] = "Range Array:";
    m_map["写"] = "Write";
    m_map["读"] = "Read";
    m_map["未使能"] = "Disabled";
    m_map["%1年%2月"] = "%1/%2";
    m_map["%1 位"] = "%1 bit";
    m_map["AXI Full 总线: %1, %2位地址, %3位数据"] = "AXI Full Bus: %1, %2-bit addr, %3-bit data";
    m_map["主版本号:"] = "Major Ver:";
    m_map["次版本号:"] = "Minor Ver:";

    // ---- SignatureDialog ----
    m_map["签名验证"] = "Signature Verification";
    m_map["请输入签名信息"] = "Please enter signature";
    m_map["当前签名:"] = "Current:";
    m_map["新签名:"] = "New:";
    m_map["从文件加载"] = "Load from File";
    m_map["验证签名"] = "Verify";
    m_map["保存到设备"] = "Save to Device";
    m_map["(无)"] = "(None)";
    m_map["签名加载成功"] = "Signature loaded";
    m_map["签名加载失败"] = "Signature load failed";
    m_map["签名验证通过"] = "Verification passed";
    m_map["验证结果"] = "Result";
    m_map["签名验证成功，设备功能正常。"] = "Verification passed. Device OK.";
    m_map["签名验证失败"] = "Verification failed";
    m_map["请先加载签名"] = "Load signature first";
    m_map["签名已保存到设备"] = "Signature saved";
    m_map["保存失败"] = "Save failed";
    m_map["签名保存失败（返回 %1）"] = "Save failed (ret %1)";
    m_map["打开签名文件"] = "Open Signature File";
    m_map["签名文件 (*.mstar);;所有文件 (*)"] = "Signature (*.mstar);;All (*)";
    m_map["无法打开文件"] = "Cannot open file";
    m_map["未找到"] = "Not Found";
    m_map["已从文件加载签名，请点击\"加载签名\"按钮"] = "Loaded from file. Click \"Load Signature\".";
    m_map["签名验证失败，设备功能可能受到限制。\n"] = "Verification failed. Device features may be limited.\n";
    m_map["签名已成功保存到设备，重启后仍然有效。"] = "Signature saved successfully. Persists after reboot.";
    m_map["文件中没有找到与当前设备ID匹配的签名。\n"] = "No signature matching current device ID found in file.\n";

    // ---- NetworkSettingsDialog ----
    m_map["网络设置"] = "Network Settings";
    m_map["MAC 地址:"] = "MAC Address:";
    m_map["IP 地址:"] = "IP Address:";
    m_map["注意：MAC 地址在网络中是唯一的标识，修改后若与其他设备重复将导致通信异常，请谨慎操作。"] =
        "Note: MAC address must be unique on the network. Duplicate MAC may cause communication issues.";

    // ---- FileViewerPanel ----
    m_map["文件: "] = "File: ";
    m_map["采样率: --"] = "Rate: --";
    m_map["采样率: %1 Hz"] = "Rate: %1 Hz";
    m_map["错误: 无法打开文件"] = "Error: Cannot open file";
    m_map["错误: 文件中没有有效数据"] = "Error: No valid data";

    // ---- MainWindow about dialog ----
    m_map["<h2>Mstar Oscilloscope</h2>"
           "<p>版本: V%1</p>"
           "<p>多设备示波器采集软件</p>"
           "<p>支持多设备并行采集，主从设备树形管理</p>"
           "<p>Copyright (C) 2026 Mstar Semiconductor, Inc.</p>"
           "<p>Email：yupc123@qq.com</p>"] =
        "<h2>Mstar Oscilloscope</h2>"
        "<p>Version: V%1</p>"
        "<p>Multi-device oscilloscope acquisition software</p>"
        "<p>Supports parallel acquisition and master-slave tree management</p>"
        "<p>Copyright (C) 2026 Mstar Semiconductor, Inc.</p>"
        "<p>Email: yupc123@qq.com</p>";

    // ---- Language toggle ----
    m_map["切换语言 / Switch Language"] = "Switch Language";
    m_map["语言已切换，部分界面需要重启后生效。"] = "Language switched. Restart to take full effect.";

    // ---- CollectParamDialog range names ----
    m_map["\u00b12.5V(\u5355\u7aef)"] = "\u00b12.5V (Single)";
    m_map["\u00b15V(\u5355\u7aef)"] = "\u00b15V (Single)";
    m_map["\u00b16.25V(\u5355\u7aef)"] = "\u00b16.25V (Single)";
    m_map["\u00b110V(\u5355\u7aef)"] = "\u00b110V (Single)";
    m_map["\u00b112.5V(\u5355\u7aef)"] = "\u00b112.5V (Single)";
    m_map["0~5V(\u5355\u7aef)"] = "0~5V (Single)";
    m_map["0~10V(\u5355\u7aef)"] = "0~10V (Single)";
    m_map["0~12.5V(\u5355\u7aef)"] = "0~12.5V (Single)";
    m_map["\u00b15V(\u5dee\u5206)"] = "\u00b15V (Diff)";
    m_map["\u00b110V(\u5dee\u5206)"] = "\u00b110V (Diff)";
    m_map["\u00b112.5V(\u5dee\u5206)"] = "\u00b112.5V (Diff)";
    m_map["\u00b120V(\u5dee\u5206)"] = "\u00b120V (Diff)";
    m_map["\u00b125V(\u5dee\u5206)"] = "\u00b125V (Diff)";
    m_map["\u00b150V(\u5dee\u5206)"] = "\u00b150V (Diff)";
    m_map["\u00b1100V(\u5dee\u5206)"] = "\u00b1100V (Diff)";
    m_map["\u00b1250V(\u5dee\u5206)"] = "\u00b1250V (Diff)";
    m_map["\u00b1500V(\u5dee\u5206)"] = "\u00b1500V (Diff)";
    m_map["\u00b150V(\u5355\u7aef)"] = "\u00b150V (Single)";
}
