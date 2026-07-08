#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QUrl>

int main(int argc, char *argv[])
{
	QGuiApplication app(argc, argv);
	QGuiApplication::setOrganizationName(QStringLiteral("SAGELauncher"));
	QGuiApplication::setApplicationName(QStringLiteral("Launcher"));

	QQmlApplicationEngine engine;
	QObject::connect(
			&engine,
			&QQmlApplicationEngine::objectCreationFailed,
			&app,
			[]() { QCoreApplication::exit(-1); },
			Qt::QueuedConnection);
	// engine.loadFromModule() would be the Qt 6.5+ way to do this, but isn't
	// available on older Qt; loading the compiled-in module's root QML file
	// by URL works the same way on every Qt6 version.
	engine.load(QUrl(QStringLiteral("qrc:/qt/qml/SAGELauncher/qml/Main.qml")));

	return app.exec();
}
