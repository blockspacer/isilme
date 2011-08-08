#include "ZombieLand.h"
#include "Die.h"

namespace action
{
	Die::Die()
	{
		SetEndingTime(0.0f);
		SetBeginningTime(0.0f);
		SetDoingTime(1.0f);
		mEffect = 0;
	}

	Die::~Die()
	{
	}

	void	Die::OnDone()
	{
		GetActor()->GetBehaviour()->SetActive(false);
	}

	void	Die::OnStart()
	{
		GetActor()->GetBody()->SetActive(false);

		if (mEffect != 0)
		Engine::GetSingleton()->GetSoundSystem()->PlayEffect(mEffect);

		//GetLevel()->GetLayer("Grass")->Add(GetActor());
	}

	void	Die::SetSound(std::string sound)
	{
		mEffect = Engine::GetSingleton()->GetResourceManager()->GetEffect(sound);
	}
};