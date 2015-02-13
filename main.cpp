#ifdef DEBUG
#include <QDebug>
#endif

#include <QApplication>
#include "Dialog.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

#if defined (Q_OS_UNIX)
    app.setWindowIcon(QIcon(":/Resources/RRMControl.png"));
#endif
    Dialog fDialog;
    fDialog.setWindowTitle(QString::fromUtf8("RRM Control"));
    fDialog.show();

    return app.exec();
}

