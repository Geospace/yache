#include <fstream>

#include "Chip8.hpp"
#include "Keypad.hpp"
#include "status.hpp"
#include "colors.hpp"

Chip8::Chip8(void) :
  _vmemory(Specs::WINDOW_WIDTH, Specs::WINDOW_HEIGHT),
  _window(sf::VideoMode(Specs::WINDOW_WIDTH * Specs::WINDOW_SCALE,
        Specs::WINDOW_HEIGHT * Specs::WINDOW_SCALE), "Yache")
{
  _window.setVerticalSyncEnabled(true);
  _window.setFramerateLimit(60);
  _loadFontset();
  _texture.create(Specs::WINDOW_WIDTH, Specs::WINDOW_HEIGHT);
  _sprite.setTexture(_texture);
  _sprite.scale(Specs::WINDOW_SCALE, Specs::WINDOW_SCALE);
}

constexpr void Chip8::_loadFontset(void) {
  constexpr std::array<uint8_t, Specs::FONTSET_SIZE> fontSet = {{
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
  }};

  for (uint32_t i = 0; i < fontSet.size(); ++i) {
    _memory[i] = fontSet[i];
  }
}

// TODO: Error handling
void Chip8::loadRomFromFile(const std::string& filePath) {
  std::ifstream rom;

  rom.open(filePath, std::ios::binary);
  if (!rom.good())
    return;

  rom.read(reinterpret_cast<char *>(_memory.data() + Specs::ROM_OFFSET),
      Specs::MEMORY_SIZE);
  rom.close();
}

void Chip8::_updateKeyStatus(void) {
  for (auto& key : Keypad::keys)
    if (sf::Keyboard::isKeyPressed(key.k))
      _keys[key.i] = true;
    else
      _keys[key.i] = false;
}

void Chip8::_windowCycle(void) {
  while (_window.pollEvent(_windowEvent)) {
    switch (_windowEvent.type) {
      case sf::Event::Closed:
        _window.close();
        exit(SUCCESS);

      case sf::Event::KeyPressed:
        _updateKeyStatus();
        break;
      case sf::Event::KeyReleased:
        _updateKeyStatus();
        break;

      default:
        break;
    }
  }

  if (_redraw) {
    _texture.update(_vmemory.raw());
    _window.clear(sf::Color::Black);
    _window.draw(_sprite);
    _window.display();

    _redraw = false;
  }
}

// TODO: Cleaner error handling
void Chip8::cycle(void) {
  _windowCycle();

  _currentOpcode = _memory[_pc] << 8 | _memory[_pc + 1];
  _pc += 2;

  for (auto& op : _opcodes)
    if ((_currentOpcode & op.mask) == op.key)
      return op.f(this);

  fprintf(stderr, "NON EXISTING OPCODE :"
      " 0x%04x\nExiting.\n", _currentOpcode);
  exit(FAILURE);
}
