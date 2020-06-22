#pragma once
#include "../driver/driver.h"
#include "../helpers/settings.h"
#include "../renderer/renderer.h"
#include <locale>
#include <codecvt>
#include "offsets.h"
#include "../helpers/settings.h"

static std::string to_string(const std::wstring& wstr)
{
	std::wstring_convert< std::codecvt_utf8<wchar_t>, wchar_t > converter;
	return converter.to_bytes(wstr);
}

enum entity_type
{
	BasePlayer,
	StorageContainer,
	SupplyDrop,
	VendingMachineMapMarker,
	ExcavatorArm,
	LightListener,
	BaseVehicleSeat,
	Boar,
	Bear,
	Deer,
	Chicken,
	Wolf,
	Horse,
	HotAirBalloon,
	MiniCopter,
	DroppedItemContainer,
	OreResourceEntity,
	ResourceEntity,
	StashContainer,
	DroppedItem,
	CollectibleEntity,
	BaseMelee,
	BaseProjectile,
	TreeEntity,
	OreHotSpot,
	LootContainer,
	FreeableLootContainer,
	Recycler,
	JunkPileWater,
	Keycard,
	HackableLockedCrate,
	ItemPickup,
	Undefined,
	WorldItem,
	BradleyAPC,
	PlayerCorpse,
	BaseHelicopter,
	Scientist,
	GunTrap,
	FlameTurret,
	Landmine,
	BearTrap,
	AutoTurret,
	Barricade,
	RidableHorse
};

namespace game
{
	extern uintptr_t game_assembly, unity_player, camera_instance;
	extern uintptr_t base_networkable, local_player, local_pos_component;

	extern uintptr_t buffer_list, object_list;
	extern std::mutex entity_mutex, draw_mutex, local_mutex;
	extern std::vector<std::pair<uintptr_t, entity_type>>draw_list;

	bool get_networkable();
	
	bool get_buffer_list();
	
	bool get_object_list();
		
	int get_object_list_size();

	uintptr_t get_base_player(uintptr_t object);

	std::string get_class_name(uintptr_t object);

	uintptr_t get_object_pos_component(uintptr_t entity, bool esp_driver = false);

	vec3_t get_object_pos(uintptr_t entity, bool esp_driver = false);

	matrix4x4 get_view_matrix(bool esp_driver = false);

	bool world_to_screen(const vec3_t& entity_pos, vec_t& screen_pos, bool esp_driver = false);

	bool set_admin();
	
	uintptr_t get_active_item(uintptr_t _local_player);

	int get_item_id(uintptr_t item);
	
	std::wstring get_item_name(uintptr_t item);
	
	bool get_recoil_properties(uintptr_t weapon, std::string name);

	uintptr_t get_active_weapon(uintptr_t _local_player);

	bool set_automatic(uintptr_t weapon);

	bool set_recoil_props(uintptr_t weapon);
}
