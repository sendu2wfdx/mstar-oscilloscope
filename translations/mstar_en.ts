<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="en_US">
<!--
  MstarOscilloscope English Translation
  Source language: Chinese (zh_CN)
  Run: cmake --build . --target lupdate   (to refresh source strings)
  Then edit this file and rebuild.
-->

<!-- ==================== CollectParamDialog ==================== -->
<context>
    <name>CollectParamDialog</name>
    <message><source>采集参数设置</source><translation>Acquisition Parameters</translation></message>
    <message><source>基本参数</source><translation>Basic Parameters</translation></message>
    <message><source>采样频率:</source><translation>Sample Rate:</translation></message>
    <message><source>过采样:</source><translation>Oversample:</translation></message>
    <message><source>1倍（无过采样）</source><translation>1x (No oversampling)</translation></message>
    <message><source>2倍</source><translation>2x</translation></message>
    <message><source>4倍</source><translation>4x</translation></message>
    <message><source>8倍</source><translation>8x</translation></message>
    <message><source>16倍</source><translation>16x</translation></message>
    <message><source>32倍</source><translation>32x</translation></message>
    <message><source>64倍</source><translation>64x</translation></message>
    <message><source>128倍</source><translation>128x</translation></message>
    <message><source>256倍</source><translation>256x</translation></message>
    <message><source>设备缓存:</source><translation>Device Buffer:</translation></message>
    <message><source>预触发:</source><translation>Pre-trigger:</translation></message>
    <message><source>板间同步</source><translation>Board Sync</translation></message>
    <message><source>同步模式:</source><translation>Sync Mode:</translation></message>
    <message><source>无级联</source><translation>None</translation></message>
    <message><source>主设备（CLK输出）</source><translation>Master (CLK Output)</translation></message>
    <message><source>从设备（CLK输入）</source><translation>Slave (CLK Input)</translation></message>
    <message><source>触发信号:</source><translation>Trigger Signal:</translation></message>
    <message><source>输出</source><translation>Output</translation></message>
    <message><source>输入</source><translation>Input</translation></message>
    <message><source>主设备IP:</source><translation>Master IP:</translation></message>
    <message><source>设备发送速度</source><translation>Device Send Speed</translation></message>
    <message><source>包间隔:</source><translation>Frame Delay:</translation></message>
    <message><source>数据包发送间隔时间（微秒），主动降低网口传输负载，可同一网口同时采集多台设备</source><translation>Packet send interval (us). Reduces network load to allow multiple devices on the same network.</translation></message>
    <message><source>注: 1000M网速时，20us大约降一半速度</source><translation>Note: At 1000M, 20us roughly halves the speed</translation></message>
    <message><source>实时采集</source><translation>Real-time Acquisition</translation></message>
    <message><source>主机缓存:</source><translation>Host Buffer:</translation></message>
    <message><source>保存使能</source><translation>Save Enable</translation></message>
    <message><source>保存地址:</source><translation>Save Path:</translation></message>
    <message><source>选择保存目录</source><translation>Select save directory</translation></message>
    <message><source>浏览...</source><translation>Browse...</translation></message>
    <message><source>文件前缀:</source><translation>File Prefix:</translation></message>
    <message><source>通道设置</source><translation>Channel Settings</translation></message>
    <message><source>通道</source><translation>Channel</translation></message>
    <message><source>采集使能</source><translation>Collect Enable</translation></message>
    <message><source>触发使能</source><translation>Trigger Enable</translation></message>
    <message><source>触发模式</source><translation>Trigger Mode</translation></message>
    <message><source>触发值(V)</source><translation>Trigger Value (V)</translation></message>
    <message><source>量程</source><translation>Range</translation></message>
    <message><source>带宽</source><translation>Bandwidth</translation></message>
    <message><source>偏移</source><translation>Offset</translation></message>
    <message><source>电平触发</source><translation>Level Trigger</translation></message>
    <message><source>上升沿触发</source><translation>Rising Edge</translation></message>
    <message><source>下降沿触发</source><translation>Falling Edge</translation></message>
    <message><source>高</source><translation>High</translation></message>
    <message><source>低</source><translation>Low</translation></message>
    <!-- Range names (Unicode escapes: \u00b1=±, \u5355\u7aef=单端, \u5dee\u5206=差分) -->
    <message><source>±2.5V(单端)</source><translation>±2.5V (Single-ended)</translation></message>
    <message><source>±5V(单端)</source><translation>±5V (Single-ended)</translation></message>
    <message><source>±6.25V(单端)</source><translation>±6.25V (Single-ended)</translation></message>
    <message><source>±10V(单端)</source><translation>±10V (Single-ended)</translation></message>
    <message><source>±12.5V(单端)</source><translation>±12.5V (Single-ended)</translation></message>
    <message><source>0~5V(单端)</source><translation>0~5V (Single-ended)</translation></message>
    <message><source>0~10V(单端)</source><translation>0~10V (Single-ended)</translation></message>
    <message><source>0~12.5V(单端)</source><translation>0~12.5V (Single-ended)</translation></message>
    <message><source>±5V(差分)</source><translation>±5V (Differential)</translation></message>
    <message><source>±10V(差分)</source><translation>±10V (Differential)</translation></message>
    <message><source>±12.5V(差分)</source><translation>±12.5V (Differential)</translation></message>
    <message><source>±20V(差分)</source><translation>±20V (Differential)</translation></message>
    <message><source>±25V(差分)</source><translation>±25V (Differential)</translation></message>
</context>

<!-- ==================== ConnectDialog ==================== -->
<context>
    <name>ConnectDialog</name>
    <message><source>Connect Device</source><translation>Connect Device</translation></message>
    <message><source>Device IP</source><translation>Device IP</translation></message>
    <message><source>Auto Scan</source><translation>Auto Scan</translation></message>
    <message><source>Scan Devices</source><translation>Scan Devices</translation></message>
    <message><source>Connect</source><translation>Connect</translation></message>
    <message><source>Cancel</source><translation>Cancel</translation></message>
    <message><source>No device found</source><translation>No device found</translation></message>
    <message><source>No new device found (all connected)</source><translation>No new device found (all connected)</translation></message>
    <message><source>Error</source><translation>Error</translation></message>
    <message><source>Please enter device IP address!</source><translation>Please enter device IP address!</translation></message>
    <message><source>提示</source><translation>Notice</translation></message>
    <message><source>设备 %1 已连接，请勿重复添加</source><translation>Device %1 is already connected. Do not add duplicates.</translation></message>
</context>

<!-- ==================== DeviceInfoDialog ==================== -->
<context>
    <name>DeviceInfoDialog</name>
    <message><source>设备详情</source><translation>Device Info</translation></message>
    <message><source>加载签名</source><translation>Load Signature</translation></message>
    <message><source>功能状态:</source><translation>Status:</translation></message>
    <message><source>协议版本:</source><translation>Protocol Version:</translation></message>
    <message><source>模块开发时间:</source><translation>Module Build Date:</translation></message>
    <message><source>IP 地址:</source><translation>IP Address:</translation></message>
    <message><source>MAC 地址:</source><translation>MAC Address:</translation></message>
    <message><source>UDP 端口:</source><translation>UDP Port:</translation></message>
    <message><source>网口速率:</source><translation>Link Speed:</translation></message>
    <message><source>主控端数量:</source><translation>Master Count:</translation></message>
    <message><source>加密状态:</source><translation>Crypto Status:</translation></message>
    <message><source>设备 ID:</source><translation>Device ID:</translation></message>
    <message><source>--- 采集信息 ---</source><translation>--- Acquisition Info ---</translation></message>
    <message><source>设备型号:</source><translation>Device Model:</translation></message>
    <message><source>采集版本:</source><translation>Collect Version:</translation></message>
    <message><source>最高采样率:</source><translation>Max Sample Rate:</translation></message>
    <message><source>缓存大小:</source><translation>Buffer Size:</translation></message>
    <message><source>通道数:</source><translation>Channels:</translation></message>
    <message><source>AD 精度:</source><translation>AD Resolution:</translation></message>
    <message><source>AD 芯片数:</source><translation>AD Chips:</translation></message>
    <message><source>量程数组大小:</source><translation>Range Array Size:</translation></message>
    <message><source>写</source><translation>Write</translation></message>
    <message><source>读</source><translation>Read</translation></message>
    <message><source>未使能</source><translation>Disabled</translation></message>
    <message><source>AXI Full 总线: %1, %2位地址, %3位数据</source><translation>AXI Full Bus: %1, %2-bit addr, %3-bit data</translation></message>
    <message><source>%1年%2月</source><translation>%1/%2</translation></message>
    <message><source>未知</source><translation>Unknown</translation></message>
    <message><source>%1 位</source><translation>%1 bit</translation></message>
    <message><source>未验证</source><translation>Unverified</translation></message>
    <message><source>已验证</source><translation>Verified</translation></message>
</context>

<!-- ==================== DevicePanel ==================== -->
<context>
    <name>DevicePanel</name>
    <message><source>瞬时采集</source><translation>Instant</translation></message>
    <message><source>单次触发</source><translation>Single Trigger</translation></message>
    <message><source>连续触发</source><translation>Multi Trigger</translation></message>
    <message><source>连续采集</source><translation>Continuous</translation></message>
    <message><source>参数设置</source><translation>Parameters</translation></message>
    <message><source>保存数据</source><translation>Save Data</translation></message>
    <message><source>Ready</source><translation>Ready</translation></message>
    <message><source>Time (s)</source><translation>Time (s)</translation></message>
    <message><source>Voltage (V)</source><translation>Voltage (V)</translation></message>
    <message><source>指示器显示通道</source><translation>Indicator channel</translation></message>
    <message><source>波形显示</source><translation>Waveform Display</translation></message>
    <message><source>显示使能</source><translation>Display Enable</translation></message>
    <message><source>CH%1 波形偏移 (V)</source><translation>CH%1 waveform offset (V)</translation></message>
    <message><source>状态：空闲（%1CH %2bit %3Msps）</source><translation>Status: Idle (%1CH %2bit %3Msps)</translation></message>
    <message><source>状态：%1 中...</source><translation>Status: %1 in progress...</translation></message>
    <message><source>状态：空闲（%1 采样点）</source><translation>Status: Idle (%1 samples)</translation></message>
    <message><source>状态：正在数据传输</source><translation>Status: Transferring data</translation></message>
    <message><source>状态：空闲（DMA 超时）</source><translation>Status: Idle (DMA timeout)</translation></message>
    <message><source>状态：空闲（读取失败）</source><translation>Status: Idle (Read failed)</translation></message>
    <message><source>状态：采集继续（已放弃保存）</source><translation>Status: Collecting (save aborted)</translation></message>
    <message><source>提示</source><translation>Notice</translation></message>
    <message><source>请先打开参数设置对话框配置主机缓存大小。</source><translation>Please open Parameters dialog to configure host buffer size.</translation></message>
    <message><source>确认开始连续采集？</source><translation>Start continuous acquisition?</translation></message>
    <message><source>同时保存数据到文件</source><translation>Also save data to file</translation></message>
    <message><source>带宽超限</source><translation>Bandwidth Exceeded</translation></message>
    <message><source>数据速率 %1 MB/s 超过网口有效带宽 %2 MB/s（40%利用率）。
请降低采样率或通道数。</source><translation>Data rate %1 MB/s exceeds effective network bandwidth %2 MB/s (40% utilization).
Please reduce sample rate or channel count.</translation></message>
    <message><source>错误</source><translation>Error</translation></message>
    <message><source>Collect start failed!</source><translation>Collect start failed!</translation></message>
    <message><source>保存速度不足</source><translation>Save Speed Insufficient</translation></message>
    <message><source>磁盘写入速度跟不上采集速度，数据正在丢失。
已保存的数据仍然有效。
建议降低采样率或通道数后重试。</source><translation>Disk write speed cannot keep up with acquisition. Data is being lost.
Already saved data is still valid.
Try reducing sample rate or channel count.</translation></message>
    <message><source>保存采集数据</source><translation>Save Acquisition Data</translation></message>
    <message><source>CSV 文件 (*.csv)</source><translation>CSV Files (*.csv)</translation></message>
    <message><source>无法创建文件: %1</source><translation>Cannot create file: %1</translation></message>
    <message><source>数据已保存: %1 (%2 采样点)</source><translation>Data saved: %1 (%2 samples)</translation></message>
    <message><source>自动保存失败: 无法创建文件 %1</source><translation>Auto-save failed: Cannot create file %1</translation></message>
    <message><source>数据已自动保存: %1 (%2 采样点)</source><translation>Data auto-saved: %1 (%2 samples)</translation></message>
    <message><source>采集进行中，无法设置参数。请先停止采集。</source><translation>Acquisition in progress. Please stop before changing parameters.</translation></message>
</context>

<!-- ==================== FileViewerPanel ==================== -->
<context>
    <name>FileViewerPanel</name>
    <message><source>文件: </source><translation>File: </translation></message>
    <message><source>采样率: --</source><translation>Sample Rate: --</translation></message>
    <message><source>Time (s)</source><translation>Time (s)</translation></message>
    <message><source>Voltage (V)</source><translation>Voltage (V)</translation></message>
    <message><source>指示器显示通道</source><translation>Indicator channel</translation></message>
    <message><source>波形显示</source><translation>Waveform Display</translation></message>
    <message><source>显示使能</source><translation>Display Enable</translation></message>
    <message><source>错误: 无法打开文件</source><translation>Error: Cannot open file</translation></message>
    <message><source>错误: 文件中没有有效数据</source><translation>Error: No valid data in file</translation></message>
    <message><source>采样率: %1 Hz</source><translation>Sample Rate: %1 Hz</translation></message>
    <message><source>CH%1 波形偏移 (V)</source><translation>CH%1 waveform offset (V)</translation></message>
</context>

<!-- ==================== MainWindow ==================== -->
<context>
    <name>MainWindow</name>
    <message><source>添加设备</source><translation>Add Device</translation></message>
    <message><source>断开连接</source><translation>Disconnect</translation></message>
    <message><source>打开数据文件</source><translation>Open Data File</translation></message>
    <message><source>软件信息</source><translation>About</translation></message>
    <message><source>DLL接口帮助</source><translation>DLL API Help</translation></message>
    <message><source>隐藏/显示设备列表</source><translation>Hide/Show device list</translation></message>
    <message><source>\n(未验证)</source><translation>\n(Unverified)</translation></message>
    <message><source>型号: %1
IP: %2
MAC: %3
主/从: %4
签名: %5</source><translation>Model: %1
IP: %2
MAC: %3
Master/Slave: %4
Signature: %5</translation></message>
    <message><source>主设备</source><translation>Master</translation></message>
    <message><source>从设备</source><translation>Slave</translation></message>
    <message><source>未验证</source><translation>Unverified</translation></message>
    <message><source>已验证</source><translation>Verified</translation></message>
    <message><source>文件: %1
采样率: %2 Hz</source><translation>File: %1
Sample Rate: %2 Hz</translation></message>
    <message><source>瞬时采集</source><translation>Instant</translation></message>
    <message><source>单次触发</source><translation>Single Trigger</translation></message>
    <message><source>连续触发</source><translation>Multi Trigger</translation></message>
    <message><source>连续采集</source><translation>Continuous</translation></message>
    <message><source>停止</source><translation>Stop</translation></message>
    <message><source>保存数据</source><translation>Save Data</translation></message>
    <message><source>参数设置</source><translation>Parameters</translation></message>
    <message><source>网络设置</source><translation>Network Settings</translation></message>
    <message><source>设备详情</source><translation>Device Info</translation></message>
    <message><source>关闭文件</source><translation>Close File</translation></message>
    <message><source>CSV 文件 (*.csv)</source><translation>CSV Files (*.csv)</translation></message>
    <message><source>错误</source><translation>Error</translation></message>
    <message><source>无法加载文件: %1</source><translation>Cannot load file: %1</translation></message>
    <message><source>Open bridge failed!</source><translation>Open bridge failed!</translation></message>
    <message><source>No device found!</source><translation>No device found!</translation></message>
    <message><source>Device init failed!</source><translation>Device init failed!</translation></message>
    <message><source>Get collect info failed!</source><translation>Get collect info failed!</translation></message>
    <message><source>设备签名验证失败</source><translation>Signature Verification Failed</translation></message>
    <message><source>设备未通过签名验证，剩余可用时间 3 小时。
3 小时后设备将不执行任何总线读写命令（网络通信仍正常）。

请联系杭州幻智星科技有限公司申请合法签名。
邮箱：yupc123@qq.com
电话：13867435883</source><translation>Device failed signature verification. 3 hours of usage remain.
After 3 hours, the device will refuse all bus read/write commands (network communication still works).

Please contact MagicStar Technology for a valid signature.
Email: yupc123@qq.com
Phone: +86-13867435883</translation></message>
    <message><source>格式错误</source><translation>Format Error</translation></message>
    <message><source>MAC 地址格式不正确，应为 XX:XX:XX:XX:XX:XX
例如: 00:1A:2B:3C:4D:5E</source><translation>Invalid MAC address format. Expected: XX:XX:XX:XX:XX:XX
Example: 00:1A:2B:3C:4D:5E</translation></message>
    <message><source>IP 地址格式不正确，应为 xxx.xxx.xxx.xxx
例如: 192.168.0.100</source><translation>Invalid IP address format. Expected: xxx.xxx.xxx.xxx
Example: 192.168.0.100</translation></message>
    <message><source>确认要修改以下网络参数？
</source><translation>Modify the following network parameters?
</translation></message>
    <message><source>MAC 地址: %1 → %2
</source><translation>MAC Address: %1 → %2
</translation></message>
    <message><source>IP 地址: %1 → %2
</source><translation>IP Address: %1 → %2
</translation></message>
    <message><source>修改后需断电重启设备才能生效。</source><translation>Power cycle required for changes to take effect.</translation></message>
    <message><source>确认修改</source><translation>Confirm Changes</translation></message>
    <message><source>修改 MAC 地址失败（返回 %1）。</source><translation>Failed to set MAC address (returned %1).</translation></message>
    <message><source>修改 IP 地址失败（返回 %1）。</source><translation>Failed to set IP address (returned %1).</translation></message>
    <message><source>提示</source><translation>Notice</translation></message>
    <message><source>MAC地址、IP地址修改成功后，要断电重启设备，新地址才能生效。</source><translation>Power cycle the device for the new MAC/IP addresses to take effect.</translation></message>
    <message><source>是否立即断开当前设备连接？
断电重启后可手动重新添加设备。</source><translation>Disconnect the current device now?
You can re-add the device after power cycling.</translation></message>
    <message><source>&lt;h2&gt;Mstar Oscilloscope&lt;/h2&gt;&lt;p&gt;版本: V%1&lt;/p&gt;&lt;p&gt;多设备示波器采集软件&lt;/p&gt;&lt;p&gt;支持多设备并行采集，主从设备树形管理&lt;/p&gt;&lt;p&gt;Copyright (C) 2026 Mstar Semiconductor, Inc.&lt;/p&gt;&lt;p&gt;Email：yupc123@qq.com&lt;/p&gt;</source><translation>&lt;h2&gt;Mstar Oscilloscope&lt;/h2&gt;&lt;p&gt;Version: V%1&lt;/p&gt;&lt;p&gt;Multi-device oscilloscope acquisition software&lt;/p&gt;&lt;p&gt;Parallel acquisition with master/slave tree management&lt;/p&gt;&lt;p&gt;Copyright (C) 2026 Mstar Semiconductor, Inc.&lt;/p&gt;&lt;p&gt;Email: yupc123@qq.com&lt;/p&gt;</translation></message>
    <message><source>未找到 DLL 接口帮助文件。
请确认 MStarEthernetBridge_API_Help.md 存在于程序目录中。</source><translation>DLL API help file not found.
Please ensure MStarEthernetBridge_API_Help.md exists in the program directory.</translation></message>
    <message><source>无法读取帮助文件。</source><translation>Cannot read help file.</translation></message>
</context>

<!-- ==================== NetworkSettingsDialog ==================== -->
<context>
    <name>NetworkSettingsDialog</name>
    <message><source>网络设置</source><translation>Network Settings</translation></message>
    <message><source>MAC 地址:</source><translation>MAC Address:</translation></message>
    <message><source>IP 地址:</source><translation>IP Address:</translation></message>
    <message><source>注意：MAC 地址在网络中是唯一的标识，修改后若与其他设备重复将导致通信异常，请谨慎操作。</source><translation>Warning: MAC address must be unique on the network. Duplicates will cause communication failures. Proceed with caution.</translation></message>
</context>

<!-- ==================== SignatureDialog ==================== -->
<context>
    <name>SignatureDialog</name>
    <message><source>签名验证</source><translation>Signature Verification</translation></message>
    <message><source>请输入签名信息</source><translation>Please enter signature</translation></message>
    <message><source>设备 ID:</source><translation>Device ID:</translation></message>
    <message><source>当前签名:</source><translation>Current Signature:</translation></message>
    <message><source>新签名:</source><translation>New Signature:</translation></message>
    <message><source>加载签名</source><translation>Load Signature</translation></message>
    <message><source>从文件加载</source><translation>Load from File</translation></message>
    <message><source>验证签名</source><translation>Verify Signature</translation></message>
    <message><source>保存到设备</source><translation>Save to Device</translation></message>
    <message><source>(无)</source><translation>(None)</translation></message>
    <message><source>提示</source><translation>Notice</translation></message>
    <message><source>签名加载成功</source><translation>Signature loaded successfully</translation></message>
    <message><source>签名加载失败</source><translation>Signature load failed</translation></message>
    <message><source>签名验证通过</source><translation>Signature verified</translation></message>
    <message><source>验证结果</source><translation>Verification Result</translation></message>
    <message><source>签名验证成功，设备功能正常。</source><translation>Signature verification passed. Device is fully functional.</translation></message>
    <message><source>签名验证失败</source><translation>Signature verification failed</translation></message>
    <message><source>签名验证失败，设备功能可能受到限制。
请确认签名信息是否正确。</source><translation>Signature verification failed. Device functionality may be limited.
Please check the signature.</translation></message>
    <message><source>请先加载签名</source><translation>Please load signature first</translation></message>
    <message><source>签名已保存到设备</source><translation>Signature saved to device</translation></message>
    <message><source>签名已成功保存到设备，重启后仍然有效。</source><translation>Signature saved to device. It persists after reboot.</translation></message>
    <message><source>保存失败</source><translation>Save failed</translation></message>
    <message><source>签名保存失败（返回 %1）</source><translation>Signature save failed (returned %1)</translation></message>
    <message><source>打开签名文件</source><translation>Open Signature File</translation></message>
    <message><source>签名文件 (*.mstar);;所有文件 (*)</source><translation>Signature Files (*.mstar);;All Files (*)</translation></message>
    <message><source>无法打开文件</source><translation>Cannot open file</translation></message>
    <message><source>未找到</source><translation>Not Found</translation></message>
    <message><source>文件中没有找到与当前设备ID匹配的签名。
当前设备ID: %1</source><translation>No matching signature found for this device ID.
Current device ID: %1</translation></message>
    <message><source>已从文件加载签名，请点击"加载签名"按钮</source><translation>Signature loaded from file. Please click "Load Signature" button.</translation></message>
    <message><source>错误</source><translation>Error</translation></message>
</context>

</TS>
