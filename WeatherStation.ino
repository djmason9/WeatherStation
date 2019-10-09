#include "CustomDisplayFonts.h"
#include "WeatherGraphics.h"
#include "MoonPhase.h"
#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "SSD1306.h"
#include "ArduinoSort.h"
#include "time.h"
#include "math.h"


//NOTE FOR A SCREEN WITH A YELLOW TOP AND BLUE BOTTOM CHECK true
bool isYellow = true;


char buf[130];
char buf2[130];

const char *ssid = "[YOUR SSID NETWORK NAME]";
const char *password = "[YOUR NETWORK PASSWORD]";

int updateWeather = 0;
int countDownSeconds = 9000;
String fullWeatherString;
String currentMoonPhase;
int graphicSize = 49;
const int btnPin = 23;
int previousButtonState = LOW;
bool displayWeather = true;
int TOTAL_SCREENS = 3;
enum Screens
{
    DAILYWEATHER = 1,
    MOONPHASE = 2,
    THREEDAY = 3
};
int screenId = DAILYWEATHER;

enum WeatherStates
{
    PARTCLOUDYDAY = 0,
    CLEAR = 1,
    RAIN = 2,
    SNOW = 3,
    PARTCLOUDYNIGHT = 4,
    SLEET = 5,
    WIND = 6,
    CLEARNIGHT = 7,
    CLOUDY = 8,
    THUNDERSTORM = 9,
    CLEAR_SM = 10,
    RAIN_SM = 11,
    SNOW_SM = 12,
    PARTCLOUDYDAY_SM = 13,
    SLEET_SM = 14,
    WIND_SM = 15,
    CLOUDY_SM = 16,
    THUNDERSTORM_SM = 17,
    PARTCLOUDYNIGHT_SM = 18,
    CLEARNIGHT_SM = 19,

};

//A map to hold icon text
String jsonMap = "{\"partly-cloudy-night-sm\":18,\"clear-day-sm\":10,\"rain-sm\":11,\"snow-sm\":12,\"partly-cloudy-day-sm\":13,\"sleet-sm\":14,\"wind-sm\":15,\"cloudy-sm\":16,\"thunderstorm-sm\":17,\"partly-cloudy-day\":0,\"clear-day\":1,\"clear-night\":7,\"rain\":2,\"snow\":3,\"partly-cloudy-night\":4,\"sleet\":5,\"wind\":6,\"cloudy\":8,\"thunderstorm\":9,\"clear-night-sm\":19}";

SSD1306 display(0x3c, 21, 22);

void setup()
{

    Serial.begin(115200);
    pinMode(btnPin, INPUT);

    display.init();
    display.flipScreenVertically();

    drawLogo(0, 0);
    delay(500);

    connectTheWifi();
    // threeDayforcast();
}

void loop()
{
    int buttonState = digitalRead(btnPin);
    // Serial.println(screenId);
    if (buttonState == HIGH) //(buttonState != previousButtonState))
    {
        screenId++;

        displayWeather = false;
        if (screenId == MOONPHASE)
        {
            display.clear();
            drawMoonPhase();
        }
        else if (screenId == THREEDAY)
        {
            threeDayforcast();
        }
        else if (screenId = DAILYWEATHER)
        {
            display.clear();
            drawCurrentWeatherData();
            screenId = DAILYWEATHER;
        }

        delay(500);
    }

    delay(100);

    updateWeather++;
    // call every 9 mins
    if (updateWeather % countDownSeconds == 0)
    {
      Serial.println("callWeatherApi");
        callWeatherApi(true);
    }
}
/**
 * Gets on the wifi
 */
void connectTheWifi()
{
    
    screenStatusWrite(0, 0, "Setting Up", true);
    delay(1000);
    if(isYellow){
      screenStatusWrite(0, 0, "Yellow Screen Enabled!", true);
    }
    delay(1000);
    screenStatusWrite(0, 0, "Attempting Connection...", true);
    delay(1000);
    WiFi.begin(ssid, password);
    strcat(buf, "Connecting to\n");
    strcat(buf, ssid);
    while (WiFi.status() != WL_CONNECTED)
    { //Check for the connection
        delay(2000);
        screenStatusWrite(0, 0, buf, true);
    }


    String localIPstr="";
    localIPstr.concat(String(WiFi.localIP()[0]));
    localIPstr.concat(".");
    localIPstr.concat(String(WiFi.localIP()[1]));
    localIPstr.concat(".");
    localIPstr.concat(String(WiFi.localIP()[2]));
    localIPstr.concat(".");
    localIPstr.concat(String(WiFi.localIP()[3]));

    char bufIP[15];
    unsigned int len = localIPstr.length();

    localIPstr.toCharArray(bufIP, len);
    strcat(buf2, "Now on network:\n");
    strcat(buf2, ssid);
    strcat(buf2, "\n");
    strcat(buf2, bufIP);


    screenStatusWrite(0, 0, buf2, true);

    delay(2000);

    //show all weather
    // screenTest();

    callWeatherApi(false);
}

void addScreenDots(int currScreen, int count)
{
    currScreen--;
    int x = 64, gap = 8, origGap = gap, dotWidth = 1, evenOffset = 0;
    float len = (count * 0.5);
    int dotArray[count];
    int sortCount = 0;

    if (count > 1) //more than one
    {
        if (count % 2 != 0) //odd draw center dot
        {
            display.drawCircle(x, 60, dotWidth); //first in center
            dotArray[sortCount] = x;
            sortCount++;
        }
        else
        {
            evenOffset = gap * 0.5;
        }

        for (int i = 0; i < len; i++)
        {
            if (count % 2 != 0 && i == 0) //odd
            {
                i = 1;
            }

            dotArray[sortCount] = (x + gap - evenOffset);
            sortCount++;
            dotArray[sortCount] = (x - gap + evenOffset);
            sortCount++;

            display.drawCircle((x + gap - evenOffset), 60, dotWidth);
            display.drawCircle((x - gap + evenOffset), 60, dotWidth);

            gap += origGap;
            display.display();
        }

        // /***** DOT ******/
        sortArray(dotArray, count);
        display.fillCircle(dotArray[currScreen], 60, dotWidth + 1);
        display.display();
    }
}

void screenTest()
{
    display.setFont(SansSerif_plain_11);
    String blank = String("");
    for (int i = 0; i < 10; i++)
    {
        display.clear();
        screenStatusWrite(0, 0, "Weather Station Setup", false);
        drawWeatherIcon(39, 15, blank, i);
        delay(300);
    }
    float phase = 0;
    for (int i = 0; i < 8; i++)
    {
        display.clear();
        screenStatusWrite(15, 0, "Moon Phase Setup", false);
        moonPhase(39, 15, phase, false);
        delay(200);
        phase = phase + 0.125;
        display.setFont(SansSerif_plain_11);
    }
}

void callWeatherApi(bool onlyDaily)
{

    char respbuf[130];
    if (WiFi.status() == WL_CONNECTED)
    { //Check WiFi connection status
        screenStatusWrite(7, 0, "Calling Weather API", true);
        HTTPClient http;

        String api = "https://api.darksky.net/forecast/[YOUR SECRET KEY]/33.31,-111.8614?exclude=minutely,hourly,alerts,flags";
        http.begin(api); //Specify destination for HTTP request

        http.addHeader("Content-Type", "application/json"); //Specify content-type header

        int httpResponseCode = http.GET(); //Send the actual POST request

        if (httpResponseCode > 0)
        {
            fullWeatherString = http.getString();

            DynamicJsonBuffer jsonBuffer;
            JsonObject &root = jsonBuffer.parseObject(fullWeatherString);

            String daily = root["daily"];
            JsonObject &dailyRoot = jsonBuffer.parseObject(daily);
            JsonArray &dataArray = dailyRoot["data"];
            String firstDay = dataArray[0];
            JsonObject &firstDayRoot = jsonBuffer.parseObject(firstDay);
            String mp = firstDayRoot["moonPhase"];
            currentMoonPhase = mp;

            drawCurrentWeatherData();
        }
        else
        {
            screenStatusWrite(0, 0, "Error on sending GET: ", true);
            delay(2000);
            screenStatusWrite(0, 0, "Trying again in \n4 seconds", true);
            delay(4000);
            callWeatherApi(false); //API fail call again
        }

        http.end(); //Free resources
    }
}

void drawMoonPhase()
{
    graphicSize = 49;
    display.setFont(SansSerif_plain_11);
    screenStatusWrite(55, 0, "Moon Phase", true);
    float floatMoon = currentMoonPhase.toFloat();
    moonPhase(0, isYellow?15:7, floatMoon, true);
    addScreenDots(MOONPHASE, TOTAL_SCREENS);
    // delay(4000);
}

void drawCurrentWeatherData()
{
    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.parseObject(fullWeatherString);

    String currently = root["currently"];
    JsonObject &currentlyRoot = jsonBuffer.parseObject(currently);

    //start showing weather stats
    String temp = currentlyRoot["temperature"];
    int tempInt = round(temp.toFloat());
    String formatedTemp = String(tempInt) + "° F";
    String icon = currentlyRoot["icon"];
    String humidity = currentlyRoot["humidity"];
    float humidityPercent = humidity.toFloat();
    int humdityWholePercent = humidityPercent * 100;

    String timeVar = currentlyRoot["time"];
    time_t newTimeVar = timeVar.toInt();

    char buf[20];
    struct tm *ts;
    ts = localtime(&newTimeVar);
    strftime(buf, sizeof(buf), "%a %m/%d", ts); //full date
    Serial.print(buf);

    display.setFont(SansSerif_plain_14);
    screenStatusWrite(50, 0, buf, true);
    display.setFont(SansSerif_plain_24);
    screenStatusWrite(50, 17, formatedTemp, false);
    display.setFont(SansSerif_plain_11);
    screenStatusWrite(50, 43, "Humidity:" + String(humdityWholePercent) + "%", false);

    graphicSize = 49;
    drawWeatherIcon(0, isYellow?18:0, icon, -1);
    addScreenDots(DAILYWEATHER, TOTAL_SCREENS);
}

void threeDayforcast()
{
    display.clear();
    graphicSize = 24;
    int highsX = 45, lowsX = 35;
    int one = 14, two = 54, three = 91;
    int iconY = 12;
    String blank = String("");

    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.parseObject(fullWeatherString);

    String daily = root["daily"];
    JsonObject &dailyRoot = jsonBuffer.parseObject(daily);
    JsonArray &dataArray = dailyRoot["data"];
    display.setFont(SansSerif_plain_11);
    screenStatusWrite(0, lowsX, "L", false);
    display.setFont(SansSerif_plain_12);
    screenStatusWrite(0, highsX, "H", false);

    for (int i = 1; i < 4; i++)
    {
        String dayObj = dataArray[i];
        JsonObject &firstDayRoot = jsonBuffer.parseObject(dayObj);
        String high = firstDayRoot["temperatureHigh"];
        int highInt = round(high.toFloat());
        String highFormatted = String(highInt);
        String low = firstDayRoot["temperatureLow"];
        int lowInt = round(low.toFloat());
        String lowFormatted = String(lowInt);
        String icon = firstDayRoot["icon"];

        String timeVar = firstDayRoot["time"];
        time_t newTimeVar = timeVar.toInt();

        char buf[20];
        struct tm *ts;
        ts = localtime(&newTimeVar);
        // strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts); //full date
        strftime(buf, sizeof(buf), "%a", ts);
        Serial.print(buf);

        display.setTextAlignment(TEXT_ALIGN_CENTER);
        screenStatusWrite(one + 10, 0, buf, false);
        display.setTextAlignment(TEXT_ALIGN_LEFT);

        //highs
        display.setFont(SansSerif_plain_11);
        screenStatusWrite(one, lowsX, lowFormatted + "°", false);

        //highs
        display.setFont(SansSerif_plain_12);
        screenStatusWrite(one, highsX, highFormatted + "°", false);

        drawWeatherIcon(one, iconY, icon + "-sm", -1);

        one = one + 14 + 24;
    }
    addScreenDots(THREEDAY, TOTAL_SCREENS);
}

void screenStatusWrite(int left, int top, String str, bool doClear)
{
    if (doClear)
    {
        display.clear();
    }

    display.drawString(left, top, str);
    display.display();
}

void screenStatusWrite(int left, int top, const char *str, bool doClear)
{

    if (doClear)
    {
        display.clear();
    }

    display.drawString(left, top, str);
    display.display();
}

void drawWeatherIcon(int x, int y, String &iconNm, int iconCase)
{

    if (iconCase < 0)
    {
        DynamicJsonBuffer jsonBuffer;
        JsonObject &mapRoot = jsonBuffer.parseObject(jsonMap);
        iconCase = mapRoot[iconNm];
    }

    switch (iconCase)
    {
    case CLEAR_SM:
        display.drawXbm(x, y, graphicSize, graphicSize, clearDay_sm);
        break;
    case PARTCLOUDYDAY_SM:
        display.drawXbm(x, y, graphicSize, graphicSize, partlyCloudyDay_sm);
        break;
    case RAIN_SM:
        display.drawXbm(x, y, graphicSize, graphicSize, rain_sm);
        break;
    case SNOW_SM:
        display.drawXbm(x, y, graphicSize, graphicSize, snow_sm);
        break;
    case SLEET_SM:
        display.drawXbm(x, y, graphicSize, graphicSize, sleet_sm);
        break;
    case WIND_SM:
        display.drawXbm(x, y, graphicSize, graphicSize, wind_sm);
        break;
    case CLOUDY_SM:
        display.drawXbm(x, y, graphicSize, graphicSize, cloudy_sm);
        break;
    case CLEARNIGHT_SM:
        display.drawXbm(x, y, graphicSize, graphicSize, clearNight_sm);
        break;
    case THUNDERSTORM_SM:
        display.drawXbm(x, y, graphicSize, graphicSize, thunderstorm_sm);
        break;
    case PARTCLOUDYNIGHT_SM:
        display.drawXbm(x, y, graphicSize, graphicSize, partlyCloudyNight_sm);
        break;
    case CLEAR:
        display.drawXbm(x, y, graphicSize, graphicSize, clearDay);
        break;
    case CLEARNIGHT:
        display.drawXbm(x, y, graphicSize, graphicSize, clearNight);
        break;    
    case PARTCLOUDYDAY:
        display.drawXbm(x, y, graphicSize, graphicSize, partlyCloudyDay);
        break;
    case PARTCLOUDYNIGHT:
        display.drawXbm(x, y, graphicSize, graphicSize, partlyCloudyNight);
        break;
    case RAIN:
        display.drawXbm(x, y, graphicSize, graphicSize, rain);
        break;
    case SNOW:
        display.drawXbm(x, y, graphicSize, graphicSize, snow);
        break;
    case SLEET:
        display.drawXbm(x, y, graphicSize, graphicSize, sleet);
        break;
    case WIND:
        display.drawXbm(x, y, graphicSize, graphicSize, wind);
        break;
    case CLOUDY:
        display.drawXbm(x, y, graphicSize, graphicSize, cloudy);
        break;
    case THUNDERSTORM:
        display.drawXbm(x, y, graphicSize, graphicSize, thunderstorm);
        break;
    default:
        display.drawXbm(x, y, graphicSize, graphicSize, partlyCloudyDay);
    }

    display.display();
}

void drawLogo(int x, int y)
{
    display.drawXbm(0, y, 123, 64, logo);
    display.display();
}

void moonPhase(int x, int y, float moon, bool showText)
{
    display.setFont(SansSerif_plain_19);
    int titleX = 55, titleY = 13;
    if (moon == 0)
    {
        display.drawXbm(x, y, graphicSize, graphicSize, newMoon);
        if (showText)
        {
            screenStatusWrite(titleX, titleY, "New", false);
            screenStatusWrite(titleX, titleY + 20, "Moon", false);
        }
    }
    else if (moon >= 0 && moon < 0.25)
    {
        display.drawXbm(x, y, graphicSize, graphicSize, waxingCresent);
        if (showText)
        {
            screenStatusWrite(titleX, titleY, "Waxing", false);
            screenStatusWrite(titleX, titleY + 20, "Cresent", false);
        }
    }
    else if (moon == 0.25)
    {
        display.drawXbm(x, y, graphicSize, graphicSize, firstQuarter);
        if (showText)
        {
            screenStatusWrite(titleX, titleY, "First", false);
            screenStatusWrite(titleX, titleY + 20, "Quarter", false);
        }
    }
    else if (moon >= 0.25 && moon < 0.5)
    {
        display.drawXbm(x, y, graphicSize, graphicSize, waxingGibbous);
        if (showText)
        {
            screenStatusWrite(titleX, titleY, "Waxing", false);
            screenStatusWrite(titleX, titleY + 20, "Gibbous", false);
        }
    }
    else if (moon == 0.5)
    {
        display.drawXbm(x, y, graphicSize, graphicSize, fullMoon);
        if (showText)
        {
            screenStatusWrite(titleX, titleY, "Full", false);
            screenStatusWrite(titleX, titleY + 20, "Moon", false);
        }
    }
    else if (moon >= 0.5 && moon < 0.75)
    {
        display.drawXbm(x, y, graphicSize, graphicSize, waningGibbous);
        if (showText)
        {
            screenStatusWrite(titleX, titleY, "Waning", false);
            screenStatusWrite(titleX, titleY + 20, "Gibbous", false);
        }
    }
    else if (moon == 0.75)
    {
        display.drawXbm(x, y, graphicSize, graphicSize, thirdQuarter);
        if (showText)
        {
            screenStatusWrite(titleX, titleY, "Third", false);
            screenStatusWrite(titleX, titleY + 20, "Quarter", false);
        }
    }
    else if (moon >= 0.75)
    {
        display.drawXbm(x, y, graphicSize, graphicSize, waningCresent);
        if (showText)
        {
            screenStatusWrite(titleX, titleY, "Waning", false);
            screenStatusWrite(titleX, titleY + 20, "Cresent", false);
        }
    }

    display.display();
}
