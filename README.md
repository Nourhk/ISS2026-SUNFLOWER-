# 🌻 Smart Sunflower – Dual-Axis Hybrid Solar Tracking & IoT Monitoring System

## 📌 Overview

**Smart Sunflower** is an advanced dual-axis hybrid solar tracking system designed to maximize photovoltaic efficiency through a combination of astronomical modeling, sensor-based feedback, and IoT connectivity.

The system intelligently separates responsibilities between a **Master–Slave architecture**:
- The **azimuth axis** is controlled using an open-loop astronomical sun position algorithm.
- The **elevation axis** is adjusted using real-time feedback from Light Dependent Resistors (LDRs).

A centralized IoT dashboard enables real-time monitoring, visualization, and manual control of all system parameters.

This architecture reduces redundant computation, improves energy efficiency, and enables scalable deployment across multiple solar units.

---

## ⚙️ Key Features

- 🌞 Dual-axis solar tracking (azimuth + elevation)
- 🧠 Hybrid control system (astronomical + LDR feedback)
- 🔗 Master–Slave distributed architecture
- 📡 IoT-based real-time monitoring dashboard
- 🎛️ Manual override control via web interface
- ⚡ Reduced energy consumption through distributed computation
- 📊 Real-time PV orientation and system status visualization

---

## 🏗️ System Architecture

The system is built on a distributed hybrid control model:

### 🔵 Master Unit
- Computes optimal **azimuth angle** using solar position (astronomical model)
- Broadcasts orientation data to slave nodes
- Acts as the central decision-making unit

### 🟢 Slave Units
- Use LDR sensors to adjust **elevation angle**
- Perform local fine-tuning for maximum light intensity
- Execute movement commands received from master

### 🌐 IoT Layer
- Displays real-time system data
- Provides manual control interface
- Visualizes PV output and orientation status

---

## 📡 System Flow

1. Master calculates sun azimuth using time + location data
2. Azimuth value is broadcast to slave units
3. Slave reads LDR sensor values
4. Elevation is adjusted to maximize light intensity
5. Data is sent to IoT dashboard
6. User can override system manually if needed

---

## 🧠 Innovation Highlights

### 🔄 Master–Slave Optimization
- Eliminates redundant solar calculations across nodes
- Reduces processing load on slave devices
- Improves scalability for multi-panel installations

### 🌞 Hybrid Tracking Approach
- Combines:
  - Open-loop astronomical tracking (azimuth)
  - Closed-loop sensor feedback (elevation)

### ⚡ Energy Efficiency
- Minimizes servo movement through optimized control logic
- Reduces unnecessary computation and communication overhead

---

## 🔩 Hardware Components

- ESP32 microcontroller (Master + Slave nodes)
- 4x LDR sensors (light intensity detection)
- Servo motors (dual-axis movement)
- Solar panel (PV module)
- Power supply system
- WiFi connectivity module (ESP32 integrated)

---

## 💻 Software Stack

- Embedded C++ (Arduino / ESP32 framework)
- HTML / CSS / JavaScript (IoT dashboard)
- Firebase / MQTT (real-time data communication)
- Solar position algorithm (astronomical calculations)
- Servo control library

---

## 🌐 IoT Dashboard Features

The dashboard provides:

- 📊 Real-time PV output monitoring
- 🌞 Sun position visualization
- 🎛️ Manual servo control (azimuth & elevation)
- 📡 Live LDR sensor readings
- 🔁 System mode switching (Auto / Manual)

> Screenshots and demo videos can be added in the `/docs` folder.

---

## 📁 Project Structure
TBA

---

## 📊 Performance Considerations

- Improved tracking accuracy compared to fixed panels
- Reduced energy waste due to optimized servo movements
- Scalable architecture for multi-panel solar farms
- Low-latency IoT communication system

---

## 🧪 Testing & Validation

The system was tested under real outdoor conditions in Tunisia with varying sunlight intensity.

Test scenarios included:
- Clear sky conditions ☀️
- Partial cloud coverage ☁️
- Low-light calibration 🌥️
- Servo response stability tests ⚙️

---

## 🚀 Future Improvements

- AI-based predictive sun tracking using weather data
- Machine learning optimization of LDR calibration
- Mesh network expansion for large solar farms
- Battery storage integration monitoring
- Mobile application for remote control

---

## 📷 Media

> Add images in `/docs`:

- System setup
- Servo movement demo
- Dashboard interface
- Field deployment

---

## 👨‍💻 Contributors

- **Nour Hajkacem** – Project Lead / Embedded Systems / IoT Integration
- Robotics Team Tunisia 🇹🇳

---

## 📜 License

This project is open-source and intended for educational and research purposes.

---

## 🌍 Impact

Smart Sunflower aims to contribute to:
- Renewable energy optimization
- Smart agriculture systems
- Distributed IoT energy monitoring
- Scalable solar farm automation

---

⭐ If you find this project useful, consider giving it a star!
