#pragma once

#include "Settings.h"

// Decides which hand / staff a cycle input applies to, and performs the cycle.
//
// Target rules (see DESIGN.md):
//   1. Exactly one staff equipped (either hand, any off-hand item)  -> that staff.
//   2. Two staffs equipped                                           -> shared selection for both staffs.
//   3. No staff                                                      -> equip the next favorited spell into a hand.
//
// Stage 1 implements rule 3 only. Staff targets are resolved and logged, but staff casting
// (stage 2) is not implemented yet.
class SpellCycler
{
public:
	enum class TargetKind
	{
		kNone,      // nothing to cycle (e.g. two-handed weapon, or both hands hold non-spell items)
		kHand,      // bare-hand spell cycling
		kStaff,     // one staff
		kDualStaff  // two staffs, shared selection
	};

	struct Target
	{
		TargetKind kind{ TargetKind::kNone };
		Hand       hand{ Hand::kRight };
	};

	static SpellCycler* GetSingleton()
	{
		static SpellCycler singleton;
		return &singleton;
	}

	// a_direction: +1 = next, -1 = previous. Must run on the main thread.
	void Cycle(int a_direction);

	static Target ResolveTarget(RE::Actor* a_actor);

private:
	SpellCycler() = default;

	static std::vector<RE::SpellItem*> CollectFavoriteSpells(RE::Actor* a_actor);
	static void                        CycleHand(RE::Actor* a_actor, Hand a_hand, int a_direction);
};
