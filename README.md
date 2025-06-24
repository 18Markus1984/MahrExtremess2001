# Mahr Extramess 2001 Serial Logger (ESP32-C3)

A low-cost ESP32-based interface to communicate with the proprietary **Mahr Extramess 2001** measurement device – bypassing the expensive original cable and software limitations.

---

## 🔧 What It Does

This project uses a cheap [ESP32-C3 Super Mini](https://de.aliexpress.com/item/1005005967641936.html) and a standard [M8 4-pin cable](https://www.amazon.de/dp/B0CNXGKMMK) to create a custom serial interface for the Mahr Extramess 2001, achieving an effective sampling rate of **~14.5 Hz** instead of the **1 Hz limit** imposed by the official software.

---

## 💡 Why?

The original Mahr cable costs **€111** – mostly due to an integrated FTDI controller. The official software only allows **1 measurement per second**, which is impractical for most real-world applications. We reverse-engineered the serial protocol and built our own logger to extract, parse, and store measurement data efficiently and affordably.

---

## 🧰 Hardware

- **[ESP32-C3 Super Mini](https://de.aliexpress.com/item/1005005967641936.html)**  
  A compact and affordable microcontroller supporting UART + USB.
  
- **[M8 4-pin cable](https://www.amazon.de/dp/B0CNXGKMMK)**  
  Connects the ESP32 to the Mahr Extramess 2001.

- **3D-Printed Enclosure (optional)**  
  STL files and Fusion360 model included in the repo.

- **Mahr Extramess 2001**  
  Proprietary digital dial gauge with UART interface.

---

## 📡 Protocol Reverse Engineering

We sniffed the communication between the official MahrConnect software and the Extramess 2001 device. The serial configuration is:

- **Baud rate**: 4800  
- **Data bits**: 7  
- **Parity**: Even  
- **Stop bits**: 2  

### Supported ASCII Commands:

| Command   | Description                          |
|-----------|--------------------------------------|
| `RES1\r`  | Set measurement range to preset 1    |
| `RES2\r`  | Set range to preset 2                |
| `RES3\r`  | Set range to preset 3                |
| `TOL?\r`  | Request tolerance settings           |
| `SET?\r`  | Query current status                 |
| `?\r`     | Request current measurement value    |
| `RST\r`   | Reset device and deactivate ABS      |
| `BAT?\r`  | Check battery status                 |
| `MAX\r`   | Display maximum measured value       |
| `MIN\r`   | Display minimum measured value       |
| `OFF\r`   | Power off                            |
| `ABS\r`   | Enable absolute measurement mode     |

---

## 🧠 How It Works

The ESP32-C3 continuously polls the device every **5 ms** and:

1. Sends the measurement request: `?\r`
2. Waits for a carriage-return (`\r`) terminated response
3. Parses the numeric measurement value (in mm)
4. Outputs the value with a timestamp in **CSV** format via USB

> 📋 Example Output (CSV-style over USB):  
> `2,531;4,879`

Use **CoolTerm** or any serial terminal to log the data as `.txt`, and import into Excel or other tools for analysis.

---

## 🔩 Serial Setup

```cpp
mySerial.begin(4800, SERIAL_7E2, 21, 20); // RX = GPIO21, TX = GPIO20
```
## ⚙️ Performance

- **Targeted polling rate**: 5 ms (200 Hz)  
- **Realistic effective data rate**: ~14.5 Hz  
- **Serial format limitations**: 7E2 configuration + protocol overhead limits data throughput to approx. 40–50 values/sec  
- **Button responsiveness issue**: Below 10 ms, the device’s physical buttons become unresponsive — likely due to high polling frequency  
- **Final configuration**: 5 ms polling interval ensures high data rate **without disrupting** device usability

---

## 📦 Files Included

- `src/`: Arduino code for ESP32-C3  
- `hardware/`: STL + Fusion360 files for 3D printed enclosure  
- `docs/`: Protocol documentation and optional wiring diagram  

---

## 🧪 To Do

- [ ] Add live plotting via WebSerial / Python  
- [ ] Implement command-line batch logger  
- [ ] Add optional OLED display support for portable measurements  

---

## 📸 Preview

> _(Add wiring diagram, case photo, and screenshot of serial output here if available)_

---

## 📎 Links

- 🔗 **ESP32-C3 Super Mini** → [AliExpress](https://de.aliexpress.com/item/1005005967641936.html)  
- 🔗 **M8 4-Pin Cable** → [Amazon](https://www.amazon.de/dp/B0CNXGKMMK)

---

## 📜 License

This project is licensed under the **MIT License** – use it, modify it, share it. Contributions welcome!

---

## 🤝 Contributing

Pull requests and ideas are welcome! Especially for:

- 🔍 Better parsing and formatting  
- 📊 Real-time plotting/dashboard tools  
- 🧩 Support for other Mahr models or protocols

