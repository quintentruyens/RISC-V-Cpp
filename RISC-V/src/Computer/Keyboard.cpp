#include "Keyboard.h"

constexpr uint32_t SHIFTL_KEY = 0xA0;
constexpr uint32_t SHIFTR_KEY = 0xA1;
constexpr uint32_t ALT_GR_KEY = 0xA5;
constexpr uint32_t CAPS_LOCK_KEY = 0x14;
constexpr uint32_t LCTRL_KEY = 0xA2;
constexpr uint32_t RCTRL_KEY = 0xA3;

constexpr double KEY_REPEAT_DELAY = 0.5;
constexpr double KEY_REPEAT_TIME = 0.03;

const sKeyMapping keyMappings[] = {
	{L'\0', L'\0', L'\0'}, // 0x00
	{L'\0', L'\0', L'\0'}, // 0x01
	{L'\0', L'\0', L'\0'}, // 0x02
	{L'\0', L'\0', L'\0'}, // 0x03
	{L'\0', L'\0', L'\0'}, // 0x04
	{L'\0', L'\0', L'\0'}, // 0x05
	{L'\0', L'\0', L'\0'}, // 0x06
	{L'\0', L'\0', L'\0'}, // 0x07
	{L'\u0008', L'\u0008', L'\0'}, // 0x08
	{L'\0', L'\0', L'\0'}, // 0x09
	{L'\0', L'\0', L'\0'}, // 0x0a
	{L'\0', L'\0', L'\0'}, // 0x0b
	{L'\0', L'\0', L'\0'}, // 0x0c
	{L'\n', L'\0', L'\0'}, // 0x0d
	{L'\0', L'\0', L'\0'}, // 0x0e
	{L'\0', L'\0', L'\0'}, // 0x0f
	{L'\0', L'\0', L'\0'}, // 0x10
	{L'\0', L'\0', L'\0'}, // 0x11
	{L'\0', L'\0', L'\0'}, // 0x12
	{L'\0', L'\0', L'\0'}, // 0x13
	{L'\0', L'\0', L'\0'}, // 0x14
	{L'\0', L'\0', L'\0'}, // 0x15
	{L'\0', L'\0', L'\0'}, // 0x16
	{L'\0', L'\0', L'\0'}, // 0x17
	{L'\0', L'\0', L'\0'}, // 0x18
	{L'\0', L'\0', L'\0'}, // 0x19
	{L'\0', L'\0', L'\0'}, // 0x1a
	{L'\0', L'\0', L'\0'}, // 0x1b
	{L'\0', L'\0', L'\0'}, // 0x1c
	{L'\0', L'\0', L'\0'}, // 0x1d
	{L'\0', L'\0', L'\0'}, // 0x1e
	{L'\0', L'\0', L'\0'}, // 0x1f
	{L' ', L' ', L'\0'}, // 0x20
	{L'\0', L'\0', L'\0'}, // 0x21
	{L'\0', L'\0', L'\0'}, // 0x22
	{L'\0', L'\0', L'\0'}, // 0x23
	{L'\0', L'\0', L'\0'}, // 0x24
	{L'\0', L'\0', L'\0'}, // 0x25
	{L'\0', L'\0', L'\0'}, // 0x26
	{L'\0', L'\0', L'\0'}, // 0x27
	{L'\0', L'\0', L'\0'}, // 0x28
	{L'\0', L'\0', L'\0'}, // 0x29
	{L'\0', L'\0', L'\0'}, // 0x2a
	{L'\0', L'\0', L'\0'}, // 0x2b
	{L'\0', L'\0', L'\0'}, // 0x2c
	{L'\0', L'\0', L'\0'}, // 0x2d
	{L'\0', L'\0', L'\0'}, // 0x2e
	{L'\0', L'\0', L'\0'}, // 0x2f
	{L'à', L'0', L'}'}, // 0x30
	{L'&', L'1', L'|'}, // 0x31
	{L'é', L'2', L'@'}, // 0x32
	{L'"', L'3', L'#'}, // 0x33
	{L'\'', L'4', L'{'}, // 0x34
	{L'(', L'5', L'['}, // 0x35
	{L'§', L'6', L'^'}, // 0x36
	{L'è', L'7', L'\0'}, // 0x37
	{L'!', L'8', L'\0'}, // 0x38
	{L'ç', L'9', L'{'}, // 0x39
	{L'\0', L'\0', L'\0'}, // 0x3a
	{L'\0', L'\0', L'\0'}, // 0x3b
	{L'\0', L'\0', L'\0'}, // 0x3c
	{L'\0', L'\0', L'\0'}, // 0x3d
	{L'\0', L'\0', L'\0'}, // 0x3e
	{L'\0', L'\0', L'\0'}, // 0x3f
	{L'\0', L'\0', L'\0'}, // 0x40
	{L'a', L'A', L'\0'}, // 0x41
	{L'b', L'B', L'\0'}, // 0x42
	{L'c', L'C', L'\0'}, // 0x43
	{L'd', L'D', L'\0'}, // 0x44
	{L'e', L'E', L'€'}, // 0x45
	{L'f', L'F', L'\0'}, // 0x46
	{L'g', L'G', L'\0'}, // 0x47
	{L'h', L'H', L'\0'}, // 0x48
	{L'i', L'I', L'\0'}, // 0x49
	{L'j', L'J', L'\0'}, // 0x4a
	{L'k', L'K', L'\0'}, // 0x4b
	{L'l', L'L', L'\0'}, // 0x4c
	{L'm', L'M', L'\0'}, // 0x4d
	{L'n', L'N', L'\0'}, // 0x4e
	{L'o', L'O', L'\0'}, // 0x4f
	{L'p', L'P', L'\0'}, // 0x50
	{L'q', L'Q', L'\0'}, // 0x51
	{L'r', L'R', L'\0'}, // 0x52
	{L's', L'S', L'\0'}, // 0x53
	{L't', L'T', L'\0'}, // 0x54
	{L'u', L'U', L'\0'}, // 0x55
	{L'v', L'V', L'\0'}, // 0x56
	{L'w', L'W', L'\0'}, // 0x57
	{L'x', L'X', L'\0'}, // 0x58
	{L'y', L'Y', L'\0'}, // 0x59
	{L'z', L'Z', L'\0'}, // 0x5a
	{L'\0', L'\0', L'\0'}, // 0x5b
	{L'\0', L'\0', L'\0'}, // 0x5c
	{L'\0', L'\0', L'\0'}, // 0x5d
	{L'\0', L'\0', L'\0'}, // 0x5e
	{L'\0', L'\0', L'\0'}, // 0x5f
	{L'0', L'\0', L'\0'}, // 0x60
	{L'1', L'\0', L'\0'}, // 0x61
	{L'2', L'\0', L'\0'}, // 0x62
	{L'3', L'\0', L'\0'}, // 0x63
	{L'4', L'\0', L'\0'}, // 0x64
	{L'5', L'\0', L'\0'}, // 0x65
	{L'6', L'\0', L'\0'}, // 0x66
	{L'7', L'\0', L'\0'}, // 0x67
	{L'8', L'\0', L'\0'}, // 0x68
	{L'9', L'\0', L'\0'}, // 0x69
	{L'*', L'*', L'\0'}, // 0x6a
	{L'+', L'+', L'\0'}, // 0x6b
	{ L'\0', L'\0', L'\0' }, // 0x6c
	{L'-', L'-', L'\0'}, // 0x6d
	{L'.', L'\0', L'\0'}, // 0x6e
	{L'/', L'/', L'\0'}, // 0x6f
	{ L'\0', L'\0', L'\0' }, // 0x70
	{ L'\0', L'\0', L'\0' }, // 0x71
	{ L'\0', L'\0', L'\0' }, // 0x72
	{ L'\0', L'\0', L'\0' }, // 0x73
	{ L'\0', L'\0', L'\0' }, // 0x74
	{ L'\0', L'\0', L'\0' }, // 0x75
	{ L'\0', L'\0', L'\0' }, // 0x76
	{ L'\0', L'\0', L'\0' }, // 0x77
	{ L'\0', L'\0', L'\0' }, // 0x78
	{ L'\0', L'\0', L'\0' }, // 0x79
	{ L'\0', L'\0', L'\0' }, // 0x7a
	{ L'\0', L'\0', L'\0' }, // 0x7b
	{ L'\0', L'\0', L'\0' }, // 0x7c
	{ L'\0', L'\0', L'\0' }, // 0x7d
	{ L'\0', L'\0', L'\0' }, // 0x7e
	{ L'\0', L'\0', L'\0' }, // 0x7f
	{ L'\0', L'\0', L'\0' }, // 0x80
	{ L'\0', L'\0', L'\0' }, // 0x81
	{ L'\0', L'\0', L'\0' }, // 0x82
	{ L'\0', L'\0', L'\0' }, // 0x83
	{ L'\0', L'\0', L'\0' }, // 0x84
	{ L'\0', L'\0', L'\0' }, // 0x85
	{ L'\0', L'\0', L'\0' }, // 0x86
	{ L'\0', L'\0', L'\0' }, // 0x87
	{ L'\0', L'\0', L'\0' }, // 0x88
	{ L'\0', L'\0', L'\0' }, // 0x89
	{ L'\0', L'\0', L'\0' }, // 0x8a
	{ L'\0', L'\0', L'\0' }, // 0x8b
	{ L'\0', L'\0', L'\0' }, // 0x8c
	{ L'\0', L'\0', L'\0' }, // 0x8d
	{ L'\0', L'\0', L'\0' }, // 0x8e
	{ L'\0', L'\0', L'\0' }, // 0x8f
	{ L'\0', L'\0', L'\0' }, // 0x90
	{ L'\0', L'\0', L'\0' }, // 0x91
	{ L'\0', L'\0', L'\0' }, // 0x92
	{ L'\0', L'\0', L'\0' }, // 0x93
	{ L'\0', L'\0', L'\0' }, // 0x94
	{ L'\0', L'\0', L'\0' }, // 0x95
	{ L'\0', L'\0', L'\0' }, // 0x96
	{ L'\0', L'\0', L'\0' }, // 0x97
	{ L'\0', L'\0', L'\0' }, // 0x98
	{ L'\0', L'\0', L'\0' }, // 0x99
	{ L'\0', L'\0', L'\0' }, // 0x9a
	{ L'\0', L'\0', L'\0' }, // 0x9b
	{ L'\0', L'\0', L'\0' }, // 0x9c
	{ L'\0', L'\0', L'\0' }, // 0x9d
	{ L'\0', L'\0', L'\0' }, // 0x9e
	{ L'\0', L'\0', L'\0' }, // 0x9f
	{ L'\0', L'\0', L'\0' }, // 0xa0
	{ L'\0', L'\0', L'\0' }, // 0xa1
	{ L'\0', L'\0', L'\0' }, // 0xa2
	{ L'\0', L'\0', L'\0' }, // 0xa3
	{ L'\0', L'\0', L'\0' }, // 0xa4
	{ L'\0', L'\0', L'\0' }, // 0xa5
	{ L'\0', L'\0', L'\0' }, // 0xa6
	{ L'\0', L'\0', L'\0' }, // 0xa7
	{ L'\0', L'\0', L'\0' }, // 0xa8
	{ L'\0', L'\0', L'\0' }, // 0xa9
	{ L'\0', L'\0', L'\0' }, // 0xaa
	{ L'\0', L'\0', L'\0' }, // 0xab
	{ L'\0', L'\0', L'\0' }, // 0xac
	{ L'\0', L'\0', L'\0' }, // 0xad
	{ L'\0', L'\0', L'\0' }, // 0xae
	{ L'\0', L'\0', L'\0' }, // 0xaf
	{ L'\0', L'\0', L'\0' }, // 0xb0
	{ L'\0', L'\0', L'\0' }, // 0xb1
	{ L'\0', L'\0', L'\0' }, // 0xb2
	{ L'\0', L'\0', L'\0' }, // 0xb3
	{ L'\0', L'\0', L'\0' }, // 0xb4
	{ L'\0', L'\0', L'\0' }, // 0xb5
	{ L'\0', L'\0', L'\0' }, // 0xb6
	{ L'\0', L'\0', L'\0' }, // 0xb7
	{ L'\0', L'\0', L'\0' }, // 0xb8
	{ L'\0', L'\0', L'\0' }, // 0xb9
	{ L'$', L'*', L']' }, // 0xba
	{ L'=', L'+', L'~' }, // 0xbb
	{ L',', L'?', L'\0' }, // 0xbc
	{ L'-', L'_', L'\0' }, // 0xbd
	{ L';', L'.', L'\0' }, // 0xbe
	{ L':', L'/', L'\0' }, // 0xbf
	{ L'ù', L'%', L'´' }, // 0xc0
	{ L'\0', L'\0', L'\0' }, // 0xc1
	{ L'\0', L'\0', L'\0' }, // 0xc2
	{ L'\0', L'\0', L'\0' }, // 0xc3
	{ L'\0', L'\0', L'\0' }, // 0xc4
	{ L'\0', L'\0', L'\0' }, // 0xc5
	{ L'\0', L'\0', L'\0' }, // 0xc6
	{ L'\0', L'\0', L'\0' }, // 0xc7
	{ L'\0', L'\0', L'\0' }, // 0xc8
	{ L'\0', L'\0', L'\0' }, // 0xc9
	{ L'\0', L'\0', L'\0' }, // 0xca
	{ L'\0', L'\0', L'\0' }, // 0xcb
	{ L'\0', L'\0', L'\0' }, // 0xcc
	{ L'\0', L'\0', L'\0' }, // 0xcd
	{ L'\0', L'\0', L'\0' }, // 0xce
	{ L'\0', L'\0', L'\0' }, // 0xcf
	{ L'\0', L'\0', L'\0' }, // 0xd0
	{ L'\0', L'\0', L'\0' }, // 0xd1
	{ L'\0', L'\0', L'\0' }, // 0xd2
	{ L'\0', L'\0', L'\0' }, // 0xd3
	{ L'\0', L'\0', L'\0' }, // 0xd4
	{ L'\0', L'\0', L'\0' }, // 0xd5
	{ L'\0', L'\0', L'\0' }, // 0xd6
	{ L'\0', L'\0', L'\0' }, // 0xd7
	{ L'\0', L'\0', L'\0' }, // 0xd8
	{ L'\0', L'\0', L'\0' }, // 0xd9
	{ L'\0', L'\0', L'\0' }, // 0xda
	{ L')', L'°', L'\0' }, // 0xdb
	{ L'µ', L'£', L'`' }, // 0xdc
	{ L'^', L'¨', L'[' }, // 0xdd
	{ L'²', L'³', L'\0' }, // 0xde
	{ L'\0', L'\0', L'\0' }, // 0xdf
	{ L'\0', L'\0', L'\0' }, // 0xe0
	{ L'\0', L'\0', L'\0' }, // 0xe1
	{ L'<', L'>', L'\\' }, // 0xe2
	{ L'\0', L'\0', L'\0' }, // 0xe3
	{ L'\0', L'\0', L'\0' }, // 0xe4
	{ L'\0', L'\0', L'\0' }, // 0xe5
	{ L'\0', L'\0', L'\0' }, // 0xe6
	{ L'\0', L'\0', L'\0' }, // 0xe7
	{ L'\0', L'\0', L'\0' }, // 0xe8
	{ L'\0', L'\0', L'\0' }, // 0xe9
	{ L'\0', L'\0', L'\0' }, // 0xea
	{ L'\0', L'\0', L'\0' }, // 0xeb
	{ L'\0', L'\0', L'\0' }, // 0xec
	{ L'\0', L'\0', L'\0' }, // 0xed
	{ L'\0', L'\0', L'\0' }, // 0xee
	{ L'\0', L'\0', L'\0' }, // 0xef
	{ L'\0', L'\0', L'\0' }, // 0xf0
	{ L'\0', L'\0', L'\0' }, // 0xf1
	{ L'\0', L'\0', L'\0' }, // 0xf2
	{ L'\0', L'\0', L'\0' }, // 0xf3
	{ L'\0', L'\0', L'\0' }, // 0xf4
	{ L'\0', L'\0', L'\0' }, // 0xf5
	{ L'\0', L'\0', L'\0' }, // 0xf6
	{ L'\0', L'\0', L'\0' }, // 0xf7
	{ L'\0', L'\0', L'\0' }, // 0xf8
	{ L'\0', L'\0', L'\0' }, // 0xf9
	{ L'\0', L'\0', L'\0' }, // 0xfa
	{ L'\0', L'\0', L'\0' }, // 0xfb
	{ L'\0', L'\0', L'\0' }, // 0xfc
	{ L'\0', L'\0', L'\0' }, // 0xfd
	{ L'\0', L'\0', L'\0' }, // 0xfe
	{ L'\0', L'\0', L'\0' }, // 0xff
};

Keyboard::Keyboard(uint32_t addr)
	: addr(addr), bCaps_lock(false), repeatingKey(0), bRepeating(false), timeSinceLastPress(0.0)
{
	buffer.reserve(256);
}

Keyboard::~Keyboard()
{
}

inline wchar_t getChar(uint32_t keyNum, bool shift, bool altGr, bool ctrl)
{
	if (keyNum == 0x4C && ctrl && !altGr) // Ctrl + L
		return '\u000C';

	sKeyMapping mapping = keyMappings[keyNum];
	if (shift)
	{
		if (altGr)
			return L'\0';
		else
			return mapping.shift;
	}
	else
	{
		if (altGr)
			return mapping.alt_gr;
		else
			return mapping.regular;
	}
}

void Keyboard::Update(olcConsoleGameEngine* cge, float fElapsedTime)
{
	if (cge->GetKey(CAPS_LOCK_KEY).bPressed)
		bCaps_lock = !bCaps_lock;

	bool shift = (cge->GetKey(SHIFTL_KEY).bHeld || cge->GetKey(SHIFTR_KEY).bHeld) != bCaps_lock;
	bool altGr = cge->GetKey(ALT_GR_KEY).bHeld;
	bool ctrl = (cge->GetKey(LCTRL_KEY).bHeld || cge->GetKey(LCTRL_KEY).bHeld);

	if (repeatingKey != 0 && !cge->GetKey(repeatingKey).bHeld)
	{
		bRepeating = false;
		repeatingKey = 0;
	}
	uint32_t oldRepeatingKey = repeatingKey;

	for (uint32_t i = 8; i < 256; i++) // Skip mouse buttons, start at first keyboard button, number 8
	{
		auto key = cge->GetKey(i);
		if (key.bPressed)
		{
			repeatingKey = i;

			wchar_t character = getChar(i, shift, altGr, ctrl);
			addCharacter(character);
		}
	}

	// Handle repeating keys
	if (oldRepeatingKey == repeatingKey && repeatingKey != 0) // No new key presses, old key still held down
	{
		timeSinceLastPress += fElapsedTime;

		while (timeSinceLastPress > (bRepeating ? KEY_REPEAT_TIME : KEY_REPEAT_DELAY))
		{
			timeSinceLastPress -= (bRepeating ? KEY_REPEAT_TIME : KEY_REPEAT_DELAY);
			bRepeating = true;

			wchar_t character = getChar(repeatingKey, shift, altGr, ctrl);
			addCharacter(character);
		}
	}
	else
	{
		// Reset repeating state
		bRepeating = false;
		timeSinceLastPress = 0;
	}
}

void Keyboard::addCharacter(wchar_t character)
{
	if (character != 0 && buffer.length() < 256)
	{
		if (buffer.length() == 0)
			bus->postInterrupt();
		buffer.push_back(character);
	}
}

MemAccessResult Keyboard::write(uint32_t addr, uint32_t data, DataSize dataSize)
{
	return MemAccessResult::NotInRange;
}

MemAccessResult Keyboard::read(uint32_t addr, uint32_t& result, bool bReadOnly, DataSize dataSize, bool isSigned)
{
	if (addr != this->addr) return MemAccessResult::NotInRange;

	if (buffer.length() > 0)
	{
		wchar_t character = buffer[0];
		if (!bReadOnly)
		{
			buffer.erase(0, 1);
			if (buffer.length() == 0)
				bus->clearInterrupt();
		}

		result = character;
		return MemAccessResult::Success;
	}
	result = 0;
	return MemAccessResult::Success;
}
