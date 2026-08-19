#pragma once
#include <array>
#include <string>

struct ShopItemInfo
{
	const char* gxtKey;
	const char* displayName;
};

inline const std::array<ShopItemInfo, 16> shopItems = { {
	{ "PISTOL",  "Pistol" },
	{ "PISTSL",  "Silenced Pistol" },
	{ "DEAGLE",  "Desert Eagle" },
	{ "GRENADE", "Grenade" },
	{ "SHOTGN1", "Shotgun" },
	{ "SHOTGN2", "Combat Shotgun" },
	{ "SHOTGN3", "Sawn-off Shotgun" },
	{ "UZI",     "Micro Uzi" },
	{ "TEC9",    "Tec-9" },
	{ "MP5",     "MP5" },
	{ "AK",      "AK-47" },
	{ "M4",      "M4" },
	{ "RIFLE",   "Country Rifle" },
	{ "SNIPE",   "Sniper Rifle" },
	{ "DETONA",  "Satchel Charge" },
	{ "ARMOUR",  "Armor" },
} };

class AmmuNationShop
{
public:
	void update();
	int pollPurchasedSlot();
	void setSlotContents(int t_slot, const std::string& t_text);
	void setSlotSold(int t_slot, bool t_sold);
	void setSlotFlags(int t_slot, int t_flags);
	const char* shopItemName(const char* t_gxtKey) const;
	int shopItemFlags(const char* t_gxtKey) const;


private:
	bool isShopScriptActive() const;
	void lockSoldRows() const;
	void updateArmourCeiling();
	void allowArmourPurchase();
	void restoreMaxArmour();
	void snapshotPlayerState();
	void restorePlayerState(bool t_refundMoney);
	int slotForKey(const char* t_gxtKey) const;

	std::array<std::string, 16> m_slotContents;
	std::array<int, 16> m_slotFlags{};
	std::array<bool, 16> m_slotSold{};

	int m_pendingPurchasedSlot = -1;
	int m_confirmPanelItem = -1;
	bool m_boughtVisible = false;

	struct WeaponSlotSnapshot
	{
		int type = 0;
		unsigned int clip = 0;
		unsigned int total = 0;
	};
	std::array<WeaponSlotSnapshot, 13> m_weaponSnapshot{};
	float m_armourSnapshot = 0.0f;
	int m_moneySnapshot = 0;
	bool m_maxArmourRaised = false;
	unsigned char m_maxArmourOriginal = 0;
	bool m_snapshotValid = false;
};
