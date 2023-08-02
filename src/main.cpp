#include "main.h"
#include "autons.hpp"
#include "pros/adi.hpp"
#include "pros/misc.h"
#include "pros/motors.h"
#include "pros/rtos.hpp"

pros::ADIDigitalIn limitSwitch('H');
pros::ADIDigitalOut wallLeft('E');
pros::ADIDigitalOut wallRight('G');
pros::ADIDigitalOut intakeLeftPneumatic('D');
pros::ADIDigitalOut intakeRightPneumatic('F');
pros::ADIDigitalOut ptoLeft('A');
pros::ADIDigitalOut ptoRight('B');

pros::Motor puncher (9, pros::E_MOTOR_GEARSET_36, true, pros::E_MOTOR_ENCODER_DEGREES);
pros::Motor intakeLeft (6, pros::E_MOTOR_GEARSET_36, false, pros::E_MOTOR_ENCODER_DEGREES);
pros::Motor intakeRight (7, pros::E_MOTOR_GEARSET_36, false, pros::E_MOTOR_ENCODER_DEGREES);
pros::Imu imu_sensor(19);

// Chassis constructor
Drive chassis (
  // Left Chassis Ports (negative port will reverse it!)
  //   the first port is the sensored port (when trackers are not used!)
  {-1, -2, -20}

  // Right Chassis Ports (negative port will reverse it!)
  //   the first port is the sensored port (when trackers are not used!)
  ,{4, 5, 7}

  // IMU Port
  ,19

  // Wheel Diameter (Remember, 4" wheels are actually 4.125!)
  //    (or tracking wheel diameter)
  ,3.25

  // Cartridge RPM
  //   (or tick per rotation if using tracking wheels)
  ,600

  // External Gear Ratio (MUST BE DECIMAL)
  //    (or gear ratio of tracking wheel)
  // eg. if your drive is 84:36 where the 36t is powered, your RATIO would be 2.333.
  // eg. if your drive is 36:60 where the 60t is powered, your RATIO would be 0.6.
  ,1.6666

  // Uncomment if using tracking wheels
  /*
  // Left Tracking Wheel Ports (negative port will reverse it!)
  // ,{1, 2} // 3 wire encoder
  // ,8 // Rotation sensor

  // Right Tracking Wheel Ports (negative port will reverse it!)
  // ,{-3, -4} // 3 wire encoder
  // ,-9 // Rotation sensor
  */

  // Uncomment if tracking wheels are plugged into a 3 wire expander
  // 3 Wire Port Expander Smart Port
  // ,1
);

bool isShot = false;
bool morePower = true;
bool intakeForward = true;
bool leftIntakeAhead = false;
bool rightIntakeAhead = false; 
bool isPTO = false;
bool isWallUp = false;
bool isOn = false;
bool hang = false;
bool instaShoot = false;
int delay = 250;

void runIntakeForward() {
  intakeLeft = 127;
  intakeRight = -127;
}

void runIntakeBackward() {
  intakeLeft = -127;
  intakeRight = 127;
}

void stopIntake() {
  intakeLeft = 0;
  intakeRight = 0;
}

void wallUp() {
  wallLeft.set_value(true);
  wallRight.set_value(true);
}

void wallDown() {
  wallLeft.set_value(false);
  wallRight.set_value(false);
}

void intakeAhead() {
  intakeLeftPneumatic.set_value(true);
  intakeRightPneumatic.set_value(true);
}

void intakeRetract() {
  intakeLeftPneumatic.set_value(false);
  intakeRightPneumatic.set_value(false);
}

void shoot() {
  isShot = true;
};

void shootAndPullBackPunch() {
   while(true) {
    if (instaShoot) {
      puncher = 127;
    } else {
      if (isShot && limitSwitch.get_value()) {
        puncher = 127;
        pros::delay(100);
        isShot = false;
        puncher = 0;
      } else if (!isShot && limitSwitch.get_value()) {
        // if the puncher has punched and the limit switch is not hit, retract the puncher
        puncher = 0;
      } else if (!isShot && !limitSwitch.get_value()) {
        // if the puncher has not punched and the limit switch is hit, stop the motor
        puncher = 127;
      } 
    }  
    pros::delay(10);  // Delay to prevent wasting resources on the V5 Brain
  }
}
void setSpeed(int speed) {
  intakeLeft = -1 * speed;
  intakeRight = -1 * speed;
}

pros::Task Punch(shootAndPullBackPunch);

const int DRIVE_SPEED = 110; // This is 110/127 (around 87% of max speed).  We don't suggest making this 127.
                             // If this is 127 and the robot tries to heading correct, it's only correcting by
                             // making one side slower.  When this is 87%, it's correcting by making one side
                             // faster and one side slower, giving better heading correction.
const int TURN_SPEED  = 90;
const int SWING_SPEED = 90;
double tileDiagonal = 33.941125497;
/// 24√2

// Autons:
void offense() {
  // Blue/Red Ball
  chassis.set_drive_pid(5, DRIVE_SPEED);
  chassis.wait_drive();
  runIntakeForward();
  pros::delay(50);
  stopIntake();
  chassis.set_drive_pid(36, DRIVE_SPEED);
  chassis.wait_drive();
  chassis.set_swing_pid(ez::RIGHT_SWING, 90, SWING_SPEED);
  chassis.wait_drive();
  chassis.set_drive_pid(20, DRIVE_SPEED);
  runIntakeBackward();
  chassis.wait_drive();
  pros::delay(100);
  stopIntake();

  // Middle Ball
  chassis.set_drive_pid(-18, DRIVE_SPEED);
  chassis.wait_drive();
  chassis.set_turn_pid(-90, TURN_SPEED);
  chassis.wait_drive();
  
  chassis.set_drive_pid(6, 40);
  chassis.wait_drive();
  runIntakeForward();
  pros::delay(50);
  stopIntake();
  chassis.set_drive_pid(-10, DRIVE_SPEED);
  chassis.wait_drive();
  chassis.set_turn_pid(90, TURN_SPEED);
  chassis.wait_drive();
  chassis.set_drive_pid(20, DRIVE_SPEED);
  chassis.wait_drive();
  runIntakeBackward();
  chassis.set_drive_pid(10, DRIVE_SPEED);
  chassis.wait_drive();
  stopIntake();

  // Near Ball
  chassis.set_drive_pid(-20, DRIVE_SPEED);
  chassis.wait_drive();
  chassis.set_turn_pid(160, TURN_SPEED);
  chassis.wait_drive();
  
  chassis.set_drive_pid(20, DRIVE_SPEED, true);
  chassis.wait_until(15);
  chassis.set_max_speed(40); 
  
  runIntakeForward();
  pros::delay(50);
  stopIntake();

  chassis.set_drive_pid(-20, DRIVE_SPEED, true);
  chassis.wait_drive();
  chassis.set_turn_pid(-160, TURN_SPEED);
  chassis.wait_drive();
  chassis.set_drive_pid(20, DRIVE_SPEED);
  chassis.wait_drive();
  runIntakeBackward();
  chassis.set_drive_pid(10, DRIVE_SPEED);
  chassis.wait_drive();
  stopIntake();

  // Pole
  chassis.set_drive_pid(-20, DRIVE_SPEED);
  chassis.wait_drive();
  chassis.set_turn_pid(120, TURN_SPEED);
  chassis.wait_drive();
  chassis.set_drive_pid(1.5*tileDiagonal, DRIVE_SPEED);
  chassis.wait_drive();
};

void defense() {
  // Middle
  
  chassis.set_drive_pid(50, DRIVE_SPEED, true);
  chassis.wait_until(43);
  chassis.set_max_speed(40); 
  chassis.wait_drive();
  runIntakeForward();
  pros::delay(200);
  stopIntake();

  // Back and Shoot and Turn
  chassis.set_drive_pid(-38, DRIVE_SPEED);
  chassis.wait_drive();
  chassis.set_turn_pid(15, TURN_SPEED);
  chassis.wait_drive();
  shoot();
  pros::delay(100);
  chassis.set_turn_pid(180, TURN_SPEED);
  chassis.wait_drive();

  // Back get Corner Turn and Shoot
  chassis.set_drive_pid(tileDiagonal, DRIVE_SPEED, true);
  chassis.wait_drive();
  intakeAhead();
  pros::delay(100);
  runIntakeForward();
  pros::delay(1500);
  intakeRetract();
  pros::delay(100);
  stopIntake();
  chassis.set_drive_pid(-10, DRIVE_SPEED, true);
  chassis.wait_drive();
  chassis.set_turn_pid(180, TURN_SPEED);
  chassis.wait_drive();
  shoot();
  pros::delay(100);
  
  // Turn push 2 things
  chassis.set_turn_pid(55, TURN_SPEED);
  chassis.wait_drive();
  chassis.set_drive_pid(48, DRIVE_SPEED, true);
  chassis.wait_drive();

  // Pole
  chassis.set_turn_pid(90, TURN_SPEED);
  chassis.wait_drive();
  chassis.set_drive_pid(10, DRIVE_SPEED, true);
  chassis.wait_drive();
};

void awp() {
  

};

void skills() {
  
};

void skillsSafe() {
  
};

void nothing() {

};
/**
 * Runs initialization code. This occurs as soon as the program is started.
 *
 * All other competition modes are blocked by initialize; it is recommended
 * to keep execution time for this mode under a few seconds.
 */
void initialize() {
  
  // Print our branding over your terminal :D
  ez::print_ez_template();
  
  pros::delay(500); // Stop the user from doing anything while legacy ports configure.

  // Configure your chassis controls
  imu_sensor.reset();
  chassis.toggle_modify_curve_with_controller(true); // Enables modifying the controller curve with buttons on the joysticks
  chassis.set_active_brake(0.1); // Sets the active brake kP. We recommend 0.1.
  chassis.set_curve_default(0, 0); // Defaults for curve. If using tank, only the first parameter is used. (Comment this line out if you have an SD card!)  
  default_constants(); // Set the drive to your own constants from autons.cpp!
  exit_condition_defaults(); // Set the exit conditions to your own constants from autons.cpp!

  // These are already defaulted to these buttons, but you can change the left/right curve buttons here!
  // chassis.set_left_curve_buttons (pros::E_CONTROLLER_DIGITAL_LEFT, pros::E_CONTROLLER_DIGITAL_RIGHT); // If using tank, only the left side is used. 
  // chassis.set_right_curve_buttons(pros::E_CONTROLLER_DIGITAL_Y,    pros::E_CONTROLLER_DIGITAL_A);

  // Autonomous Selector using LLEMUdrive_and_turn
  ez::as::auton_selector.add_autons({
    Auton("Offense. Preload + 2 green in. Touch pole last.", offense),
    Auton("Defense. Start forward-face in middle of tile. In bewteen, then corner, then side, then pole.", defense),
    Auton("Do this to run nothing if autons are glitchy", nothing),
  });

  // Initialize chassis and auton selector
  chassis.initialize();
  ez::as::initialize();

  //My initializations
  chassis.set_drive_brake(pros::E_MOTOR_BRAKE_COAST);
  puncher.set_brake_mode(pros::E_MOTOR_BRAKE_HOLD);
  intakeLeft.set_brake_mode(pros::E_MOTOR_BRAKE_COAST);
  intakeRight.set_brake_mode(pros::E_MOTOR_BRAKE_COAST);
  Punch.resume();
}


/**
 * Runs while the robot is in the disabled state of Field Management System or
 * the VEX Competition Switch, following either autonomous or opcontrol. When
 * the robot is enabled, this task will exit.
 */
void disabled() {
  
}



/**
 * Runs after initialize(), and before autonomous when connected to the Field
 * Management System or the VEX Competition Switch. This is intended for
 * competition-specific initialization routines, such as an autonomous selector
 * on the LCD.
 *
 * This task will exit when the robot is enabled and autonomous or opcontrol
 * starts.
 */
void competition_initialize() {
  // . . .
}

void autonomous() {
  chassis.reset_pid_targets(); // Resets PID targets to 0
  chassis.reset_gyro(); // Reset gyro position to 0
  chassis.reset_drive_sensor(); // Reset drive sensors to 0
  chassis.set_drive_brake(pros::E_MOTOR_BRAKE_HOLD); // Set motors to hold.  This helps autonomous consistency.
  ez::as::auton_selector.call_selected_auton(); // Calls selected auton from autonomous selector.
}


// bool isOn = false; 

void opcontrol() {
  // This is preference to what you like to drive on.
  chassis.set_drive_brake(pros::E_MOTOR_BRAKE_COAST);
  puncher.set_brake_mode(pros::E_MOTOR_BRAKE_HOLD);
  intakeLeft.set_brake_mode(pros::E_MOTOR_BRAKE_COAST);
  intakeLeft.set_brake_mode(pros::E_MOTOR_BRAKE_COAST);


  Punch.resume();

  while (true) {
    chassis.tank(); // Tank control
    // chassis.arcade_standard(ez::SPLIT); // Standard split arcade
    // chassis.arcade_standard(ez::SINGLE); // Standard single arcade
    // chassis.arcade_flipped(ez::SPLIT); // Flipped split arcade
    // chassis.arcade_flipped(ez::SINGLE); // Flipped single arcade
    if (master.get_digital(pros::E_CONTROLLER_DIGITAL_R2)) {
      instaShoot = !instaShoot;
      if (instaShoot) {
        master.print(0, 0, "Instashoot ON");
      } else if (!instaShoot) {
        master.print(0, 0, "Instashoot OFF");
      };
      pros::delay(delay);
    }
    // Shoot
    if (master.get_digital(pros::E_CONTROLLER_DIGITAL_R1)) {
      shoot();
      pros::delay(delay);
    }

    // Toggle Intake Code
    // // Intake Boolean
    // if (master.get_digital(pros::E_CONTROLLER_DIGITAL_L1)) {
    //   isOn = true;
    //   intakeForward = true;  
    // } else if (master.get_digital(pros::E_CONTROLLER_DIGITAL_L2)) {
    //   isOn = true;
    //   intakeForward = false;
    // } 

    // // Reset Intake Spin (NOT Position)
    // if (master.get_digital(pros::E_CONTROLLER_DIGITAL_X)) {
    //   isOn = false;
    //   intakeForward = true;
    // }
    // // Intake Spinning
    // if (isOn) {
    //   if (intakeForward) {
    //   intakeLeft = 127;
    //   intakeRight = -127;
    //   } else if (!intakeForward) {
    //   intakeLeft = -127;
    //   intakeRight = 127;
    //   } 
    // } else {
    //   intakeLeft = 0;
    //   intakeRight = 0;
    // }

    // Hold Intake Code
    if (master.get_digital(pros::E_CONTROLLER_DIGITAL_L1)) {
      intakeLeft = 127;
      intakeRight = -127;
    } else if (master.get_digital(pros::E_CONTROLLER_DIGITAL_L2)) {
      intakeLeft = -127;
      intakeRight = 127;
    } else {
      intakeLeft = 0;
      intakeRight = 0;
    }

    // Intake Pneumatic Logic
    if (master.get_digital(pros::E_CONTROLLER_DIGITAL_UP) && !(leftIntakeAhead || rightIntakeAhead)) {
      intakeAhead();
      leftIntakeAhead = true;
      rightIntakeAhead = true;
      pros::delay(delay);
    } else if (master.get_digital(pros::E_CONTROLLER_DIGITAL_UP) && (leftIntakeAhead || rightIntakeAhead)) {
      intakeRetract();
      leftIntakeAhead = false;
      rightIntakeAhead = false;
      pros::delay(delay);
    } 
    if (master.get_digital(pros::E_CONTROLLER_DIGITAL_RIGHT)) {
      intakeRightPneumatic.set_value(true);
      rightIntakeAhead = true;
      pros::delay(delay);
    } 
    if (master.get_digital(pros::E_CONTROLLER_DIGITAL_LEFT)) {
      intakeLeftPneumatic.set_value(true);
      leftIntakeAhead = true;
      pros::delay(delay);
    }

    // WALL Logic
    if (master.get_digital(pros::E_CONTROLLER_DIGITAL_DOWN) && !isWallUp) {
      wallUp();
      isWallUp = true;
      pros::delay(delay);
    } else if (master.get_digital(pros::E_CONTROLLER_DIGITAL_DOWN) && isWallUp) {
      wallDown();
      isWallUp = false;
      pros::delay(delay);
    }
    
    // PTO Logic
    if (master.get_digital(pros::E_CONTROLLER_DIGITAL_Y) && !isPTO) {
      ptoLeft.set_value(true);
      ptoRight.set_value(true);
      isPTO = true;
      pros::delay(delay);
    } else if (master.get_digital(pros::E_CONTROLLER_DIGITAL_Y) && isPTO) {
      ptoRight.set_value(false);
      ptoRight.set_value(false);
      isPTO = false;
      pros::delay(delay);
    }

    pros::delay(ez::util::DELAY_TIME); // This is used for timer calculations!  Keep this ez::util::DELAY_TIME
  }
}
