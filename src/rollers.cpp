#include "rollers.h"

using namespace vex;

Rollers::Rollers(Motor& motor, vex::optical& opticalSensor, vex::timer& timer)
	: Motor(motor), opticalSensor(opticalSensor), timer(timer)
{
	setBrake(brakeType::coast);
}

bool Rollers::isPaperDetected() const
{
	return opticalSensor.isNearObject();
}

bool Rollers::isClear() const
{
	static bool wasPaperDetected = false;
	if (!isPaperDetected() && wasPaperDetected)
	{
		wasPaperDetected = false;
		return true;
	}
	wasPaperDetected = isPaperDetected();
	return false;
}

bool Rollers::isOutputTrayFull() const
{
	static double st = -1;

	if (isPaperDetected())
	{
		if (st == -1)
			st = timer.value();
		else if (timer.value() - st >= TIME_THRESHOLD_SEC)
			return true;
	}
	else
	{
		st = -1;
	}

	return false;
}

void Rollers::spin()
{
	setMaxTorque(100, percentUnits::pct);
	setVelocity(80, velocityUnits::rpm);
	Motor::spin(directionType::fwd);
}