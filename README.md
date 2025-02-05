# ups_controller

It's not a UPS. It's just my addition for my simple UPS [AccordTek ББП-20](https://secur.by/katalog/istochniki-pitaniya/besperebojnye-bloki-pitaniya/bbp-20.html) to switch off connected devices (up to three) depends on battery voltage. Including graceful shutdown of Raspberry Pi.

![image](DSC_4442.webp)

Features:
- Switch off relays (up to three) depends of voltage
- Graceful shutdown of Raspberry Pi
- Notify Raspberry Pi about battery voltage via UART
- Raspberry Pi reboot assitant

The projects consist of three parts: PCB, Arduino sketch, Linux Raspberry Pi app.

## PCB

[EasyEda Project](https://easyeda.com/editor#project_id=0bf0c4ab24b245bfa950abd184b83c0f)

Components:
- 1 x Arduino Nano
- 3 x Relay [Panasonic TQ2-L2-5V](https://belchip.by/product/?selected_product=50890)
- 1 x Relay [Tianbo HJR-3FF-12VDC-S-ZF](https://belchip.by/product/?selected_product=s1_83820)
- 1 x DC-DC converter [SAIBO DCBK3836](https://belchip.by/product/?selected_product=33037)
- 1 x [4 channel logic converter](https://aliexpress.ru/item/32361199306.html)
- 1 x SMD LED 0603
- 4 x LED 3mm

TBD

## Arduino sketch

[sketch](https://github.com/vvampirius/ups_controller/tree/master/arduino)

First of all you must adjust real voltage measuring by tuning A0_VR and checking Serial Output.

### arduino.ino

    Relay routerRelay = Relay(2, 5, 1000, 10000, 0);

Shutdown voltage (1000): 10V

    Raspberry raspberryRelay = Raspberry(6, 7, 3, 4, 10, 11, 1050, 60000, 10000, 1);
    Relay thirdRelay = Relay(8, 9, 1050, 10000, 2);

Shutdown voltage (1050): 10.5V

## Linux Raspberry Pi companion app

Add to /boot/config.txt:

    dtoverlay=gpio-shutdown,gpio_pin=23,active_low=0,gpio_pull=down
    dtoverlay=gpio-poweroff,gpiopin=17
    enable_uart=1

Remove `console=serial0,115200` from /boot/cmdline.txt (if exists)

Build and copy `rpi-ups-client` to /usr/local/sbin/

Place /srv/rpi-ups-client.yml from [example](https://github.com/vvampirius/ups_controller/blob/master/rpi-ups-client/config.yml.example)

/lib/systemd/system/rpi-ups-client.service:

    [Unit]
    Description=rpi-ups-client
    After=network.target

    [Service]
    Type=simple
    ExecStart=/usr/local/sbin/rpi-ups-client -c /srv/rpi-ups-client.yml
    Restart=always

    [Install]
    WantedBy=multi-user.target

Enable SystemdD service:

    systemctl daemon-reload
    systemctl enable rpi-ups-client
    systemctl start rpi-ups-client



[Blog Post](https://wampi.re/village_ups)
