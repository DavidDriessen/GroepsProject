#pragma config(Sensor, S1,     Sonar,          sensorSONAR)
#pragma config(Sensor, S2,     LichtR,         sensorLightActive)
#pragma config(Sensor, S3,     LichtL,         sensorLightActive)
#pragma config(Sensor, S4,     sensorColor,    sensorColorNxtFULL)
#pragma config(Motor,  motorA,          MotorLinks,    tmotorNXT, PIDControl, encoder)
#pragma config(Motor,  motorB,          Head,          tmotorNXT, openLoop)
#pragma config(Motor,  motorC,          MotorRechts,   tmotorNXT, PIDControl)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

//protoype van functies en tasks.
float get_offset(void);
void turnStreat(void);
void turnLeft(void);
void turnRight(void);
void stop_rij_auto(void);
task rij_auto();
task scancode();
task sonar();


// Globale variable
int max_light = 0;
int min_light = 0;
float Tp = 70; // target power bij het rechtdoorrijden van de robot.
float Kp = 5; // constante varriable die representatief is aan de proportional range van de error (met hoeveel gaat de snelheid omhoog / omlaag per error niveau).
float Ki = 1; // Constante varriable die een correctie geeft voor de integral. deze word grotendeels getweakt via trial en error
float Kd = 70; // Constante varriable die een correctie geeft voor de derivative. Ook deze word grootendeels getweakt via trial en error.
float offset = 0;//get_offset(); // gemiddelde van de minimum en maximum leeswaardes van de sensor ((minlicht + maxlicht) / 2) + 2
float integral = 0; // corriectiewaarde die aan de error word toegevoegd om te compenseren voor het "verleden"
float perverror = 0; //hierin word de error opgeslagen uit de vorrige scanloop om de derivative mee te berekenen.
float derivative = 0; //correctiewaarde die aan de error word toegevoegd om te "voorspellen" wat te volgende error gaat worden en hier zo goed mogelijk rekening mee te houden.
float error = 0;
int triggerScan = 0;
int telzwart = 0;
int pos = 0;
bool auto = false;

//int array[20];			//debug varriable voor het checken van colorsensoren
//int x = -1;					//
//int len = 20;				//-
//float lightrr = 0; 	//debug varriable voor het checken van lichtsensoren tijdens de rit
//float lightll = 0; 	//debug varriable voor het checken van lichtsensoren tijdens de rit

// Kruispunt functies
void turnRight(void)
{
	motor[MotorRechts] = 0;
	motor[MotorLinks] = 0;

	nMotorEncoderTarget[Head] = 100;
	motor[Head] = 50;
	wait1Msec(2000);

	nMotorEncoderTarget[Head] = 100;
	motor[Head] = -50;
	if(SensorValue[Sonar] > 33)
	{
		int en = nMotorEncoder[MotorRechts] + 250;
		nMotorEncoderTarget[MotorRechts] = en;
		motor[MotorRechts] = -20;
		motor[MotorLinks] = 50;
		while(nMotorEncoder[MotorRechts] != en){}
	}
	else
	{
		turnStreat();
	}
}

void turnLeft(void)
{
	motor[MotorRechts] = 0;
	motor[MotorLinks] = 0;

	nMotorEncoderTarget[Head] = 100;
	motor[Head] = -50;
	wait1Msec(2000);

	nMotorEncoderTarget[Head] = 100;
	motor[Head] = 50;
	if(SensorValue[Sonar] > 33)
	{
		int en = nMotorEncoder[MotorRechts] + 300;
		nMotorEncoderTarget[MotorRechts] = en;
		motor[MotorRechts] = 50;
		motor[MotorLinks] = -20;
		while(nMotorEncoder[MotorRechts] != en){}
	}
	else
	{
		turnStreat();
	}
}

void turnStreat(void)
{
	if(SensorValue[Sonar] > 33)
	{
		int en = nMotorEncoder[MotorRechts] + 70;
		nMotorEncoderTarget[MotorRechts] = en;
		motor[MotorRechts] = Tp;
		motor[MotorLinks] = Tp;
		while(nMotorEncoder[MotorRechts] != en){}
	}
}


float get_offset(void)
{
	//playSound(soundBeepBeep);
	//wait10Msec(500);
	max_light = SensorValue[LichtR];
	min_light = SensorValue[LichtL];
	playSound(soundBeepBeep);
	float offset = ((max_light + min_light) / 2) + 2;
	return offset;
}

void stop_rij_auto(void){
	stopTask(sonar);
	stopTask(scancode);
	stopTask(rij_auto);
	integral = perverror = derivative = error = triggerScan = pos = telzwart = 0;
	auto = false;
	motor[MotorRechts]=0;
	motor[MotorLinks]=0;
}

task rij_auto()
{
	startTask(sonar);
	startTask(scancode);
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
		//displayString(6, "R= %d, L= %d", speedR, speedL);
		motor[MotorLinks] = speedL;
		motor[MotorRechts] = speedR;
		perverror = error;
		//lightrr = SensorValue[LichtL]; //debug
		//lightll = SensorValue[LichtR]; //debug

		if (SensorValue[LichtR] < offset)
		{
			switch(pos){
			case 2:
				turnStreat();
				break;
			case 3:
				turnLeft();
				break;
			case 4:
				turnRight();
				break;
			default:
				turnStreat();
				break;
			}
			pos = 0;
		}
	}
}


task scancode(){                           							 //deze task leest de sensor in en houd bij wanneer er een verandering in kleur op treed
	int Kleur = 0;
	while(true){																						 // als er een verandering optreed wordt dit bijgehouden

		Kleur= SensorValue[sensorColor];
		switch(Kleur){
		case 0:
		case 1: //black
			if(triggerScan == 1){
				telzwart = telzwart +1;

			}
			break;

		case 5:	//red
			if(triggerScan == 0){
				triggerScan = 1;
			}
			else{
				triggerScan = 0;
				displayString(4, "%d", telzwart);
				pos = telzwart;
				//array[x] = telzwart;
				//if(x < len){
				//	x++;
				//}
				telzwart =0;
			}
			break;
		}
		while(SensorValue[sensorColor] == Kleur){
			wait1Msec(20);
		}
	}
}

task sonar()
{
	while (true)
	{
		if (SensorValue[Sonar] <= 30)
		{
			stop_rij_auto();
			break;
		}
	}
}

task main()
{
	int mailbox = 5;
	string btmessage="";
	nMotorEncoder[Head] = 0;

	while(true)
	{
		// lees mailbox
		int a = cCmdMessageGetSize(mailbox);  //haal de hoeveel bytes in eerst volgende bericht
		if(a > 0)
		{
			cCmdMessageRead(btmessage, a, mailbox); // lees aantal bytes a in en plaats ze in btmessage

			if(btmessage == "AUTO")
			{
				auto = true;
				startTask(rij_auto);
			}
			else if(btmessage == "MANUAL")
			{
				stop_rij_auto();
			}
			else if(btmessage == "CALIBRATE")
			{
				stop_rij_auto();
				offset = get_offset();
			}
			else if(!auto)
			{
				if(btmessage == "UP")
				{
					motor[MotorRechts]=100;
					motor[MotorLinks]=100;
				}
				else if(btmessage == "DOWN")
				{
					motor[MotorRechts]=-100;
					motor[MotorLinks]=-100;
				}
				else if(btmessage == "LEFT")
				{
					motor[MotorRechts]=100;
					motor[MotorLinks]=-100;
				}
				else if(btmessage == "RIGHT")
				{
					motor[MotorRechts]=-100;
					motor[MotorLinks]=100;
				}
				else if(btmessage == "NULL")
				{
					motor[MotorRechts]=0;
					motor[MotorLinks]=0;
				}
			}
			btmessage="";
		}
	}
}
