#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDir>
#include "FilesModel.h"
#include "ImageHandler.h"

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QGuiApplication app(argc, argv);

    QDir dir;
    QString path = dir.absolutePath();

    if(argc>=2)
    {
        path = argv[1];
    }

    FilesModel model(path);
    ImageHandler imageHandler(model);

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("fileModel", &model);
    engine.rootContext()->setContextProperty("imageHandler", &imageHandler);
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
