#include "Keyboard.h"




bool Keyboard::KeyIsPressed(unsigned char keycode) const noexcept
{
	return keyStates[keycode];
}

Keyboard::Event Keyboard::ReadKey() noexcept
{
	if (keyBuffer.size() > 0u)
	{
		Keyboard::Event e = keyBuffer.front();
		keyBuffer.pop();
		return e;
	}
	else
	{
		return Keyboard::Event();
	}
}

bool Keyboard::KeyIsEmpty() const noexcept
{
	return keyBuffer.empty();
}

void Keyboard::FlushKey() noexcept
{
	keyBuffer = std::queue<Event>();
}

char Keyboard::ReadChar() noexcept
{
	if (charBuffer.size() > 0u)
	{
		char c = charBuffer.front();
		charBuffer.pop();
		return c;
	}
	else
	{
		return 0;
	}
}

bool Keyboard::charIsEmpty() const noexcept
{
	return charBuffer.empty();
}

void Keyboard::FlushChar() noexcept
{
	charBuffer = std::queue<char>();
}

void Keyboard::Flush() noexcept
{
	Keyboard::FlushChar();
	Keyboard::FlushKey();
}

void Keyboard::EnableAutoRepeat() noexcept
{
	Keyboard::autoRepeatEnabled = true;
}

void Keyboard::DisableAutoRepeat() noexcept
{
	Keyboard::autoRepeatEnabled = false;
}

bool Keyboard::AutoRepeatIsEnabled() const noexcept
{
	return Keyboard::autoRepeatEnabled;
}

void Keyboard::OnKeyPressed(unsigned char keycode) noexcept
{
	keyStates[keycode] = true;
	keyBuffer.push(Keyboard::Event(Keyboard::Event::Type::Press, keycode));
	Keyboard::TrimBuffer(keyBuffer);
}

void Keyboard::OnKeyReleased(unsigned char keycode) noexcept
{
	keyStates[keycode] = false;
	keyBuffer.push(Keyboard::Event(Keyboard::Event::Type::Release, keycode));
	Keyboard::TrimBuffer(keyBuffer);
}

void Keyboard::OnChar(char character) noexcept
{
	charBuffer.push(character);
	Keyboard::TrimBuffer(charBuffer);
}

void Keyboard::ClearState() noexcept
{
	keyStates.reset();
}
