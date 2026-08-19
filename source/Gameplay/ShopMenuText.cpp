#include "ShopMenuText.h"
#include "AmmuNationShop.h"
#include "CMenuSystem.h"
#include <CText.h>
#include <Patch.h>

namespace
{
	constexpr uintptr_t MENU_ROW_TITLE_CALL_SITE = 0x58143E;
	constexpr float CONFIRM_NAME_COLUMN_WIDTH = 600.0f;
	const AmmuNationShop* g_shop = nullptr;

	tMenuPanel* panelHoldingKey(const char* t_key)
	{
		for (int panel = 0; panel < 2; ++panel)
		{
			tMenuPanel* menu = MenuNumber[panel];
			if (!menu) continue;

			const char* first = &menu->m_aaacRowTitles[0][0][0];
			if (t_key >= first && t_key < first + sizeof(menu->m_aaacRowTitles)) return menu;
		}
		return nullptr;
	}


	char* __fastcall onMenuRowTitle(CText* t_text, void*, char* t_key)
	{
		if (g_shop)
		{
			if (const char* replacement = g_shop->shopItemName(t_key))
			{
				tMenuPanel* menu = panelHoldingKey(t_key);
				if (menu && menu->m_nNumColumns == 2) menu->m_afColumnWidth[0] = CONFIRM_NAME_COLUMN_WIDTH;

				return const_cast<char*>(replacement);
			}
		}
		return const_cast<char*>(t_text->Get(t_key));
	}
}

void ShopMenuText::install(const AmmuNationShop& t_shop)
{
	if (g_shop) return;

	g_shop = &t_shop;
	plugin::patch::RedirectCall(MENU_ROW_TITLE_CALL_SITE, &onMenuRowTitle);
}
