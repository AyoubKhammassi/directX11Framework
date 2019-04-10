#pragma once
#include <queue>
#include <bitset>

class Keyboard
{

	friend class Window;

public: class Event
	{
	public:
		enum class Type
		{
			Press,
			Release,
			Invalid
		};

	private:
		Type type;
		unsigned char code;
	public:
		Event() noexcept :type(Type::Invalid), code(0u) 
		{}

		Event(Type t, unsigned char c) noexcept : type(t), code(c) 
		{}

		bool isPress() const noexcept
		{
			return type == Type::Press;
		}

		bool isRelease() const noexcept
		{
			return type == Type::Release;
		}

		bool isValid() const noexcept
		{
			return type != Type::Invalid;
		}

		unsigned char GetCode()
		{
			return code;
		}

	};
	//this is the public interface used by the client code to manage keyboard input
public:
	Keyboard() = default;
	Keyboard(const Keyboard&) = delete;
	Keyboard& operator=(const Keyboard&) = delete;
	//key event thingies
	bool KeyIsPressed(unsigned char keycode) const noexcept;
	Event ReadKey() noexcept;
	bool KeyIsEmpty() const noexcept;
	void FlushKey() noexcept;

	//char event thingies
	char ReadChar() noexcept;
	bool charIsEmpty() const noexcept;
	void FlushChar() noexcept;
	//flush both char and event
	void Flush() noexcept;
	//autorepeat control
	void EnableAutoRepeat() noexcept;
	void DisableAutoRepeat() noexcept;
	bool AutoRepeatIsEnabled() const noexcept;

	//this is the private interface used by window (friend) to generate events for client
private:
	void OnKeyPressed(unsigned char keycode) noexcept;
	void OnKeyReleased(unsigned char keycode) noexcept;
	void OnChar(char character) noexcept;
	void ClearState() noexcept;
	template<typename T> static void TrimBuffer(std::queue<T>& buffer) noexcept;
private:
	static constexpr unsigned int nKeys = 256u;
	static constexpr unsigned int bufferSize = 16u;
	bool autoRepeatEnabled = false;

	std::bitset<nKeys> keyStates;
	std::queue<Event> keyBuffer;
	std::queue<char> charBuffer;
};

template<typename T>
inline void Keyboard::TrimBuffer(std::queue<T>& buffer) noexcept
{
	while (buffer.size() > bufferSize)
	{
		buffer.pop();
	}
}
