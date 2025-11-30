#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include <TFT_eSPI.h>
TFT_eSPI tft = TFT_eSPI();

const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

const char* apiKey = "YOUR_API";

const char* jsonBody = R"(
{
  "query": "{ stop(id:\"HSL:1320120\") { name stoptimesWithoutPatterns(numberOfDepartures:5) { trip { route { shortName } tripHeadsign } realtimeDeparture serviceDay realtime } } }" //JSON body for request example
}
)";

void setup() {
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_YELLOW);  // Fill background with HSL blue
  tft.setTextColor(TFT_BLACK);  // Set White text 
  tft.setTextSize(5);  //FONT SIZE

  Serial.begin(115200);
//CONNECT TO WOFI
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(200);
  }
  Serial.println("\nWiFi Connected");
  // Sync ESP32 RTC with NTP
  configTime(2 * 3600, 0, "pool.ntp.org", "time.nist.gov"); // UTC+2 for Finland

  // Wait a bit to get time
  delay(2000);

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "Current time: %H:%M:%S");
}

void loop() {

  WiFiClientSecure client;
  client.setInsecure();

//HTTP POST REQUEST TO SERVER
  HTTPClient http;
  http.begin(client, "https://api.digitransit.fi/routing/v2/hsl/gtfs/v1");

  http.addHeader("Content-Type", "application/json");
  http.addHeader("digitransit-subscription-key", apiKey);

  int code = http.POST(jsonBody);

  Serial.print("\nHTTP: ");
  Serial.println(code);
//PRINT RESPONSE JSON
  if (code > 0) {
    String body = http.getString();
    Serial.println("RAW:");
    Serial.println(body);
    JsonDocument doc;
    deserializeJson(doc, body);
    

// Access stop object
JsonObject stop = doc["data"]["stop"];

// Print stop name
const char* stopName = stop["name"];
Serial.println(stopName);

// Access stoptimesWithoutPatterns array
JsonArray times = stop["stoptimesWithoutPatterns"];
//CLEAR SCREEN
tft.fillScreen(TFT_YELLOW);
tft.setCursor(0,0);
//PRINT TIMETABLE HEADERS
char buf[10];
sprintf(buf, "%3s   %4s", "BUS", "TIME"); 
tft.println(buf);
//GETTING STOP'S UPCOMING BUSSES
for (JsonObject t : times) {
    const char* shortName = t["trip"]["route"]["shortName"];
    const char* headsign = t["trip"]["tripHeadsign"];
    long serviceDay = t["serviceDay"];
    long realtimeDeparture = t["realtimeDeparture"];
    bool realtime = t["realtime"];
//CALCULATE MINUTES UNTIL DEPARTURE
    long departureEpoch = serviceDay + realtimeDeparture;
    long nowEpoch = time(nullptr);
    int minutes = (departureEpoch - nowEpoch) / 60;
    if (minutes < 0) minutes = 0;

//PRINT BUS NUMBER AND MINUTES
    char buf2[10];
    sprintf(buf2, "%4s %3sm", String(shortName), String(minutes));
    tft.println(buf2);
}
  }

  http.end();
  delay(60000);  // refresh every 60 sec
  
  ;
}

