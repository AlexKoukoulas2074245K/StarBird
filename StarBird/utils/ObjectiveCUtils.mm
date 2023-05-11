///------------------------------------------------------------------------------------------------
///  ObjectiveCUtils.mm
///  StarBird
///
///  Created by Alex Koukoulas on 01/02/2023
///------------------------------------------------------------------------------------------------

#include "StringUtils.h"
#include "FileUtils.h"
#include "OSMessageBox.h"
#include "ObjectiveCUtils.h"
#import "AVAudioPlayerManager.h"
#import "SSZipArchive.h"
#import <AudioToolbox/AudioServices.h>
#import <AudioToolbox/AudioToolbox.h>
#import <AVFoundation/AVFoundation.h>

#include <SDL.h>
#include <SDL_syswm.h>

///------------------------------------------------------------------------------------------------

static AVAudioPlayerManager* manager;

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

void objectiveC_utils::PreloadSfx(const std::string& sfxResPath)
{
    if (strutils::StringEndsWith(sfxResPath, ".mp3") || strutils::StringEndsWith(sfxResPath, ".wav"))
    {
        ospopups::ShowMessageBox(ospopups::MessageBoxType::WARNING, "Sound Path contains extension", "Sound res path " + sfxResPath + " includes extension and will be ignored.");
        return;
    }
    
    NSString* objectiveCresPath = [NSString stringWithCString:sfxResPath.data() encoding:[NSString defaultCStringEncoding]];
    
    if (manager != nil)
    {
        [manager preloadSfxWith:objectiveCresPath];
    }
}

///------------------------------------------------------------------------------------------------

void objectiveC_utils::PlaySound(const std::string& soundResPath, const bool isMusic)
{
    if (strutils::StringEndsWith(soundResPath, ".mp3") || strutils::StringEndsWith(soundResPath, ".wav"))
    {
        ospopups::ShowMessageBox(ospopups::MessageBoxType::WARNING, "Sound Path contains extension", "Sound res path " + soundResPath + " includes extension and will be ignored.");
        return;
    }
    
    NSString* objectiveCresPath = [NSString stringWithCString:soundResPath.data() encoding:[NSString defaultCStringEncoding]];
    
    if (manager != nil)
    {
        [manager playSoundWith:objectiveCresPath isMusic:isMusic];
    }
}

///------------------------------------------------------------------------------------------------

void objectiveC_utils::InitAudio()
{
    manager = [[AVAudioPlayerManager alloc] init];
}

///------------------------------------------------------------------------------------------------

void objectiveC_utils::ResumeAudio()
{
    if (manager != nil)
    {
        [manager resumeAudio];
    }
}

///------------------------------------------------------------------------------------------------

void objectiveC_utils::PauseAudio()
{
    if (manager != nil)
    {
        [manager pauseAudio];
    }
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
