//INITIAL SPAGHETTI CODE, REMEMBER TO DESPAGHETTIFY BEFORE FURTHER DEVELOPMENT

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>

String ssid;
String password;
String apiKey;
String calendarId;

void loadSettingsFromJson()
{
    if (!SPIFFS.begin(true))
    {
        Serial.println("Failed to mount SPIFFS");
        return;
    }

    File file = SPIFFS.open("/settings.json");
    if (!file)
    {
        Serial.println("Failed to open settings.json");
        return;
    }

    size_t size = file.size();
    std::unique_ptr<char[]> buf(new char[size]);
    file.readBytes(buf.get(), size);
    file.close();

    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, buf.get());

    if (error)
    {
        Serial.println("Failed to parse settings.json");
        return;
    }

    ssid = doc["ssid"].as<String>();
    password = doc["password"].as<String>();
    apiKey = doc["apiKey"].as<String>();
    calendarId = doc["calendarId"].as<String>();
}

void setup()
{
    Serial.begin(115200);
    loadSettingsFromJson();

    WiFi.begin(ssid.c_str(), password.c_str());
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("Connecting to Wi-Fi...");
        //TODO: maybe after like 5 successive failures, prompt user to reenter password or try different network?
        
    }
    Serial.println("Connected to Wi-Fi");
}

void loop()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        HTTPClient http;
        String url = "https://www.googleapis.com/calendar/v3/calendars/" + String(calendarId) + "/events?key=" + String(apiKey);
        http.begin(url);
        int httpResponseCode = http.GET();

        if (httpResponseCode > 0)
        {
            String payload = http.getString();
            DynamicJsonDocument doc(4096);
            deserializeJson(doc, payload);

            JsonArray items = doc["items"];

            for (JsonObject item : items)
            {
                String summary = item["summary"].as<String>();
                String location = item["location"].as<String>();
                String startTime = item["start"]["dateTime"].as<String>();
                String endTime = item["end"]["dateTime"].as<String>();

                Serial.println("Summary: " + summary);
                Serial.println("Location: " + location);
                Serial.println("Start Time: " + startTime);
                Serial.println("End Time: " + endTime);
                Serial.println("---------------------------");
            }
        }
        else
        {
            Serial.printf("Error: %d", httpResponseCode);
        }
        http.end();
    }

    //TODO: CURRENTLY FETCHES AND REFRESHES EVERY FIVE MINUTES, CHANGE AS NEEDED

    delay(300000);
}