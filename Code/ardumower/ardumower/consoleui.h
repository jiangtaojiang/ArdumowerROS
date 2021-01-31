// serial console menu etc.

void Robot::purgeConsole() {
  while (Console.available())
    Console.read();
}

char Robot::waitCharConsole() {
  while (Console.available() == 0);
  return (char)Console.read();
}

String Robot::waitStringConsole() {
  String s = "";
  char ch;
  while (true) {
    ch = waitCharConsole();
    if (ch == '\n' || ch == '\r')
      break;
    else
      s += ch;
  };
  return s;
}


// ROS need to be replaced with ROS debug info?
void Robot::printInfo(Stream &s) {

  if (consoleMode == CONSOLE_OFF) {
  } else {
    Streamprint(s, "t%6u ", (millis() - stateStartTime) / 1000);
    Streamprint(s, "L%3u ", loopsPerSec);
    //Streamprint(s, "r%4u ", freeRam());
    Streamprint(s, "m%1d ", consoleMode);
    Streamprint(s, "%4s ", stateNames[stateCurr]);
    if (consoleMode == CONSOLE_PERIMETER) {
      Streamprint(s, "sig min %4d max %4d avg %4d mag %5d qty %3d",
                  (int)perimeter.getSignalMin(0), (int)perimeter.getSignalMax(0), (int)perimeter.getSignalAvg(0),
                  perimeterLeftMag, (int)(perimeter.getFilterQuality(0) * 100.0));
      Streamprint(s, "  in %2d  cnt %4d  on %1d\r\n",
                  (int)perimeterLeftInside, perimeterLeftCounter, (int)(!perimeter.signalTimedOut(0)) );
    } else {
      Streamprint(s, "rpm %4d %4d ", (int)motorLeftRpmCurr, (int)motorRightRpmCurr);
      Streamprint(s, "set %4d %4d ", (int)motorLeftSpeedRpmSet, (int)motorRightSpeedRpmSet);
      if (consoleMode == CONSOLE_SENSOR_VALUES) {
        // sensor values
        Streamprint(s, "sen %4d %4d %4d ", (int)motorLeftSense, (int)motorRightSense, (int)motorMowSense);
        Streamprint(s, "bum %4d %4d ", bumperLeft, bumperRight);
        Streamprint(s, "dro %4d %4d ", dropLeft, dropRight);                                                                                      // Dropsensor - Absturzsensor
        Streamprint(s, "son %4d %4d %4d ", sonarDistLeft, sonarDistCenter, sonarDistRight);
        Streamprint(s, "yaw %3d ", (int)(imu.ypr.yaw / PI * 180.0));
        Streamprint(s, "pit %3d ", (int)(imu.ypr.pitch / PI * 180.0));
        Streamprint(s, "rol %3d ", (int)(imu.ypr.roll / PI * 180.0));
        if (perimeterUse) Streamprint(s, "per %3d ", (int)perimeterLeftInside);
        if (lawnSensorUse) Streamprint(s, "lawn %3d %3d ", (int)lawnSensorFront, (int)lawnSensorBack);
      } else {
        // sensor counters
        Streamprint(s, "sen %4d %4d %4d ", motorLeftSenseCounter, motorRightSenseCounter, motorMowSenseCounter);
        Streamprint(s, "bum %4d %4d ", bumperLeftCounter, bumperRightCounter);
        Streamprint(s, "dro %4d %4d ", dropLeftCounter, dropRightCounter);                                                                      // Dropsensor - Absturzsensor
        Streamprint(s, "son %3d ", sonarDistCounter);
        Streamprint(s, "yaw %3d ", (int)(imu.ypr.yaw / PI * 180.0));
        Streamprint(s, "pit %3d ", (int)(imu.ypr.pitch / PI * 180.0));
        Streamprint(s, "rol %3d ", (int)(imu.ypr.roll / PI * 180.0));
        //Streamprint(s, "per %3d ", perimeterLeft);
        if (perimeterUse) Streamprint(s, "per %3d ", perimeterLeftCounter);
        if (lawnSensorUse) Streamprint(s, "lawn %3d ", lawnSensorCounter);
        if (gpsUse) Streamprint(s, "gps %2d ", (int)gps.satellites());
      }
      Streamprint(s, "bat %2d.%01d ", (int)batVoltage, (int)((batVoltage * 10) - ((int)batVoltage * 10)) );
      Streamprint(s, "chg %2d.%01d %2d.%01d ",
                  (int)chgVoltage, (int)((chgVoltage * 10) - ((int)chgVoltage * 10)),
                  (int)chgCurrent, (int)((abs(chgCurrent) * 10) - ((int)abs(chgCurrent) * 10))
                 );
      Streamprint(s, "imu%3d ", imu.getCallCounter());
      Streamprint(s, "adc%3d ", ADCMan.getCapturedChannels());
      Streamprint(s, "%s\r\n", name.c_str());
    }
  }
}


void Robot::printMenu() {
  Console.println();
  Console.println(F("-----------MAIN MENU----------"));
  Console.println(F("f=delete all USER settings (reset all user settings in EEPROM)"));
  Console.println(F("e=delete all ERRORS (reset error counters in EEPROM)"));
  Console.println(F("r=delete all STATISTICS (reset statistics in EEPROM)"));
  Console.println(F("s=save USER settings (save user settings to EEPROM)"));
  Console.println(F("x=print USER settings"));
  Console.println(F("p=test EEPROM module"));
  Console.println(F("c=test RTC module"));
  Console.println(F("i=scan for I2C devices"));
  Console.println(F("1=test motors"));
  Console.println(F("2=test odometry"));
  Console.println(F("3=communications menu (setup Bluetooth)"));
  Console.println(F("4=ADC calibration (perimeter sender & charger must be off)"));
  Console.println(F("5=calibrate IMU acceleration next side"));
  Console.println(F("6=calibrate IMU compass start/stop"));
  Console.println(F("7=delete IMU calibration"));
  Console.println(F("8=show EEPROM data (for backup)"));
  Console.println(F("9=enter EEPROM data (for restore)"));
  Console.println(F("0=exit"));
  Console.println();
}



void Robot::delayInfo(int ms) {
  unsigned long endtime = millis() + ms;
  while (millis() < endtime) {
    readSensors();
    // printInfo(Console);
    delay(1000);
  }
}

void Robot::testOdometry() {
  char ch;
  int lastLeft = 0;
  int lastRight = 0;
  motorLeftPWMCurr = motorSpeedMaxPwm / 2; motorRightPWMCurr = motorSpeedMaxPwm / 2;
  setMotorPWM(motorLeftPWMCurr, motorRightPWMCurr, false);
  while (true) {
    resetIdleTime();
    if ((odometryLeft != lastLeft) || (odometryRight != lastRight)) {
      //Console.print(F("Press'f' forward, 'r' reverse, 'z' reset  "));
      Console.print(F("Press'f' forward, 'r' reverse, 'z' reset, 's' stop, 'o' one revolution, '0' exit  "));
      Console.print(F("left="));
      Console.print(odometryLeft);
      Console.print(F("  right="));
      Console.println(odometryRight);
      lastLeft = odometryLeft;
      lastRight = odometryRight;
    }
    delay(100);
    if (Console.available() > 0) {
      ch = (char)Console.read();
      if (ch == '0') break;
      if (ch == 'f') {
        motorLeftPWMCurr = motorSpeedMaxPwm / 2; motorRightPWMCurr = motorSpeedMaxPwm / 2;
        setMotorPWM(motorLeftPWMCurr, motorRightPWMCurr, false);
      }
      if (ch == 'r') {
        motorLeftPWMCurr = -motorSpeedMaxPwm / 2; motorRightPWMCurr = -motorSpeedMaxPwm / 2;
        setMotorPWM(motorLeftPWMCurr, motorRightPWMCurr, false);
      }
      if (ch == 'z') {
        odometryLeft = 0; odometryRight = 0;
      }
      if (ch == 's') {
        motorLeftPWMCurr = 0; motorRightPWMCurr = 0;
        setMotorPWM(motorLeftPWMCurr, motorRightPWMCurr, false);
      }
      if (ch == 'o')
      {
        odometryLeft = 0; odometryRight = 0;
        motorLeftPWMCurr = motorSpeedMaxPwm / 2; motorRightPWMCurr = motorSpeedMaxPwm / 2;
        setMotorPWM(motorLeftPWMCurr, motorRightPWMCurr, false);
        while (true)
        {
          if (Console.available() > 0)
          {
            ch = (char)Console.read();
            if (ch == '0') break;
          }

          if ((odometryLeft != lastLeft) || (odometryRight != lastRight)) {
            //Console.print(F("Press'f' forward, 'r' reverse, 'z' reset  "));
            Console.print(F("Press'f' forward, 'r' reverse, 'z' reset, 's' stop, 'o' one revolution  "));
            Console.print(F("left="));
            Console.print(odometryLeft);
            Console.print(F("  right="));
            Console.println(odometryRight);
            lastLeft = odometryLeft;
            lastRight = odometryRight;
          }

//          if ((odometryLeft >= odometryTicksPerRevolution) || (odometryRight >= odometryTicksPerRevolution))
//          {
//            motorLeftPWMCurr = 0; motorRightPWMCurr = 0;
//            setMotorPWM(motorLeftPWMCurr, motorRightPWMCurr, false);
//            break;
//          }
        }
      }
    }
  };
  motorLeftPWMCurr = 0; motorRightPWMCurr = 0;
  setMotorPWM(motorLeftPWMCurr, motorRightPWMCurr, false);
}

void Robot::testRTC() {
  Console.println("reading RTC time...");
  if (readDS1307(datetime)) {
    Console.print(F("RTC date received: "));
    Console.println(date2str(datetime.date));
  }
  Console.println("writing new RTC datetime Sun 28-02-2016 23:59...");
  datetime.time.hour = 23;
  datetime.time.minute = 59;
  datetime.date.dayOfWeek = 0;
  datetime.date.day = 28;
  datetime.date.month = 2;
  datetime.date.year = 2016;
  setDS1307(datetime);
  Console.println("reading RTC datetime...");
  if (readDS1307(datetime)) {
    Console.print(F("RTC datetime received: "));
    Console.print(date2str(datetime.date));
    Console.print(F("  "));
    Console.println(time2str(datetime.time));
  }
}

void Robot::testMotors() {
  motorLeftPWMCurr = 0; motorRightPWMCurr = 0;
  setMotorPWM(motorLeftPWMCurr, motorRightPWMCurr, false);

  Console.println(F("testing left motor (forward) half speed..."));
  delay(1000);
  motorLeftPWMCurr = motorSpeedMaxPwm / 2; motorRightPWMCurr = 0;
  setMotorPWM(motorLeftPWMCurr, motorRightPWMCurr, false);
  delayInfo(5000);
  motorLeftPWMCurr = 0; motorRightPWMCurr = 0;
  setMotorPWM(motorLeftPWMCurr, motorRightPWMCurr, false);

  Console.println(F("testing left motor (reverse) full speed..."));
  delay(1000);
  motorLeftPWMCurr = -motorSpeedMaxPwm; motorRightPWMCurr = 0;
  setMotorPWM(motorLeftPWMCurr, motorRightPWMCurr, false);
  delayInfo(5000);
  motorLeftPWMCurr = 0; motorRightPWMCurr = 0;
  setMotorPWM(motorLeftPWMCurr, motorRightPWMCurr, false);

  Console.println(F("testing right motor (forward) half speed..."));
  delay(1000);
  motorLeftPWMCurr = 0; motorRightPWMCurr = motorSpeedMaxPwm / 2;
  setMotorPWM(motorLeftPWMCurr, motorRightPWMCurr, false);
  delayInfo(5000);
  motorLeftPWMCurr = 0; motorRightPWMCurr = 0;
  setMotorPWM(motorLeftPWMCurr, motorRightPWMCurr, false);

  Console.println(F("testing right motor (reverse) full speed..."));
  delay(1000);
  motorLeftPWMCurr = 0; motorRightPWMCurr = -motorSpeedMaxPwm;
  setMotorPWM(motorLeftPWMCurr, motorRightPWMCurr, false);
  delayInfo(5000);
  motorLeftPWMCurr = 0; motorRightPWMCurr = 0;
  setMotorPWM(motorLeftPWMCurr, motorRightPWMCurr, false);
}

void Robot::menu() {
  char ch;
  printMenu();
  while (true) {
    // ROS?
    /*     if (!rmcsUse){
    */
    resetIdleTime();

    imu.update();
    if (Console.available() > 0) {
      ch = (char)Console.read();
      switch (ch) {
        case '0':
          return;
        case '1':
          testMotors();
          printMenu();
          break;
        case '2':
          testOdometry();
          printMenu();
          break;
        case '3':
          if (bluetoothUse)
            commsMenuBT();
          printMenu();
          break;
        case '5':
          imu.calibAccNextAxis();
          printMenu();
          break;
        case '6':
          imu.calibComStartStop();
          break;
        case '7':
          imu.deleteCalib();
          printMenu();
          break;
        case '8':
          Flash.dump();
          printMenu();
          break;
        case '9':
          Flash.restore();
          printMenu();
          break;
        case '4':
          ADCMan.calibrate();
          printMenu();
          break;
        case 's':
          saveUserSettings();
          printMenu();
          break;
        case 'p':
          Flash.test();
          break;
        case 'c':
          testRTC();
          printMenu();
          break;
        case 'f':
          //printSettingSerial();
          deleteUserSettings();
          printMenu();
          break;
        case 'i':
          I2CScanner();
          break;
        case 'r':
          //printSettingSerial();
          deleteRobotStats();
          printMenu();
          break;
        case 'x':
          printSettingSerial();
          Console.println(F("DONE"));
          printMenu();
          break;
        case 'e':
          resetErrorCounters();
          setNextState(STATE_OFF);
          Console.println(F("ALL ERRORS ARE DELETED"));
          printMenu();
          break;
      }
    }
    delay(10);
  }
}



void Robot::commsMenuBT() {
  while (true) {
    Console.println();
    Console.println(F("COMMUNICATIONS MENU  == Bluetooth =="));
    Console.println(F(" 1=Select other communication method"));
    Console.println(F(" 2=setup BT module config (quick baudscan (recommended))"));
    Console.println(F(" 3=setup BT module config (extensive baudscan)"));
    Console.println(F(" 0=Main Menu"));
    Console.println();

    delay(100);
    purgeConsole();

    switch (waitCharConsole()) {
      case '0':
        return;
      case '1':
        commsMenuSelect();
        return;
      case '2':
        configureBluetooth(true);
        break;
      case '3':
        configureBluetooth(false);
        break;
    }
  }
}



void Robot::commsMenuSelect(void) {
  bluetoothUse = 0;

  while (true) {
    Console.println(F("Select communication method"));
    Console.println(F(" 1=Bluetooth"));

    delay(100);
    purgeConsole();

    switch (waitCharConsole()) {
      case '1': bluetoothUse = 1; return;
    }
  }
}

// ROS only read serial during setup
void Robot::readSerial() {
  // serial input
  if (Console.available() > 0) {
    String cmd = waitStringConsole();

    char ch = cmd[0];
    switch (ch) {
      case 'd':
        menu(); // menu
        break;
      case 'l':
        bumperLeft = true; // press 'l' to simulate left bumper
        bumperLeftCounter++;
        break;
      case 'r':
        bumperRight = true; // press 'r' to simulate right bumper
        bumperRightCounter++;
        break;
      case 'i':
        // press 'i' to toggle imuUse
        imuUse = !imuUse;
        break;
    }
  }
}
