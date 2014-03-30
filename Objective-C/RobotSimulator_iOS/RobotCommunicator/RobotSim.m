//
//  RobotSim.m
//  RobotCommunicator
//
//  Created by Govind on 22/2/14.
//  Copyright (c) 2014 CYZ. All rights reserved.
//

#import "RobotSim.h"

@interface RobotSim ()
@property (nonatomic,strong) CMMotionManager *motionManager;
@property (nonatomic,strong) CMMagnetometerData *magData;

@end

#define SIM_SPEED 0.5
@implementation RobotSim

- (id) init
{
    self = [super init]; // get an object from NSObject
    
    // Setup for Motion Manager (accelerometer and others)
    self.motionManager = [[CMMotionManager alloc] init];
    if (self.motionManager.isDeviceMotionAvailable) {
        NSLog(@"Device supports motion capture.");
        [self.motionManager startDeviceMotionUpdates]; // This will cause the motion data to be updated automatically
        if(self.motionManager.isMagnetometerAvailable){
            NSLog(@"Magneto is available!");
            [self.motionManager startMagnetometerUpdates];
        }
        if(self.motionManager.isGyroAvailable){
            NSLog(@"Gyro is available!");
            [self.motionManager startGyroUpdates];
        }
    }
    
    // Setup for Location Manager (GPS and others)
    self.locMgr = [[CLLocationManager alloc] init];
    self.locMgr.delegate = self;
    [self.locMgr startUpdatingLocation];
    
    // Register a timer to simulate a thread
   [NSTimer scheduledTimerWithTimeInterval:SIM_SPEED target:self selector:@selector(simTimerThread) userInfo:nil repeats:YES];
    
    // Create a Queue for Asynchronous Delegate example
    myQ = dispatch_queue_create("com.example.myQ", NULL);
    
    return(self);
}

- (void) simTimerThread
{
    NSLog(@"Tick");
    id delegate = [self CommDelegate];
    if ([delegate respondsToSelector:@selector(handleRxedData:)]) {
        [delegate handleRxedData:self.motionManager];
        // An alternateive that would work too for this case:
        //[delegate performSelectorOnMainThread:@selector(handleRxedData:) withObject:self.motionManager waitUntilDone:NO];
    }
}

// To access Core Location data, you need a delegate

- (void)dealloc{
    [self.motionManager stopDeviceMotionUpdates];
    [self.motionManager stopDeviceMotionUpdates];
    [self.locMgr stopUpdatingLocation];
}

- (void)locationManager:(CLLocationManager *)manager didUpdateToLocation:(CLLocation *)newLocation fromLocation:(CLLocation *)oldLocation {
    // Pass the information to GUI over a asynchronous delegate (GCD)
    dispatch_async(myQ, ^{
        id delegate = [self CommDelegate];
        if ([delegate respondsToSelector:@selector(handleGPSData:with:)]) {
            [delegate handleGPSData:manager with:newLocation];
        }
        
    });

}

- (void)locationManager:(CLLocationManager *)manager didFailWithError:(NSError *)error {

}


@end
