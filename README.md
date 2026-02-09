# garrett's great esp32 clock
![ggec-title](https://github.com/user-attachments/assets/0e08aa24-caac-4bc6-96d7-806e18c3a043)


have you seen how expensive those little desktop clocks have gotten? and any cheap one from aliexpress would take too long to arrive. and i have a 3d printer. and i have a soldering iron. and i have an esp32. and i have an ssd1306 display.

and now they've been fused together and they're a clock.

![ggec-distant](https://github.com/user-attachments/assets/2b3b9bea-92f2-4a2d-9f1b-6da6c7c4a826)

## instructions
the code needs a bit of configuration for your particular use (connecting to wifi, setting the timezone, etc.). all the code is in main/main.ino.
### ssd1306 config
some "bad" batches of ssd1306 displays (screen height of 64 version) have an address of 0x3C instead of the intended 0x3D. if the screen isn't working, try using the other screen address in `#define SCREEN_ADDRESS`.
### wifi setup
be sure to replace the values for `const char* ssid` and `const char* password` with your own wifi's ssid and password. the ssid is the name of your wifi. 
### timezone setup
make sure the timezone in `const char* timezone` matches your specific timezone. the new york timezone is in the code provided, which is a good example of how regions and locale with spaces are handled. for a complete list of timezones, please refer to the timeapi documentation [here](https://timeapi.io/documentation/iana-timezones). 

if you're unsure it works, you can make a test call on the timeapi website [here](https://timeapi.io/swagger/index.html), if you click on the section labeled `GET /api/v1/time/current/zone`, and then press "try it out". enter in your timezone, and press execute. if the section below populates with the code 200 response and a response body containing expected information (for example, `date` and `time` line up with today's date and the current time), then you've got the right timezone. if you instead get code 400 or the response body/headers aren't what you're expecting, check that you entered in the right timezone and try again.
