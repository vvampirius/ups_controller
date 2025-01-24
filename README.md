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

TBD

## Linux Raspberry Pi app

TDB

[Blog Post](https://wampi.re/village_ups)
