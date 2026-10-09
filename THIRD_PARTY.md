# Third-Party Dependencies

This document lists all third-party components used by MstarOscilloscope.

## Qt 6

- **License**: LGPL v3 / GPL v3 (dual-licensed)
- **URL**: https://www.qt.io/
- **Usage**: UI framework, widgets, core utilities
- **Notes**: Dynamically linked. Qt source code is available from qt.io.

## QCustomPlot

- **License**: GPL v3
- **URL**: https://www.qcustomplot.com/
- **Usage**: Waveform plotting and chart rendering
- **Notes**: Integrated as source files (qcustomplot.h / qcustomplot.cpp).
  Since QCustomPlot is GPL-licensed, MstarOscilloscope is distributed under GPL v3.

## Summary

| Component | License | Linkage |
|-----------|---------|---------|
| Qt 6 | LGPL v3 / GPL v3 | Dynamic (shared libraries) |
| QCustomPlot | GPL v3 | Static (source files) |

## Proprietary Components (Not Included in Open Source Release)

The following components are proprietary and are **not** part of this open-source release:

- **MStarEthernetBridge.dll** — Network bridge and AXI read/write operations
- **MstarCollect** — Device acquisition control module

These DLLs are dynamically linked at runtime and are distributed separately.
