#include "OPSystem.h"

namespace ohday
{
	OPSystem::OPSystem(void)
	{
		numParticles_ = 0;
		currentTime_ = 0;
		bAlive_ = -1;
	}

	OPSystem::OPSystem(int numParticles, float deltaParticleTime)
	{
		numParticles_ = numParticles;
		currentTime_ = 0;
		bAlive_ = -1;

		particles_.resize(numParticles);
		aliveParticles_.reserve(numParticles);
		deadParticles_.reserve(numParticles);

		emitDeltaTime_ = deltaParticleTime;


		for(int i = 0; i < particles_.size(); i++)
		{
			particles_[i].lifeTime_ = -1;
			particles_[i].initialTime_ = 0;
		}
	}

	OPSystem::~OPSystem(void)
	{

	}

	void OPSystem::InitialSystem()
	{
		bAlive_ = 1;
		currentTime_ = 0;
	}

	void OPSystem::CloseSystem()
	{
		bAlive_ = -1;
		currentTime_ = 0;
	}

	void OPSystem::Update(float dt)
	{
		currentTime_ += dt;

		aliveParticles_.clear();
		deadParticles_.clear();

		for(int i = 0; i < numParticles_; i++)
		{

			if(particles_[i].bAlive_ > 0 && (currentTime_ - particles_[i].initialTime_ < particles_[i].lifeTime_))
				aliveParticles_.push_back(i);
			else
				deadParticles_.push_back(i);
		}

		// 只有在系统激活时，才发射粒子
		if(emitDeltaTime_ > 0 && bAlive_ > 0)
		{
			static float timeAccum = 0;
			timeAccum += dt;
			while(timeAccum > emitDeltaTime_)
			{
				timeAccum -= emitDeltaTime_;
				AddParticle();
			}
		}
	}

	void OPSystem::AddParticle()
	{
		if(deadParticles_.empty())
			return;

		int index = deadParticles_.back();

		InitialParticle(particles_[index]);

		deadParticles_.pop_back();
		aliveParticles_.push_back(index);
	}


}
