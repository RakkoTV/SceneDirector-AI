# SceneDirector-AI for OBS Studio

Smart scene switcher with AI-powered prediction capabilities. Automatically switch scenes based on window focus, audio levels, time schedules, or machine learning predictions.

![Version](https://img.shields.io/badge/version-1.0.0-blue)
![License](https://img.shields.io/badge/license-GPL--3.0-green)
![OBS](https://img.shields.io/badge/OBS-29.0%2B-orange)
![C++](https://img.shields.io/badge/C++-17-blue)

## Features

- **Multiple Switching Modes**
  - Window Focus Detection
  - Audio Level Triggering
  - Time-Based Scheduling
  - Manual Control
  - AI-Powered Prediction

- **Intelligent Prediction**
  - Machine learning-based scene recommendations
  - Pattern recognition from usage history
  - Time-of-day awareness
  - Confidence scoring

- **WebSocket API**
  - External control via WebSocket
  - JSON message format
  - Real-time scene switching

- **Custom Rules**
  - Create unlimited switching rules
  - Priority-based execution
  - Wildcard pattern matching
  - Enable/disable per rule

- **Smooth Transitions**
  - Configurable transition effects
  - Adjustable transition duration
  - Delay control

## Installation

### Windows

1. Download the latest release from [Releases](releases)
2. Extract `SceneDirectorAI.dll` to:
   ```
   C:\Program Files\obs-studio\obs-plugins\64bit\
   ```
3. Restart OBS Studio
4. Add SceneDirector-AI as a filter to any source

### macOS

1. Download the macOS version
2. Copy `.so` file to:
   ```
   /Applications/OBS.app/Contents/Plugins/
   ```
3. Restart OBS Studio

### Linux

1. Build from source or download `.so` file
2. Copy to:
   ```
   /usr/lib/obs-plugins/
   ```
3. Restart OBS Studio

## Configuration

### Basic Setup

1. Add SceneDirector-AI as a filter to any source
2. Select your switching mode
3. Configure rules for each scene

### Switching Modes

| Mode | Description | Best For |
|------|-------------|----------|
| Window Focus | Switches based on active window | Gaming, applications |
| Audio Level | Switches when audio exceeds threshold | Voice-activated scenes |
| Time Schedule | Switches at specific times | Scheduled shows |
| AI Prediction | Uses ML to predict next scene | Advanced automation |
| Manual | Hotkey or WebSocket control | On-demand switching |

### Rules Configuration

Each rule contains:
- **Scene**: Target scene to switch to
- **Condition**: What triggers the switch
- **Pattern/Value**: Match criteria
- **Threshold**: Value threshold (for audio)
- **Priority**: Higher priority rules execute first
- **Enabled**: Toggle rule on/off

### WebSocket API

Connect to `ws://localhost:8080` (configurable port)

**Switch Scene:**
```json
{
  "action": "switch",
  "scene": "Scene Name"
}
```

**Get Current State:**
```json
{
  "action": "getState"
}
```

## Requirements

- **OBS Studio**: 29.0 or higher
- **Operating System**: Windows 10+, macOS 12+, Linux (Ubuntu 20.04+)
- **RAM**: 50MB minimum
- **CPU**: Minimal impact

## Building from Source

### Prerequisites

- CMake 3.15+
- C++17 compiler
- OBS Studio dev files
- Qt 5

### Build Steps

```bash
git clone https://github.com/RakkoTV/SceneDirector-AI.git
cd SceneDirector-AI
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

## Troubleshooting

**Problem**: Scenes not switching
- **Solution**: Check rule priorities and ensure patterns match

**Problem**: High CPU usage
- **Solution**: Increase prediction window, disable unused modes

**Problem**: WebSocket not working
- **Solution**: Check firewall settings and port availability

## Changelog

### Version 1.0.0 (2026-05-02)
- Initial release
- Window focus detection
- Audio level monitoring
- Time-based scheduling
- ML-based prediction
- WebSocket API
- Multi-language support

## Support

- Issues: [GitHub Issues](https://github.com/RakkoTV/SceneDirector-AI/issues)
- Discussions: [GitHub Discussions](https://github.com/RakkoTV/SceneDirector-AI/discussions)

## Donate

If you find this project useful, please consider supporting the development:

[![Donate](https://img.shields.io/badge/PayPal-Donate-blue)](https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=ramiro.silva.1993%40gmail%2ecom&lc=US&item_name=Support+Open+Source+Development&currency_code=USD)

**[Donate via PayPal](https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=ramiro.silva.1993%40gmail%2ecom&lc=US&item_name=Support+Open+Source+Development&currency_code=USD)**

## Connect With Me

- **[GitHub](https://github.com/RakkoTV)** - ⭐ 3 Stars
- **[LinkedIn](https://www.linkedin.com/in/ramiro-silva/)** - 👥 449 Contacts
- **[Instagram](https://www.instagram.com/Rakko.Tech)** - 👥 6,666 Followers
- **[Twitch](https://www.twitch.tv/RakkoTech)** - 👥 8,800 Followers
- **[X/Twitter](https://www.x.com/RakkoTech)** - 👥 245 Followers

## License

This project is licensed under GPL-3.0 - see [LICENSE](LICENSE) for details.

## Acknowledgments

- OBS Studio team
- libwebsockets for WebSocket support
- Community contributors

---

Made with ❤️ by [RakkoTV](https://github.com/RakkoTV)
