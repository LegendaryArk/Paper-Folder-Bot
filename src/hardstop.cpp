#include "hardstop.h"

using namespace vex;

Hardstop::Hardstop(Motor& motor, double kp, double ki, double kd, vex::timer& timer)
	: motor(motor), pid(kp, ki, kd, timer), timer(timer) {
	motor.setBrake(brakeType::coast);
}

void Hardstop::setTarget(double foldPos, double maxPower) {
	this->tgtPos = 43.056 * foldPos - 6.985;
	this->maxPower = maxPower;
}

void Hardstop::update() {
	if (isSettled()) {
		motor.stop();
		return;
	}

	double ctrl = pid.calculate(tgtPos, motor.position(rotationUnits::raw));
	motor.setVelocity(maxPower * ctrl, velocityUnits::rpm);
	motor.setMaxTorque(100, percentUnits::pct);
	motor.spin(directionType::fwd);

	printf("tgt: %.2f, act: %.2f, ctrl: %.2f\n", tgtPos, motor.position(rotationUnits::raw), ctrl);
}

bool Hardstop::isSettled(double posThreshold, double velThreshold) const {
	return fabs(motor.position(rotationUnits::raw) - tgtPos) < posThreshold && fabs(motor.velocity(velocityUnits::rpm)) < velThreshold;
}

void Hardstop::calibrate() {
	if (calibrated)
		return;

	motor.setVelocity(20, velocityUnits::rpm);
	motor.setMaxTorque(10, percentUnits::pct);
	motor.spin(directionType::rev);

	if (!motor.isStalling(10, 30))
		return;

	motor.stop();
	motor.setPosition(0, rotationUnits::raw);
	calibrated = true;
}