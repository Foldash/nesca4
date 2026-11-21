#include "STh.h"
#include <iostream>
#include <QTimer>
#include <memory>
#include "ModernTheme.h"
#include "nesca_3.h"
#include <QApplication>
#include <qfontdatabase.h>
#include <QMessageBox>
#include <qpixmap.h>
#include "Utils.h"
#include "externData.h"

int main(int argc, char *argv[])
{
	bool isWM = false;
	//DWORD Type;
	//char value[512] = { 0 };
	//char resultString[512] = { 0 };
	//HKEY hkey;
	//if (RegOpenKey(HKEY_LOCAL_MACHINE,
	//	TEXT("HARDWARE\\DESCRIPTION\\System"), &hkey) == ERROR_SUCCESS)
	//{
	//	DWORD value_length = 512;
	//	RegQueryValueEx(hkey, L"VideoBiosVersion", 0, &Type, (BYTE*)&value, &value_length);
	//	RegCloseKey(hkey);

	//	for (int i = 0, j = 0; i < 256; ++i, j += 2) {
	//		char ch = value[j];
	//		if (ch != '\0') {
	//			resultString[i] = ch;
	//		}
	//		else {
	//			resultString[i] = '\n';
	//		}
	//	}
	//	resultString[256] = '\0';
	//}
	//if (strstr(resultString, "VirtualBox") ||
	//	strstr(resultString, "virtualbox")) {
	//	isWM = true;
	//};

	QApplication a(argc, argv);
    ModernTheme::configureApplication(a);
    std::unique_ptr<nesca_3> gui;
    int checkResult = 0;

	if (isWM) {
		//QMessageBox msgBox(
		//	QMessageBox::Information,
		//	"Nope",
		//	"");
		//QPalette palette;
		//palette.setBrush(QPalette::Background, Qt::cyan);
		//msgBox.setIconPixmap(QPixmap(":/nesca_3/xc.jpg"));
		//msgBox.setPalette(palette);
		//msgBox.setVisible(true);
		//msgBox.exec();
	} else {
		QStringList list;
		list << "small_font.ttf";
		int fontID(-1);
		bool fontWarningShown(false);
		for (QStringList::const_iterator constIterator = list.constBegin(); constIterator != list.constEnd(); ++constIterator) {
			QFile res(":/nesca_3/" + *constIterator);
			if (res.open(QIODevice::ReadOnly) == false) {
				if (fontWarningShown == false) {
					fontWarningShown = true;
				}
			}
			else {
				fontID = QFontDatabase::addApplicationFontFromData(res.readAll());
				if (fontID == -1 && fontWarningShown == false) {
					fontWarningShown = true;
				}
			}
		}

		gui = std::make_unique<nesca_3>(isWM, nullptr);
		//if (isWM) {
		//	Utils::emitScaryError();
		//}
		gui->showNormal();
        if (a.arguments().contains(QStringLiteral("--ui-check")))
            QTimer::singleShot(100, gui.get(), &QWidget::close);
        if (a.arguments().contains(QStringLiteral("--session-check"))) {
            auto attempts = std::make_shared<int>(0);
            auto begin = [window = gui.get()] {
                ui->ipLine->setText(QStringLiteral("invalid"));
                QMetaObject::invokeMethod(window, "IPScanSeq", Qt::QueuedConnection);
            };
            QObject::connect(stt, &QThread::finished, gui.get(), [&, attempts, begin] {
                QTimer::singleShot(0, gui.get(), [&, attempts, begin] {
                    if (!ui->ipLine->isEnabled() || ui->startScanButton_3->text() != QStringLiteral("Начать")) {
                        checkResult = 1;
                        gui->close();
                        return;
                    }
                    if (++*attempts < 2) begin();
                    else {
                        std::cout << "Legacy session: invalid input recovery, restart and close OK\n";
                        gui->close();
                    }
                });
            });
            QTimer::singleShot(0, gui.get(), begin);
            QTimer::singleShot(5000, gui.get(), [&, attempts] {
                if (*attempts != 2) {
                    checkResult = 1;
                    gui->close();
                }
            });
        }
	}
	const int result = a.exec();
    return checkResult != 0 ? checkResult : result;
}
	

