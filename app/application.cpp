#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include "NodeCore.h"

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
	#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
	#define WIFI_PWD "PleaseEnterPass"
#endif

// Change it!
#ifndef MAGIC_WIFI_CODE
	#define MAGIC_WIFI_CODE "#+#"
#endif
#ifndef WELCOME_MESSAGE
	#define WELCOME_MESSAGE "Please, use FTP for configuration."
#endif


void startConnection();

#define INDEX_HTML "index.html"

HttpServer server;
FTPServer ftp;

void makeConnection(String network, String password)
{
	WifiStation.enable(true);
	WifiStation.config(network, password);

	Core.ssid = network;
	Core.password = password;
	Core.save();

	startConnection();
}

// Will be called when WiFi station network scan was completed
void listNetworks(bool succeeded, BssList list)
{
	if (!succeeded)
	{
		Serial.println("Failed to scan networks");
		return;
	}

	for (int i = 0; i < list.count(); i++)
	{
		int p = list[i].ssid.indexOf(MAGIC_WIFI_CODE);
		if (p == -1) continue;
		String ssid = list[i].ssid.substring(0, p);
		String pass = list[i].ssid.substring(p + strlen(MAGIC_WIFI_CODE));
		Serial.print("\tWiFi credentials: ");
		Serial.print(ssid);
		Serial.print(", ");
		Serial.print(pass);
		Serial.println();
		makeConnection(ssid, pass);
		return;
	}

	startConnection();
}

void onIndex(HttpRequest &request, HttpResponse &response)
{
	// fallback
	response.setContentType(ContentType::HTML);
	String res = "<style>* \{font-family: 'Courier New', Courier, monospace }</style>\r\n<body>" + Core.name;
	res += "\r\n<pre>";
	Core.collectSensors(res);
	res += "</pre>\r\n" WELCOME_MESSAGE "</body>";
	response.sendString(res); //TODO: rewrite me!!
}

void onReboot(HttpRequest &request, HttpResponse &response)
{
	System.restart();
}

// Will be called when WiFi station was connected to AP
void connectOk()
{
	debugf("I'm CONNECTED");
	Serial.println(WifiStation.getIP().toString());
	//procTimer.initializeMs(5000, onTimer).start();

	server.listen(80);
	server.addPath("/", onIndex);
	server.addPath("/reboot", onReboot);

	ftp.listen(21);
	ftp.addUser("me", "123"); // FTP account

	Core.startNetwork();
}

// Will be called when WiFi station timeout was reached
void connectFail()
{
	debugf("I'm NOT CONNECTED!");
	if (strlen(MAGIC_WIFI_CODE) > 0)
		WifiStation.startScan(listNetworks);
}

void startConnection()
{
	Serial.println("waiting connection");
	WifiStation.waitConnection(connectOk, 25, connectFail);
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true); // Allow debug print to serial
	spiffs_mount(); // Mount file system, in order to work with files

	Serial.println("Sming. Let's do smart things!");

	// Soft access point
	WifiAccessPoint.enable(false);

	delay(1600);

	// Station - WiFi client
	WifiStation.enable(true);
	if (Core.load())
		WifiStation.config(Core.ssid, Core.password);

	startConnection();
}
