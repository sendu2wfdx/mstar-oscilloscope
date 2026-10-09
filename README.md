# MstarOscilloscope — 多通道波形显示软件

MstarOscilloscope 是基于 MStarEthernetBridge DLL 的 Qt6 桌面应用，用于连接幻智星科技 FPGA 数据采集设备，实现多通道波形的实时采集与显示。

## 主要功能

- **设备连接**：支持自动扫描网络设备或手动输入 IP 连接，可多设备同时工作
- **采集模式**：
  - 即时采集（Instant）— 单次抓取当前数据
  - 单次触发（Single Trigger）— 满足触发条件后采集一帧
  - 多次触发（Multi Trigger）— 连续触发采集多帧
  - 连续采集（Continuous）— 不间断采集，环形缓存滚动显示
- **触发配置**：支持电平/上升沿/下降沿触发，可设置触发电平和预触发长度
- **波形显示**：QCustomPlot 多通道波形绘制，支持缩放、平移、通道偏移调节
- **窗口指示器**：连续模式下显示当前可视窗口在整体数据中的位置
- **数据保存**：支持手动导出 CSV 文件，连续模式支持后台自动分片保存
- **设备配置持久化**：每台设备的采集参数通过 JSON 文件自动保存/加载
- **多板同步**：支持主从板 CLK 和触发信号互联配置
- **签名验证**：设备授权签名校验
- **文件查看**：内置 CSV 文件波形回放面板

## 技术栈

| 组件 | 说明 |
|------|------|
| Qt 6.7+ | GUI 框架（Widgets + PrintSupport） |
| C++17 | 编程语言 |
| QCustomPlot | 波形绘图控件（内置集成） |
| CMake | 构建系统 |
| MStarEthernetBridge DLL | 底层设备通信（以太网 AXI 总线） |

## 依赖下载

构建前需先下载 MStarEthernetBridge DLL（含头文件和链接库）：

- 百度网盘：https://pan.baidu.com/s/1LkRXYsZrZuWL4JUcbq3mnw?pwd=1234 提取码: 1234

下载后按以下结构放置（或自定义路径）：

```
MstarOscilloscope/
├── dll/
│   ├── include/          # 头文件：MstarEthBridge.h, MstarCollect.h
│   └── lib/              # 链接库：MStarEthernetBridge.lib, MStarEthernetBridge.dll
```

> **注意**：MStarEthernetBridge DLL 是闭源商业组件，不包含在本开源仓库中。本项目通过动态链接调用 DLL，两者物理分离。

## 构建方法

```bash
# 确保已安装 Qt6 和 MSVC 2022
mkdir build && cd build

# 默认查找 dll/include 和 dll/lib 目录
# 如 DLL 在其他位置，通过参数指定：
cmake .. -G "Visual Studio 17 2022" -A x64 -DDLL_INCLUDE_DIR="D:/path/to/dll/include" -DDLL_PROJECT_OUT="D:/path/to/dll/lib"

cmake --build . --config Release
```

Release 构建后自动运行 `windeployqt` 收集 Qt 依赖 DLL。

## 目录结构

| 文件 | 说明 |
|------|------|
| `MainWindow` | 主窗口，管理设备列表与面板切换 |
| `DevicePanel` | 单设备面板：采集控制 + 波形图 + 状态显示 |
| `ConnectDialog` | 设备连接对话框（IP 输入 / 网络扫描） |
| `CollectParamDialog` | 采集参数配置对话框 |
| `NetworkSettingsDialog` | 网络参数设置对话框 |
| `DeviceInfoDialog` | 设备详细信息查看 |
| `SignatureDialog` | 设备签名验证对话框 |
| `DeviceProfileManager` | 设备参数 JSON 持久化管理 |
| `DataFileWriter` | 后台 CSV 文件写入器（双缓冲流水线） |
| `WaveformWidget` | 波形绘制控件 |
| `WindowIndicator` | 连续模式窗口位置指示器 |
| `FileViewerPanel` | CSV 文件波形回放面板 |
| `qcustomplot` | QCustomPlot 绘图库（第三方集成） |

## 许可证

GPL-3.0-or-later

## 联系方式

- 公司：杭州幻智星科技有限公司
- Email：yupc123@qq.com
- 电话：13867435883
- 淘宝店：https://magicstar.taobao.com/
