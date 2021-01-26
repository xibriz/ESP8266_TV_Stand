# ESP8266_TV_stand

Hacking the remote control of iiglo motorisert TV-stativ TVL1002

Project info:
https://www.hjemmeautomasjon.no/forums/topic/7285-automatisert-motorisert-tv-stativ-via-mqtt/

Copy config and fill out variables
```
cp secrets_dist.h secrets.h
nano secrets.h
```

Update firmware
```
mosquitto_pub -t 'esp/tvstand/fw/update' -m 'http://webserver.localdomain/ESP8266_TV_Stand.ino.bin'
```

Control TV Stand
```
mosquitto_pub -t 'esp/tvstand/control' -m 'ON'
mosquitto_pub -t 'esp/tvstand/control' -m 'OFF'
```