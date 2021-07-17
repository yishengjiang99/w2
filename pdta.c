#include <stdlib.h>
#include "read2.h"
#define export __attribute__((visibility("default")))

export void pdta(char *pdtadata)
{
	section_header *sh = (section_header *)malloc(sizeof(section_header));
#define memread(dest, source, size) \
	memcpy(dest, source, size);       \
	source = source + size;

#define readSection(section)                     \
	memread(sh, pdtadata, sizeof(section_header)); \
	n##section##s = sh->size / sizeof(section);    \
	section##s = (section *)malloc(sh->size);      \
	memread(section##s, pdtadata, sh->size);

	readSection(phdr);
	readSection(pbag);
	readSection(pmod);
	readSection(pgen);
	readSection(inst);
	readSection(ibag);
	readSection(imod);
	readSection(igen);
	readSection(shdr);
	return 1;
}