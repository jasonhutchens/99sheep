//==============================================================================

#include <clump_manager.hpp>
#include <cloud.hpp>
#include <engine.hpp>

#include <hgeresource.h>


//------------------------------------------------------------------------------
Clump::Clump()
: m_clouds()
{
}

//------------------------------------------------------------------------------
Clump::~Clump()
{
	m_clouds.clear();
}

//------------------------------------------------------------------------------
void Clump::addCloud(Cloud* cloud)
{
	m_clouds.push_back(cloud);
}

//------------------------------------------------------------------------------
void Clump::removeCloud(Cloud* cloud)
{
	m_clouds.remove(cloud);
}


//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
ClumpManager::ClumpManager()
: m_interactions(),
  m_clumps(), 
  m_clumpPoints(0), 
  m_clumpPointsMultiplier(1), 
  m_isTopClusterFull(0),
  m_clearingClump(NULL),
  m_clearClumpInterval(0.0f),
  m_timeToNextClear(0.0f)
{
}


//------------------------------------------------------------------------------
ClumpManager::~ClumpManager() 
{
	fini();
}

//------------------------------------------------------------------------------
void 
ClumpManager::init()
{
	m_isTopClusterFull = false;
	m_clumpPoints = 0;
	m_clumpPointsMultiplier = 1; 
    m_clearingClump = NULL;
	m_clearClumpInterval = 0.0f;
	m_timeToNextClear = 0.0f;
}

//------------------------------------------------------------------------------
void joinEntities(Entity* entity, Entity* otherEntity, b2Vec2 position)
{
	b2RevoluteJointDef joint;
	joint.Initialize( entity->getBody(), otherEntity->getBody(), position);
	Engine::instance()->b2d()->CreateJoint( & joint );
	Engine::instance()->hge()->Effect_PlayEx( Engine::rm()->GetEffect( "thunder"), 30 );
}

//------------------------------------------------------------------------------
void joinClumps(Clump* bigClump, Clump* littleClump)
{
	while (littleClump->getClouds()->size() > 0)
	{
		Cloud* cloud = littleClump->getClouds()->back();

		cloud->setClump(bigClump);
		bigClump->addCloud(cloud);

		littleClump->getClouds()->pop_back();
	}
}

//------------------------------------------------------------------------------
void ClumpManager::update(float dt)
{
	//deal with any interactions that have occurred since the last update
	m_clumpPoints =0;
	while (m_interactions.size() > 0)
	{
		InteractionData& i = m_interactions.back();
		
		Clump* entClump = i.m_entity->getClump();
		Clump* otherClump = i.m_other->getClump();

		if (entClump == NULL && otherClump == NULL)
		{
			//neither entity is in a clump, so create a new clump and put them both in it
			Clump* newClump = new Clump();
			m_clumps.push_back(newClump);

			newClump->addCloud(i.m_entity);
			newClump->addCloud(i.m_other);
			i.m_entity->setClump(newClump);
			i.m_other->setClump(newClump);
			joinEntities(i.m_entity, i.m_other, i.m_collidePosition);

			// give clump starter multiplier
			m_clumpPointsMultiplier +=0.1f;
			// give some base points as well
			m_clumpPoints+= 1;
		}
		else if (entClump == NULL)
		{
			//first entity is not in a clump, but other entity is, so first needs to join other
			i.m_other->getClump()->addCloud(i.m_entity);
			i.m_entity->setClump(i.m_other->getClump());
			// determine how many items are in the clump already 
			joinEntities(i.m_entity, i.m_other, i.m_collidePosition);
			
			int clumpSize = otherClump->getClouds()->size();
			calcPoints(clumpSize, i.m_entity->getZoom());;



			
		}
		else if (otherClump == NULL)
		{
			//first entity is in a clump, but other entity is not, so other entity should join first entity's clump
			i.m_entity->getClump()->addCloud(i.m_other);
			i.m_other->setClump(i.m_entity->getClump());
			joinEntities(i.m_entity, i.m_other, i.m_collidePosition);
			
			int clumpSize = entClump->getClouds()->size();
			calcPoints(clumpSize, i.m_entity->getZoom());
		}
		else
		{
			//both entities already in clumps, need to combine clumps as long as the clumps are different
			if (entClump != otherClump)
			{
				if (entClump->getClouds()->size() > otherClump->getClouds()->size())
				{
					//all of the clouds in otherClump need to be put in entClump
					joinClumps(entClump, otherClump);
					int clumpSize = entClump->getClouds()->size();
					calcPoints(clumpSize, i.m_entity->getZoom());

					//now destroy the empty clump
					destroyClump(otherClump);
				}
				else
				{
					//all of the clouds in entClump need to be put into otherClump
					joinClumps(otherClump, entClump);
				
					int clumpSize = otherClump->getClouds()->size();
					calcPoints(clumpSize, i.m_entity->getZoom());
					//now destroy the empty clump

					destroyClump(entClump);
				}

				//make the actual physics connection between the two clouds that collided
				joinEntities(i.m_entity, i.m_other, i.m_collidePosition);
			}
		}


		m_interactions.pop_back();
	}


	//do any clump clearing - for removing all the clouds at game end
	m_timeToNextClear -= dt;
	if (m_clearingClump != NULL)
	{
		if (m_timeToNextClear < 0.0f)
		{
			if (m_clearingClump->getClouds()->size() > 0)
			{
				Cloud* cloud = m_clearingClump->getClouds()->back();
				//removeCloudFromClump(cloud, m_clearingClump);
				m_clearingClump->removeCloud(cloud);
				cloud->removeFromClump(false);
				cloud->removeFromWorld();
			}
			else
			{
				m_clearingClump = NULL;
			}

			m_timeToNextClear = m_clearClumpInterval;
		}
	}

}

//------------------------------------------------------------------------------
void ClumpManager::fini()
{
	//TODO: any cleanup
	m_interactions.clear();
	while (m_clumps.size() > 0)
	{
		delete m_clumps.back();
		m_clumps.pop_back();
	}
}

//------------------------------------------------------------------------------
void ClumpManager::reportCollision(Cloud* ent, Cloud* other, b2Vec2 collidePosition)
{
	//a new interaction has occurred, so save the data for now, and deal with it in our own update
	InteractionData data;
	data.m_entity = ent;
	data.m_other = other;
	data.m_collidePosition = collidePosition;
	m_interactions.push_back(data);
}

//------------------------------------------------------------------------------
void ClumpManager::removeCloudFromClump(Cloud* cloud, Clump* clump)
{
	clump->removeCloud(cloud);
	if (clump->getClouds()->size() == 1)
	{
		//one cloud does not a clump make. So tell it that it's no longer in a clump, and destroy the clump
		Cloud* cloud = clump->getClouds()->back();
		cloud->setClump(NULL);

		clump->getClouds()->pop_back();
	}

	if (clump->getClouds()->size() == 0)
	{
		destroyClump(clump);
	}
}

//------------------------------------------------------------------------------
void ClumpManager::startClearingClump(float time)
{
	//assumption: this is at game completion time, so there is only one clump 
	//left, with all clouds in it
	m_clearingClump = m_clumps.front();
	m_clearClumpInterval = (time / m_clearingClump->getClouds()->size());
	m_timeToNextClear = 0.001f;
}

//------------------------------------------------------------------------------
void ClumpManager::destroyClump(Clump* emptyClump)
{
	//destroy an empty clump
	if (emptyClump->getClouds()->size() == 0)
	{
		m_clumps.remove(emptyClump);
		delete emptyClump;
	}
}

float ClumpManager::getClumpMultiplier()
{
	return m_clumpPointsMultiplier;
}

int ClumpManager::getClumpPoints()
{
	return (int)(m_clumpPoints * m_clumpPointsMultiplier);
}

void ClumpManager::calcPoints( int sizeOfClump, int zoomLevel )
{
    float base( 1 + static_cast<float>(zoomLevel) / 2.0f );
	if(sizeOfClump <=5)
	{
		m_clumpPoints+= base;
		m_clumpPointsMultiplier +=0.1f;

	}
	else if(sizeOfClump >5 && sizeOfClump <= 12)
	{
		m_clumpPoints+= 5 * base;
		m_clumpPointsMultiplier +=0.2f;
	}
	else if (sizeOfClump >12 && sizeOfClump <= 22)
	{
		m_clumpPoints+= 10 * base;
		m_clumpPointsMultiplier +=0.03f;
	}
	else if (sizeOfClump >22 && sizeOfClump <= 32)
	{
		m_clumpPoints+= 15 * base;	
		m_clumpPointsMultiplier +=0.5f;
	}
	else if (sizeOfClump >32 && sizeOfClump <=39)
	{
		m_clumpPoints+= 25 * base;
		m_clumpPointsMultiplier +=0.8f;
	}
	else
	{
		// we have a 40 clump, are we on the top most zoom?
		if(zoomLevel == 0)
		{
			// we have finished, time to trigger the explosions and end the game
			m_clumpPoints+= 50 * base;
			m_clumpPointsMultiplier +=3.0f;
			m_isTopClusterFull= true;

		}
		else
		{

			// otherwise we need to give a bonus for a 40 clump
			m_clumpPoints+= 50 * base;
			m_clumpPointsMultiplier +=1.5f;
		}
	}
}

bool ClumpManager::isTopClusterFull()
{
		return m_isTopClusterFull;
}
//==============================================================================





