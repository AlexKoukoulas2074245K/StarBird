///------------------------------------------------------------------------------------------------
///  Game.h                                                                                          
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 27/01/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef Game_h
#define Game_h

///------------------------------------------------------------------------------------------------

#include <string>

///------------------------------------------------------------------------------------------------

class Game final
{
public:
    Game();
    ~Game();
    
private:
    bool InitSystems();
    void Run();
    
private:
    void OnTextInput(const std::string& text);
    
private:
    bool mIsFinished;
};

///------------------------------------------------------------------------------------------------

#endif /* Game_h */
