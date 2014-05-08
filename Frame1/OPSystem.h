#ifndef OHDAY_PSYSTEM
#define OHDAY_PSYSTEM
#include "OUtilities.h"

namespace ohday
{
	class OPSystem
	{
	public:
		OPSystem(void);
		~OPSystem(void);
		OPSystem(int numParticles, float deltaParticleTime);

		virtual void InitialSystem();
		virtual void CloseSystem();

		virtual void Update(float dt);
		virtual void AddParticle();

		virtual void InitialParticle(OParticle& p) = 0;

		bool bAlive_;
		float currentTime_;
		IDirect3DVertexBuffer9 *vertexBuffer_;
		IDirect3DTexture9 *texture_;

		vector<OParticle> particles_;
		vector<int> aliveParticles_;
		vector<int> deadParticles_;
	protected:


		float emitDeltaTime_;
		int numParticles_;





	};

	class ORainPSystem : public OPSystem
	{
	public:
		ORainPSystem(int numParticles, float deltaParticleTime)
			: OPSystem(numParticles, deltaParticleTime)
		{}

		void InitialParticle(OParticle& p)
		{
			RandomDevice randomDevice;

			p.initialPos_.x = randomDevice.GetFloatLine(-30, 30);
			p.initialPos_.z = randomDevice.GetFloatLine(-30, 30);
			p.initialPos_.y = randomDevice.GetFloatLine(40, 45);

			p.initialVel_.x = randomDevice.GetFloatLine(-1.5, 1.5);
			p.initialVel_.z = randomDevice.GetFloatLine(-1.5, 1.5);
			p.initialVel_.y = randomDevice.GetFloatLine(-60, -50);
//			p.initialVel_.w = randomDevice.GetFloatLine(1, 2);
//			p.initialVel_.w = 0;

			p.initialSize_ = randomDevice.GetFloatLine(10, 11);

			p.initialTime_ = currentTime_;
//			p.initialTime_ = 3.0f;

			p.lifeTime_ = randomDevice.GetFloatLine(1.0f, 1.5f);

			p.bAlive_ = 1.0f;
		}

	};

}



#endif


