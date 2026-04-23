/*
    Example file for generating Secrets.h file.
*/

/*
    WIFI SSID + Password
*/
#define WIFI_SSID "mynetwork"
#define WIFI_PASSWORD "mypassword"

/*
    Thingspeak API
*/
#define THINGSPEAK_API_KEY "mykey"
#define THINGSPEAK_CHANNEL_ID mychannelid

/*
    REST api config
*/
#define REST_API_SERVER "http://0.0.0.0" // Remember 'http://' or it wont work
#define REST_API_PORT 8080

/*
    Webserver mode
*/
#define WEBSERVER_MODE "REST_API" // Or "THINGSPEAK"