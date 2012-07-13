#include <sys/modctl.h>
#include <sys/conf.h>
#include <sys/devops.h>
#include <sys/cmn_err.h>
#include <sys/ddi.h>
#include <sys/sunddi.h>

#define	QOTD_MAXLEN	128

static const char qotd[QOTD_MAXLEN]
	= "Be careful about reading health books. \
You may die of a misprint. - Mark Twain\n";

static struct dev_ops qotd_dev_ops = {
	DEVO_REV,		/* devo_rev */
	0,			/* devo_refcnt */
	ddi_no_info,		/* devo_getinfo */
	nulldev,		/* devo_identify */
	nulldev,		/* devo_probe */
	nulldev,		/* devo_attach */
	nulldev,		/* devo_detach */
	nodev,			/* devo_reset */
	(struct cb_ops *)NULL,	/* devo_cb_ops */
	(struct bus_ops *)NULL,	/* devo_bus_ops */
	nulldev			/* devo_power */
};

static struct modldrv modldrv = {
	&mod_driverops,
	"Quote of the Day 1.0",
	&qotd_dev_ops
};

static struct modlinkage modlinkage = {
	MODREV_1,
	(void *)&modldrv,
	NULL
};

int
_init(void)
{
	cmn_err(CE_CONT, "QOTD: %s\n", qotd);
	return (mod_install(&modlinkage));
}

int
_info(struct modinfo *modinfop)
{
	return (mod_info(&modlinkage, modinfop));
}

int
_fini(void)
{
	return (mod_remove(&modlinkage));
}
