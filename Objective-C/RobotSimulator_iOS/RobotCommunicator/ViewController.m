//
//  ViewController.m
//  RobotCommunicator
//
//  Created by Govind on 22/2/14.
//  Copyright (c) 2014 CYZ. All rights reserved.
//

#import "ViewController.h"

@interface ViewController ()

@property (nonatomic, weak) IBOutlet UIView *containerView;
@property (nonatomic, strong) RobotSim* bot;
@property (nonatomic, strong) NSNumberFormatter *numberFormatter;


@end

@implementation ViewController

@synthesize shutterLayer;
//@synthesize <#property#>

- (void)viewDidLoad
{
    [super viewDidLoad];
	// Do any additional setup after loading the view, typically from a nib.
    //create path
    self.bot = [[RobotSim alloc] init];
    self.bot.CommDelegate = self;
    
}

- (IBAction)still:(id)sender
{
    [ self animateShutter:true];
}

- (void) animateShutter: (bool) black_white
{
    

    
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}


- (void) handleRxedData: (CMMotionManager*)mm
{
    CMDeviceMotion *motion = mm.deviceMotion;
    CMMagnetometerData* mag = mm.magnetometerData;
    CMGyroData* gyro = mm.gyroData;
    NSLog(@"Agx = %f,Agy = %f,Agz = %f", motion.gravity.x,motion.gravity.y, motion.gravity.z);
    NSLog(@"Roll = %f,Pitch = %f,Yaw = %f",motion.attitude.roll,motion.attitude.pitch,motion.attitude.yaw );
    NSLog(@"Magx = %f ,Magy = %f,Magz = %f", mag.magneticField.x, mag.magneticField.y, mag.magneticField.z);
    NSLog(@"RRx = %f, RRy = %f, RRz = %f", gyro.rotationRate.x, gyro.rotationRate.y, gyro.rotationRate.z);
    
    self.lAux.text = [NSString stringWithFormat:@"%2.2f", motion.userAcceleration.x];
    self.lAuy.text = [NSString stringWithFormat:@"%2.2f", motion.userAcceleration.y];
    self.lAuz.text = [NSString stringWithFormat:@"%2.2f", motion.userAcceleration.z];
    
    self.lGx.text = [NSString stringWithFormat:@"%2.2f", gyro.rotationRate.x];
    self.lGy.text = [NSString stringWithFormat:@"%2.2f", gyro.rotationRate.y];
    self.lGz.text = [NSString stringWithFormat:@"%2.2f", gyro.rotationRate.z];
    
    self.lMx.text = [NSString stringWithFormat:@"%4.0f", mag.magneticField.x];
    self.lMy.text = [NSString stringWithFormat:@"%4.0f", mag.magneticField.y];
    self.lMz.text = [NSString stringWithFormat:@"%4.0f", mag.magneticField.z];
    
    self.lAx.text = [NSString stringWithFormat:@"%2.2f", motion.attitude.roll];
    self.lAy.text = [NSString stringWithFormat:@"%2.2f", motion.attitude.pitch];
    self.lAz.text = [NSString stringWithFormat:@"%2.2f", motion.attitude.yaw ];
}

// Other classes will post back info here.
- (void) handleGPSData:(CLLocationManager*)lm with:(CLLocation*)newLocation{
    
     if ([NSThread isMainThread]) {
         self.lSpeed.text = [NSString stringWithFormat:@"%3.5f", newLocation.speed];
         self.lLatitude.text = [NSString stringWithFormat:@"%3.5f", newLocation.coordinate.latitude];
         self.lLongitude.text = [NSString stringWithFormat:@"%3.5f", newLocation.coordinate.longitude];
         self.lAltitude.text = [NSString stringWithFormat:@"%3.5f", newLocation.altitude];
     }
     else{
         dispatch_async(dispatch_get_main_queue(), ^{
                 [self handleGPSData:lm with:newLocation];
         });
     }
    
}

@end

