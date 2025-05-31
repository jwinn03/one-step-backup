#include "one_step_backup.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    one_step_backup w;
    w.show();
    return a.exec();
}
