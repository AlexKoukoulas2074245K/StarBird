///------------------------------------------------------------------------------------------------
///  PersistenceUtils.h                                                                                          
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 24/04/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef PersistenceUtils_h
#define PersistenceUtils_h

///------------------------------------------------------------------------------------------------

namespace persistence_utils
{
    bool ProgressSaveFileExists();
    void LoadFromProgressSaveFile();
    void GenerateNewProgressSaveFile();
    void BuildProgressSaveFile();
}

///------------------------------------------------------------------------------------------------

#endif /* PersistenceUtils_h */
