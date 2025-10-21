// ESP32 WiFi Recon — reliable scanner with diagnostics
#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

WebServer server(80);
unsigned long lastScan = 0;
const unsigned long SCAN_INTERVAL = 5000; // ms

struct APinfo {
  String ssid;
  String bssid;
  int32_t rssi;
  uint8_t channel;
  String auth;
  uint32_t lastSeen;
  uint32_t seenCount;
};

std::vector<APinfo> aps;

// helper
String authModeToString(wifi_auth_mode_t m) {
  switch(m) {
    case WIFI_AUTH_OPEN: return "OPEN";
    case WIFI_AUTH_WEP: return "WEP";
    case WIFI_AUTH_WPA_PSK: return "WPA";
    case WIFI_AUTH_WPA2_PSK: return "WPA2";
    case WIFI_AUTH_WPA_WPA2_PSK: return "WPA/WPA2";
    case WIFI_AUTH_WPA2_ENTERPRISE: return "WPA2-E";
    default: return "UNKNOWN";
  }
}

// Synchronous, blocking scan and update — more reliable for small projects
void scanAndUpdate() {
  Serial.println("Starting synchronous scan...");
  int n = WiFi.scanNetworks(false, true); // synchronous, show_hidden = true
  Serial.printf("Scan finished: %d networks\n", n);
  unsigned long now = millis();

  // update aps vector
  for (int i = 0; i < n; ++i) {
    String s = WiFi.SSID(i);
    String b = WiFi.BSSIDstr(i);
    int r = WiFi.RSSI(i);
    uint8_t ch = WiFi.channel(i);
    String a = authModeToString(WiFi.encryptionType(i));
    bool found = false;
    for (auto &ap : aps) {
      if (ap.bssid == b) {
        ap.ssid = s; ap.rssi = r; ap.channel = ch; ap.auth = a;
        ap.lastSeen = now;
        ap.seenCount++;
        found = true; break;
      }
    }
    if (!found) {
      APinfo newap;
      newap.ssid = s;
      newap.bssid = b;
      newap.rssi = r;
      newap.channel = ch;
      newap.auth = a;
      newap.lastSeen = now;
      newap.seenCount = 1;
      aps.push_back(newap);
    }
    Serial.printf("%d: SSID='%s' BSSID=%s RSSI=%d ch=%d auth=%s\n", i, s.c_str(), b.c_str(), r, ch, a.c_str());
  }

  if (n == 0) Serial.println("No APs found in this scan.");
  WiFi.scanDelete(); // free memory
}

String networksToJson() {
  String out = "[";
  for (size_t i = 0; i < aps.size(); ++i) {
    APinfo &ap = aps[i];
    out += "{";
    out += "\"ssid\":\"" + ap.ssid + "\",";
    out += "\"bssid\":\"" + ap.bssid + "\",";
    out += "\"rssi\":" + String(ap.rssi) + ",";
    out += "\"channel\":" + String(ap.channel) + ",";
    out += "\"auth\":\"" + ap.auth + "\",";
    out += "\"seen\":" + String(ap.seenCount) + ",";
    out += "\"lastseen\":" + String(ap.lastSeen);
    out += "}";
    if (i + 1 < aps.size()) out += ",";
  }
  out += "]";
  return out;
}

void handleRoot() {
  String page = R"rawliteral(
<!doctype html><html><head><meta charset="utf-8"><title>ESP32 WiFi Recon</title>
<style>body{font-family:Arial;background:#0b0f14;color:#e6eef3;padding:20px}table{border-collapse:collapse;width:100%}th,td{padding:8px;border-bottom:1px solid #263238;text-align:left}th{background:#123;position:sticky;top:0}</style>
</head><body>
<h1>ESP32 WiFi Recon</h1>
<p>Live passive scan of nearby APs. For authorized testing only.</p>
<table id="tbl"><thead><tr><th>SSID</th><th>BSSID</th><th>RSSI (dBm)</th><th>Channel</th><th>Auth</th><th>Seen</th></tr></thead><tbody></tbody></table>
<script>
async function fetchList(){
  try{
    let r = await fetch('/networks.json');
    let j = await r.json();
    let tbody = document.querySelector('#tbl tbody');
    tbody.innerHTML = '';
    j.sort((a,b)=>b.rssi - a.rssi);
    j.forEach(item=>{
      let tr = document.createElement('tr');
      tr.innerHTML = `<td>${item.ssid}</td><td>${item.bssid}</td><td>${item.rssi}</td><td>${item.channel}</td><td>${item.auth}</td><td>${item.seen}</td>`;
      tbody.appendChild(tr);
    });
  }catch(e){ console.error(e); }
}
setInterval(fetchList, 3000);
fetchList();
</script>
</body></html>
  )rawliteral";
  server.send(200, "text/html", page);
}

void handleJson() {
  String json = networksToJson();
  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);
  delay(500);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.printf("Connecting to %s\n", ssid);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("WiFi connected, IP: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/networks.json", HTTP_GET, handleJson);
  server.begin();

  // Force an immediate scan at startup
  scanAndUpdate();
  lastScan = millis();
}

void loop() {
  server.handleClient();
  unsigned long now = millis();
  if (now - lastScan >= SCAN_INTERVAL) {
    lastScan = now;
    scanAndUpdate();
  }
}
