//
//  ViewController.h
//  RobotCommunicator
//
//  Created by Govind on 22/2/14.
//  Copyright (c) 2014 CYZ. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <CoreMotion/CoreMotion.h>
#import "RobotSim.h"

@interface ViewController : UIViewController <RobotCommDelegate>


@property(nonatomic, weak) IBOutlet UIButton *captureImageButton;
@property(nonatomic, weak) CAShapeLayer *shutterLayer;

@property (nonatomic,weak) IBOutlet UILabel *lAux;
@property (nonatomic,weak) IBOutlet UILabel *lAuy;
@property (nonatomic,weak) IBOutlet UILabel *lAuz;

@property (nonatomic,weak) IBOutlet UILabel *lMx;
@property (nonatomic,weak) IBOutlet UILabel *lMy;
@property (nonatomic,weak) IBOutlet UILabel *lMz;

@property (nonatomic,weak) IBOutlet UILabel *lGx;
@property (nonatomic,weak) IBOutlet UILabel *lGy;
@property (nonatomic,weak) IBOutlet UILabel *lGz;

@property (nonatomic,weak) IBOutlet UILabel *lAx;
@property (nonatomic,weak) IBOutlet UILabel *lAy;
@property (nonatomic,weak) IBOutlet UILabel *lAz;

@property (nonatomic,weak) IBOutlet UILabel *lSpeed;
@property (nonatomic,weak) IBOutlet UILabel *lLatitude;
@property (nonatomic,weak) IBOutlet UILabel *lLongitude;
@property (nonatomic,weak) IBOutlet UILabel *lAltitude;

@end
