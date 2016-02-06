# SmingNode

SmingNode is the smart home automation brick which combine scripting languages flexibility with Sming Framework powerful. Joined into network, SmingNode sensors can provide unified and complex smart automation solution which will work even without internet connection.

### Smart home automation brick
All sensors and automation algorithms described in universal JSON configuration. SmingNode collects data and work with slaves devices as described in that internal config, which provides a very easy and fast maintenance.

All sensor values aggregated and accessible realtime by MQTT and HTTP protocols.

### Configuration
[Read full config specification](https://github.com/SmingHub/SmingNode/wiki/config-specification)

Minimal configuration starrts from single node (and optional MQTT broker to collect sensors data). Below is the example of bathroom automation config:
```JSON
{
	"name": "esp_bathroom",
	"network":
	{
		"ssid": "MyWiFi",
		"password": "secretkey"
	},
	"mqtt":
	{
		"host": "test.mosquitto.org",
		"port": 1883
	},
	"sensors":
	{
		"dht22":
		{
			"pins": 2,
			"output": {"temperature": "t", "humidity": "rh"}
		},
		"light":
		{
			"type": "analog",
			"interval": 3000,
			"output": "intensity"
		},
		"forceMode":
		{
			"type": "button",
			"pins": 4,
			"changed": "fanAutoControl.setEnabled(false); fan.write(!fan.read()); restoreAutoMode.startOnce();"
		}
	},
	"slaves":
	{
		"fan" : 0
	},
	"triggers":
	{
		"fanAutoControl": "fan.write(dht22.rh > 70 & light.intensity < 12);"
	},
	"timers":
	{
		"restoreAutoMode":
		{
			"interval": 600000,
			"enabled": false,
			"action": "fanAutoControl.setEnabled(true);"
		}
	}
}
```
Wiring for inputs:
* pin 2 -> dht22
* pin 4 -> button for manual fan switch
* pin A0 (TOUT/ADC) -> photodiode for light measurement

Wiring for outputs:
* pin 0 -> relay for fan control

(Don't forget connect sensors power/ground lines of course :)

This configuration collect temperature, hummidity, light intensity, and send it to MQTT broker. It will turn fun ON if it's dark and wet in the bathroom, but you can start or stop fan manually by switching hard mode button (timer will switch system back to auto mode after 10 minutes).

More details about configuration here: [config specification](https://github.com/SmingHub/SmingNode/wiki/config-specification)

SmingNode script engine based on https://github.com/gfwilliams/tiny-js with small adaptations. It's very simple but really powerfull and extensible because of Sming Framework backend.

### How to build
To build project please update to current Sming **development branch**.

https://github.com/SmingHub/Sming/tree/develop

Use Espressif SDK v1.5

### TODO
* RBoot OTA updates
* Web interface for config editor
* WebSocket realtime sensors data streaming (like current MQTT implementation)
* Friendly Web UI
