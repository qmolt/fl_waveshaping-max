#include "fl_ws.vfolder~.h"

/* initialization routine */
void ext_main(void *r)
{
	fl_ws_vfolder_class = class_new("fl_ws.vfolder~", (method)fl_ws_vfolder_new, (method)fl_ws_vfolder_free, sizeof(t_fl_ws_vfolder), 0, A_GIMME, 0);

	class_addmethod(fl_ws_vfolder_class, (method)fl_ws_vfolder_dsp64, "dsp64", A_CANT, 0);
	class_addmethod(fl_ws_vfolder_class, (method)fl_ws_vfolder_float, "float", A_FLOAT, 0);
	class_addmethod(fl_ws_vfolder_class, (method)fl_ws_vfolder_assist, "assist", A_CANT, 0);

	class_dspinit(fl_ws_vfolder_class);
	class_register(CLASS_BOX, fl_ws_vfolder_class);
}

/* max stuff: 'new instance' routine, float method , assist method --------------------------------------------------*/
void *fl_ws_vfolder_new(t_symbol *s, short argc, t_atom *argv)
{
	t_fl_ws_vfolder *x = (t_fl_ws_vfolder *)object_alloc(fl_ws_vfolder_class); /* instantiate a new object */

	dsp_setup((t_pxobject *)x, NUM_INLETS);
	outlet_new((t_object *)x, "signal");
	x->obj.z_misc |= Z_NO_INPLACE;

	x->param = DEFAULT_PARAM;

	return x;
}

void fl_ws_vfolder_float(t_fl_ws_vfolder *x, double farg)
{
	long inlet = ((t_pxobject *)x)->z_in;
	float num = (float)(1. - farg);

	switch (inlet) {
	case 1:
		x->param = (float)max(0., min(1., num)); /* parse parameter */
		break;
	default:
		break;
	}
}

void fl_ws_vfolder_assist(t_fl_ws_vfolder *x, void *b, long msg, long arg, char *dst)
{
	if (msg == ASSIST_INLET) {
		switch (arg) {
		case I_INPUT: sprintf(dst, "(signal) Input"); break;
		case I_PARAM: sprintf(dst, "(float/signal) Param en [0, 1]"); break;
		}
	}

	else if (msg == ASSIST_OUTLET) {
		switch (arg) {
		case O_OUTPUT: sprintf(dst, "(signal) Output"); break;
		}
	}
}

/* memory */
void fl_ws_vfolder_free(t_fl_ws_vfolder *x)
{
	dsp_free((t_pxobject *)x);
}

/* audio */
void fl_ws_vfolder_dsp64(t_fl_ws_vfolder *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
	x->param_connected = count[1]; /* save connection state */

	object_method(dsp64, gensym("dsp_add64"), x, fl_ws_vfolder_perform64, 0, NULL);
}

void fl_ws_vfolder_perform64(t_fl_ws_vfolder *x, t_object *dsp64, double **inputs, long numinputs, double **outputs, long numoutputs, long vectorsize, long flags, void *userparams)
{
	/* Copy signal pointers */
	t_double *input = inputs[0];
	t_double *param_sig = inputs[1];
	t_double *output = outputs[0];

	long n = vectorsize;

	float param = x->param;
	double a_in = 0.;
	double val = 0.;

	/* perform dsp loop */
	while (n--) {
		if (x->param_connected) {
			param = (float)(1. - *param_sig++);
			param = (float)max(0., min(1., param));
		}

		a_in = *input++;

		if (a_in > param) {
			val = -a_in + 2 * param;
		}
		else if (a_in < -param) {
			val = -a_in - 2 * param;
		}
		else {
			val = a_in;
		}

		*output++ = val;
	}
}