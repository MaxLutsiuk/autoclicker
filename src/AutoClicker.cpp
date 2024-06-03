#include "AutoClicker.h"
#include <thread>
#include <chrono>

namespace
{
	//////////////////////////////////////////////////////////////////////////////////
	// Get button down and button up events from mouse button id

	std::pair<int, int> _GetMouseUpDownEvents(int i_mouse_btn)
	{
		if (i_mouse_btn == VK_LBUTTON)
			return std::make_pair(MOUSEEVENTF_LEFTUP, MOUSEEVENTF_LEFTDOWN);
		if (i_mouse_btn == VK_RBUTTON)
			return std::make_pair(MOUSEEVENTF_RIGHTUP, MOUSEEVENTF_RIGHTDOWN);
		if (i_mouse_btn == VK_MBUTTON)
			return std::make_pair(MOUSEEVENTF_MIDDLEUP, MOUSEEVENTF_MIDDLEDOWN);
		if (i_mouse_btn == VK_XBUTTON1)
			return std::make_pair(MOUSEEVENTF_XUP, MOUSEEVENTF_XDOWN);

		throw std::invalid_argument("Unsupported mouse button");
	};

	//////////////////////////////////////////////////////////////////////////////////
	// Get mouse button id from button down and button up mouse events

	int _GetMouseButton(int i_mouse_event) {
		if (i_mouse_event == WM_LBUTTONUP || i_mouse_event == WM_LBUTTONDOWN)
			return VK_LBUTTON;
		if (i_mouse_event == WM_RBUTTONUP || i_mouse_event == WM_RBUTTONDOWN)
			return VK_RBUTTON;
		if (i_mouse_event == WM_MBUTTONUP || i_mouse_event == WM_MBUTTONDOWN)
			return VK_MBUTTON;
		if (i_mouse_event == WM_XBUTTONUP || i_mouse_event == WM_XBUTTONDOWN) {
			return VK_XBUTTON1;
		}

		throw std::invalid_argument("Unsupported mouse event");
	}

	//////////////////////////////////////////////////////////////////////////////////
	// Keyboard hook

	LRESULT WINAPI KeyPressedHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
		if (nCode == HC_ACTION) {
			if (wParam == WM_KEYDOWN || wParam == WM_KEYUP)
			{
				KBDLLHOOKSTRUCT* pKeyboard = (KBDLLHOOKSTRUCT*)lParam;
				if (ULONG_PTR ignore_event = pKeyboard->dwExtraInfo; ignore_event == 1)
					return CallNextHookEx(AutoClicker::getInstance().GetKeyBoardHook(), nCode, wParam, lParam);

				auto key = pKeyboard->vkCode;
				auto mouse_button = std::nullopt;
				auto is_down = WM_KEYDOWN == wParam;
				AutoClicker::getInstance().BindKey({ key, mouse_button , is_down });
			}

		}
		return CallNextHookEx(AutoClicker::getInstance().GetKeyBoardHook(), nCode, wParam, lParam);
	}

	//////////////////////////////////////////////////////////////////////////////////
	//Mouse hook

	LRESULT WINAPI MousePressedHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
		if (nCode == 0) {
			auto key = std::nullopt;
			MSLLHOOKSTRUCT* pMouse = (MSLLHOOKSTRUCT*)lParam;
			if (ULONG_PTR ignore_event = pMouse->dwExtraInfo; ignore_event == 1)
				return CallNextHookEx(AutoClicker::getInstance().GetMouseHook(), nCode, wParam, lParam);

			if (wParam == WM_LBUTTONDOWN ||
				wParam == WM_RBUTTONDOWN ||
				wParam == WM_MBUTTONDOWN ||
				wParam == WM_XBUTTONDOWN) {
				AutoClicker::getInstance().BindKey(InputKey(key, _GetMouseButton(wParam), true));
			}
			else if (wParam == WM_LBUTTONUP ||
				wParam == WM_RBUTTONUP ||
				wParam == WM_MBUTTONUP ||
				wParam == WM_XBUTTONUP) {
				AutoClicker::getInstance().BindKey(InputKey(key, _GetMouseButton(wParam), false));
			}
		}
		return CallNextHookEx(AutoClicker::getInstance().GetMouseHook(), nCode, wParam, lParam);
	}

	//////////////////////////////////////////////////////////////////////////////////
	// Send down up events for input key

	void _AutoClickKey(const InputKey& i_input_key)
	{
		if (!i_input_key.IsValid())
			return;

		ULONG_PTR ignore_event = 1;
		if (i_input_key.m_key.has_value() && i_input_key.m_is_down) {
			INPUT input[2] = { 0 };
			input[0].type = INPUT_KEYBOARD;
			input[0].ki.wVk = i_input_key.m_key.value();
			input[0].ki.dwExtraInfo = ignore_event;

			input[1].type = INPUT_KEYBOARD;
			input[1].ki.wVk = i_input_key.m_key.value();
			input[1].ki.dwFlags = KEYEVENTF_KEYUP;
			input[1].ki.dwExtraInfo = ignore_event;

			::SendInput(2, input, sizeof(INPUT));
		}
		else if (i_input_key.m_mouse_btn.has_value() && i_input_key.m_is_down)
		{
			auto up_down = _GetMouseUpDownEvents(i_input_key.m_mouse_btn.value());
			INPUT input[2] = { 0 };
			input[0].type = INPUT_MOUSE;
			input[0].mi.dwFlags = up_down.second;
			input[0].mi.dwExtraInfo = ignore_event;

			input[1].type = INPUT_MOUSE;
			input[1].mi.dwFlags = up_down.first;
			input[1].mi.dwExtraInfo = ignore_event;

			::SendInput(2, input, sizeof(INPUT));
		}
	}

	//////////////////////////////////////////////////////////////////////////////////
	// autoclicker loop

	void _DoAutoClick(const InputKey& i_input_first_key, const InputKey& i_input_second_key,
		const bool& i_do_autoclick)
	{
		while (true) {
			if (!i_do_autoclick)
				std::this_thread::sleep_for(std::chrono::milliseconds(100)); // avoid high cpu usage

			bool two_buttons_toggled = i_input_first_key.m_is_down && i_input_second_key.m_is_down;
			if (i_do_autoclick && !two_buttons_toggled)
			{
				_AutoClickKey(i_input_first_key);
				_AutoClickKey(i_input_second_key);
				std::this_thread::sleep_for(std::chrono::milliseconds(60));
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////////////
	// Get title for keyboard key or mouse button

	QString _GetKeyTitle(const InputKey& i_key) {
		if (i_key.m_key.has_value()) {
			wchar_t buf[32]{};
			GetKeyNameTextW(MapVirtualKeyW(i_key.m_key.value(), MAPVK_VK_TO_VSC) << 16, buf, sizeof(buf) / sizeof(buf[0]));
			return QString::fromWCharArray(buf);
		}
		if (i_key.m_mouse_btn.has_value())
		{
			const auto mouse_btn = i_key.m_mouse_btn.value();
			if (mouse_btn == VK_LBUTTON)
				return { "Left mouse button" };
			if (mouse_btn == VK_RBUTTON)
				return { "Right mouse button" };
			if (mouse_btn == VK_MBUTTON)
				return { "Middle mouse button" };
			if (mouse_btn == VK_XBUTTON1)
				return { "X mouse button" };
		}

		throw std::invalid_argument("invalid key or mouse button");
	}
}


//////////////////////////////////////////////////////////////////////////////////

void InputKey::Reset() {
	m_key.reset();
	m_mouse_btn.reset();
};

//////////////////////////////////////////////////////////////////////////////////

bool InputKey::IsValid() const { return m_key.has_value() || m_mouse_btn.has_value(); }

//////////////////////////////////////////////////////////////////////////////////

AutoClicker::AutoClicker()
{
	std::thread autoclick_thread(_DoAutoClick, std::ref(m_binded_key_first), std::ref(m_binded_key_second), std::ref(m_do_autoclick));
	autoclick_thread.detach();
	_InstallHooks();
}

//////////////////////////////////////////////////////////////////////////////////

AutoClicker::~AutoClicker()
{
	_UnInstallHooks();
}

//////////////////////////////////////////////////////////////////////////////////

bool AutoClicker::_InstallHooks()
{
	mp_keyboard_hook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyPressedHookProc, NULL, 0);
	mp_mouse_hook = SetWindowsHookEx(WH_MOUSE_LL, MousePressedHookProc, NULL, 0);
	return mp_keyboard_hook && mp_mouse_hook;
}

//////////////////////////////////////////////////////////////////////////////////

bool AutoClicker::_UnInstallHooks()
{
	return UnhookWindowsHookEx(mp_keyboard_hook) && UnhookWindowsHookEx(mp_mouse_hook);
}

//////////////////////////////////////////////////////////////////////////////////

HHOOK AutoClicker::GetMouseHook() { return mp_mouse_hook; };

//////////////////////////////////////////////////////////////////////////////////

HHOOK AutoClicker::GetKeyBoardHook() { return mp_keyboard_hook; }

//////////////////////////////////////////////////////////////////////////////////
// bind keys
void AutoClicker::BindKey(const InputKey& i_input)
{
	bool is_down = i_input.m_is_down;

	if (m_key_to_detect != KeyToDetect::NONE)
	{
		auto& target_key = _GetTargetKey(m_key_to_detect);
		target_key = i_input;
		m_key_to_detect = KeyToDetect::NONE;
		const auto key_title = _GetKeyTitle(target_key);
		target_key.m_is_down = false;
		emit keyDetected(_GetKeyTitle(target_key));
		return;
	}

	if (i_input == m_activation_key)
		m_do_autoclick = is_down;
	if (i_input == m_binded_key_first)
		m_binded_key_first.m_is_down = is_down;
	if (i_input == m_binded_key_second)
		m_binded_key_second.m_is_down = is_down;
}

//////////////////////////////////////////////////////////////////////////////////

InputKey& AutoClicker::_GetTargetKey(KeyToDetect i_key_to_detect)
{
	if (i_key_to_detect == KeyToDetect::KEYFIRST)
		return m_binded_key_first;
	if (i_key_to_detect == KeyToDetect::KEYSECOND)
		return m_binded_key_second;
	if (i_key_to_detect == KeyToDetect::KEYACTIVATION)
		return m_activation_key;
}

//////////////////////////////////////////////////////////////////////////////////

void AutoClicker::ActivateDetection(KeyToDetect i_key_to_detect) {

	m_key_to_detect = i_key_to_detect;
	_GetTargetKey(i_key_to_detect).Reset();
}