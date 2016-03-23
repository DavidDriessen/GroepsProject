#pragma config(Sensor, S1,     Bumper,         sensorTouch)
#pragma config(Sensor, S2,     Sonar,          sensorSONAR)
#pragma config(Sensor, S3,     LichtRGB,       sensorCOLORRED)
#pragma config(Sensor, S4,     Licht,          sensorLightActive)
#pragma config(Motor,  motorA,          MotorLinks,    tmotorNXT, PIDControl, encoder)
#pragma config(Motor,  motorB,           ,             tmotorNXT, openLoop)
#pragma config(Motor,  motorC,          MotorRechts,   tmotorNXT, PIDControl)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

// Globale variable
int speedR = 0;
int speedL = 0;

task rij(){
	//long R = nMotorEncoder[MotorRechts];
	//long L = nMotorEncoder[MotorLinks];
	//nxtDisplayString(4, "Rechts = %d", nMotorEncoder[MotorRechts] - R);
	//nxtDisplayString(5, "Links = %d", nMotorEncoder[MotorLinks] - L);
	while(speedR < 50 && speedL < 50){
		speedR++;
		speedL++;
		motor[MotorRechts]=speedR;
		motor[MotorLinks]=speedL;
		//nxtDisplayString(4, "Rechts = %d", nMotorEncoder[MotorRechts] - R);
		//nxtDisplayString(5, "Links = %d", nMotorEncoder[MotorLinks] - L);
		wait1Msec(50);
	}
	while(true){
		//nxtDisplayString(4, "Rechts = %d", nMotorEncoder[MotorRechts] - R);
		//nxtDisplayString(5, "Links = %d", nMotorEncoder[MotorLinks] - L);
	}
}
task stopRij(){
	wait1Msec(50);
	while(speedR > 0 && speedL > 0){
		nxtDisplayString(3, "R= %d, L= %d", speedR, speedL);
		if(speedR > 0){
			speedR--;
		}
		if(speedL > 0){
			speedL--;
		}
		nxtDisplayString(4, "R= %d, L= %d", speedR, speedL);
		motor[MotorRechts]=speedR;
		motor[MotorLinks]=speedL;
		nxtDisplayString(5, "R= %d, L= %d", speedR, speedL);
		wait1Msec(10);
	}
}

task sense(){
	// lees de sensoren
	while(true){
		nxtDisplayString(6, "R= %d, L= %d", speedR, speedL);
		if(SensorValue[Bumper] == 1){
			// stop na botsing en back-off
			StopTask(rij);
			StopTask(stopRij);
			speedR=0;
			speedL=0;
			motor[MotorRechts]=-5;
			motor[MotorLinks]=-5;
			wait1Msec(1000);
			motor[MotorRechts]=0;
			motor[MotorLinks]=0;
		}

		nxtDisplayString(2, "Licht = %d", SensorValue[LichtRGB]);
		// licht sensore
		if(SensorValue[Licht] < 60){
			// stop na botsing en back-off
			StopTask(rij);
			StartTask(stopRij);
		}

	}
}

task main()
{
	nMotorEncoder[MotorRechts] = 0;                // reset the Motor Encoder of Motor B
	nMotorEncoder[MotorLinks] = 0;

	// start met rijden
	//StartTask(rij);
	// start met het lezen van sensoren
	StartTask(sense);
	while(true){}
	/*nxtDisplayString(4, "%d", SensorValue[Bumper]);
	wait1Msec(5000);*/

}
