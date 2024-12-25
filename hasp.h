#pragma once
#include <cstdint>

#pragma pack(push, 1)

struct PointLimitData {
	// ?? this can also be settlement_month
	bool initialized; // 0
	char pad[1]; // 1
	uint16_t settlementMonth; // 2, BE
	uint32_t upperToken; // 4, BE
	uint32_t lowerToken; // 8, BE
	uint32_t checksum; // 12

	void UpdateChecksum();
};

struct PointData {
	uint32_t cost; // 0,  BE
	uint32_t limit; // 4, BE
	uint32_t current; // 8, BE
	uint32_t checksum;

	void UpdateChecksum();
};

struct ApplicationData {
	PointLimitData pointLimit;
	PointData points[2]; // 16

	void Initialize();

	void SetPoints(uint32_t points);
};

struct SystemData {
	char serial[12]; // 0
	char otherInfo[48]; // 12
	uint16_t reserved; //60
	uint8_t checksum; // 62
	uint8_t neg_checksum; // 63

	void UpdateChecksum();

	void SetSerial(const char* serial);
};

struct HaspData {
	ApplicationData app; // 0
	char pad[3280]; // 48
	SystemData sys; // 3328
};

#pragma pack(pop)


void hasp_init(const char* serial);