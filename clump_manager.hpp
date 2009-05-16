//==============================================================================

#ifndef ArseClumpManager
#define ArseClumpManager

#pragma once

#include <vector>
#include <list>
#include <Box2D.h>

class Cloud;

//------------------------------------------------------------------------------
class Clump
{
public:
	Clump();
	virtual ~Clump();

	void addCloud(Cloud* cloud);
	void removeCloud(Cloud* cloud);
	std::list< Cloud *>* getClouds() { return &m_clouds; }

private:
	Clump( const Clump & );
    Clump & operator=( const Clump & );

	std::list< Cloud *> m_clouds;

};


//------------------------------------------------------------------------------
class ClumpManager
{
public:
	ClumpManager();
	virtual ~ClumpManager();

    void init();
    void update( float dt );
    void fini();

	void reportCollision(Cloud* ent, Cloud* other, b2Vec2 collidePosition);
	void removeCloudFromClump(Cloud* cloud, Clump* clump);
	void startClearingClump(float time);
	float getClumpMultiplier();
	int getClumpPoints();
	
	bool isTopClusterFull();


private:
    ClumpManager( const ClumpManager & );
    ClumpManager & operator=( const ClumpManager & );
	float m_clumpPointsMultiplier;
	float m_clumpPoints;
	bool m_isTopClusterFull;

	void destroyClump(Clump* emptyClump);
	void calcPoints(int sizeOfClump, int zoomLevel);
	struct InteractionData
	{
		Cloud* m_entity;
		Cloud* m_other;
		b2Vec2 m_collidePosition;
	};

	std::vector<InteractionData> m_interactions;
	std::list< Clump *> m_clumps;
	Clump* m_clearingClump;
	float m_clearClumpInterval;
	float m_timeToNextClear;
};

#endif

//==============================================================================