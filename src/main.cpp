#include "vex.h"
#include "motor.h"
#include "feeder.h"
#include "hardstop.h"
#include "rollers.h"
#include "util.h"

vex::brain brain;
vex::controller controller;

Motor feederMotor(vex::PORT7, true);
Motor rollerMotor(vex::PORT1, false);
Motor hardstop1Motor(vex::PORT6, false);
Motor hardstop2Motor(vex::PORT8, true);
vex::distance distanceSensor(vex::PORT2);
vex::optical opticalSensor(vex::PORT5);
vex::touchled touchLed(vex::PORT4);

void inputFoldLocations(Hardstop& hardstop1, Hardstop& hardstop2, double& fold1, double& fold2)
{
    brain.Screen.clearScreen();

    brain.Screen.setCursor(5, 1);
    brain.Screen.setFont(vex::fontType::mono15);
    brain.Screen.print("Press Touch LED");

    double precision = 1.0;

    double* foldLocs[2] = {&fold1, &fold2};
    const vex::controller::button* btns[4] = {
        &controller.ButtonLUp,
        &controller.ButtonLDown,
        &controller.ButtonRUp,
        &controller.ButtonRDown
    };
    double pressSt[4] = {};
    bool precisionDebounce[2] = {};

    bool ledPressed = false, confirmPressed = false, controllerPressed = false, confirmed = false;
    bool resetPressed = false;
    while (!confirmed) 
    {
        if (touchLed.pressing() && !ledPressed)
            ledPressed = true;
        if (!touchLed.pressing() && ledPressed)
            confirmed = true;
        
        if (controller.ButtonEUp.pressing() && !controllerPressed)
            controllerPressed = true;
        if (!controller.ButtonEUp.pressing() && controllerPressed)
            confirmed = true;
        
        if (brain.buttonCheck.pressing() && !confirmPressed)
            confirmPressed = true;
        if (!brain.buttonCheck.pressing() && confirmPressed)
            confirmed = true;

        if (controller.ButtonEDown.pressing() && !resetPressed) 
        {
            resetPressed = true;
        }
        else if (!controller.ButtonEDown.pressing() && resetPressed) 
        {
            fold1 = 9;
            fold2 = 19;
            resetPressed = false;
        }

        for (int i = 0; i < 2; i++) 
        {
            brain.Screen.setCursor(i + 1, 1);
            brain.Screen.clearLine();
            brain.Screen.print("Fold %d: %.1f cm", i + 1, *foldLocs[i]);
        }

        brain.Screen.setCursor(3, 1);
        brain.Screen.clearLine();
        brain.Screen.print("Precision: %.1f", precision);

        for (int i = 0; i < 4; i++) 
        {
            if (btns[i]->pressing() && !pressSt[i])
                pressSt[i] = brain.Timer.value();
        }

        for (int i = 0; i < 4; i++) 
        {
            if (btns[i]->pressing() && brain.Timer.value() - pressSt[i] >= 0.5) 
            {
                if (i % 2)
                    *foldLocs[i / 2] -= precision;
                else
                    *foldLocs[i / 2] += precision;
                
                if (*foldLocs[1] - *foldLocs[0] < hardstop1.getMinFoldPos())
                {
                    // i / 2 gives the current fold index (integer division)
                    // (i / 2 + 1) % 2 gives the other fold index
                    // Util::sgn(1 - i) gives +1 if i is 0 or 2 (increasing first fold), -1 if i is 1 or 3 (decreasing second fold)
                    *foldLocs[(i / 2 + 1) % 2] = *foldLocs[i / 2] + hardstop1.getMinFoldPos() * Util::sgn(1 - i);
                }
            }
            else if (!btns[i]->pressing()) 
            {
                if (brain.Timer.value() - pressSt[i] < 0.5) 
                {
                    if (i % 2)
                        *foldLocs[i / 2] -= precision;
                    else
                        *foldLocs[i / 2] += precision;
                    
                    if (*foldLocs[1] - *foldLocs[0] < hardstop1.getMinFoldPos())
                    {
                        // i / 2 gives the current fold index (integer division)
                        // (i / 2 + 1) % 2 gives the other fold index
                        // Util::sgn(1 - i) gives +1 if i is 0 or 2 (increasing first fold), -1 if i is 1 or 3 (decreasing second fold)
                        *foldLocs[(i / 2 + 1) % 2] = *foldLocs[i / 2] + hardstop1.getMinFoldPos() * Util::sgn(1 - i);
                    }
                }
                pressSt[i] = 0;
            }
        }
        
        if (controller.ButtonFUp.pressing() && !precisionDebounce[0]) 
        {
            if (precision < 10)
                precision *= 10;
            precisionDebounce[0] = true;
        }
        else if (!controller.ButtonFUp.pressing() && precisionDebounce[0]) 
        {
            precisionDebounce[0] = false;
        }
        if (controller.ButtonFDown.pressing() && !precisionDebounce[1]) 
        {
            if (precision > 0.1)
                precision /= 10;
            precisionDebounce[1] = true;
        }
        else if (!controller.ButtonFDown.pressing() && precisionDebounce[1]) 
        {
            precisionDebounce[1] = false;
        }

        fold1 = Util::clamp(fold1, hardstop1.getMinFoldPos(), hardstop1.getMaxFoldPos());
        fold2 = Util::clamp(fold2, hardstop2.getMinFoldPos() + hardstop1.getMinFoldPos(), hardstop2.getMaxFoldPos() + hardstop1.getMaxFoldPos());

        if (fold2 - fold1 < fold1) 
        {
            brain.Screen.setCursor(6, 1);
            brain.Screen.clearLine();
            brain.Screen.print("Folds overlap:");
            brain.Screen.setCursor(7, 1);
            brain.Screen.clearLine();
            brain.Screen.print("Will cause 3 folds");
        }
        else 
        {
            brain.Screen.setCursor(6, 1);
            brain.Screen.clearLine();
            brain.Screen.setCursor(7, 1);
            brain.Screen.clearLine();
        }

        wait(100, vex::msec);
    }
}

void displayError(int errorCode) 
{
    brain.Screen.clearScreen();
    brain.Screen.setCursor(3, 1);
    switch (errorCode) 
    {
        case 0:
            brain.Screen.print("Folding Complete!");

            touchLed.setBlink(vex::color::green, 0.5, 0.5);
            brain.playSound(vex::tada);
            break;
        case 1:
            brain.Screen.print("Error: Paper Jammed!");

            touchLed.setBlink(vex::color::red, 0.5, 0.5);
            for (int i = 0; i < 3; i++) 
            {
                brain.playSound(vex::alarm);
                wait(500, vex::msec);
            }
            break;
        case 2:
            brain.Screen.print("Error: Folding Timeout!");

            touchLed.setBlink(vex::color::red, 0.5, 0.5);
            for (int i = 0; i < 3; i++) 
            {
                brain.playSound(vex::alarm);
                wait(500, vex::msec);
            }
            break;
        case 3:
            brain.Screen.print("Output Tray Full!");
            
            touchLed.setBlink(vex::color::blue, 0.5, 0.5);
            brain.playSound(vex::alarm2);
            break;
    }

    while (!touchLed.pressing()) {}
    while (touchLed.pressing()) {}
}

void pause(double& st)
{
    while (controller.ButtonEUp.pressing()) {}
    
    touchLed.setColor(vex::color::yellow);
    brain.Screen.setCursor(4, 1);
    brain.Screen.clearLine();
    brain.Screen.print("Paused");

    while (!controller.ButtonEUp.pressing()) {}
    while (controller.ButtonEUp.pressing()) {}

    touchLed.off();
    st = brain.Timer.value();
}

int main()
{
    printf("\033[2J\n");

    Feeder feeder(feederMotor, distanceSensor, brain.Timer);
    Rollers rollers(rollerMotor, opticalSensor, brain.Timer);
    Hardstop hardstop1(hardstop1Motor, 0.02, 0.015, 0.0013, 7.2, 17.1, 1352, brain.Timer);
    Hardstop hardstop2(hardstop2Motor, 0.019, 0.015, 0.002, 8.5, 17.4, 1262, brain.Timer);

    int errorCode = 0;

    brain.Screen.setCursor(1, 1);
    brain.Screen.print("Calibrating...");
    while (!hardstop1.isCalibrated() || !hardstop2.isCalibrated()) 
    {
        hardstop1.calibrate();
        hardstop2.calibrate();
        wait(50, vex::msec);
    }

	double fold1Location = 9;
    double fold2Location = 19;
    inputFoldLocations(hardstop1, hardstop2, fold1Location, fold2Location);

    printf("Fold Locations: %.1f, %.1f\n", fold1Location, fold2Location);

    brain.Screen.clearLine(3);
    brain.Screen.clearLine(5);

    brain.Screen.setCursor(4, 1);
    brain.Screen.clearLine();
    brain.Screen.print("Initializing...");

    hardstop1.setTarget(fold1Location);
    hardstop2.setTarget(fold2Location - fold1Location);

    while (!hardstop1.isSettled() || !hardstop2.isSettled())
    {
        hardstop1.update();
        hardstop2.update();
        wait(20, vex::msec);
    }

    hardstop1.stop();
    hardstop2.stop();

    printf("%f, %f\n", hardstop1Motor.position(vex::rotationUnits::raw), hardstop2Motor.position(vex::rotationUnits::raw));

    rollers.spin();

    int numPaper = 0;
    brain.Screen.setCursor(4, 1);
    brain.Screen.clearLine();
    brain.Screen.print("Folding... %d", numPaper);

    double st = 0;
    bool isTrayEmpty = false;
    bool hasError = false;
    while (!isTrayEmpty && !hasError)
    {
        st = brain.Timer.value();
        while (!feeder.index(40) || !isTrayEmpty)
        {
            if (brain.Timer.value() - st >= 3 && !feeder.isIndexingPaper())
                isTrayEmpty = true;
            
            if (controller.ButtonEUp.pressing())
            {
                feeder.stop();
                rollers.stop();

                pause(st);

                rollers.spin();
                brain.Screen.setCursor(4, 1);
                brain.Screen.clearLine();
                brain.Screen.print("Folding... %d", numPaper);
            }

            wait(20, vex::msec);
        }

        if (!isTrayEmpty)
        {
            st = brain.Timer.value();

            numPaper++;
            brain.Screen.setCursor(4, 1);
            brain.Screen.clearLine();
            brain.Screen.print("Folding... %d", numPaper);

            bool outputFull = false;
            while (!rollers.isClear() && !outputFull && !hasError)
            {
                if (rollers.isOutputTrayFull())
                {
                    outputFull = true;
                    rollers.stop();

                    displayError(errorCode = 3);
                    
                    errorCode = 0;
                    st = brain.Timer.value();
                    rollers.spin();
                }
                
                if (controller.ButtonEUp.pressing())
                {
                    rollers.stop();
                    feeder.stop();

                    pause(st);
                    
                    rollers.spin();
                    brain.Screen.setCursor(4, 1);
                    brain.Screen.clearLine();
                    brain.Screen.print("Folding... %d", numPaper);
                }

                if (rollers.isStalling(60, 70))
                {
                    rollers.stop();
                    printf("Rollers stalled: %f, %f\n", rollers.velocity(vex::velocityUnits::rpm), rollers.current(vex::percentUnits::pct));
                    hasError = true;
                    errorCode = 1;
                }

                if (brain.Timer.value() - st >= 10)
                {
                    hasError = true;
                    errorCode = 2;
                }

                wait(20, vex::msec);
            }
        }
    }

    rollers.stop();
    feeder.stop();
    hardstop1.stop();
    hardstop2.stop();

    displayError(errorCode);

    brain.programStop();
}