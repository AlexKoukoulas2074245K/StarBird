///------------------------------------------------------------------------------------------------
///  AVAudioPlayerManager.mm
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 10/05/2023                                                       
///------------------------------------------------------------------------------------------------

#import "AVAudioPlayerManager.h"

///------------------------------------------------------------------------------------------------

@implementation AVAudioPlayerManager

///------------------------------------------------------------------------------------------------

-(id) init
{
    self = [super init];
    _sfxPlayers = [[NSMutableDictionary alloc] init];
    _musicPlayer = nil;
    _firstAppStateCall = YES;
    return self;
}

///------------------------------------------------------------------------------------------------

- (void) preloadSfxWith:(NSString*) sfxResPath
{
    NSString* sandboxFilePath = [[NSBundle mainBundle] pathForResource:sfxResPath ofType:@"mp3"];

    if (sandboxFilePath != nil)
    {
        if ([_sfxPlayers objectForKey:sandboxFilePath] == nil)
        {
            NSError *error;
            AVAudioPlayer* sfxPlayer = [[AVAudioPlayer alloc] initWithContentsOfURL:[NSURL fileURLWithPath:sandboxFilePath] error:&error];
            
            if (error)
            {
                NSLog(@"%@",[error localizedDescription]);
            }
            
            sfxPlayer.numberOfLoops = 0;
            [sfxPlayer prepareToPlay];
            
            _sfxPlayers[sandboxFilePath] = sfxPlayer;
        }
    }
}

///------------------------------------------------------------------------------------------------

- (void) playSoundWith: (NSString*) soundResPath isMusic:(BOOL) isMusic
{
    NSString* sandboxFilePath = [[NSBundle mainBundle] pathForResource:soundResPath ofType:@"mp3"];

    if (sandboxFilePath != nil)
    {
        NSError *error;
        
        if (isMusic)
        {
            if (_musicPlayer == nil || [_musicPlayer isPlaying])
            {
                if (_musicPlayer != nil)
                {
                    [_musicPlayer stop];
                }
                
                _musicPlayer = [[AVAudioPlayer alloc] initWithContentsOfURL:[NSURL fileURLWithPath:sandboxFilePath] error:&error];
                
                if (error)
                {
                    NSLog(@"%@",[error localizedDescription]);
                }
                
                _musicPlayer.numberOfLoops = -1;
                [_musicPlayer play];
                _musicPlayer.volume = 0.07f;
            }
        }
        else
        {
            AVAudioPlayer* targetSfxPlayer = [_sfxPlayers objectForKey:sandboxFilePath];
            if (targetSfxPlayer)
            {
                dispatch_queue_t backgroundQueue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_BACKGROUND, 0);
                dispatch_async(backgroundQueue, ^{
                    targetSfxPlayer.currentTime = 0;
                    [targetSfxPlayer play];
                });
            }
            else
            {
                NSLog(@"Can't find audio player for %@", sandboxFilePath);
            }
        }
    }
    else
    {
        NSLog(@"Can't open sound file %@", sandboxFilePath);
    }
}

///------------------------------------------------------------------------------------------------

- (void) pauseAudio
{
    for(id key in _sfxPlayers)
    {
        AVAudioPlayer* sfxPlayer = [_sfxPlayers objectForKey:key];
        [sfxPlayer pause];
    }

    
    if (_musicPlayer != nil)
    {
        [_musicPlayer pause];
    }
}

///------------------------------------------------------------------------------------------------

- (void) resumeAudio
{
    if (_firstAppStateCall)
    {
        _firstAppStateCall = NO;
    }
    else
    {
        for(id key in _sfxPlayers)
        {
            AVAudioPlayer* sfxPlayer = [_sfxPlayers objectForKey:key];
            [sfxPlayer play];
        }
        
        if (_musicPlayer != nil)
        {
            [_musicPlayer play];
        }
    }
}


///------------------------------------------------------------------------------------------------

@end
