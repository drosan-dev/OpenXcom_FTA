/*
 * Copyright 2010-2016 OpenXcom Developers.
 *
 * This file is part of OpenXcom.
 *
 * OpenXcom is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenXcom is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenXcom.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <algorithm>
#include "RuleBaseFacility.h"
#include "Mod.h"
#include "MapScript.h"
#include "../Battlescape/Position.h"

namespace OpenXcom
{

/**
 * Creates a blank ruleset for a certain
 * type of base facility.
 * @param type String defining the type.
 */
RuleBaseFacility::RuleBaseFacility(const std::string &type) : _type(type), _spriteShape(-1), _spriteFacility(-1), _lift(false), _hyper(false), _mind(false), _grav(false), _size(1), _buildCost(0), _refundValue(0), _buildTime(0), _monthlyCost(0), _storage(0), _personnel(0), _aliens(0), _crafts(0), _labs(0), _workshops(0), _psiLabs(0), _radarRange(0), _radarChance(0), _defense(0), _hitRatio(0), _fireSound(0), _hitSound(0), _listOrder(0), _trainingRooms(0), _maxAllowedPerBase(0), _sickBayAbsoluteBonus(0.0f), _sickBayRelativeBonus(0.0f), _prisonType(0), _rightClickActionType(0), _verticalLevels(), _removalTime(0), _canBeBuiltOver(false)
{
}

/**
 *
 */
RuleBaseFacility::~RuleBaseFacility()
{
}

/**
 * Loads the base facility type from a YAML file.
 * @param node YAML node.
 * @param mod Mod for the facility.
 * @param listOrder The list weight for this facility.
 */
void RuleBaseFacility::load(const YAML::Node &node, Mod *mod, int listOrder)
{
	if (const YAML::Node &parent = node["refNode"])
	{
		load(parent, mod, listOrder);
	}
	_type = node["type"].as<std::string>(_type);
	_requires = node["requires"].as< std::vector<std::string> >(_requires);
	_requiresBaseFunc = node["requiresBaseFunc"].as< std::vector<std::string> >(_requiresBaseFunc);
	_provideBaseFunc = node["provideBaseFunc"].as< std::vector<std::string> >(_provideBaseFunc);
	_forbiddenBaseFunc = node["forbiddenBaseFunc"].as< std::vector<std::string> >(_forbiddenBaseFunc);

	std::sort(_requiresBaseFunc.begin(), _requiresBaseFunc.end());
	std::sort(_provideBaseFunc.begin(), _provideBaseFunc.end());
	std::sort(_forbiddenBaseFunc.begin(), _forbiddenBaseFunc.end());

	if (node["spriteShape"])
	{
		_spriteShape = mod->getSpriteOffset(node["spriteShape"].as<int>(_spriteShape), "BASEBITS.PCK");
	}
	if (node["spriteFacility"])
	{
		_spriteFacility = mod->getSpriteOffset(node["spriteFacility"].as<int>(_spriteFacility), "BASEBITS.PCK");
	}
	_lift = node["lift"].as<bool>(_lift);
	_hyper = node["hyper"].as<bool>(_hyper);
	_mind = node["mind"].as<bool>(_mind);
	_grav = node["grav"].as<bool>(_grav);
	_size = node["size"].as<int>(_size);
	_buildCost = node["buildCost"].as<int>(_buildCost);
	_refundValue = node["refundValue"].as<int>(_refundValue);
	_buildTime = node["buildTime"].as<int>(_buildTime);
	_monthlyCost = node["monthlyCost"].as<int>(_monthlyCost);
	_storage = node["storage"].as<int>(_storage);
	_personnel = node["personnel"].as<int>(_personnel);
	_aliens = node["aliens"].as<int>(_aliens);
	_crafts = node["crafts"].as<int>(_crafts);
	_labs = node["labs"].as<int>(_labs);
	_workshops = node["workshops"].as<int>(_workshops);
	_psiLabs = node["psiLabs"].as<int>(_psiLabs);
	_radarRange = node["radarRange"].as<int>(_radarRange);
	_radarChance = node["radarChance"].as<int>(_radarChance);
	_defense = node["defense"].as<int>(_defense);
	_hitRatio = node["hitRatio"].as<int>(_hitRatio);
	if (node["fireSound"])
	{
		_fireSound = mod->getSoundOffset(node["fireSound"].as<int>(_fireSound), "GEO.CAT");
	}
	if (node["hitSound"])
	{
		_hitSound = mod->getSoundOffset(node["hitSound"].as<int>(_hitSound), "GEO.CAT");
	}
	_mapName = node["mapName"].as<std::string>(_mapName);
	_listOrder = node["listOrder"].as<int>(_listOrder);
	_trainingRooms = node["trainingRooms"].as<int>(_trainingRooms);
	_maxAllowedPerBase = node["maxAllowedPerBase"].as<int>(_maxAllowedPerBase);
	_sickBayAbsoluteBonus = node["sickBayAbsoluteBonus"].as<float>(_sickBayAbsoluteBonus);
	_sickBayRelativeBonus = node["sickBayRelativeBonus"].as<float>(_sickBayRelativeBonus);
	_prisonType = node["prisonType"].as<int>(_prisonType);
	_rightClickActionType = node["rightClickActionType"].as<int>(_rightClickActionType);
	if (!_listOrder)
	{
		_listOrder = listOrder;
	}
	if (const YAML::Node &items = node["buildCostItems"])
	{
		for (YAML::const_iterator i = items.begin(); i != items.end(); ++i)
		{
			std::string id = i->first.as<std::string>();
			std::pair<int, int> &cost = _buildCostItems[id];

			cost.first = i->second["build"].as<int>(cost.first);
			cost.second = i->second["refund"].as<int>(cost.second);

			if (cost.first <= 0 && cost.second <= 0)
			{
				_buildCostItems.erase(id);
			}
		}
	}

	// Load any VerticalLevels into a map if we have them
	if (node["verticalLevels"])
	{
		_verticalLevels.clear();
		for (YAML::const_iterator i = node["verticalLevels"].begin(); i != node["verticalLevels"].end(); ++i)
		{
			if ((*i)["type"])
			{
				VerticalLevel level;
				level.load(*i);
				_verticalLevels.push_back(level);
			}
		}
	}

	_leavesBehindOnSell = node["leavesBehindOnSell"].as< std::vector<std::string> >(_leavesBehindOnSell);
	_removalTime = node["removalTime"].as<int>(_removalTime);
	_canBeBuiltOver = node["canBeBuiltOver"].as<bool>(_canBeBuiltOver);
	_buildOverFacilities = node["buildOverFacilities"].as< std::vector<std::string> >(_buildOverFacilities);
	std::sort(_buildOverFacilities.begin(), _buildOverFacilities.end());

	_storageTiles = node["storageTiles"].as<std::vector<Position> >(_storageTiles);
}

/**
 * Gets the language string that names
 * this base facility. Each base facility type
 * has a unique name.
 * @return The facility's name.
 */
std::string RuleBaseFacility::getType() const
{
	return _type;
}

/**
 * Gets the list of research required to
 * build this base facility.
 * @return A list of research IDs.
 */
const std::vector<std::string> &RuleBaseFacility::getRequirements() const
{
	return _requires;
}

/**
 * Gets the list of required functions in base to build thins building.
 * @return List of function IDs.
 */
const std::vector<std::string> &RuleBaseFacility::getRequireBaseFunc() const
{
	return _requiresBaseFunc;
}

/**
 * Get the list of provided functions by this building.
 * @return List of function IDs.
 */
const std::vector<std::string> &RuleBaseFacility::getProvidedBaseFunc() const
{
	return _provideBaseFunc;
}

/**
 * Gets the list of forbiden functions by this building.
 * @return List of function IDs.
 */
const std::vector<std::string> &RuleBaseFacility::getForbiddenBaseFunc() const
{
	return _forbiddenBaseFunc;
}
/**
 * Gets the ID of the sprite used to draw the
 * base structure of the facility that defines its shape.
 * @return The sprite ID.
 */
int RuleBaseFacility::getSpriteShape() const
{
	return _spriteShape;
}

/**
 * Gets the ID of the sprite used to draw the
 * facility's contents inside the base shape.
 * @return The sprite ID.
 */
int RuleBaseFacility::getSpriteFacility() const
{
	return _spriteFacility;
}

/**
 * Gets the size of the facility on the base grid.
 * @return The length in grid squares.
 */
int RuleBaseFacility::getSize() const
{
	return _size;
}

/**
 * Checks if this facility is the core access lift
 * of a base. Every base has an access lift and all
 * facilities have to be connected to it.
 * @return True if it's a lift.
 */
bool RuleBaseFacility::isLift() const
{
	return _lift;
}

/**
 * Checks if this facility has hyperwave detection
 * capabilities. This allows it to get extra details about UFOs.
 * @return True if it has hyperwave detection.
 */
bool RuleBaseFacility::isHyperwave() const
{
	return _hyper;
}

/**
 * Checks if this facility has a mind shield,
 * which covers your base from alien detection.
 * @return True if it has a mind shield.
 */
bool RuleBaseFacility::isMindShield() const
{
	return _mind;
}

/**
 * Checks if this facility has a grav shield,
 * which doubles base defense's fire ratio.
 * @return True if it has a grav shield.
 */
bool RuleBaseFacility::isGravShield() const
{
	return _grav;
}

/**
 * Gets the amount of funds that this facility costs
 * to build on a base.
 * @return The building cost.
 */
int RuleBaseFacility::getBuildCost() const
{
	return _buildCost;
}

/**
 * Gets the amount that is refunded when the facility
 * is dismantled.
 * @return The refund value.
 */
int RuleBaseFacility::getRefundValue() const
{
	return _refundValue;
}

/**
 * Gets the amount of items that this facility require to build on a base or amount of items returned after dismantling.
 * @return The building cost in items.
 */
const std::map<std::string, std::pair<int, int> >& RuleBaseFacility::getBuildCostItems() const
{
	return _buildCostItems;
}

/**
 * Gets the amount of time that this facility takes
 * to be constructed since placement.
 * @return The time in days.
 */
int RuleBaseFacility::getBuildTime() const
{
	return _buildTime;
}

/**
 * Gets the amount of funds this facility costs monthly
 * to maintain once it's fully built.
 * @return The monthly cost.
 */
int RuleBaseFacility::getMonthlyCost() const
{
	return _monthlyCost;
}

/**
 * Gets the amount of storage space this facility provides
 * for base equipment.
 * @return The storage space.
 */
int RuleBaseFacility::getStorage() const
{
	return _storage;
}

/**
 * Gets the number of base personnel (soldiers, scientists,
 * engineers) this facility can contain.
 * @return The number of personnel.
 */
int RuleBaseFacility::getPersonnel() const
{
	return _personnel;
}

/**
 * Gets the number of captured live aliens this facility
 * can contain.
 * @return The number of aliens.
 */
int RuleBaseFacility::getAliens() const
{
	return _aliens;
}

/**
 * Gets the number of base craft this facility can contain.
 * @return The number of craft.
 */
int RuleBaseFacility::getCrafts() const
{
	return _crafts;
}

/**
 * Gets the amount of laboratory space this facility provides
 * for research projects.
 * @return The laboratory space.
 */
int RuleBaseFacility::getLaboratories() const
{
	return _labs;
}

/**
 * Gets the amount of workshop space this facility provides
 * for manufacturing projects.
 * @return The workshop space.
 */
int RuleBaseFacility::getWorkshops() const
{
	return _workshops;
}

/**
 * Gets the number of soldiers this facility can contain
 * for monthly psi-training.
 * @return The number of soldiers.
 */
int RuleBaseFacility::getPsiLaboratories() const
{
	return _psiLabs;
}

/**
 * Gets the radar range this facility provides for the
 * detection of UFOs.
 * @return The range in nautical miles.
 */
int RuleBaseFacility::getRadarRange() const
{
	return _radarRange;
}

/**
 * Gets the chance of UFOs that come within the facility's
 * radar range being detected.
 * @return The chance as a percentage.
 */
int RuleBaseFacility::getRadarChance() const
{
	return _radarChance;
}

/**
 * Gets the defense value of this facility's weaponry
 * against UFO invasions on the base.
 * @return The defense value.
 */
int RuleBaseFacility::getDefenseValue() const
{
	return _defense;
}

/**
 * Gets the hit ratio of this facility's weaponry
 * against UFO invasions on the base.
 * @return The hit ratio as a percentage.
 */
int RuleBaseFacility::getHitRatio() const
{
	return _hitRatio;
}

/**
 * Gets the battlescape map block name for this facility
 * to construct the base defense mission map.
 * @return The map name.
 */
std::string RuleBaseFacility::getMapName() const
{
	return _mapName;
}

/**
 * Gets the hit sound of this facility's weaponry.
 * @return The sound index number.
 */
int RuleBaseFacility::getHitSound() const
{
	return _hitSound;
}

/**
 * Gets the fire sound of this facility's weaponry.
 * @return The sound index number.
 */
int RuleBaseFacility::getFireSound() const
{
	return _fireSound;
}

/**
 * Gets the facility's list weight.
 * @return The list weight for this research item.
 */
int RuleBaseFacility::getListOrder() const
{
	return _listOrder;
}

/**
 * Returns the amount of soldiers this facility can contain
 * for monthly training.
 * @return Amount of room.
 */
int RuleBaseFacility::getTrainingFacilities() const
{
	return _trainingRooms;
}

/**
* Gets the maximum allowed number of facilities per base.
* @return The number of facilities.
*/
int RuleBaseFacility::getMaxAllowedPerBase() const
{
	return _maxAllowedPerBase;
}

/**
* Gets the facility's bonus to hp healed.
* @return Amount of HP healed.
*/
float RuleBaseFacility::getSickBayAbsoluteBonus() const
{
	return _sickBayAbsoluteBonus;
}

/**
* Gets the facility's bonus to hp healed (as percentage of max hp of the soldier).
* @return Amount of HP healed as percentage of max HP.
*/
float RuleBaseFacility::getSickBayRelativeBonus() const
{
	return _sickBayRelativeBonus;
}

/**
* Gets the prison type.
* @return 0=alien containment, 1=prison, 2=animal cages, etc.
*/
int RuleBaseFacility::getPrisonType() const
{
	return _prisonType;
}

/**
* Gets the action type to perform on right click.
* @return 0=default, 1 = prison, 2 = manufacture, 3 = research, 4 = training, 5 = psi training, 6 = soldiers, 7 = sell
*/
int RuleBaseFacility::getRightClickActionType() const
{
	return _rightClickActionType;
}

/*
 * Gets the vertical levels for a base facility map
 * @return the vector of VerticalLevels
 */
const std::vector<VerticalLevel> &RuleBaseFacility::getVerticalLevels() const
{
	return _verticalLevels;
}

/**
 * Gets the facility/facilities left behind when this one is sold
 * @return the list of facilities
 */
const std::vector<std::string> &RuleBaseFacility::getLeavesBehindOnSell() const
{
	return _leavesBehindOnSell;
}

/**
 * Gets how long facilities left behind when this one is sold should take to build
 * @return the number of days, -1 = from other facilities' rulesets, 0 = instant, > 0 is that many days
 */
int RuleBaseFacility::getRemovalTime() const
{
	return _removalTime;
}

/**
 * Gets whether or not this facility can be built over
 * @return can we build over this?
 */
bool RuleBaseFacility::getCanBeBuiltOver() const
{
	return _canBeBuiltOver;
}

/**
 * Gets the list of other base facilities this one can be built over
 * If empty, it can be built over anything with canBeBuiltOver: true
 * @return the list of facilities
 */
const std::vector<std::string> &RuleBaseFacility::getBuildOverFacilities() const
{
	return _buildOverFacilities;
}

/**
 * Gets the list of tile positions where to place items in this facility's storage
 * If empty, vanilla checkerboard pattern will be used
 * @return the list of positions
 */
const std::vector<Position> &RuleBaseFacility::getStorageTiles() const
{
	return _storageTiles;
}

}
