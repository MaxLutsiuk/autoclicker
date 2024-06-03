#include <Windows.h>
#include <optional>
#include <QObject>

struct InputKey
{
	std::optional<int> m_key = std::nullopt;
	std::optional<int> m_mouse_btn = std::nullopt;
	bool m_is_down = false;

	void Reset();
	bool IsValid() const;

	bool operator==(const InputKey& i_other) const {
		return m_key == i_other.m_key && m_mouse_btn == i_other.m_mouse_btn;
	}
};

enum KeyToDetect
{
	NONE = 0,
	KEYFIRST,
	KEYSECOND,
	KEYACTIVATION
};

class AutoClicker : public QObject {
	Q_OBJECT
private:
	AutoClicker();
public:
	~AutoClicker();

	static AutoClicker& getInstance()
	{
		static AutoClicker INSTANCE;
		return INSTANCE;
	}
	void operator=(AutoClicker const&) = delete;

	HHOOK GetMouseHook();
	HHOOK GetKeyBoardHook();
	void BindKey(const InputKey& i_input);
	void ActivateDetection(KeyToDetect i_key_to_detect);

signals:
	void keyDetected(QString);
private:
	InputKey& _GetTargetKey(KeyToDetect i_key_to_detect);
	bool _InstallHooks();
	bool _UnInstallHooks();

private:
	HHOOK mp_keyboard_hook{ nullptr };
	HHOOK mp_mouse_hook{ nullptr };

	InputKey m_activation_key;
	InputKey m_binded_key_first;
	InputKey m_binded_key_second;
	bool m_do_autoclick = false;
	KeyToDetect m_key_to_detect;
};