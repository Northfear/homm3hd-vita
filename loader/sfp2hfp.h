#ifndef __SFP2HFP_H__
#define __SFP2HFP_H__

void glClearDepthf_sfp(int32_t depth)
{
	float fa1;

	fa1 = *(float *)(&depth);
	glClearDepthf(fa1);
}

void glDepthRangef_sfp(int32_t fZNear, int32_t fZFar)
{
	float fa1, fa2;

	fa1 = *(float *)(&fZNear);
	fa2 = *(float *)(&fZFar);
	glDepthRangef(fa1, fa2);
}

void glTexParameterf_sfp(GLenum target,	GLenum pname, int32_t param)
{
	float fa1;

	fa1 = *(float *)(&param);
	glTexParameterf(target, pname, fa1);
}

void glUniform1f_sfp(GLint location, int32_t v0)
{
	float fa1;

	fa1 = *(float *)(&v0);
	glUniform1f(location, fa1);
}

void glUniform2f_sfp(GLint location, int32_t v0, int32_t v1)
{
	float fa1, fa2;

	fa1 = *(float *)(&v0);
	fa2 = *(float *)(&v1);
	glUniform2f(location, fa1, fa2);
}

void glUniform3f_sfp(GLint location, int32_t v0, int32_t v1, int32_t v2)
{
	float fa1, fa2, fa3;

	fa1 = *(float *)(&v0);
	fa2 = *(float *)(&v1);
	fa3 = *(float *)(&v2);
	glUniform3f(location, fa1, fa2, fa3);
}

void glUniform4f_sfp(GLint location, int32_t v0, int32_t v1, int32_t v2, int32_t v3)
{
	float fa1, fa2, fa3, fa4;

	fa1 = *(float *)(&v0);
	fa2 = *(float *)(&v1);
	fa3 = *(float *)(&v2);
	fa4 = *(float *)(&v3);
	glUniform4f(location, fa1, fa2, fa3, fa4);
}



int64_t atan_sfp(int64_t a1)
{
	double fa1;
	int64_t ires;

	fa1 = *(double *)(&a1);
	double fres = atan(fa1);
	ires = *(int64_t *)(&fres);

	return ires;
}

int64_t atan2_sfp(int64_t a1, int64_t a2)
{
	double fa1, fa2;
	int64_t ires;

	fa1 = *(double *)(&a1);
	fa2 = *(double *)(&a2);
	double fres = atan2(fa1, fa2);
	ires = *(int64_t *)(&fres);

	return ires;
}

int64_t ceil_sfp(int64_t a1)
{
	double fa1;
	int64_t ires;

	fa1 = *(double *)(&a1);
	double fres = ceil(fa1);
	ires = *(int64_t *)(&fres);

	return ires;
}

int32_t cosf_sfp(int32_t a1)
{
	float fa1;
	int32_t ires;

	fa1 = *(float *)(&a1);
	float fres = cosf(fa1);
	ires = *(int32_t *)(&fres);

	return ires;
}

int64_t fmod_sfp(int64_t a1, int64_t a2)
{
	double fa1, fa2;
	int64_t ires;

	fa1 = *(double *)(&a1);
	fa2 = *(double *)(&a2);
	double fres = fmod(fa1, fa2);
	ires = *(int64_t *)(&fres);

	return ires;
}

int32_t sinf_sfp(int32_t a1)
{
	float fa1;
	int32_t ires;

	fa1 = *(float *)(&a1);
	float fres = sinf(fa1);
	ires = *(int32_t *)(&fres);

	return ires;
}

int64_t th_granule_time_sfp(void *_encdec, ogg_int64_t _granpos)
{
	int64_t ires;

	double fres = th_granule_time(_encdec, _granpos);
	ires = *(int64_t *)(&fres);

	return ires;
}

int64_t vorbis_granule_time_sfp(vorbis_dsp_state *v, ogg_int64_t granulepos)
{
	int64_t ires;

	double fres = vorbis_granule_time(v, granulepos);
	ires = *(int64_t *)(&fres);

	return ires;
}

#endif
