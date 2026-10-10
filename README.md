# MstarOscilloscope — Multi-Channel Waveform Display Software

MstarOscilloscope is a Qt6 desktop application built on the MStarEthernetBridge DLL, designed to connect MagicStar FPGA data acquisition devices for real-time multi-channel waveform acquisition and display.

## Features

- **Device Connection**: Auto-scan network devices or manual IP input; supports multiple devices simultaneously
- **Acquisition Modes**:
  - Instant — Single-shot capture of current data
  - Single Trigger — Capture one frame upon trigger condition
  - Multi Trigger — Sequential multi-frame triggered acquisition
  - Continuous — Non-stop acquisition with ring-buffer rolling display
- **Trigger Configuration**: Level / rising-edge / falling-edge trigger with adjustable trigger level and pre-trigger length
- **Waveform Display**: QCustomPlot multi-channel waveform rendering with zoom, pan, and channel offset adjustment
- **Window Indicator**: Shows the position of the current view within the full data range in continuous mode
- **Data Export**: Manual CSV file export; continuous mode supports background auto-segmented saving
- **Device Profile Persistence**: Acquisition parameters per device auto-saved/loaded via JSON
- **Multi-Board Sync**: Master-slave CLK and trigger signal interconnection configuration
- **Signature Verification**: Device authorization signature validation
- **File Viewer**: Built-in CSV waveform playback panel

## Tech Stack

| Component | Description |
|-----------|-------------|
| Qt 6.7+ | GUI framework (Widgets + PrintSupport) |
| C++17 | Programming language |
| QCustomPlot | Waveform plotting library (bundled) |
| CMake | Build system |
| MStarEthernetBridge DLL | Low-level device communication (Ethernet AXI bus) |

## Dependencies

Before building, download the MStarEthernetBridge DLL (headers and libraries included):

- Baidu Netdisk: https://pan.baidu.com/s/1LkRXYsZrZuWL4JUcbq3mnw?pwd=1234 (Code: 1234)

Place the files in the following structure (or specify a custom path):

```
MstarOscilloscope/
├── dll/
│   ├── include/          # Headers: MstarEthBridge.h, MstarCollect.h
│   └── lib/              # Libraries: MStarEthernetBridge.lib, MStarEthernetBridge.dll
```

> **Note**: MStarEthernetBridge DLL is a proprietary commercial component and is not included in this open-source repository. This project calls the DLL via dynamic linking — the two are physically separated.

## Building

```bash
# Prerequisites: Qt6 and MSVC 2022 installed
mkdir build && cd build

# By default, searches dll/include and dll/lib directories
# To specify a different DLL location:
cmake .. -G "Visual Studio 17 2022" -A x64 -DDLL_INCLUDE_DIR="D:/path/to/dll/include" -DDLL_PROJECT_OUT="D:/path/to/dll/lib"

cmake --build . --config Release
```

After a Release build, `windeployqt` runs automatically to collect Qt dependency DLLs.

## Source Structure

| File | Description |
|------|-------------|
| `MainWindow` | Main window — device list management and panel switching |
| `DevicePanel` | Per-device panel: acquisition control + waveform plot + status display |
| `ConnectDialog` | Device connection dialog (IP input / network scan) |
| `CollectParamDialog` | Acquisition parameter configuration dialog |
| `NetworkSettingsDialog` | Network settings dialog |
| `DeviceInfoDialog` | Device detail viewer |
| `SignatureDialog` | Device signature verification dialog |
| `DeviceProfileManager` | Device parameter JSON persistence manager |
| `DataFileWriter` | Background CSV file writer (double-buffered pipeline) |
| `WaveformWidget` | Waveform rendering widget |
| `WindowIndicator` | Continuous-mode window position indicator |
| `FileViewerPanel` | CSV waveform playback panel |
| `qcustomplot` | QCustomPlot plotting library (third-party integration) |

## License

GPL-3.0-or-later

## Contact

- Company: Hangzhou Magicstar Technology Co., Ltd.
- Email: yupc123@qq.com
- Phone: +86-13867435883
- Taobao: https://magicstar.taobao.com/
