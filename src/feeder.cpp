#include "feeder.h"

using namespace vex;

Feeder::Feeder(Motor& motor, distance& distanceSensor, vex::timer& timer)
	: Motor(motor), distanceSensor(distanceSensor), timer(timer)
{
	setBrake(brakeType::coast);	
}

bool Feeder::isIndexingPaper() const
{
	return distanceSensor.objectDistance(distanceUnits::mm) < PAPER_DETECTION_DISTANCE_MM;
}

bool Feeder::index(double power)
{
	setMaxTorque(100, percentUnits::pct);
	setVelocity(power, velocityUnits::rpm);
	spin(directionType::fwd);
	
	static double st = -1;

	if (isIndexingPaper())
		st = timer.value();
	else
		st = -1;

	if (timer.value() - st >= 3 && st != -1)
	{
		stop();
		st = -1;
		wait(100, msec);

		setMaxTorque(30, percentUnits::pct);
		setVelocity(50, velocityUnits::rpm);
		spin(directionType::rev);
		wait(700, msec);
		stop();
		return true;
	}

	return false;
}