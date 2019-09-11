//References the object representing the gyro
void printCycleGyro(){
	printGyro();
	printAccel();
	printMag();
	
	printAttitude(imu.ax, imu.ay, imu.az,
	-imu.my, -imu.mx, imu.mz);
	
	Serial.println();
	lastPrint = millis();
}

//References object representing the GPS unit
void printCycleGPS(){
	Serial.print(F("Location: "));
	if (gps.location.isValid()){
		Serial.print(gps.location.lat(), 6); 
		Serial.print(F(","));
		Serial.print(gps.location.lng(), 6);
		}else{
		Serial.print(F("INV"));
	}

	Serial.println();
	Serial.print(F("Date/Time: "));
	if (gps.date.isValid()){
		Serial.print(gps.date.month());
		Serial.print(F("/"));
		Serial.print(gps.date.day());
		Serial.print(F("/"));
		Serial.print(gps.date.year());
		}else{
		Serial.print(F("INV"));
	}

	Serial.print(F(" "));
	if (gps.time.isValid()){
		if (gps.time.hour() < 10) Serial.print(F("0"));
		Serial.print(gps.time.hour());
		Serial.print(F(":"));
		if (gps.time.minute() < 10) Serial.print(F("0"));
		Serial.print(gps.time.minute());
		Serial.print(F(":"));
		if (gps.time.second() < 10) Serial.print(F("0"));
		Serial.print(gps.time.second());
		Serial.print(F("."));
		if (gps.time.centisecond() < 10) Serial.print(F("0"));
		Serial.print(gps.time.centisecond());
		}else{
		Serial.print(F("INV"));
	}
	
	Serial.println();
	Serial.print(F("Satellites: "));
	if(gps.satellites.isValid()){
		Serial.print(gps.satellites.value());
		}else{
		Serial.print(F("INV"));
	}

	Serial.println();
	Serial.print(F("Speed: "));
	if(gps.speed.isValid()){
		Serial.print(gps.speed.mph());
		Serial.print(F("mph"));
		}else{
		Serial.print(F("INV"));
	}
	
	Serial.println();
}

//Prints gx, gy, gz converted to DPS
void printGyro()
{
	Serial.print(F("G: "));
	Serial.print(imu.calcGyro(imu.gx), 2);
	Serial.print(F(", "));
	Serial.print(imu.calcGyro(imu.gy), 2);
	Serial.print(F(", "));
	Serial.print(imu.calcGyro(imu.gz), 2);
	Serial.println(F(" deg/s"));
}

//Prints the acceleration ax, ay, az as g's
void printAccel()
{
	Serial.print(F("A: "));
	Serial.print(imu.calcAccel(imu.ax), 2);
	Serial.print(F(", "));
	Serial.print(imu.calcAccel(imu.ay), 2);
	Serial.print(F(", "));
	Serial.print(imu.calcAccel(imu.az), 2);
	Serial.println(F(" g"));
}

//Prints mx, my, mz values either as Gauss
void printMag()
{
	Serial.print(F("M: "));
	Serial.print(imu.calcMag(imu.mx), 2);
	Serial.print(F(", "));
	Serial.print(imu.calcMag(imu.my), 2);
	Serial.print(F(", "));
	Serial.print(imu.calcMag(imu.mz), 2);
	Serial.println(F(" gauss"));
}

// Prints the pitch, roll and heading
// Pitch/roll calculations take from this app note:
// http://cache.freescale.com/files/sensors/doc/app_note/AN3461.pdf?fpsp=1
// Heading calculations taken from this app note:
// http://www51.honeywell.com/aero/common/documents/myaerospacecatalog-documents/Defense_Brochures-documents/Magnetic__Literature_Application_notes-documents/AN203_Compass_Heading_Using_Magnetometers.pdf
void printAttitude(float ax, float ay, float az, float mx, float my, float mz)
{
	float pitch = atan2(ay, az);						//Was roll
	float roll = atan2(-ax, sqrt(ay * ay + az * az));   //Was pitch
	
	float heading;
	if (my == 0)
	heading = (mx < 0) ? PI : 0;
	else
	heading = atan2(mx, my);
	
	heading -= DECLINATION * PI / 180;
	
	if (heading > PI) heading -= (2 * PI);
	else if (heading < -PI) heading += (2 * PI);
	
	/* Convert everything from radians to degrees:
	heading *= 180.0 / PI;
	pitch *= 180.0 / PI;
	roll  *= 180.0 / PI;*/
	pitch-= pitchOffset;
	roll-= rollOffset;	

	if (roll > PI) roll -= (2 * PI);
	else if (roll < -PI) roll += (2 * PI);

	if (pitch > PI) pitch -= (2 * PI);
	else if (pitch < -PI) pitch += (2 * PI);
	
	Serial.print(F("Pitch, Roll: "));
	Serial.print(pitch, 2);
	Serial.print(F(", "));
	Serial.println(roll, 2);
	Serial.print(F("Heading: "));
	Serial.print(heading, 2);
}

