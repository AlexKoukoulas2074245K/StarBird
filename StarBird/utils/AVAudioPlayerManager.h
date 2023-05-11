///------------------------------------------------------------------------------------------------
///  AVAudioPlayerManager.h                                                                                          
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 10/05/2023                                                       
///------------------------------------------------------------------------------------------------

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>

///------------------------------------------------------------------------------------------------

@interface AVAudioPlayerManager : NSObject

@property NSMutableDictionary* sfxPlayers;
@property AVAudioPlayer* musicPlayer;
@property BOOL firstAppStateCall;

- (id) init;
- (void) preloadSfxWith:(NSString*) sfxResPath;
- (void) playSoundWith:(NSString*) soundResPath isMusic:(BOOL) isMusic;
- (void) pauseAudio;
- (void) resumeAudio;

@end

///------------------------------------------------------------------------------------------------

