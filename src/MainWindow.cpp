#include "MainWindow.h"
#include "AutoClicker.h"

namespace
{
	const QString _press_any = "PRESS ANY KEY/BTN";
}
AutoClickerWindow::AutoClickerWindow()
	: mp_ui(std::make_unique<Ui::MainWindow>()),
	m_requested_key(KeyToDetect::NONE)
{
	mp_ui->setupUi(this);
	mp_ui->mp_cps_edit->setMinimum(1);
	mp_ui->mp_cps_edit->setMaximum(100);

	auto connected = QObject::connect(mp_ui->mp_detect_activation_btn, &QAbstractButton::clicked, this, [this]() {
		mp_ui->mp_activation_key_lbl->setText(_press_any);
		m_requested_key = KeyToDetect::KEYACTIVATION;
		emit detectKey(m_requested_key);
		});
	assert(connected);

	connected = QObject::connect(mp_ui->mp_detect_autoclick_btn_first, &QAbstractButton::clicked, this, [this]() {
		mp_ui->mp_detect_autoclick_btn_first->setText(_press_any);
		m_requested_key = KeyToDetect::KEYFIRST;
		emit detectKey(m_requested_key);
		});
	assert(connected);

	connected = QObject::connect(mp_ui->mp_detect_autoclick_btn_second, &QAbstractButton::clicked, this, [this]() {
		mp_ui->mp_autoclicker_key_lbl_second->setText(_press_any);
		m_requested_key = KeyToDetect::KEYSECOND;
		emit detectKey(m_requested_key);
		});
	assert(connected);

	connected = QObject::connect(mp_ui->mp_cps_edit, &QSlider::valueChanged, this, [this](int i_cps) {
		emit cpsChanged(static_cast<unsigned int>(i_cps));
		mp_ui->mp_cps_label->setText(QString::number(i_cps));
		});
	assert(connected);

	mp_ui->mp_cps_edit->setValue(1);
	mp_ui->mp_cps_label->setText("1");
}

void AutoClickerWindow::SetKeyTitle(const QString& i_key_title)
{
	if (m_requested_key == KeyToDetect::KEYACTIVATION)
		mp_ui->mp_activation_key_lbl->setText(i_key_title);

	if (m_requested_key == KeyToDetect::KEYFIRST)
		mp_ui->mp_autoclicker_key_lbl_first->setText(i_key_title);

	if (m_requested_key == KeyToDetect::KEYSECOND)
		mp_ui->mp_autoclicker_key_lbl_second->setText(i_key_title);
}
