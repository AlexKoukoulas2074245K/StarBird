///------------------------------------------------------------------------------------------------
///  ObjectiveCUtils.mm
///  StarBird
///
///  Created by Alex Koukoulas on 01/02/2023
///------------------------------------------------------------------------------------------------

#include "ObjectiveCUtils.h"
#import "SSZipArchive.h"
#import <AudioToolbox/AudioServices.h>

///------------------------------------------------------------------------------------------------

void objectiveC_utils::UnzipAssets(const char* zippedFolderPath, const char* resFolderPath)
{
    NSString* zipPath = [NSString stringWithCString:zippedFolderPath
                                       encoding:[NSString defaultCStringEncoding]];
    NSString* resPath = [NSString stringWithCString:resFolderPath
                                           encoding:[NSString defaultCStringEncoding]];
    
    [SSZipArchive unzipFileAtPath:zipPath toDestination:resPath];
}

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
