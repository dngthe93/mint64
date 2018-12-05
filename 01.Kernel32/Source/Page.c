
#include "Page.h"

void kInitializePageTables()
{
	PML4TENTRY *pstPML4TEntry;
	PDPTENTRY *pstPDPTEntry;
	PDENTRY *pstPDEntry;
	unsigned int i;

	pstPML4TEntry = (PML4TENTRY*)0x100000;
	kSetPageEntryData(pstPML4TEntry, 0x00, 0x101000, PAGE_FLAGS_DEFAULT, 0);
	for (i = 1; i < PAGE_MAXENTRYCOUNT; i++)
		kSetPageEntryData(pstPML4TEntry + i, 0, 0, 0, 0);


	pstPDPTEntry = (PDPTENTRY*)0x101000;
	for (i = 0; i < 64; i++)
		kSetPageEntryData(pstPDPTEntry + i, 0x00, 0x102000 + (i * PAGE_TABLESIZE), PAGE_FLAGS_DEFAULT, 0);

	for (; i < PAGE_MAXENTRYCOUNT; i++)
		kSetPageEntryData(pstPDPTEntry + i, 0, 0, 0, 0);


	pstPDEntry = (PDENTRY*)0x102000;
	for (i = 0; i < 512 * 64; i++)
		kSetPageEntryData(pstPDEntry + i, i >> 11, i << 21, PAGE_FLAGS_DEFAULT | PAGE_FLAGS_PS, 0);
}

void kSetPageEntryData(PTENTRY *pstEntry,
						DWORD dwUpperBaseAddress,
						DWORD dwLowerBaseAddress,
						DWORD dwLowerFlags,
						DWORD dwUpperFlags)
{
	pstEntry->dwAttributeAndLowerBaseAddress = (dwLowerBaseAddress & 0xfffff000) | dwLowerFlags;
	pstEntry->dwUpperBaseAddressAndEXB = (dwUpperBaseAddress & 0xFF) | dwUpperFlags;
}
