## 🛰️ ESP32 WiFi Recon — Reliable WiFi Scanner with Web UI

A lightweight, real-time WiFi network scanner for the ESP32.
It passively scans for nearby access points, logs signal strength and channel info, and serves a live-updating dashboard over HTTP.
Perfect for network diagnostics, signal testing, or educational WiFi analysis — for authorized use only.

## 🚀 Features

🔍 Reliable synchronous scanning (no async crashes or freezes)

🌐 Built-in web server showing a live table of nearby WiFi networks

📶 Displays SSID, BSSID, RSSI, Channel, Auth type, and detection count

⏱️ Automatic rescan every 5 seconds (configurable)

💾 Keeps historical data (seen count + last seen timestamp)

🧠 Minimal dependencies — just WiFi.h and WebServer.h

## 🧩 Requirements

Hardware: ESP32 board (e.g., ESP32-DevKitC, NodeMCU-32S, etc.)

Software:

Arduino IDE


## ⚙️ Setup & Usage

🔧 1. Clone the repo

https://github.com/szov/esp32-wifi-scanner
cd esp32-wifi-scanner

🧱 2. Open and configure

Open esp32-wifi-scanner (Arduino IDE).

Edit your WiFi credentials near the top:
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

🔌 3. Upload to ESP32

Connect your board via USB and upload the code.

🌍 4. Access the dashboard

Open the Serial Monitor at 115200 baud to see the ESP32’s IP address:
WiFi connected, IP: 192.168.1.45
Then open that IP in a browser:
http://192.168.1.45/
You’ll see a live-updating table of all nearby WiFi networks.

## Written by SZOV
