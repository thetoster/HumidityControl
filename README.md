# Summary.
Simple project for controlling humidity in my toilet :)
You need Platform IO to compile and be happy :)

## Setup.
After uploading code to Esp12E system will run in configuration mode. You will see info on LCD with ESP adderss and password needed to access it. All will run in Access Point mode so you can connect to it and send single HTTP request with configuration. Use ```/config``` endpoint to send configuration. After few seconds ESP will try to connect to given router and switch to normal mode. You will see info about connection state on the yellow line, and humidity and temperarue in bottom lines.

## Accessing.
After configration node will start to boardcast mDNS with node name. So you can find it throught name.local in your lan computer. For oxample if you give name ```bulba``` to your node, you should be able to see it configuration in browser by typing URL: ```http://bulba.local/config```

## Rest API.
Here is shourt summary of rest api. Each call requires authentication (see proper paragraph), all arguments which need to be delivered (POST) are send in x-www-form-urlencoded.

| URL suffix    | Method | description |
| ------------- | ------ | ----------- |
| /             | GET    | Request chart with history measurements, and allow manual turn on of fan. |
| /config       | GET    | Get current node configuration in JSON format|
| /config       | POST   | Configure node, field names are this same as returned by this same url with configuration |
| /factoryReset | GET    | Request hard reset of node and switch to configuration mode|
| /status       | GET    | Returns last measured values (T-temperature, H-humidity, D-timestamp |
| /history      | GET    | Returns JSON encoded history of mesurements, in this same format as /status. It also contains ```now``` field which allows to put those measurements in time line |
| /clearHistory | GET    | Wipeouts all historical readings. |
| /run          | POST   | Enable fan relay for given amount of seconds, regardles of humidity reading. Single argument ```time``` is expected with runtime in seconds |
| /setup        | GET    | Request configuration page. |
| /update       | POST   | Starts firmware update, it accepts single agrument ```url``` which should point to new firmware image. |
| /version      | GET    | To get current version of firmware. |

## Authentication.
Currently HTTP Digest auth is used.

## Hardware
In folder hardware are all needed things to work with board and schematic. If you would like you can also use this: 
(Board)[https://oshpark.com/shared_projects/PgFfqdfC]

# Thanks goes to

Bombel - For building me PCB, which saves a lot of my time and vulgar words :)
