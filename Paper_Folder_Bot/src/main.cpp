#include "vex.h"
#include "motor.h"
#include "feeder.h"
#include "hardstop.h"

vex::brain brain;
vex::controller controller;

Motor feederMotor(vex::PORT1, brain.Timer, false);
Motor rollers(vex::PORT2, brain.Timer, false);
Motor hardstop1Motor(vex::PORT3, brain.Timer, false);
Motor hardstop2Motor(vex::PORT4, brain.Timer, false);
vex::distance distanceSensor(vex::PORT5);
vex::touchled touchLed(vex::PORT6);

Feeder feeder(feederMotor, distanceSensor, brain.Timer);
Hardstop hardstop1(hardstop1Motor, 0.1, 0.01, 0.05, brain.Timer);
Hardstop hardstop2(hardstop2Motor, 0.1, 0.01, 0.05, brain.Timer);

void inputFoldLocations(double& fold1, double& fold2) {
    brain.Screen.clearScreen();

    double precision = 1.0;

    while (!touchLed.pressing()) {
        brain.Screen.setCursor(1, 1);
        brain.Screen.print("Fold 1 Location: %.2f", fold1);
        brain.Screen.setCursor(2, 1);
        brain.Screen.print("Fold 2 Location: %.2f", fold2);

        brain.Screen.setCursor(3, 1);
        brain.Screen.print("Precision: %.2f", precision);

        brain.Screen.setCursor(5, 1);
        brain.Screen.print("Press Touch LED to Start Folding");

        if (controller.ButtonLUp.pressing())
            fold1 += precision;
        if (controller.ButtonLDown.pressing())
            fold1 -= precision;

        if (controller.ButtonRUp.pressing())
            fold2 += precision;
        if (controller.ButtonRDown.pressing())
            fold2 -= precision;
        
        if (controller.ButtonFUp.pressing())
            precision *= 10;
        if (controller.ButtonFDown.pressing())
            precision /= 10;

        wait(50, vex::msec);
    }
}

int main() {
    int errorCode = 0;

	double fold1Location = 0;
    double fold2Location = 0;
    inputFoldLocations(fold1Location, fold2Location);

    hardstop1.setTarget(fold1Location);
    hardstop2.setTarget(fold2Location);

    while (!hardstop1.isSettled() || !hardstop2.isSettled()) {
        hardstop1.update();
        hardstop2.update();
        wait(20, vex::msec);
    }

    rollers.spin(vex::directionType::fwd, 50, vex::velocityUnits::pct);

    bool isTrayEmpty = false;
    bool isJammed = false;
    while (!isTrayEmpty && !isJammed) {
        feeder.index(50, 10);

        if (rollers.isStalling(2)) {
            rollers.stop();
            isJammed = true;
            errorCode = 1;
        }
    }

    rollers.stop();

    switch (errorCode) {
        case 0:
            brain.Screen.clearScreen();
            brain.Screen.setCursor(1, 1);
            brain.Screen.print("Folding Complete!");

            touchLed.setBlink(vex::color::green, 0.5, 0.5);
            while (!touchLed.pressing()) {}
            break;
        case 1:
            brain.Screen.clearScreen();
            brain.Screen.setCursor(1, 1);
            brain.Screen.print("Error: Paper Jammed!");

            touchLed.setBlink(vex::color::red, 0.5, 0.5);
            while (!touchLed.pressing()) {}
            break;
    }

    brain.programStop();
}