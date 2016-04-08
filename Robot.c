#pragma config(Sensor, S1,     sensorColor,    sensorColorNxtFULL)
#pragma config(Sensor, S2,     Sonar,          sensorSONAR)
#pragma config(Sensor, S3,     LichtL,         sensorLightActive)
#pragma config(Sensor, S4,     LichtR,         sensorLightActive)
#pragma config(Motor,  motorA,          MotorLinks,    tmotorNXT, PIDControl, encoder)
#pragma config(Motor,  motorB,          Head,          tmotorNXT, openLoop)
#pragma config(Motor,  motorC,          MotorRechts,   tmotorNXT, PIDControl)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//
#pragma systemFile // eliminates warning for "unreferenced" functions
#include "queue.c"
#include "kruispunt.c"
#include "manual.c"

//protoype van functies en tasks.
float get_offset(void);
task stop_rij_auto();
task rij_auto();
task ramp_up_auto();
task scancode();
task sound();
task sonar();

// Globale variable
int que = 0;
Queue q;
float turn = 0;
int max_light = 0;
int min_light = 0;
float Kp = 3.5; // constante varriable die representatief is aan de proportional range van de error (met hoeveel gaat de snelheid omhoog / omlaag per error niveau).
float Ki = 1; // Constante varriable die een correctie geeft voor de integral. deze word grotendeels getweakt via trial en error
float Kd = 250; // Constante varriable die een correctie geeft voor de derivative. Ook deze word grootendeels getweakt via trial en error.
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

//kalibreer de PID op basis van de twee licht sensoren.
float get_offset(void)
{
	//kalibreer
	max_light = SensorValue[LichtL];
	min_light = SensorValue[LichtR];
	playSound(soundBeepBeep);
	float offset = ((max_light + min_light) / 2) + 2;
	return offset;
}

//verhoog rustig de snelheid van de motoren voor de PID
task ramp_up_auto(){
	Tp = 10;
	while(Tp < fTp){
		Tp += 4;
		if (Tp > fTp)
		{
			Tp = fTp;
		}
		wait10Msec(10);
	}
}

//stop alle tasks die te maken hebben met het lijn volg systeem
//en reset alle dynamische variabele.
task stop_rij_auto(){
	stopTask(rij_auto);
	stopTask(sonar);
	stopTask(sound);
	stopTask(scancode);
	init_queue(&q);
	integral = perverror = derivative = error = triggerScan = pos = telzwart = 0;
	auto = false;
	//neem geleidelijk de snelheid af, tot stoppen.
	while(Tp > 0){
		Tp -= 6;
		if (Tp < 0)
		{
			Tp = 0;
		}
		motor[MotorRechts]=Tp;
		motor[MotorLinks]=Tp;
		wait10Msec(10);
	}
	//reset het hoofdje.
	wait10Msec(200);
	motor[Head] = -nMotorEncoder[Head] / 4;
	while(nMotorEncoder[Head] != 0){}
	motor[Head] = 0;
}

//vold de lijn en start alle tasks.
task rij_auto()
{
	startTask(ramp_up_auto);
	startTask(sound);
	startTask(sonar);
	startTask(scancode);
	wait10Msec(10);
	float speedL = 0, speedR = 0, Grayscale = 0;
	while(true)
	{
		Grayscale = SensorValue[LichtR];
		if (error == 0 || (perverror < 0 && error > 0) || (perverror > 0 && error < 0))
		{
			integral = 0;
		}
		error = Grayscale - offset; // berekent de error value (hoever de sensor van de lijn zit.)
		integral = (2/3)*integral + error; //opsomming van de integrals
		derivative = error - perverror;
		//datalogAddValue(derivative, derivative);
		turn = (Kp * error) + (Ki * integral) + (Kd * derivative); // berekening waarbij bepaald word hoeveel er bijgestuurd moet worden om op de lijn te blijven.

		speedL = (Tp - turn);
		speedR = (Tp + turn);
		//displayString(6, "R= %d, L= %d", speedR, speedL);
		motor[MotorLinks] = speedL;
		motor[MotorRechts] = speedR;
		perverror = error;

		//lightrr = SensorValue[LichtR]; //debug
		//lightll = SensorValue[LichtL]; //debug

		//maak een beslissing bij een kruispunt.
		if (SensorValue[LichtL] < offset && SensorValue[LichtR] < offset)
		{
			que = dequeue(&q);

			if(que != 0){
				pos = que;
			}
			switch(pos){
			case 1:
				break;
				turnStreat();
			case 2:
				turnBack();
				break;
			case 3:
				turnLeft();
				break;
			case 4:
				turnRight();
				break;
			default:
				motor[MotorLinks] = 0;
				motor[MotorRechts] = 0;
				Tp = 1;
				startTask(stop_rij_auto);
			}
			pos = 0;
		}
	}
}


task scancode()
{                           							 //deze task leest de sensor in en houd bij wanneer er een verandering in kleur op treed
	int Kleur = 0;
	while(true)
	{																					 // als er een verandering optreed wordt dit bijgehouden

		Kleur = SensorValue[sensorColor];
		switch(Kleur)
		{
		case 0:
		case 1: //black
			if(triggerScan == 1)
			{
				telzwart = telzwart +1;
			}
			break;

		case 5:	//red
			if(triggerScan == 0)
			{
				triggerScan = 1;
			}
			else
			{
				triggerScan = 0;
				displayString(4, "%d", telzwart);
				pos = telzwart;
				//array[x] = telzwart;
				//if(x < len){
				//	x++;
				//}
				telzwart =0;
			}
		}
		while(SensorValue[sensorColor] == Kleur){}
		wait1Msec(15);
	}
}

//scant voor opstakels
task sonar()
{
	while (true)
	{
		if (SensorValue[Sonar] <= 30)
		{
			startTask(stop_rij_auto);
		}
	}
}

//star wars melody
task sound(){
	//star wars geluitje
	while(true){
		playTone(695, 14); while(bSoundActive){}
		playTone(695, 14); while(bSoundActive){}
		playTone(695, 14); while(bSoundActive){}
		playTone(929, 83); while(bSoundActive){}
		playTone(1401, 83); while(bSoundActive){}
		playTone(1251, 14); while(bSoundActive){}
		playTone(1188, 14); while(bSoundActive){}
		playTone(1054, 14); while(bSoundActive){}
		playTone(1841, 83); while(bSoundActive){}
		playTone(1401, 41); while(bSoundActive){}
		playTone(1251, 14); while(bSoundActive){}
		playTone(1188, 14); while(bSoundActive){}
		playTone(1054, 14); while(bSoundActive){}
		playTone(1841, 83); while(bSoundActive){}
		playTone(1401, 41); while(bSoundActive){}
		playTone(1251, 14); while(bSoundActive){}
		playTone(1188, 14); while(bSoundActive){}
		playTone(1251, 14); while(bSoundActive){}
		playTone(1054, 55); while(bSoundActive){}
		wait1Msec(280);
		playTone(695, 14); while(bSoundActive){}
		playTone(695, 14); while(bSoundActive){}
		playTone(695, 14); while(bSoundActive){}
		playTone(929, 83); while(bSoundActive){}
		playTone(1401, 83); while(bSoundActive){}
		playTone(1251, 14); while(bSoundActive){}
		playTone(1188, 14); while(bSoundActive){}
		playTone(1054, 14); while(bSoundActive){}
		playTone(1841, 83); while(bSoundActive){}
		playTone(1401, 41); while(bSoundActive){}
		playTone(1251, 14); while(bSoundActive){}
		playTone(1188, 14); while(bSoundActive){}
		playTone(1054, 14); while(bSoundActive){}
		playTone(1841, 83); while(bSoundActive){}
		playTone(1401, 41); while(bSoundActive){}
		playTone(1251, 14); while(bSoundActive){}
		playTone(1188, 14); while(bSoundActive){}
		playTone(1251, 14); while(bSoundActive){}
		playTone(1054, 55); while(bSoundActive){}
		wait1Msec(280);
	}
}

task main()
{
	int mailbox = 5, a = 0;
	string btmessage="";
	nMotorEncoder[Head] = 0;
	nMotorEncoder[MotorLinks] = 0;
	nMotorEncoder[MotorRechts] = 0;
	init_queue(&q);

	while(true)
	{
		// lees mailbox
		a = cCmdMessageGetSize(mailbox);  //haal de hoeveel bytes in eerst volgende bericht
		if(a > 0)
		{
			cCmdMessageRead(btmessage, a, mailbox); // lees aantal bytes a in en plaats ze in btmessage


			if(btmessage == "AUTO")
			{
				//start het lijn volg systeem
				auto = true;
				startTask(rij_auto);
			}
			else if(btmessage == "MANUAL")
			{
				//stop het lijn volg systeem
				startTask(stop_rij_auto);
			}
			else if(btmessage == "CALIBRATE")
			{
				//kalibreer de PID
				startTask(stop_rij_auto);
				offset = get_offset();
			}
			else if(!auto)
			{
				if(btmessage == "UP")
				{
					btn = "UP";
					stopTask(man_ramp_down);
					startTask(man_ramp_up);
				}
				else if(btmessage == "DOWN")
				{
					btn = "DOWN";
					stopTask(man_ramp_down);
					startTask(man_ramp_up);
				}
				else if(btmessage == "LEFT")
				{
					btn = "LEFT";
					stopTask(man_ramp_down);
					startTask(man_ramp_up);
				}
				else if(btmessage == "RIGHT")
				{
					btn = "RIGHT";
					stopTask(man_ramp_down);
					startTask(man_ramp_up);
				}
				else if(btmessage == "NULL")
				{
					stopTask(man_ramp_up);
					startTask(man_ramp_down);
				}
			}
			else
			{
				//zet instructies in de que om bij kruispunten uit te voeren.
				if(btmessage == "UP")
				{
					enqueue(&q, 1);
				}
				else if(btmessage == "DOWN")
				{
					enqueue(&q, 2);
				}
				else if(btmessage == "LEFT")
				{
					enqueue(&q, 3);
				}
				else if(btmessage == "RIGHT")
				{
					enqueue(&q, 4);
				}
			}
			btmessage="";
		}
	}
}
