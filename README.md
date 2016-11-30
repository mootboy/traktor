# ESP8266 Wifi servo and motor control #

Uses [esp-open-sdk][1] and contains code from the xtensa examples
(`hw_timer.c`). Extremely basic and in no way optimised or an example of
"clean" c code.

# Target ESP #
I'm using a ESP01 with 256+256 flash, tweaking the build will be
necessary if you want to run it on anything else. Whenever I remember
where I put my notes I'll link to how.

# Motor and Servo control #
The API, if you want to call it that, accepts UDP datagrams on port
53850. An integer will be interpreted as the desired servo pulse length
in microseconds. A number prefixed by the letter M will be interpreted
as desired motor speed in percent.

# Issues #
Too many to list at the moment.

[1]https://github.com/pfalcon/esp-open-sdk
