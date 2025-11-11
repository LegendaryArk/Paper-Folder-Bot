#include "motor.h"

using namespace vex;

Motor::Motor(int32_t port, vex::timer& timer, bool reverse, double gearRatio)
	: vex::motor(port, reverse), gearRatio(gearRatio), timer(timer) {}

double Motor::getDeltaPosition() {
	double currEncPos = this->position(rotationUnits::raw);
	double delta = currEncPos - prevEncPos;
	prevEncPos = currEncPos;
	return delta;
}

bool Motor::isStalling(double threshold) {
	double dt = timer.value() - prevTime;
	double velocity = getDeltaPosition() / dt;
	prevTime = timer.value();

	return fabs(velocity) < threshold;
}