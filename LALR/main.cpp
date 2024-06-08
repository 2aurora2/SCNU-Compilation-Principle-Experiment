#include "mainwindow.h"
#include "infodialog.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    InfoDialog* infoDialog = new InfoDialog();
    auto mark = infoDialog->exec();
    if(mark == QDialog::Accepted){
        MainWindow w;
        w.show();
        return a.exec();
    }
    else{
        return 0;
    }
}
