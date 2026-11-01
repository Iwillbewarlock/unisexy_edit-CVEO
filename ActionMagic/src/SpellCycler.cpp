#include "SpellCycler.h"

namespace
{
	const char* HandName(Hand a_hand)
	{
		return a_hand == Hand::kLeft ? "Left" : "Right";
	}

	RE::TESForm* GetHandObject(RE::Actor* a_actor, Hand a_hand)
	{
		return a_actor->GetEquippedObject(a_hand == Hand::kLeft);
	}

	bool IsStaff(const RE::TESForm* a_form)
	{
		const auto weapon = a_form ? a_form->As<RE::TESObjectWEAP>() : nullptr;
		return weapon && weapon->IsStaff();
	}

	bool OccupiesBothHands(const RE::TESForm* a_form)
	{
		const auto weapon = a_form ? a_form->As<RE::TESObjectWEAP>() : nullptr;
		return weapon && (weapon->IsTwoHandedSword() || weapon->IsTwoHandedAxe() || weapon->IsBow() || weapon->IsCrossbow());
	}

	// A hand can receive a spell if it is empty or already holds a spell.
	// Weapons, shields and torches are never replaced by cycling.
	bool CanHoldCycledSpell(const RE::TESForm* a_form)
	{
		return !a_form || a_form->Is(RE::FormType::Spell);
	}

	const RE::BGSEquipSlot* GetHandSlot(Hand a_hand)
	{
		const auto dom = RE::BGSDefaultObjectManager::GetSingleton();
		if (!dom) {
			return nullptr;
		}
		return dom->GetObject<RE::BGSEquipSlot>(a_hand == Hand::kLeft ? RE::DEFAULT_OBJECTS::kLeftHandEquip : RE::DEFAULT_OBJECTS::kRightHandEquip);
	}

	void Notify(const std::string& a_message)
	{
		if (Settings::GetSingleton()->showNotifications) {
			RE::DebugNotification(a_message.c_str(), nullptr, false);
		}
	}
}

SpellCycler::Target SpellCycler::ResolveTarget(RE::Actor* a_actor)
{
	const auto right = GetHandObject(a_actor, Hand::kRight);
	const auto left = GetHandObject(a_actor, Hand::kLeft);

	const bool rightStaff = IsStaff(right);
	const bool leftStaff = IsStaff(left);

	if (rightStaff && leftStaff) {
		return { TargetKind::kDualStaff, Hand::kRight };
	}
	if (rightStaff) {
		return { TargetKind::kStaff, Hand::kRight };
	}
	if (leftStaff) {
		return { TargetKind::kStaff, Hand::kLeft };
	}

	if (OccupiesBothHands(right)) {
		return {};
	}

	const auto preferred = Settings::GetSingleton()->bareHandTarget;
	const auto other = preferred == Hand::kRight ? Hand::kLeft : Hand::kRight;

	if (CanHoldCycledSpell(GetHandObject(a_actor, preferred))) {
		return { TargetKind::kHand, preferred };
	}
	if (CanHoldCycledSpell(GetHandObject(a_actor, other))) {
		return { TargetKind::kHand, other };
	}
	return {};
}

std::vector<RE::SpellItem*> SpellCycler::CollectFavoriteSpells(RE::Actor* a_actor)
{
	std::vector<RE::SpellItem*> result;

	const auto favorites = RE::MagicFavorites::GetSingleton();
	if (!favorites) {
		return result;
	}

	for (const auto form : favorites->spells) {
		const auto spell = form ? form->As<RE::SpellItem>() : nullptr;
		if (!spell) {
			continue;  // shouts and other non-spell favorites
		}
		if (spell->GetSpellType() != RE::MagicSystem::SpellType::kSpell) {
			continue;  // powers and lesser powers go to the voice slot, not a hand
		}
		if (!a_actor->HasSpell(spell)) {
			continue;
		}
		result.push_back(spell);
	}

	return result;
}

void SpellCycler::CycleHand(RE::Actor* a_actor, Hand a_hand, int a_direction)
{
	const auto spells = CollectFavoriteSpells(a_actor);
	if (spells.empty()) {
		Notify("No favorited spells to cycle");
		return;
	}

	const auto currentForm = GetHandObject(a_actor, a_hand);
	const auto current = currentForm ? currentForm->As<RE::SpellItem>() : nullptr;
	const auto it = std::ranges::find(spells, current);
	const auto count = static_cast<std::ptrdiff_t>(spells.size());

	std::ptrdiff_t index = 0;
	if (it == spells.end()) {
		index = a_direction > 0 ? 0 : count - 1;
	} else {
		if (count == 1) {
			return;  // the only favorite is already equipped
		}
		index = ((it - spells.begin()) + a_direction + count) % count;
	}

	const auto spell = spells[static_cast<std::size_t>(index)];

	// Two-handed spells (e.g. rituals) use their own slot; everything else goes to the target hand.
	const auto slot = spell->IsTwoHanded() ? nullptr : GetHandSlot(a_hand);

	RE::ActorEquipManager::GetSingleton()->EquipSpell(a_actor, spell, slot);

	logger::info("Cycled {} hand -> {} ({:08X})", HandName(a_hand), spell->GetName(), spell->GetFormID());
	Notify(fmt::format("{}: {}", HandName(a_hand), spell->GetName()));
}

void SpellCycler::Cycle(int a_direction)
{
	const auto player = RE::PlayerCharacter::GetSingleton();
	if (!player || !player->Is3DLoaded()) {
		return;
	}

	const auto target = ResolveTarget(player);

	switch (target.kind) {
	case TargetKind::kHand:
		CycleHand(player, target.hand, a_direction);
		break;
	case TargetKind::kStaff:
	case TargetKind::kDualStaff:
		// Stage 2: staff spell selection and casting. Not implemented yet.
		logger::info("Cycle ignored: staff target ({} hand{}) is not implemented yet",
			HandName(target.hand), target.kind == TargetKind::kDualStaff ? ", dual" : "");
		Notify("Staff spell cycling is not available yet");
		break;
	case TargetKind::kNone:
	default:
		break;
	}
}
