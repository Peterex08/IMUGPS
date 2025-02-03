# IMUGPS
A project that uses data from IMU, GPS and Barometer modules to estimate position, velocity, attitude and a rocket parachute behavior based on space state filters implementation using a Extended Kalman Filter Algorithm.

## How to install everything?
To use the application you will need multiple stuff, including from android apps to c++, python and javascript libs.

 1. Download **SensorServer** in your phone to stream IMU and GPS data for Extended Kalman FIlter Analysis.
    **NOTE:** Your cellphone plays the role for a rocket, in this case, substitutes a TAU 1201 GPS Module and a MPU 9250 IMU. It does not work properly for the filtering conditions, but can be used to test all the process.
	 [umer0586 - SensorServer (Github Repo)](https://github.com/umer0586/SensorServer)
	 
 3. Download all Cpp libraries:
**Threads**:`sudo apt-get install libpthread-stubs0-dev`
**OpenSSL**:`sudo apt-get install libssl-dev`
**Boost**:`sudo apt-get install libboost-system-dev`
**Eigen3**:`sudo apt-get install libeigen3-dev`
**Paho MQTT C++**:`sudo apt-get install libpaho-mqttpp-dev libpaho-mqtt3a-dev`
**CMake**:`sudo apt-get install cmake`
**Compiler C++**:`sudo apt-get install g++`
The projects also needs **nlohmann/json** and **WebSocket++** libraries, but they were configured to be downloaded during the execution of cmake.

 5. Download [NodeJS](https://nodejs.org/) to run frontend application based on React + Vite
	 Then, go to EPTALAUNCHSERVER/frontend and run `npm install`, this will install everything inside package.json, making the frontend capable of running everything.
	
 6. Python needs to be installed as well to run all backend user management using Django, so run `sudo apt install python3` and `sudo apt install python3-pip` to make sure pip is installed.
	 Then go to EPTALAUNCHSERVER/backend and run `pip install -r requirements.txt` to install all python libs.

## How to use it?

 1. Go on your cellphone and start Sensor Server app, then copy the websocket ip that the cellphone will display to you after clicking on start.
 2. Prepare 3 ubuntu terminals to run all the 3 applications at the same time, the c++, python and JavaScript.
 3. Open IMUGPSBAR/src/main code and on "int main()" section, substitute the current URL `192.168.1.4:8080` with the new one generated by your sensor server app.
 4. Then, in **terminal ONE**, go to IMUGPSBAR, and run `rm -rf build`, then `mkdir build`, then `cd build` and finally `cmake ..` and then `make`
 5. In **terminal TWO**, go to EPTALAUNCHSERVER/backend and run `python3 manage.py runserver`
 6. In **terminal THREE**, go to EPTALAUNCHSERVER/frontend and run `npm run dev`, this will prompt a local URL, you can click it to go to the website responsible to show the "rocket" data streaming.
 7. Finally, to start streaming the data from your phone, go to **terminal ONE** and run `./sensor_monitoring`
 
 You should see the screen of the app being populated with the data streamed from the cellphone. It should look like that:
![login](https://github.com/user-attachments/assets/6062f6f5-a925-4231-8431-43bcf62d43a7)
![main](https://github.com/user-attachments/assets/852eb18f-2b5c-446e-ad89-275e6968216d)

For User and Password, try "manga" for both!

The algorithm scheme for the filter idealization is inside IMUGPSBAR folder, in CANVAS format.

 ## Disclaimer
Using the cellphone to simulate the data streaming from a TAU 1201 and a MPU 9250 module didn't result in good results. The GPS from the phone has a ridiculous refresh rate [0.3 Hz], compared to TAU 1201 that is at least 5 Hz, it is a massive difference. 
This low performance on gps leads to significantly bad results on Space State Estimator to create a route path!

## Appropriate components

|Componente| Preço |
|--|--|
| [LilyGo T-Call A7670E (Modem 4G, 2G para stream de dados via MQTT)](https://pt.aliexpress.com/item/1005008308661519.html?src=google&pdp_npi=4@dis!BRL!241.80!209.66!!!!!@!12000044567251667!ppc!!!&src=google&albch=shopping&acnt=768-202-3196&isdl=y&slnk=&plac=&mtctp=&albbt=Google_7_shopping&aff_platform=google&aff_short_key=UneMJZVf&gclsrc=aw.ds&&albagn=888888&&ds_e_adid=&ds_e_matchtype=&ds_e_device=c&ds_e_network=x&ds_e_product_group_id=&ds_e_product_id=pt1005008308661519&ds_e_product_merchant_id=107688876&ds_e_product_country=BR&ds_e_product_language=pt&ds_e_product_channel=online&ds_e_product_store_id=&ds_url_v=2&albcp=21106536414&albag=&isSmbAutoCall=false&needSmbHouyi=false&gad_source=1&gclid=CjwKCAiAzPy8BhBoEiwAbnM9O-S69O-FxePB9MVHXYg27ltqMeTAid3u3O52nlinMTM8azi1eSOJHxoCk1gQAvD_BwE) | R$ 209,57 |
| [Allystar TAU 1201](https://pt.aliexpress.com/item/1005004364185318.html) | R$ 139,99 |
| [MPU 9250 (IMU)](https://pt.aliexpress.com/item/1005006318431209.html?src=google&pdp_npi=4@dis!BRL!37.19!12.16!!!!!@!12000036741443813!ppc!!!&src=google&albch=shopping&acnt=768-202-3196&isdl=y&slnk=&plac=&mtctp=&albbt=Google_7_shopping&aff_platform=google&aff_short_key=UneMJZVf&gclsrc=aw.ds&&albagn=888888&&ds_e_adid=&ds_e_matchtype=&ds_e_device=c&ds_e_network=x&ds_e_product_group_id=&ds_e_product_id=pt1005006318431209&ds_e_product_merchant_id=5089736139&ds_e_product_country=BR&ds_e_product_language=pt&ds_e_product_channel=online&ds_e_product_store_id=&ds_url_v=2&albcp=21106536414&albag=&isSmbAutoCall=false&needSmbHouyi=false&gad_source=1&gclid=CjwKCAiAzPy8BhBoEiwAbnM9O1pj5p0NZzBiHIB7lFiaEFBzFbAc7wOytZN0T7MJe00ljiCjP2tOeBoCA4YQAvD_BwE) | R$ 12,06 |

## References

1. https://github.com/FlyTheThings/uNavINS
2. https://github.com/AlexCrownshaw/Quaternion-to-Rotation-Matrix
3. https://github.com/balamuruganky/EKF_IMU_GPS
4. https://www.kalmanfilter.net/default.aspx
