#include "hardstop.h"

using namespace vex;

Hardstop::Hardstop(Motor& motor, double kp, double ki, double kd, vex::timer& timer)
	: motor(motor), pid(kp, ki, kd, timer), timer(timer) {}

void Hardstop::setTarget(double foldPos, double maxPower) {
	this->tgtPos = foldPos; // TODO: convert fold position on paper to motor encoder ticks
	this->maxPower = maxPower;
}

void Hardstop::update() {
	double ctrl = pid.calculate(tgtPos, motor.position(rotationUnits::raw));
	motor.spin(directionType::fwd, maxPower * ctrl, velocityUnits::pct);
}

bool Hardstop::isSettled() {
	return fabs(motor.position(rotationUnits::raw) - tgtPos) < 5;
}