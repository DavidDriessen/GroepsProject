#pragma config(Sensor, S1,     Bumper,         sensorNone)
#pragma config(Sensor, S2,     Sonar,          sensorSONAR)
#pragma config(Sensor, S3,     LichtR,         sensorLightActive)
#pragma config(Sensor, S4,     LichtL,         sensorLightActive)
#pragma config(Motor,  motorA,          MotorLinks,    tmotorNXT, PIDControl, encoder)
#pragma config(Motor,  motorB,           ,             tmotorNXT, openLoop)
#pragma config(Motor,  motorC,          MotorRechts,   tmotorNXT, PIDControl)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

//protoype van functies.
float get_offset(void);


// Globale variable
int max_light = 0;
int min_light = 0;
float Tp = 70; // target power bij het rechtdoorrijden van de robot.
float Kp = 5; // constante varriable die representatief is aan de proportional range van de error (met hoeveel gaat de snelheid omhoog / omlaag per error niveau).
float Ki = 1; // Constante varriable die een correctie geeft voor de integral. deze word grotendeels getweakt via trial en error
float Kd = 70; // Constante varriable die een correctie geeft voor de derivative. Ook deze word grootendeels getweakt via trial en error.
float offset = get_offset(); // gemiddelde van de minimum en maximum leeswaardes van de sensor ((minlicht + maxlicht) / 2) + 2
float integral = 0; // corriectiewaarde die aan de error word toegevoegd om te compenseren voor het "verleden"
float perverror = 0; //hierin word de error opgeslagen uit de vorrige scanloop om de derivative mee te berekenen.
float derivative = 0; //correctiewaarde die aan de error word toegevoegd om te "voorspellen" wat te volgende error gaat worden en hier zo goed mogelijk rekening mee te houden.
float error = 0;
float lightrr = 0;
float lightll = 0;


float get_offset(void)
{
	playSound(soundBeepBeep);
	wait10Msec(500);
	max_light = SensorValue[LichtR];
	playSound(soundBeepBeep);
	wait10Msec(500);
	min_light = SensorValue[LichtL];
	playSound(soundBeepBeep);
	float offset = ((max_light + min_light) / 2) + 2;
	return offset;
}





task main()
{

	while(true)
	{
		float Grayscale = SensorValue[LichtL];
		if (error == 0 || (perverror < 0 && error > 0) || (perverror > 0 && error < 0))
		{
			integral = 0;
		}
		error = Grayscale - offset; // berekent de error value (hoever de sensor van de lijn zit.)
		integral = (2/3)*integral + error; //opsomming van de integrals
		derivative = error - perverror;
		//datalogAddValue(derivative, derivative);
		float turn = (Kp * error) + (Ki * integral) + (Kd * derivative); // berekening waarbij bepaald word hoeveel er bijgestuurd moet worden om op de lijn te blijven.

		float speedL = (Tp - turn);
		float speedR = (Tp + turn);
		nxtDisplayString(6, "R= %d, L= %d", speedR, speedL);
		motor[MotorLinks] = speedL;
		motor[MotorRechts] = speedR;
		perverror = error;
		lightrr = SensorValue[LichtL];
		lightll = SensorValue[LichtR];

	if (SensorValue[LichtR] < offset)
		{
			motor[MotorLinks] = 0;
			motor[MotorRechts] = 0;
			break;
		}
	}
}
