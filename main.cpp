// main.cpp
// Licensed under Apache 2.0

#include "one_step_backup.h"
#include <QtWidgets/QApplication>
#include <QDebug>

static QtMessageHandler originalHandler = nullptr;

void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    // Suppress window geometry warnings
    if (msg.startsWith("QWindowsWindow::setGeometry: Unable to set geometry")) {
        return;
    }

    if (originalHandler) {
        originalHandler(type, context, msg);
    }
}

int main(int argc, char *argv[])
{
    originalHandler = qInstallMessageHandler(messageHandler);

    QApplication a(argc, argv);
    one_step_backup w;
    w.show();
    return a.exec();
}
