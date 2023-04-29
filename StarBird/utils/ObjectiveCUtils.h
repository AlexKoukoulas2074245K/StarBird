///------------------------------------------------------------------------------------------------
///  ObjectiveCUtils.h
///  StarBird
///
///  Created by Alex Koukoulas on 01/02/2023
///------------------------------------------------------------------------------------------------

#ifndef ObjectiveCUtils_h
#define ObjectiveCUtils_h

///------------------------------------------------------------------------------------------------

#include <string>

///------------------------------------------------------------------------------------------------

namespace objectiveC_utils
{
    void UnzipAssets(const char* zippedFolderPath, const char* resFolderPath);
    void Vibrate();
    std::string GetLocalFileSaveLocation();
    std::string BuildLocalFileSaveLocation(const std::string& fileName);
}

///------------------------------------------------------------------------------------------------

#endif /* ObjectiveCUtils_h */
