///------------------------------------------------------------------------------------------------
///  ObjectiveCUtils.mm
///  StarBird
///
///  Created by Alex Koukoulas on 01/02/2023
///------------------------------------------------------------------------------------------------

#include "ObjectiveCUtils.h"
#import <AudioToolbox/AudioServices.h>

///------------------------------------------------------------------------------------------------

void Vibrate()
{
    AudioServicesPlaySystemSound(kSystemSoundID_Vibrate);
}

///------------------------------------------------------------------------------------------------