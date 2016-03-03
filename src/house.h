/**
 * The Forgotten Server - a free and open-source MMORPG server emulator
 * Copyright (C) 2015  Mark Samman <mark.samman@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef FS_HOUSE_H_EB9732E7771A438F9CD0EFA8CB4C58C4
#define FS_HOUSE_H_EB9732E7771A438F9CD0EFA8CB4C58C4

#include <regex>

#include "container.h"
#include "housetile.h"
#include "position.h"

class House;
class BedItem;
class Player;

class AccessList
{
	public:
		void parseList(const std::string& _list);
		void addPlayer(const std::string& name);
		void addGuild(const std::string& name);
		void addExpression(const std::string& expression);

		bool isInList(const Player* player);

		void getList(std::string& _list) const;

	private:
		std::string list;
		std::unordered_set<uint32_t> playerList;
		std::unordered_set<uint32_t> guildList; // TODO: include ranks
		std::list<std::string> expressionList;
		std::list<std::pair<std::regex, bool>> regExList;
};

class Door final : public Item
{
	public:
		explicit Door(uint16_t _type);
		~Door();

		// non-copyable
		Door(const Door&) = delete;
		Door& operator=(const Door&) = delete;

		Door* getDoor() final {
			return this;
		}
		const Door* getDoor() const final {
			return this;
		}

		House* getHouse() {
			return house;
		}

		//serialization
		Attr_ReadValue readAttr(AttrTypes_t attr, PropStream& propStream) final;
		void serializeAttr(PropWriteStream&) const final {}

		void setDoorId(uint32_t _doorId) {
			setIntAttr(ITEM_ATTRIBUTE_DOORID, _doorId);
		}
		uint32_t getDoorId() const {
			return getIntAttr(ITEM_ATTRIBUTE_DOORID);
		}

		bool canUse(const Player* player);

		void setAccessList(const std::string& textlist);
		bool getAccessList(std::string& list) const;

		void onRemoved() final;

	protected:
		void setHouse(House* _house);

	private:
		House* house;
		AccessList* accessList;
		friend class House;
};

enum AccessList_t {
	GUEST_LIST = 0x100,
	SUBOWNER_LIST = 0x101,
};

enum AccessHouseLevel_t {
	HOUSE_NOT_INVITED = 0,
	HOUSE_GUEST = 1,
	HOUSE_SUBOWNER = 2,
	HOUSE_OWNER = 3,
};

typedef std::list<HouseTile*> HouseTileList;
typedef std::list<BedItem*> HouseBedItemList;

class HouseTransferItem final : public Item
{
	public:
		static HouseTransferItem* createHouseTransferItem(House* house);

		explicit HouseTransferItem(House* _house) : Item(0) {
			house = _house;
		}

		void onTradeEvent(TradeEvents_t event, Player* owner) final;
		bool canTransform() const final {
			return false;
		}

	protected:
		House* house;
};

class House
{
	public:
		explicit House(uint32_t _houseid);

		void addTile(HouseTile* tile);
		void updateDoorDescription() const;

		bool canEditAccessList(uint32_t listId, const Player* player);
		// listId special values:
		// GUEST_LIST	 guest list
		// SUBOWNER_LIST subowner list
		void setAccessList(uint32_t listId, const std::string& textlist);
		bool getAccessList(uint32_t listId, std::string& list) const;

		bool isInvited(const Player* player);

		AccessHouseLevel_t getHouseAccessLevel(const Player* player);
		bool kickPlayer(Player* player, Player* target);

		void setEntryPos(Position pos) {
			posEntry = pos;
		}
		const Position& getEntryPosition() const {
			return posEntry;
		}

		void setName(std::string houseName) {
			this->houseName = houseName;
		}
		const std::string& getName() const {
			return houseName;
		}

		void setOwner(uint32_t guid, bool updateDatabase = true, Player* player = nullptr);
		uint32_t getOwner() const {
			return owner;
		}

		void setPaidUntil(time_t paid) {
			paidUntil = paid;
		}
		time_t getPaidUntil() const {
			return paidUntil;
		}

		void setRent(uint32_t _rent) {
			rent = _rent;
		}
		uint32_t getRent() const {
			return rent;
		}

		void setPayRentWarnings(uint32_t warnings) {
			rentWarnings = warnings;
		}
		uint32_t getPayRentWarnings() const {
			return rentWarnings;
		}

		void setTownId(uint32_t _town) {
			townid = _town;
		}
		uint32_t getTownId() const {
			return townid;
		}

		uint32_t getId() const {
			return id;
		}

		void addDoor(Door* door);
		void removeDoor(Door* door);
		Door* getDoorByNumber(uint32_t doorId) const;
		Door* getDoorByPosition(const Position& pos);

		HouseTransferItem* getTransferItem();
		void resetTransferItem();
		bool executeTransfer(HouseTransferItem* item, Player* player);

		const HouseTileList& getTiles() const {
			return houseTiles;
		}

		const std::list<Door*>& getDoors() const {
			return doorList;
		}

		void addBed(BedItem* bed);
		const HouseBedItemList& getBeds() const {
			return bedsList;
		}
		uint32_t getBedCount() {
			return static_cast<uint32_t>(std::ceil(bedsList.size() / 2.)); //each bed takes 2 sqms of space, ceil is just for bad maps
		}

	private:
		bool transferToDepot() const;
		bool transferToDepot(Player* player) const;

		AccessList guestList;
		AccessList subOwnerList;

		Container transfer_container;

		HouseTileList houseTiles;
		std::list<Door*> doorList;
		HouseBedItemList bedsList;

		std::string houseName;
		std::string ownerName;

		HouseTransferItem* transferItem;

		time_t paidUntil;

		uint32_t id;
		uint32_t owner;
		uint32_t rentWarnings;
		uint32_t rent;
		uint32_t townid;

		Position posEntry;

		bool isLoaded;
};

typedef std::map<uint32_t, House*> HouseMap;

enum RentPeriod_t {
	RENTPERIOD_DAILY,
	RENTPERIOD_WEEKLY,
	RENTPERIOD_MONTHLY,
	RENTPERIOD_YEARLY,
	RENTPERIOD_NEVER,
};

class Houses
{
	public:
		Houses() = default;
		~Houses() {
            for (const auto& it : houseMap) {
				delete it.second;
            }
		}

        // non-copyable
        Houses(const Houses&) = delete;
        Houses& operator=(const Houses&) = delete;

		House* addHouse(uint32_t id) {
			auto it = houseMap.find(id);
			if (it != houseMap.end()) {
				return it->second;
			}

			House* house = new House(id);
			houseMap[id] = house;
			return house;
		}

		House* getHouse(uint32_t houseid) {
			auto it = houseMap.find(houseid);
			if (it == houseMap.end()) {
				return nullptr;
			}
			return it->second;
		}

		House* getHouseByPlayerId(uint32_t playerId);

		bool loadHousesXML(const std::string& filename);

		void payHouses(RentPeriod_t rentPeriod) const;

		const HouseMap& getHouses() const {
			return houseMap;
		}

	private:
		HouseMap houseMap;
};

#endif
