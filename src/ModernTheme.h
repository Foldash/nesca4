#ifndef NESCA4_MODERN_THEME_H
#define NESCA4_MODERN_THEME_H

#include <QApplication>
#include <QFont>
#include <QMainWindow>
#include <QStyleFactory>
#include <QTabBar>
#include <QWidget>

namespace ModernTheme {
inline void apply(QWidget *window) {
    if (window == nullptr) return;
    for (auto *bar : window->findChildren<QTabBar *>()) bar->setDrawBase(false);
    window->setStyleSheet(QStringLiteral(R"QSS(
QWidget { background: #10141c; color: #e6ecf4; font-family: "DejaVu Sans", "Segoe UI", sans-serif; font-size: 13px; }
QLabel { background: transparent; border: none; }
QLabel[role="brand"] { font-size: 30px; font-weight: 700; letter-spacing: -1px; }
QLabel[role="version"] { color: #4dd6b2; font-size: 24px; font-weight: 600; padding-right: 16px; }
QLabel[role="heading"] { font-size: 23px; font-weight: 600; }
QLabel[role="subheading"] { font-size: 14px; font-weight: 600; }
QLabel[role="muted"] { color: #8f9bae; font-size: 12px; }
QLabel[role="eyebrow"] { color: #8f9bae; font-size: 10px; font-weight: 600; letter-spacing: 1px; }
QLabel[role="metric"] { font-size: 30px; font-weight: 600; }
QLabel[role="badge"] { background: #25332c; color: #a8c6b4; border-radius: 12px; padding: 6px 12px; font-size: 11px; }
QFrame[role="card"] { background: #181e29; border: 1px solid #283141; border-radius: 14px; }
QWidget#tab_4, QWidget#tab_5, QWidget#tab_6, QWidget#tab, QWidget#optionsContent { background: transparent; }
QLineEdit { background: #111722; border: 1px solid #354052; border-radius: 7px; padding: 9px 10px; selection-background-color: #285f54; min-height: 18px; }
QLineEdit:focus { border-color: #4dd6b2; }
QLineEdit:disabled { color: #647185; border-color: #252e3c; }
QLineEdit:read-only { color: #8f9bae; background: #141b26; }
QPushButton { background: #222c3b; border: 1px solid #354052; border-radius: 7px; padding: 9px 13px; min-height: 18px; }
QPushButton:hover { background: #2c394c; border-color: #53627a; }
QPushButton:pressed { background: #17202d; }
QPushButton:focus { border-color: #4dd6b2; }
QPushButton:disabled { color: #627085; background: #1a2230; border-color: #283141; }
QPushButton[role="primary"] { background: #4dd6b2; color: #0a2520; border-color: #4dd6b2; font-weight: 600; padding: 12px; }
QPushButton[role="primary"]:hover { background: #70e4c5; }
QPushButton[role="primary"]:disabled { background: #293e3b; color: #718e88; border-color: #293e3b; }
QPushButton[role="link"] { color: #7dddc4; background: transparent; border: none; padding: 2px 0 8px 0; text-align: left; font-size: 11px; }
QPushButton#exitButton, QPushButton#trayButton { background: transparent; border-color: #283141; font-size: 11px; padding: 7px 10px; }
QPushButton#exitButton:hover { background: #442a31; border-color: #88505b; }
QTabWidget::pane { background: #181e29; border: 1px solid #283141; border-radius: 10px; top: -1px; }
QTabWidget#tabMainWidget::pane { border: none; background: transparent; }
QTabWidget#tabMainWidget, QWidget#qt_tabwidget_stackedwidget { background: transparent; }
QTabBar::tab { color: #8f9bae; background: transparent; border: none; padding: 11px 12px; border-bottom: 2px solid transparent; margin-right: 4px; }
QTabBar::tab:selected { color: #dce9e5; border-bottom-color: #4dd6b2; }
QTabBar::tab:hover { color: #f0f5fa; }
QTabBar::tab:disabled { color: #49566b; }
QTextBrowser, QTableView, QTableWidget { background: #141a24; alternate-background-color: #1b2330; border: 1px solid #283141; border-radius: 8px; padding: 8px; selection-background-color: #245a50; gridline-color: #283141; }
QTextBrowser { font-family: "DejaVu Sans Mono", "Consolas", monospace; font-size: 12px; }
QHeaderView::section { background: #1c2533; color: #aebccc; border: none; padding: 10px; font-size: 11px; }
QTableView QTableCornerButton::section { background: #1c2533; border: none; }
QCheckBox { spacing: 8px; padding: 4px 0; font-size: 12px; }
QCheckBox::indicator { width: 16px; height: 16px; background: #111722; border: 1px solid #53627a; border-radius: 4px; }
QCheckBox::indicator:checked { background: #4dd6b2; border: 4px solid #245a50; }
QScrollArea { background: transparent; border: none; }
QScrollBar:vertical { background: transparent; width: 8px; margin: 0; }
QScrollBar::handle:vertical { background: #3b485d; border-radius: 4px; min-height: 28px; }
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: transparent; }
QToolTip { background: #263246; color: #edf4fb; border: 1px solid #53627a; padding: 6px; }
QMenu { background: #1c2533; border: 1px solid #354052; padding: 6px; }
QMenu::item { padding: 8px 20px; }
QMenu::item:selected { background: #285348; }
)QSS"));
}

inline void configureApplication(QApplication &app) {
    app.setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    app.setApplicationName(QStringLiteral("NESCA 4"));
    app.setOrganizationName(QStringLiteral("Foldash"));
    app.setFont(QFont(QStringLiteral("DejaVu Sans"), 10));
}
}
#endif
