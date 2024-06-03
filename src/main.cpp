#include <QApplication>
#include "MainWindow.h"
#include "AutoClicker.h"

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);
	AutoClickerWindow main_window;
	main_window.setFixedSize(main_window.size());
	auto connected = QObject::connect(&main_window, &AutoClickerWindow::detectKey, &main_window, [&](int i_key_to_detect_id) {
		AutoClicker::getInstance().ActivateDetection(static_cast<KeyToDetect>(i_key_to_detect_id)); });
	assert(connected);

	connected = QObject::connect(&AutoClicker::getInstance(), &AutoClicker::keyDetected, &main_window, &AutoClickerWindow::SetKeyTitle);
	assert(connected);

	main_window.show();
	return app.exec();
}
