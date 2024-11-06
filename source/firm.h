#include <stdint.h>

typedef struct FIRMSection {
	/* 0x00 */	uint32_t offset;
	/* 0x04 */	uint32_t addr;
	/* 0x08 */	uint32_t size;
	/* 0x0C */	uint32_t copyMethod;
	/* 0x10 */	char hash[SHA256_BLOCK_SIZE];
} FIRMSection;

_Static_assert(sizeof(FIRMSection) == 0x30, "FIRMSection size incorrect");

typedef struct FIRMHeader {
	/* 0x000 */	char magic[4]; // "FIRM"
	/* 0x004 */	uint32_t bootPriority;
	/* 0x008 */	uint32_t arm9Ep;
	/* 0x00C */	uint32_t arm11Ep;
	/* 0x010 */	char reserved[0x30];
	/* 0x040 */	FIRMSection sections[4];
	/* 0x100 */ char RSA2048SHA256Sig[0x100];
} FIRMHeader;

_Static_assert(sizeof(FIRMHeader) == 0x200, "FIRMHeader size incorrect");
