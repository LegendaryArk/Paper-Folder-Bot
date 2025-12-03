#include "hardstop.h"

using namespace vex;

Hardstop::Hardstop(Motor& motor, double kp, double ki, double kd, double minFoldPos, double maxFoldPos, double maxEnc, vex::timer& timer)
	: Motor(motor), pid(kp, ki, kd, timer), minFoldPos(minFoldPos), maxFoldPos(maxFoldPos), maxEnc(maxEnc), timer(timer)
{
	setBrake(brakeType::coast);
}

void Hardstop::setTarget(double foldPos, double maxPower)
{
	this->tgtPos = (maxEnc / (maxFoldPos - minFoldPos)) * (foldPos - minFoldPos + 1.1);
	this->maxPower = maxPower;
}

void Hardstop::update()
{
	if (isSettled())
	{
		stop();
		return;
	}

	double ctrl = pid.calculate(tgtPos, position(rotationUnits::raw));
	setVelocity(maxPower * ctrl, velocityUnits::rpm);
	setMaxTorque(100, percentUnits::pct);
	spin(directionType::fwd);
}

bool Hardstop::isSettled(double posThreshold, double velThreshold)
{
	return fabs(position(rotationUnits::raw) - tgtPos) < posThreshold && fabs(velocity(velocityUnits::rpm)) < velThreshold;
}

void Hardstop::calibrate()
{
	if (calibrated)
		return;

	setVelocity(20, velocityUnits::rpm);
	setMaxTorque(10, percentUnits::pct);
	spin(directionType::rev);

	if (!isStalling(10, 20))
		return;

	stop();
	setPosition(0, rotationUnits::raw);
	calibrated = true;
}