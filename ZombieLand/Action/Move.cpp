#include "ZombieLand.h"
#include "ZombieLand/Action/Move.h"
#include "ZombieLand/Behaviour/Creature.h"

namespace action
{
	Move::Move()
	{
		mSpeed = 0.0f;
		mSound = 0;
		mChannel = 0;
	}

	Move::~Move()
	{
	}

	void	Move::SetSound(std::string sound)
	{
		mSound = Engine::GetSingleton()->GetResourceManager()->GetEffect(sound);
	}

	void	Move::SetSpeed(float speed)
	{
		mSpeed = speed;
	}

	float	Move::GetSpeed()
	{
		return mSpeed;
	}

	void	Move::OnThink(float elapsedTime)
	{
		EntityPtr e = GetBehaviour()->GetActor();
		Vector2 v(cos(e->GetAngle()) * mSpeed , sin(e->GetAngle()) * mSpeed);

		b2Body* body = ((Box2DBody*)e->GetBody())->GetBox2DBody();

		//body->SetLinearVelocity(v);

		v.x *= body->GetMass() * 10;
		v.y *= body->GetMass() * 10;
		body->ApplyForce(v, e->GetPosition());

		Vector2 cv = body->GetLinearVelocity();
		float fv = abs(cv.Length());
		if (fv > abs(mSpeed) && fv > 0.00001)
		{
			cv.x *= abs(mSpeed) / fv;
			cv.y *= abs(mSpeed) / fv;
			body->SetLinearVelocity(cv);
		}
	}

	void	Move::OnStart()
	{
		mChannel = Engine::GetSingleton()->GetSoundSystem()->PlayEffect(mSound, true);
	}

	void	Move::OnDone()
	{
		Engine::GetSingleton()->GetSoundSystem()->StopChannel(mChannel);

		EntityPtr e = GetBehaviour()->GetActor();
		Vector2 v(0.0f, 0.0f);

		e->GetBody()->SetLinearVelocity(v);
	}
};