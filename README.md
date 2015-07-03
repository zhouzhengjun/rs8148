# rs8148

rs8148 develop board support source code

1. This rdk is based on TI dvrrdk 2.8.0, kernel and uboot changed for rs8148 board. new features include sdi video capture, sdi audio capture, AIC913x audio play/record, ethernet driver
2. to support cmos sensor, iss also is added into ti tools. rs8148 doesn't support cmos sensor. some boards of rs81xx support cmos sensor.
3. A8 side si9135 driver is added. avoid m3 I2C control I2C bus, since audio codec and si9135 share same bus.
4. avcap demo program is provided. 
