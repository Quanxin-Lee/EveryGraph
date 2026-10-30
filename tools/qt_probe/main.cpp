#include <QApplication>
#include <QByteArray>
#include <QGuiApplication>
#include <QLabel>
#include <QMainWindow>
#include <QStatusBar>
#include <QString>
#include <QTimer>

#include <cstdio>
#include <cstring>

namespace {

constexpr auto kWindowTitle = "EveryGraph Qt qualification probe";
constexpr auto kCentralText = "Qt Widgets probe; no scientific business logic";
constexpr auto kOffscreenPlatform = "offscreen";

}

int main(int argc, char* argv[]) {
    const bool smoke_test = argc > 1 && std::strcmp(argv[1], "--smoke-test") == 0;

    QApplication app(argc, argv);

    QMainWindow window;
    window.setWindowTitle(QString::fromLatin1(kWindowTitle));

    auto* central = new QLabel(QString::fromLatin1(kCentralText), &window);
    central->setAlignment(Qt::AlignCenter);
    window.setCentralWidget(central);
    window.statusBar()->showMessage(
        QStringLiteral("Qt %1 | platform %2").arg(QString::fromLatin1(qVersion()),
                                                   QGuiApplication::platformName()));
    window.resize(640, 400);
    window.show();

    const QByteArray platform = QGuiApplication::platformName().toLatin1();
    std::printf("qt_probe: Qt %s, platform %s, central widget %s\n", qVersion(),
                platform.constData(), window.centralWidget() != nullptr ? "present" : "missing");

    if (smoke_test) {
        if (!window.isVisible() || window.centralWidget() == nullptr) {
            std::fprintf(stderr,
                         "qt_probe: smoke test failed: window is not visible or has no "
                         "central widget\n");
            return 1;
        }
        if (QGuiApplication::platformName() != QLatin1String(kOffscreenPlatform)) {
            std::fprintf(stderr, "qt_probe: smoke test requires the offscreen platform, got %s\n",
                         platform.constData());
            return 2;
        }
        QTimer::singleShot(0, &app, &QCoreApplication::quit);
    }

    const int exit_code = app.exec();
    if (exit_code != 0) {
        std::fprintf(stderr, "qt_probe: event loop exited with %d\n", exit_code);
    }
    return exit_code;
}
