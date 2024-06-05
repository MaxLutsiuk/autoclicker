#include <QObject>
#include <QMainWindow>
#include "ui_mainwindowdesign.h"

class AutoClickerWindow : public QMainWindow
{
	Q_OBJECT
public:
	AutoClickerWindow();
	~AutoClickerWindow() = default;

signals:
	void detectKey(int);
	void cpsChanged(unsigned int);

public slots:
	void SetKeyTitle(const QString& i_key_title);

private:
	std::unique_ptr<Ui::MainWindow> mp_ui;
	int m_requested_key;
};