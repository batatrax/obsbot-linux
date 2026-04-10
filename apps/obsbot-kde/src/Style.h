#pragma once
#include <QString>

namespace ObsbotStyle {

// Palette de couleurs
constexpr auto COLOR_BG         = "#1e1e2e";
constexpr auto COLOR_SURFACE    = "#24243e";
constexpr auto COLOR_CARD       = "#313244";
constexpr auto COLOR_ACCENT     = "#89b4fa";
constexpr auto COLOR_ACCENT2    = "#cba6f7";
constexpr auto COLOR_SUCCESS    = "#a6e3a1";
constexpr auto COLOR_WARNING    = "#f9e2af";
constexpr auto COLOR_ERROR      = "#f38ba8";
constexpr auto COLOR_TEXT       = "#cdd6f4";
constexpr auto COLOR_SUBTEXT    = "#a6adc8";

inline QString globalStyleSheet() {
    return R"(
QMainWindow, QDialog, QWidget {
    background-color: #1e1e2e;
    color: #cdd6f4;
    font-family: "Inter", "Noto Sans", "Segoe UI", sans-serif;
    font-size: 13px;
}
QTabWidget::pane {
    border: 1px solid #45475a;
    background: #24243e;
    border-radius: 8px;
}
QTabBar::tab {
    background: #313244;
    color: #a6adc8;
    padding: 8px 18px;
    border-radius: 6px 6px 0 0;
    margin-right: 2px;
}
QTabBar::tab:selected {
    background: #89b4fa;
    color: #1e1e2e;
    font-weight: bold;
}
QTabBar::tab:hover:!selected {
    background: #45475a;
    color: #cdd6f4;
}
QPushButton {
    background: #313244;
    color: #cdd6f4;
    border: 1px solid #45475a;
    border-radius: 6px;
    padding: 6px 16px;
    font-weight: 500;
}
QPushButton:hover { background: #45475a; border-color: #89b4fa; }
QPushButton:pressed { background: #585b70; }
QPushButton:disabled { background: #24243e; color: #585b70; border-color: #313244; }
QPushButton[accent="true"] {
    background: #89b4fa;
    color: #1e1e2e;
    border: none;
    font-weight: bold;
}
QPushButton[accent="true"]:hover { background: #b4d0fa; }
QPushButton[danger="true"] {
    background: #f38ba8;
    color: #1e1e2e;
    border: none;
    font-weight: bold;
}
QPushButton[success="true"] {
    background: #a6e3a1;
    color: #1e1e2e;
    border: none;
    font-weight: bold;
}
QSlider::groove:horizontal {
    height: 4px;
    background: #45475a;
    border-radius: 2px;
}
QSlider::handle:horizontal {
    background: #89b4fa;
    width: 16px; height: 16px;
    margin: -6px 0;
    border-radius: 8px;
}
QSlider::sub-page:horizontal { background: #89b4fa; border-radius: 2px; }
QComboBox {
    background: #313244;
    color: #cdd6f4;
    border: 1px solid #45475a;
    border-radius: 6px;
    padding: 5px 10px;
}
QComboBox::drop-down { border: none; }
QComboBox QAbstractItemView {
    background: #313244;
    color: #cdd6f4;
    selection-background-color: #89b4fa;
    selection-color: #1e1e2e;
}
QGroupBox {
    border: 1px solid #45475a;
    border-radius: 8px;
    margin-top: 12px;
    padding-top: 8px;
    font-weight: bold;
    color: #89b4fa;
}
QGroupBox::title {
    subcontrol-origin: margin;
    left: 12px;
    padding: 0 6px;
}
QLabel { color: #cdd6f4; }
QLabel[dim="true"] { color: #a6adc8; font-size: 11px; }
QCheckBox { color: #cdd6f4; spacing: 8px; }
QCheckBox::indicator {
    width: 16px; height: 16px;
    border-radius: 4px;
    border: 1px solid #45475a;
    background: #313244;
}
QCheckBox::indicator:checked {
    background: #89b4fa;
    border-color: #89b4fa;
    image: url(none);
}
QProgressBar {
    background: #313244;
    border-radius: 4px;
    height: 8px;
    text-align: center;
    color: transparent;
}
QProgressBar::chunk {
    background: #89b4fa;
    border-radius: 4px;
}
QScrollBar:vertical {
    background: #24243e;
    width: 8px;
    border-radius: 4px;
}
QScrollBar::handle:vertical {
    background: #45475a;
    border-radius: 4px;
    min-height: 20px;
}
QScrollBar::handle:vertical:hover { background: #89b4fa; }
QStatusBar { background: #181825; color: #a6adc8; }
QToolTip {
    background: #313244;
    color: #cdd6f4;
    border: 1px solid #45475a;
    border-radius: 4px;
    padding: 4px 8px;
}
    )";
}

} // namespace ObsbotStyle
