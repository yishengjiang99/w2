
#include "read2.h"
#include <math.h>
#define sr 48000
static float p2over1200LUT[2400];
static inline float p2over1200(float x)
{
	if (x < -12000)
		return 0;
	if (x < 0)
		return 1.f / p2over1200(-x);
	else if (x > 2400.0f)
	{
		return 2 * p2over1200(x - 2400.0f);
	}
	else
	{
		return p2over1200LUT[(unsigned short)(x)];
	}
}
static float centdbLUT[960];
#define export __attribute__((visibility("default")))
export int pdtar(char *pdtadata)
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

export zone_t *index22(int pid, int bkid, int key, int vel)
{
	int found = 0;
	short igset[60] = {-1};
	int instID = -1;
	int lastSampId = -1;
	short pgdef[60] = {-1};
	for (int i = 0; i < nphdrs - 1; i++)
	{
		if (phdrs[i].bankId != bkid || phdrs[i].pid != pid)
			continue;
		int lastbag = phdrs[i + 1].pbagNdx;

		for (int j = phdrs[i].pbagNdx; j < lastbag; j++)
		{
			pbag *pg = pbags + j;
			pgen_t *lastg = pgens + pg[j + 1].pgen_id;
			int pgenId = pg->pgen_id;
			int lastPgenId = j < npbags - 1 ? pbags[j + 1].pgen_id : npgens - 1;
			short pgset[60] = {-1};
			instID = -1;
			for (int k = pgenId; k < lastPgenId; k++)
			{
				pgen_t *g = pgens + k;
				if (g->operator== 44 &&(g->val.ranges.lo > vel || g->val.ranges.hi < vel))
					break;
				if (g->operator== 43 &&(g->val.ranges.lo > key || g->val.ranges.hi < key))
					break;
				if (g->operator== 41)
				{
					instID = g->val.shAmount;
				}
				pgset[g->operator] = g->val.shAmount;
			}
			if (instID == -1)
			{
				memcpy(pgdef, pgset, sizeof(pgset));
			}
			else
			{
				inst *ihead = insts + instID;
				int ibgId = ihead->ibagNdx;
				int lastibg = (ihead + 1)->ibagNdx;
				lastSampId = -1;
				short igdef[60] = {-1};
				for (int ibg = ibgId; ibg < lastibg; ibg++)
				{
					short igset[60];
					ibag *ibgg = ibags + ibg;
					pgen_t *lastig = ibg < nibags - 1 ? igens + (ibgg + 1)->igen_id : igens + nigens - 1;
					for (pgen_t *g = igens + ibgg->igen_id; g->operator!= 60 && g != lastig; g++)
					{
						if (g->operator== 44 &&(g->val.ranges.lo > vel || g->val.ranges.hi < vel))
							break;
						if (g->operator== 43 &&(g->val.ranges.lo > key || g->val.ranges.hi < key))
							break;

						igset[g->operator]=g->val.shAmount;
						if (g->operator== 53)
						{
							lastSampId = g->val.shAmount; // | (ig->val.ranges.hi << 8);
							short *attributes = (short *)malloc(sizeof(short) * 60);
							for (int i = 0; i < 60; i++)
							{
								if (igset[i])
									*(attributes + i) = igset[i];
								else if (igdef[i])
									*(attributes + i) = igset[i];

								if (pgset[i])
									*(attributes + i) += pgset[i];
								else if (pgdef[i])
									*(attributes + i) += pgdef[i];
							}
							zone_t *z = (zone_t *)malloc(sizeof(zone_t));
							memcpy(z, attributes, 60 * sizeof(short));

							//		//fprintf(stdout,"\n samp%d", lastSampId);
							shdrcast *sh = (shdrcast *)(shdrs + lastSampId);
							z->sample = sh;

							z->sample->start += (z->StartAddrCoarseOfs << 15) + z->StartAddrOfs;
							z->sample->end += (z->EndAddrCoarseOfs << 15) + z->EndAddrOfs;
							z->sample->endloop += (z->EndLoopAddrCoarseOfs << 15) + z->EndLoopAddrOfs;

							z->sample->startloop += (z->StartLoopAddrCoarseOfs << 15) + z->StartLoopAddrOfs;
							return z;
						}
					}
				}
			}
		}
	}
	return NULL;
}

export void initLUTs()
{
	for (int i = 0; i < 2400; i++)
	{
		p2over1200LUT[i] = powf(2.0f, i / 1200.0f);
	}
	for (int i = 0; i < 960; i++)
	{
		centdbLUT[i] = powf(10.0f, 200.0f / 960.f * i);
	}
}

export adsr_t *newEnvelope(short centAtt, short centRelease, short centDecay, short sustain, int sampleRate)
{
	adsr_t *env = (adsr_t *)malloc(sizeof(adsr_t));
	env->att_steps = fmax(p2over1200(centAtt) * sampleRate, 2);
	env->decay_steps = fmax(p2over1200(centDecay) * sampleRate, 2);
	env->release_steps = fmax(p2over1200(centRelease) * sampleRate, 2);
	env->att_rate = -960.0f / env->att_steps;
	env->decay_rate = ((float)1.0f * sustain) / (env->decay_steps);
	env->release_rate = (float)(960.0f - sustain) / (env->release_steps);
	env->db_attenuate = 960.0f;
	return env;
}
float envShift(adsr_t *env)
{
	if (env->att_steps > 0)
	{
		env->att_steps--;
		env->db_attenuate += env->att_rate;
	}
	else if (env->decay_steps > 0)
	{
		env->decay_steps--;
		env->db_attenuate += env->decay_rate;
	}
	else if (env->release_steps > 0)
	{
		env->release_steps--;
		env->db_attenuate += env->release_rate;
	}
	if (env->db_attenuate > 960)
	{
		env->db_attenuate = 960.0f;
	}
	if (env->db_attenuate < 0.0)
	{
		env->db_attenuate = 0.0f;
	}
	return env->db_attenuate;
}
void adsrRelease(adsr_t *env)
{
	env->decay_steps = 0;
	env->att_steps = 0;
}

voice *newVoice(zone_t *z, int key, int vel)
{

	voice *v = (voice *)malloc(sizeof(voice));
	v->ampvol = newEnvelope(z->VolEnvAttack, z->VolEnvRelease, z->VolEnvDecay, z->VolEnvSustain, 48000);
	short rt = z->OverrideRootKey > -1 ? z->sample->originalPitch : z->sample->originalPitch;
	float sampleTone = rt * 100.0f + z->CoarseTune * 100.0f + (float)z->FineTune;
	float octaveDivv = ((float)key * 100 - sampleTone) / 1200.0f;
	v->ratio = 1.0f * pow(2.0f, octaveDivv) * z->sample->sampleRate / 48000;

	v->pos = z->sample->start;
	v->frac = 0.0f;
	v->z = z;
	v->midi = key;
	v->velocity = vel;

	return v;
}
typedef struct _channel
{
	voice *voice;
	int program_number;
	float midi_gain_cb;
	float midi_pan;
} channel_t;
typedef struct
{
	int sampleRate;
	int currentFrame_start;
	int samples_per_frame;
	channel_t *channels;
} ctx_t;

static float outputLeft[128];
static float outputRight[128];
static float outputInterweaved[128 * 2];
ctx_t *ctx;
void loop(voice *v, channel_t *ch)
{
	uint32_t loopLength = v->z->sample->endloop - v->z->sample->startloop;
	int cb_attentuate = v->z->Attenuation + (int)envShift(v->ampvol) - ch->midi_gain_cb;
	uint16_t pan = ch->midi_pan > 0 ? ch->midi_pan * 960 / 127 : v->z->Pan;
	float panLeft = 1.01;
	float panright = 0.9;
	for (int i = 0; i < 128; i++)
	{
		float f1 = *(sdta + v->pos);
		float f2 = *(sdta + v->pos + 1);
		float gain = f1;
		float mono = gain * centdbLUT[(int)envShift(v->ampvol) + v->z->Attenuation];
		outputInterweaved[2 * i] += mono * (1 + pan / 1000) / 5;
		outputInterweaved[2 * i + 1] += mono * (1 - pan / 1000) / 3;
		v->frac += v->ratio;
		while (v->frac >= 1.0f)
		{
			v->frac--;
			v->pos++;
		}
		if (v->pos >= v->z->sample->endloop)
		{
			v->pos -= loopLength;
		}
	}
}
void render(int frames, FILE *output)
{
	while (frames >= 0)
	{
		bzero(outputInterweaved, sizeof(float) * ctx->samples_per_frame);
		for (int ch = 0; ch < 16; ch++)
		{
			voice *v = (ctx->channels + ch)->voice;
			if (v->midi > 0)
			{
				loop(v, ctx->channels + ch);
			}
		}
		frames -= 128;
		fwrite(outputInterweaved, sizeof(float), 2 * 128, output);
	}
}
void noteOn(int channelNumber, int midi, int vel)
{
	int programNumber = (ctx->channels + channelNumber)->program_number;
	zone_t *z = index22(programNumber & 0x7f, programNumber & 0x80, midi, vel);
	(ctx->channels + channelNumber)->voice = newVoice(z, midi, vel);
}
void noteOff(int channelNumber, int midi)
{
	voice *v = (ctx->channels + channelNumber)->voice;
	adsrRelease(v->ampvol);
}
void init_ctx()
{
	ctx = (ctx_t *)malloc(sizeof(ctx_t));
	ctx->sampleRate = 48000;
	ctx->currentFrame_start = 0;
	ctx->samples_per_frame = 127;
	ctx->channels = (channel_t *)malloc(sizeof(channel_t) * 16);
	for (int i = 0; i < 16; i++)
	{
		(ctx->channels + i)->program_number = 0;
		(ctx->channels + i)->midi_pan = 1.f;
		(ctx->channels + i)->midi_gain_cb = 89.0f;
		(ctx->channels + i)->voice = (voice *)malloc(sizeof(voice));
	}
}
