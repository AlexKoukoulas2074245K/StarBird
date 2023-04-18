///------------------------------------------------------------------------------------------------
///  ObjectiveCUtils.mm
///  StarBird
///
///  Created by Alex Koukoulas on 01/02/2023
///------------------------------------------------------------------------------------------------

#include "ObjectiveCUtils.h"
#import <AudioToolbox/AudioServices.h>

///------------------------------------------------------------------------------------------------

void objectiveC_utils::Vibrate()
{
    AudioServicesPlaySystemSound(kSystemSoundID_Vibrate);
}

///------------------------------------------------------------------------------------------------

std::string objectiveC_utils::GetLocalFileSaveLocation()
{
    return std::string(getenv("HOME")) + "/Documents/";
}

///------------------------------------------------------------------------------------------------

std::string objectiveC_utils::BuildLocalFileSaveLocation(const std::string& fileName)
{
    return std::string(getenv("HOME")) + "/Documents/" + fileName;
}

///------------------------------------------------------------------------------------------------
