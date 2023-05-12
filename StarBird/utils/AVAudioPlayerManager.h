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
@property NSString* currentPlayingMusicPath;
@property NSString* nextQueuedMusicPath;

- (id) init;
- (void) preloadSfxWith:(NSString*) sfxResPath;
- (void) playSoundWith:(NSString*) soundResPath isMusic:(BOOL) isMusic forceLoop:(BOOL) forceLoop;
- (void) pauseMusic;
- (void) pauseSfx;
- (void) pauseAudio;
- (void) resumeAudio;
- (void) updateAudioWith:(float) dtMillis;

@end

///------------------------------------------------------------------------------------------------

