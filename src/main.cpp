#include <cstdlib>
#include <ctime>

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "game/game_controller.hpp"

int main(int argc, char* argv[]) {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    QGuiApplication app(argc, argv);
    QCoreApplication::setOrganizationName("xingheluqi");
    QCoreApplication::setApplicationName("DualBallShooter");
    QQmlApplicationEngine engine;
    GameController gameController;
    app.installEventFilter(&gameController);

    qmlRegisterUncreatableType<GameController>("GameApp", 1, 0, "GameController", "Use the shared controller instance");
    engine.rootContext()->setContextProperty("gameController", &gameController);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection
    );

    engine.loadFromModule("GameApp", "Main");

    return app.exec();
}
