//Joshua Kurien, Luca Cristiano, David Feldt, Bryan Guan

#include"mindsensors-motormux.h"

int const SWITCH = 0;
int const COLOUR_THRESHOLD = 35;
int const SORT_ANGLE = 100;
int const COLOUR_MOTOR_ANGLE = -155;
int const TIMEOUT = 60000;

void configure()
{
	SensorType[S1] = sensorI2CCustom;
	MSMMUXinit();
	SensorType[S2] = sensorEV3_Color;
	wait1Msec (50);
	SensorMode[S2] = modeEV3Color_Reflected;
	wait1Msec (50);
	SensorType[S3] = sensorEV3_Touch;
	wait1Msec(50);
}

void initialUserPrompt(bool & terminateRobot)
{
	eraseDisplay();
	displayString (4, "     Hey, I am LaundryBot!    ");
	displayString (5, "I can fold t-shirts or shorts.");
	displayString (6, "Then I will sort them for you.");
	displayString (7, "Place your article of clothing");
	displayString (8, "according to the outlines and ");
	displayString (9, "and I will get to work. Press ");
	displayString (10, "   any button to continue.    ");

	time1[T1] = 0;

	while(!getButtonPress(buttonAny) && terminateRobot == false){
		if (time1[T1] >= TIMEOUT)	{
			terminateRobot = true;
		}
	}

	while (getButtonPress(buttonAny))	{}
}

bool waitForInput()
{
	string buttonChoice = "invalid";
	string message = "";

	eraseDisplay();
	displayString (5, "   After you have placed an   ");
	displayString (6, "  article of clothing. Please ");
	displayString (7, " press the right button if it ");
	displayString (8, "  is a pair of shorts or the  ");
	displayString (9, "left button if it is a t-shirt");

	while (buttonChoice == "invalid")
	{
		while (!getButtonPress(buttonAny))
		{}
		if (getButtonPress (buttonLeft))
		{
			buttonChoice = "left";
		}
		else if (getButtonPress (buttonRight))
		{
			buttonChoice = "right";
		}
		else
		{
			buttonChoice = "invalid";
		}

		while (getButtonPress(buttonAny))
		{}
	}

	if (buttonChoice == "left")
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool colourCheck()
{
	int colourValue = 0;

	MSMMotorEncoderReset(mmotor_S1_1);
 	wait1Msec(50);

	MSMMotor(mmotor_S1_1, -50);
	while (MSMMotorEncoder(mmotor_S1_1) > COLOUR_MOTOR_ANGLE)
	{}
	MSMotorStop(mmotor_S1_1);

	wait1Msec(1000);
	colourValue = SensorValue[S2];

	MSMMotor(mmotor_S1_1, 20);
	while (MSMMotorEncoder(mmotor_S1_1) < 0)
	{}
	MSMotorStop(mmotor_S1_1);

	displayString (2,"%i", colourValue);

	if (colourValue >= COLOUR_THRESHOLD)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void deadMansSwitch()
{
	eraseDisplay();
	displayString (5, "Press the switch firmly and I ");
	displayString (6, "will begin folding and sorting");
	displayString (7, " Do not let go of the switch  ");
	displayString (8, "while folding or sorting is in");
	displayString (9, "   progress or I will reset.  ");

	while (SensorValue[S3] == 0)
	{}
	wait1Msec(200);
}

void resetFlap(tMotor motorPort)
{
	if (nMotorEncoder[motorPort] == 0)
	{}
	else
	{
		if (nMotorEncoder[motorPort] < 0)
		{
			motor[motorPort] = 10;

			while (nMotorEncoder[motorPort] < 0){}

			motor[motorPort] = 0;
		}
		else
		{
			motor[motorPort] = -10;

			while (nMotorEncoder[motorPort] > 0){}

			motor[motorPort] = 0;
		}
	}
}

void resetSort()
{
	if (nMotorEncoder[motorD] == 0)
	{}
	else
	{
		if (nMotorEncoder[motorD] < 0)
		{
			motor[motorD] = 10;

			while (nMotorEncoder[motorD] < 5)
			{}

			motor[motorD] = 0;
		}
		else
		{
			motor[motorD] = -10;

			while (nMotorEncoder[motorD] > -5)
			{}

			motor[motorD] = 0;
		}
	}
}

void reset()
{
	resetFlap(motorA);
	resetFlap(motorB);
	resetFlap(motorC);
	resetSort();
}

void foldFlap(tMotor motorPort, bool & emergencyStop, bool & checkClothes)
{
	motor[motorPort] = 100;

	time1[T1] = 0;

	while (nMotorEncoder[motorPort] < 170 && emergencyStop == false && checkClothes == true)
	{
		if (getButtonPress (buttonEnter) || time1[T1] > 3000)
		{
			emergencyStop = true;
		}
		wait1Msec(50);
		if (SensorValue[S3] == SWITCH)
		{
			checkClothes = false;
		}
	}

	motor[motorPort] = 0;

	motor[motorPort] = -100;

	time1[T1] = 0;

	while (nMotorEncoder[motorPort] > 0 && emergencyStop == false && checkClothes == true)
	{
		if (getButtonPress (buttonEnter) || time1[T1] > 3000)
		{
			emergencyStop = true;
		}
		wait1Msec(50);
		if (SensorValue[S3] == SWITCH)
		{
			checkClothes = false;
		}
	}

	motor[motorPort] = 0;
}

void fold(bool clothingType, bool & emergencyStop, bool & checkClothes)
{
	if (clothingType == true)
	{
		foldFlap(motorA, emergencyStop, checkClothes);

		if (emergencyStop == false && checkClothes == true)
		{
			foldFlap(motorC, emergencyStop, checkClothes);
		}
		if (emergencyStop == false && checkClothes == true)
		{
			foldFlap(motorB, emergencyStop, checkClothes);
		}
	}
	else
	{
		foldFlap(motorA, emergencyStop, checkClothes);

		if (emergencyStop == false && checkClothes == true)
		{
			foldFlap(motorB, emergencyStop, checkClothes);
		}
	}
}

void sort(bool clothingColour, bool & emergencyStop)
{
	nMotorEncoder[motorD] = 0;
	if (clothingColour == true)
	{
		motor[motorD] = 10;

		while (nMotorEncoder[motorD] < SORT_ANGLE - 25 && emergencyStop == false)
		{
			if (getButtonPress (buttonEnter))
			{
				emergencyStop = true;
			}
		}

		motor[motorD] = 0;

		wait1Msec (169);

		motor[motorD] = 100;

		while (nMotorEncoder[motorD] < SORT_ANGLE && emergencyStop == false)
		{
			if (getButtonPress (buttonEnter))
			{
				emergencyStop = true;
			}
		}

		motor[motorD] = 0;
	}
	else
	{
		motor[motorD] = -10;

		while (nMotorEncoder[motorD] > -1 * SORT_ANGLE + 25 && emergencyStop == false)
		{
			if (getButtonPress (buttonEnter))
			{
				emergencyStop = true;
			}
		}

		motor[motorD] = 0;

		wait1Msec (169);

		motor[motorD] = -100;

		while (nMotorEncoder[motorD] > -1*SORT_ANGLE && emergencyStop == false)
		{
			if (getButtonPress (buttonEnter))
			{
				emergencyStop = true;
			}
		}

		motor[motorD] = 0;
	}
	if (emergencyStop == false)
	{
		resetSort();
	}
}

task main()
{
	bool terminateRobot = false;
	bool emergencyStop = false;
	bool checkClothes = true;

	bool clothingColour = true;
	bool clothingType = true;

	configure();

	while (terminateRobot == false && emergencyStop == false)
	{
		checkClothes = true;
		initialUserPrompt(terminateRobot);

		if (terminateRobot == false){
			clothingType = waitForInput();

			clothingColour = colourCheck();

			deadMansSwitch();

			fold(clothingType, emergencyStop, checkClothes);

			if (emergencyStop == false && checkClothes == true)
			{
				sort(clothingColour, emergencyStop);
			}

			if (emergencyStop == false)
			{
				reset();
			}
		}
	}
}
