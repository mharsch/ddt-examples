/*
 * Minimalist pseudo-device.
 * Writes a message whenever a routine is entered.
 *
 * Build the driver:
 * 	cc -D_KERNEL -c dummy.c
 *	ld -r -o dummy dummy.o
 * Copy the driver and the configuration file to /usr/kernel/drv:
 *	cp dummy.conf /usr/kernel/drv
 *	cp dummy /tmp
 *	ln -s /tmp/dummy /usr/kernel/drv/dummy
 * Add the driver:
 *	add_drv dummy
 * Test (1) read from driver (2) write to driver:
 *	cat /devices/pseudo/dummy@*
 *	echo hello > 'ls /devices/pseudo/dummy@*'
 * Verify the tests in another window:
 *	tail -f /var/adm/messages
 * Remove the driver:
 *	rem_drv dummy
 */

#include <sys/devops.h>	/* used by dev_ops */
#include <sys/conf.h>	/* used by dev_ops and cb_ops */
#include <sys/modctl.h>	/* used by modlinkage, modldrv, _init, _info, */
			/* and _fini */
#include <sys/types.h>	/* used by open, close, read, write, prop_op, */
			/* and ddi_prop_op */
#include <sys/file.h>	/* used by open, close */
#include <sys/errno.h>	/* used by open, close, read, write */
#include <sys/open.h>	/* used by open, close, read, write */
#include <sys/cred.h>	/* used by open, close, read */
#include <sys/uio.h>	/* used by read */
#include <sys/stat.h>	/* defines S_IFCHR used by ddi_create_minor_node */
#include <sys/cmn_err.h>	/* used by all entry points for this driver */
#include <sys/ddi.h>	/* used by all entry points for this driver */
			/* also used by cb_ops, ddi_get_instance, and */
			/* ddi_prop_op */
#include <sys/sunddi.h>	/* used by all entry points for this driver */
			/* also used by cb_ops, ddi_create_minor_node, */
			/* ddi_get_instance, and ddi_prop_op */

static int dummy_attach(dev_info_t *dip, ddi_attach_cmd_t cmd);
static int dummy_detach(dev_info_t *dip, ddi_detach_cmd_t cmd);
static int dummy_getinfo(dev_info_t *dip, ddi_info_cmd_t cmd, void *arg,
    void **resultp);
static int dummy_prop_op(dev_t dev, dev_info_t *dip, ddi_prop_op_t prop_op,
    int flags, char *name, caddr_t valuep, int *lengthp);
static int dummy_open(dev_t *devp, int flag, int otyp, cred_t *cred);
static int dummy_close(dev_t dev, int flag, int otyp, cred_t *cred);
static int dummy_read(dev_t dev, struct uio *uiop, cred_t *credp);
static int dummy_write(dev_t dev, struct uio *uiop, cred_t *credp);

/* cb_ops structure */
static struct cb_ops dummy_cb_ops = {
	dummy_open,
	dummy_close,
	nodev,		/* no strategy - nodev returns ENXIO */
	nodev,		/* no print */
	nodev,		/* no dump */
	dummy_read,
	dummy_write,
	nodev,		/* no ioctl */
	nodev,		/* no devmap */
	nodev,		/* no mmap */
	nodev,		/* no segmap */
	nochpoll,	/* returns ENXIO for non-pollable devices */
	dummy_prop_op,
	NULL,		/* streamtab struct; if not NULL, all above */
			/* fields are ignored */
	D_NEW | D_MP,	/* compatibility flags: see conf.h */
	CB_REV,		/* cb_ops revision number */
	nodev,		/* no aread */
	nodev,		/* no awrite */
};

/* dev_ops structure */
static struct dev_ops dummy_dev_ops = {
	DEVO_REV,
	0,			/* reference count */
	dummy_getinfo,
	nulldev,		/* no identify - nulldev returns 0 */
	nulldev,		/* no probe */
	dummy_attach,
	dummy_detach,
	nodev,			/* no reset - nodev returns ENXIO */
	&dummy_cb_ops,
	(struct bus_ops *)NULL,
	nodev			/* no power */
};

/* modldrv structure */
static struct modldrv md = {
	&mod_driverops,		/* Type of module.  This is a driver. */
	"dummy driver",		/* Name of the module. */
	&dummy_dev_ops
};

/* modlinkage structure */
static struct modlinkage ml = {
	MODREV_1,
	&md,
	NULL
};

/* dev_info structure */
dev_info_t *dummy_dip;	/* keep track of one instance */

/* Loadable module configuration entry points */
int
_init(void)
{
	cmn_err(CE_NOTE, "Inside _init");
	return (mod_install(&ml));
}

int
_info(struct modinfo *modinfop)
{
	cmn_err(CE_NOTE, "Inside _info");
	return (mod_info(&ml, modinfop));
}

int
_fini(void)
{
	cmn_err(CE_NOTE, "Inside _fini");
	return (mod_remove(&ml));
}

/* Device configuration entry points */
static int
dummy_attach(dev_info_t *dip, ddi_attach_cmd_t cmd)
{
	cmn_err(CE_NOTE, "Inside dummy_attach");
	switch (cmd) {
	case DDI_ATTACH:
		dummy_dip = dip;
		if (ddi_create_minor_node(dip, "0", S_IFCHR,
		    ddi_get_instance(dip), DDI_PSEUDO, 0)
		    != DDI_SUCCESS) {
			cmn_err(CE_NOTE,
			    "%s%d: attach: could not add character node.",
			    "dummy", 0);
			return (DDI_FAILURE);
		} else {
			return (DDI_SUCCESS);
		}
	default:
		return (DDI_FAILURE);
	}
}

static int
dummy_detach(dev_info_t *dip, ddi_detach_cmd_t cmd)
{
	cmn_err(CE_NOTE, "Inside dummy_detach");
	switch (cmd) {
	case DDI_DETACH:
		dummy_dip = 0;
		ddi_remove_minor_node(dip, NULL);
		return (DDI_SUCCESS);
	default:
		return (DDI_FAILURE);
	}
}

static int
dummy_getinfo(dev_info_t *dip, ddi_info_cmd_t cmd, void *arg,
    void **resultp)
{
	cmn_err(CE_NOTE, "Inside dummy_getinfo");
	switch (cmd) {
	case DDI_INFO_DEVT2DEVINFO:
		*resultp = dummy_dip;
		return (DDI_SUCCESS);
	case DDI_INFO_DEVT2INSTANCE:
		*resultp = 0;
		return (DDI_SUCCESS);
	default:
		return (DDI_FAILURE);
	}
}

/* Main entry points */
static int
dummy_prop_op(dev_t dev, dev_info_t *dip, ddi_prop_op_t prop_op,
    int flags, char *name, caddr_t valuep, int *lengthp)
{
	cmn_err(CE_NOTE, "Inside dummy_prop_op");
	return (ddi_prop_op(dev, dip, prop_op, flags, name, valuep, lengthp));
}

static int
dummy_open(dev_t *devp, int flag, int otyp, cred_t *cred)
{
	cmn_err(CE_NOTE, "Inside dummy_open");
	return (DDI_SUCCESS);
}

static int
dummy_close(dev_t dev, int flag, int otyp, cred_t *cred)
{
	cmn_err(CE_NOTE, "Inside dummy_close");
	return (DDI_SUCCESS);
}

static int
dummy_read(dev_t dev, struct uio *uiop, cred_t *cred)
{
	cmn_err(CE_NOTE, "Inside dummy_read");
	return (DDI_SUCCESS);
}

static int
dummy_write(dev_t dev, struct uio *uiop, cred_t *cred)
{
	cmn_err(CE_NOTE, "Inside dummy_write");
	return (DDI_SUCCESS);
}
