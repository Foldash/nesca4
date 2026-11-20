#include "ModernTheme.h"
#include "ui_nesca_3.h"
#include <QCommandLineParser>
#include <QDir>
#include <QFileInfo>
#include <QHeaderView>
#include <QImage>
#include <QStandardItemModel>
#include <QTimer>
#include <iostream>

int main(int argc, char **argv) {
    QApplication app(argc, argv);
    ModernTheme::configureApplication(app);
    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("NESCA 4 — автономный предпросмотр UI без сетевого ядра"));
    parser.addHelpOption();
    parser.addOption({QStringLiteral("screenshot"), QStringLiteral("Сохранить изображение и выйти"), QStringLiteral("file")});
    parser.addOption({QStringLiteral("smoke"), QStringLiteral("Проверить вкладки, размеры и сохранение полей, затем выйти")});
    parser.process(app);

    QMainWindow window;
    Ui::nesca_3Class ui;
    ui.setupUi(&window);
    ModernTheme::apply(&window);
    ui.developmentBadge->setText(QStringLiteral("Предпросмотр UI"));
    ui.currentDirectoryLine->setText(QStringLiteral("results/"));
    for (auto *line : {ui.ipmPortLine, ui.dnsPortLine, ui.importPortLine}) line->setCursorPosition(0);
    for (auto *button : {ui.startScanButton_3, ui.startScanButton_4, ui.importButton, ui.checkKeyBut,
                         ui.edit_filter_button, ui.DataflowModeBut}) {
        button->setEnabled(false);
        button->setToolTip(QStringLiteral("Автономный предпросмотр: сетевое ядро не запускается"));
    }
    QStandardItemModel model(0, 3, &window);
    model.setHorizontalHeaderLabels({QStringLiteral("Адрес"), QStringLiteral("Учётная запись"), QStringLiteral("Состояние")});
    ui.BATableView->setModel(&model);
    ui.BATableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    QObject::connect(ui.exitButton, &QPushButton::clicked, &window, &QWidget::close);
    QObject::connect(ui.trayButton, &QPushButton::clicked, &window, &QWidget::showMinimized);
    QObject::connect(ui.clearLogBut, &QPushButton::clicked, ui.dataText, &QTextBrowser::clear);
    window.show();

    if (parser.isSet(QStringLiteral("screenshot")) || parser.isSet(QStringLiteral("smoke"))) {
        QTimer::singleShot(100, &app, [&] {
            if (parser.isSet(QStringLiteral("smoke"))) {
                const QString original = ui.ipLine->text();
                ui.ipLine->setText(QStringLiteral("192.0.2.1"));
                for (const QSize size : {QSize(1060, 740), QSize(1320, 860), QSize(1600, 1000)}) {
                    window.resize(size);
                    for (int page = 0; page < ui.tabMainWidget->count(); ++page) {
                        ui.tabMainWidget->setCurrentIndex(page);
                        for (int result = 0; result < ui.outputTabs->count(); ++result) {
                            ui.outputTabs->setCurrentIndex(result);
                            app.processEvents();
                            if (window.size() != size) {
                                std::cerr << "Layout forced a larger window than requested\n";
                                app.exit(1);
                                return;
                            }
                            for (auto *button : {ui.startScanButton_3, ui.startScanButton_4, ui.importButton}) {
                                if (button->isVisible() && !ui.configurationCard->rect().contains(
                                    button->rect().translated(button->mapTo(ui.configurationCard, QPoint())))) {
                                    std::cerr << "Task action escaped its configuration panel\n";
                                    app.exit(1);
                                    return;
                                }
                            }
                            if (!window.rect().contains(ui.exitButton->geometry().translated(ui.exitButton->parentWidget()->mapTo(&window, QPoint())))) {
                                std::cerr << "Exit control escaped the window\n";
                                app.exit(1);
                                return;
                            }
                        }
                    }
                }
                if (ui.ipLine->text() != QStringLiteral("192.0.2.1")) {
                    app.exit(1);
                    return;
                }
                ui.ipLine->setText(original);
                ui.tabMainWidget->setCurrentIndex(0);
                ui.outputTabs->setCurrentIndex(0);
                window.resize(1320, 860);
                app.processEvents();
                std::cout << "UI smoke: 3 sizes, 4 input tabs, 3 result tabs, field state OK\n";
            }
            if (parser.isSet(QStringLiteral("screenshot"))) {
                const QString path = parser.value(QStringLiteral("screenshot"));
                QDir().mkpath(QFileInfo(path).absolutePath());
                if (!window.grab().save(path)) { app.exit(1); return; }
            }
            app.quit();
        });
    }
    return app.exec();
}
