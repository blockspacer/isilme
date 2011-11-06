#ifndef AI_AGRESSION_PACKAGE_H
#define AI_AGRESSION_PACKAGE_H

#include <Isilme.h>
#include <Engine/include/AIPackage.h>
#include "ZombieLand/Include/Definitions.h"
#include "ZombieLand/Action/AttackTarget.h"

/// ����� ��, ���������� �� ��������� �� ������ �������
class AgressionPackage : public AIPackage
{
public:
	AgressionPackage();
	virtual ~AgressionPackage();

	/// ���������, ���� �� ���������� ��������� ����������� ������
	virtual bool CheckCondition(BehaviourPtr behaviour);

	/// ������� � ����������� �������� ����� ����
	virtual ActionPtr CreateAction();
protected:
private:
};

#endif