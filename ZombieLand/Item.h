#ifndef ZOMBIELAND_ITEM_H
#define ZOMBIELAND_ITEM_H

#include "Definitions.h"
#include <guichan.hpp>

class Item : public boost::enable_shared_from_this<Item>
{
	friend class Inventory;
public:
	enum Slot
	{
		Other = 0,
		Weapon = 1,
		Armor = 2,
		Spell = 3,
		Ammo
	};
	Item();
	virtual ~Item();

	/// ������������ �������
	void	UseBy(EntityPtr actor);

	/// ���������� ����, � ������� ����������� �������
	Slot	GetSlot();

	/// ���������� ���� �������������
	HEFFECT	GetUseSound();

	/// ������������� ���� �������������
	void	SetUseSound(std::string sound);

	/// ���������� �����, ������������� �� �������������
	float	GetUsingTime();

	/// ���������� ����������������� ����������� ��������
	float	GetReloadingTime();

	/// ������������� ����������������� ������������� ��������
	void	SetUsingTime(float time);

	/// ������������� ����������������� �����������
	void	SetReloadingTime(float time);

	/// ���������� ��� ��������
	std::string	GetTag();

	void	SetTag(std::string tag);

	/// ���������� ������ ��������
	gcn::Image*	GetIcon();

	void	SetIcon(std::string fileName);

	bool	IsInfinity();

	void	SetInfinity(bool flag);

	int		GetAmmo();

	int		GetMaxAmmo();

	void	SetAmmo(int count);

	void	SetMaxAmmo(int count);

	InventoryPtr GetInventory();
protected:
	virtual void	OnAdd();
	virtual void	OnEquip();
	virtual void	OnUnequip();
	virtual void	OnUse(EntityPtr actor);
	void	SetSlot(Slot type);
	
private:
	int mAmmo;
	int mMaxAmmo;
	bool	isInfinity;
	std::string mName;
	std::string mDescription;
	std::string mIcon;
	std::string mTag;
	bool isQuestItem;
	Slot	mSlot;

	float	mUsingTime;
	float	mReloadingTime;

	HEFFECT	mUseSound;
	gcn::Image*	mIconImage;
	InventoryWPtr mInventory;
};

#endif