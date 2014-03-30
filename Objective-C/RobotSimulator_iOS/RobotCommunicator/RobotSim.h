//
//  RobotSim.h
//  RobotCommunicator
//
//  Created by Govind on 22/2/14.
//  Copyright (c) 2014 CYZ. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreMotion/CoreMotion.h>
#import <CoreLocation/CoreLocation.h>

/*
 Define the protocol that any delegate must conform to outside the interface definition
 */
@protocol RobotCommDelegate
// Required APIs go here:

- (void) handleRxedData: (CMMotionManager*) mm;
- (void) handleGPSData:(CLLocationManager*)lm with:(CLLocation*)newLocation;
@optional
// Optional APIs go here:

@end

@interface RobotSim : NSObject <CLLocationManagerDelegate>{
        dispatch_queue_t myQ;
}

// A property to hold the pointer to my delegate. delegates should be "weak" pointers
@property (nonatomic, weak) id <RobotCommDelegate> CommDelegate;
@property (nonatomic, retain) CLLocationManager *locMgr;
//@property (nonatomic, weak) id delegate;

@end
