#pragma once

#include "FormIDManager.h"
#include "RE/B/BGSHeadPart.h"
#include "RE/Skyrim.h"
#include "Settings.h"

namespace HeadPartUtils
{
	// Check if a TESFile is CVEO plugin
	bool IsCVEOFile(const RE::TESFile* a_file);

	// Check if a HeadPart type matches CVEO slider types (171..193 or 2)
	bool IsCVEOSliderType(std::uint32_t a_type);

	// Generate a Unisexy EditorID for the given head part
	// Returns empty string if the head part has no EditorID
	std::string GenerateUnisexyEditorID(const RE::BGSHeadPart* a_headPart);

	// Create a gender-flipped copy of the source head part
	// Returns nullptr only if memory allocation fails
	RE::BGSHeadPart* CreateUnisexyHeadPart(
		RE::IFormFactory* a_factory,
		const RE::BGSHeadPart* a_sourcePart,
		const std::string& a_newEditorID,
		bool a_toFemale);

	// Process and create gender-flipped versions of extra parts
	// Updates a_createdCount with number of extra parts created
	// Appends FormID conflict details to a_conflictDetails
	bool ProcessExtraParts(
		RE::BGSHeadPart* a_newHeadPart,
		const RE::BGSHeadPart* a_sourcePart,
		FormIDManager& a_formIDManager,
		const RE::TESFile* a_targetFile,
		std::set<std::string>& a_existingEditorIDs,
		std::unordered_map<std::string, RE::BGSHeadPart*>& a_editorIDToForm,
		const Settings& a_settings,
		int& a_createdCount,
		std::vector<std::tuple<std::string, std::uint32_t, std::uint32_t>>& a_conflictDetails);
}
