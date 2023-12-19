#include "MPQReader.h"
#include "BH.h"

std::map<std::string, MPQData*> MpqDataMap;

MPQData::MPQData(std::string buffer) {
	std::stringstream ss(buffer);
	std::string line;
	std::string field;
	if (std::getline(ss, line)) {  // read the header first
		std::stringstream hss(line);
		while (std::getline(hss, field, '\t')) {
			fields.push_back(field);
		}
		while (std::getline(ss, line)) {
			std::map<std::string, std::string> linedata;
			std::stringstream fss(line);
			for (std::vector<std::string>::iterator it = fields.begin(); it != fields.end(); it++) {
				if (!std::getline(fss, field, '\t')) {
					field.clear();
				}
				linedata[(*it)] = field;
			}
			data.push_back(linedata);
		}
	}
}

MPQData::~MPQData() {}

BufferData loadFile(const std::string& file_path)
{
	std::string path = file_path;

	void* ref_file;
	char c_filepath[MAX_PATH];
	strncpy_s(c_filepath, path.c_str(), path.size());

	if (FOG_mpqOpenFile(c_filepath, &ref_file)) {
		size_t return_size = 0;
		DWORD fileSize = FOG_mpqGetFileSize(ref_file, NULL);
		uint8_t* cache = new uint8_t[fileSize + 1];

		if (FOG_mpqReadFile(ref_file, cache, fileSize, &return_size, NULL, NULL, NULL)) {
			FOG_mpqCloseFile(ref_file);

			cache[fileSize] = 0;
			return { return_size, cache };
		}

		delete[] cache;
		return { 0 };
	}

	return { 0 };
}

bool ReadMPQFiles() {
	const int NUM_MPQS = 15;
	std::string mpqFiles[NUM_MPQS] = {
		"UniqueItems",
		"Armor",
		"Weapons",
		"Misc",
		"ItemTypes",
		"ItemStatCost",
		"Properties",
		"Runes",
		"SetItems",
		"skills",
		"MagicPrefix",
		"MagicSuffix",
		"RarePrefix",
		"RareSuffix",
		"CharStats"
	};
	for (int i = 0; i < NUM_MPQS; i++) {
		std::string path = "data\\global\\excel\\" + mpqFiles[i] + ".txt";
		auto data_buffer = loadFile(path);
		std::string txt_data((const char*)data_buffer.data, data_buffer.size);

		std::string key = mpqFiles[i];
		std::transform(key.begin(), key.end(), key.begin(), ::tolower);
		MpqDataMap[key] = new MPQData(txt_data);
	}
	return true;
}

void FindAncestorTypes(std::string type, std::set<std::string>& ancestors, std::map<std::string, std::string>& map1, std::map<std::string, std::string>& map2) {
	ancestors.insert(type);
	std::map<std::string, std::string>::iterator it1 = map1.find(type);
	if (it1 != map1.end()) {
		FindAncestorTypes(it1->second, ancestors, map1, map2);
	}
	std::map<std::string, std::string>::iterator it2 = map2.find(type);
	if (it2 != map2.end()) {
		FindAncestorTypes(it2->second, ancestors, map1, map2);
	}
}

unsigned int AssignClassFlags(std::string type, std::set<std::string>& ancestors, unsigned int flags) {
	if (ancestors.find("amaz") != ancestors.end()) {
		flags |= ITEM_GROUP_AMAZON_WEAPON;
	} else if (ancestors.find("barb") != ancestors.end()) {
		flags |= ITEM_GROUP_BARBARIAN_HELM;
	} else if (ancestors.find("necr") != ancestors.end()) {
		flags |= ITEM_GROUP_NECROMANCER_SHIELD;
	} else if (ancestors.find("pala") != ancestors.end()) {
		flags |= ITEM_GROUP_PALADIN_SHIELD;
	} else if (ancestors.find("sorc") != ancestors.end()) {
		flags |= ITEM_GROUP_SORCERESS_ORB;
	} else if (ancestors.find("assn") != ancestors.end()) {
		flags |= ITEM_GROUP_ASSASSIN_KATAR;
	} else if (ancestors.find("drui") != ancestors.end()) {
		flags |= ITEM_GROUP_DRUID_PELT;
	}
	return flags;
}
