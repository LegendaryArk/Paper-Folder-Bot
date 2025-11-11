#include "pid_controller.h"
#include "util.h"

PidController::PidController(double kp, double ki, double kd, vex::timer& timer, double outputMax, double outputMin)
			: kp(kp), ki(ki), kd(kd), timer(timer), outputMax(outputMax), outputMin(outputMin) {}

double PidController::calculate(double error) {
	double dt = timer.value() - prevTime;

	integral += error * dt;
	double derivative = (error - prevError) / dt;
	prevError = error;
	return Util::clamp(kp * error + ki * integral + kd * derivative, outputMin, outputMax);
}
double PidController::calculate(double target, double current) {
	return calculate(target - current);
}