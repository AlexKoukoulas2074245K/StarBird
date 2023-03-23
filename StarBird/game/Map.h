///------------------------------------------------------------------------------------------------
///  Map.h                                                                                          
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 22/03/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef Map_h
#define Map_h

///------------------------------------------------------------------------------------------------

#include "../utils/MathUtils.h"
#include "../utils/StringUtils.h"

#include <map>
#include <unordered_set>

///------------------------------------------------------------------------------------------------

struct MapCoord
{
    MapCoord(const int col, const int row)
        : mCol(col)
        , mRow(row)
    {
    }
    
    std::string ToString() const { return std::to_string(mCol) + "," + std::to_string(mRow); }
    
    int mCol;
    int mRow;
};

inline bool operator < (const MapCoord& lhs, const MapCoord& rhs)
{
    if (lhs.mCol == rhs.mCol)
    {
        return lhs.mRow < rhs.mRow;
    }
    else
    {
        return lhs.mCol < rhs.mCol;
    }
}

inline bool operator == (const MapCoord& lhs, const MapCoord& rhs)
{
    return lhs.mCol == rhs.mCol && lhs.mRow == rhs.mRow;
}

struct MapCoordHasher
{
    std::size_t operator()(const MapCoord& key) const
    {
        return strutils::StringId(key.ToString()).GetStringId();
    }
};

///------------------------------------------------------------------------------------------------

class Scene;
class Map final
{
public:
    enum class NodeType
    {
        NORMAL_ENCOUNTER, BOSS_ENCOUNTER
    };
    
    struct NodeData
    {
        NodeType mNodeType;
        glm::vec3 mPosition;
        std::unordered_set<MapCoord, MapCoordHasher> mNodeLinks;
    };
    
public:
    Map(Scene& scene, const glm::ivec2& mapDimensions, const MapCoord& currentMapCoord, const bool singleEntryPoint);
    
    const std::map<MapCoord, Map::NodeData>& GetMapData() const;
    
private:
    void GenerateMapData();
    void CreateMapSceneObjects();
    bool DetectedCrossedEdge(const MapCoord& currentCoord, const MapCoord& targetTestCoord) const;
    glm::vec3 GenerateNodePositionFromCoord(const MapCoord& mapCoord) const;
    MapCoord RandomlySelectNextMapCoord(const MapCoord& currentMapCoord) const;
    
private:
    Scene& mScene;
    const glm::ivec2 mMapDimensions;
    const MapCoord mCurrentMapCoord;
    const bool mHasSingleEntryPoint;
    std::map<MapCoord, NodeData> mMapData;
};


///------------------------------------------------------------------------------------------------

#endif /* Map_h */
