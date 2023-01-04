# Buzzer
## Build
```shell
pio run -t uploadfs
pio run -t upload
```

## Monitor
```shell
pio device monitor
```

## OTA
```shell
pio run
pio run -t buildfs
```

Go to http://<IP>/upload and upload `firmware.bin` and `spiffs.bin` to the firmware and filesystem sections.
