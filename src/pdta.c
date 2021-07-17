static int nphdrs, npbags, npgens, npmods, nshdrs, ninsts, nimods, nigens, nibags, nshrs;

typedef unsigned int uint32_t;
typedef unsigned char uint8_t;
typedef struct
{
	char name[4];
	unsigned int size;
} section_header;
typedef struct
{
	char name[4];
	unsigned int size;
	char *data;
} pdta;
typedef struct
{
	char name[20];
	unsigned short pid, bankId, pbagNdx;
	char idc[12];
} phdr;
typedef struct
{
	unsigned short pgen_id, pmod_id;
} pbag;
typedef struct
{
	unsigned short igen_id, imod_id;
} ibag;
struct range
{
	unsigned char lo, hi;
};
typedef union
{
	unsigned short uAmount;
	signed short shAmount;
	struct range range;
} genAmountType;
typedef struct
{
	unsigned short operator;
	genAmountType val;
} pgen_t;
typedef struct
{
	char data[10];
} pmod;
typedef struct
{
	char name[20];
	unsigned short ibagNdx;
} inst;
typedef struct
{
	char data[10];
} imod;
typedef pgen_t pgen;

typedef pgen_t igen;
typedef struct
{
	// shdr's 46 byters is malloc aligned to 48 and I don't make it stop
	// so we first read 46 chars explicitly and then casted to shdrcast defined
	//beloe
	uint8_t dc[46];
} shdr;

typedef struct
{
	char name[20];
	unsigned int start, end, startloop, endloop, sampleRate;
	short originalPitch;
	signed char pitchCorrection;
	unsigned short wSampleLink;
	unsigned short sfSampleType;
} shdrcast;
static phdr *phdrs;
static pbag *pbags;
static pmod *pmods;
static pgen *pgens;
static inst *insts;
static ibag *ibags;
static imod *imods;
static igen *igens;
static shdr *shdrs;

typedef struct pdatlist
{
	phdr *phdrs;
	pbag *pbags;
	pmod *pmods;
	pgen *pgens;
	inst *insts;
	ibag *ibags;
	imod *imods;
	igen *igens;
	shdr *shdrs;
} pdtalist;

/* we are not reallly reading anyting other than headers
then assigning pointers to memory already read (file) according to header sizes
*/
void readpdta(void *file, pdtalist *list)
{

#define readSection(sec)                 \
	{                                      \
		section_header *shh;                 \
		shh = (section_header *)file;        \
		n##sec##s = shh->size / sizeof(sec); \
		sec##s = (sec *)file;                \
		file += shh->size;                   \
	}

	readSection(phdr);
	readSection(pbag);
	readSection(pmod);
	readSection(pgen);
	readSection(inst);
	readSection(ibag);
	readSection(imod);
	readSection(igen);
	readSection(shdr);
	list = &(pdtalist){phdrs, pbags, pmods, pgens, insts, ibags, imods, igens, shdrs};
}
void index22(pdtalist *list)
{
	consolelog(list->phdrs->name);
}
