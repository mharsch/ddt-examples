#include <sys/types.h>
#include <sys/file.h>
#include <sys/errno.h>
#include <sys/open.h>
#include <sys/cred.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <sys/modctl.h>
#include <sys/conf.h>
#include <sys/devops.h>
#include <sys/debug.h>
#include <sys/cmn_err.h>
#include <sys/ddi.h>
#include <sys/sunddi.h>

#define	QOTD_NAME	"qotd"
#define	QOTD_MAXLEN	128

static const char qotd[QOTD_MAXLEN]
	= "You can't have everything. \
Where would you put it? - Steven Wright\n";

static void *qotd_state_head;

struct qotd_state {
	int		instance;
	dev_info_t	*devi;
};

static int qotd_getinfo(dev_info_t *, ddi_info_cmd_t, void *, void **);
static int qotd_attach(dev_info_t *, ddi_attach_cmd_t);
static int qotd_detach(dev_info_t *, ddi_detach_cmd_t);
static int qotd_open(dev_t *, int, int, cred_t *);
static int qotd_close(dev_t, int, int, cred_t *);
static int qotd_read(dev_t, struct uio *, cred_t *);

static struct cb_ops qotd_cb_ops = {
	qotd_open,		/* cb_open */
	qotd_close,		/* cb_close */
	nodev,			/* cb_strategy */
	nodev,			/* cb_print */
	nodev,			/* cb_dump */
	qotd_read,		/* cb_read */
	nodev,			/* cb_write */
	nodev,			/* cb_ioctl */
	nodev,			/* cb_devmap */
	nodev,			/* cb_mmap */
	nodev,			/* cb_segmap */
	nochpoll,		/* cb_chpoll */
	ddi_prop_op,		/* cb_cb_prop_op */
	(struct streamtab *)NULL,		/* cb_str */
	D_MP | D_64BIT,		/* cb_flag */
	CB_REV,			/* cb_rev */
	nodev,			/* cb_aread */
	nodev			/* cb_awrite */
};

static struct dev_ops qotd_dev_ops = {
	DEVO_REV,		/* devo_rev */
	0,			/* devo_refcnt */
	qotd_getinfo,		/* devo_getinfo */
	nulldev,		/* devo_identify */
	nulldev,		/* devo_probe */
	qotd_attach,		/* devo_attach */
	qotd_detach,		/* devo_detach */
	nulldev,		/* devo_reset */
	&qotd_cb_ops,		/* devo_cb_ops */
	(struct bus_ops *)NULL,	/* devo_bus_ops */
	nulldev,		/* devo_power */
};

static struct modldrv modldrv = {
	&mod_driverops,
	"Quote of the Day 2.0",
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
	int retval;

	if ((retval = ddi_soft_state_init(&qotd_state_head,
	    sizeof (struct qotd_state), 1)) != 0)
		return (retval);

	if ((retval = mod_install(&modlinkage)) != 0) {
		ddi_soft_state_fini(&qotd_state_head);
		return (retval);
	}

	return (retval);
}

int
_info(struct modinfo *modinfop)
{
	return (mod_info(&modlinkage, modinfop));
}

int
_fini(void)
{
	int retval;

	if ((retval = mod_remove(&modlinkage)) != 0)
		return (retval);

	ddi_soft_state_fini(&qotd_state_head);

	return (retval);
}

/*ARGSUSED*/
static int
qotd_getinfo(dev_info_t *dip, ddi_info_cmd_t cmd, void *arg, void **resultp)
{
	struct qotd_state *qsp;
	int retval = DDI_FAILURE;

	ASSERT(resultp != NULL);

	switch (cmd) {
	case DDI_INFO_DEVT2DEVINFO:
		if ((qsp = ddi_get_soft_state(qotd_state_head,
		    getminor((dev_t)arg))) != NULL) {
			*resultp = qsp->devi;
			retval = DDI_SUCCESS;
		} else {
			*resultp = NULL;
		}

		break;
	}

	return (retval);
}

static int
qotd_attach(dev_info_t *dip, ddi_attach_cmd_t cmd)
{
	int instance = ddi_get_instance(dip);
	struct qotd_state *qsp;

	switch (cmd) {
	case DDI_ATTACH:
		if (ddi_soft_state_zalloc(qotd_state_head, instance)
		    != DDI_SUCCESS) {
			cmn_err(CE_WARN, "Unable to allocate state for %d",
			    instance);
			return (DDI_FAILURE);
		}
		if ((qsp = ddi_get_soft_state(qotd_state_head, instance))
		    == NULL) {
			cmn_err(CE_WARN, "Unable to obtain state for %d",
			    instance);
			ddi_soft_state_free(dip, instance);
			return (DDI_FAILURE);
		}
		if (ddi_create_minor_node(dip, QOTD_NAME, S_IFCHR, instance,
		    DDI_PSEUDO, 0) != DDI_SUCCESS) {
			cmn_err(CE_WARN, "Cannot create minor node for %d",
			    instance);
			ddi_soft_state_free(dip, instance);
			ddi_remove_minor_node(dip, NULL);
			return (DDI_FAILURE);
		}
		qsp->instance = instance;
		qsp->devi = dip;

		ddi_report_dev(dip);
		return (DDI_SUCCESS);
	case DDI_RESUME:
		return (DDI_SUCCESS);
	default:
		return (DDI_FAILURE);
	}
}

static int
qotd_detach(dev_info_t *dip, ddi_detach_cmd_t cmd)
{
	int instance = ddi_get_instance(dip);

	switch (cmd) {
	case DDI_DETACH:
		ddi_soft_state_free(qotd_state_head, instance);
		ddi_remove_minor_node(dip, NULL);
		return (DDI_SUCCESS);
	case DDI_SUSPEND:
		return (DDI_SUCCESS);
	default:
		return (DDI_FAILURE);
	}
}

/*ARGSUSED*/
static int
qotd_open(dev_t *devp, int flag, int otyp, cred_t *credp)
{
	int instance = getminor(*devp);
	struct qotd_state *qsp;

	if ((qsp = ddi_get_soft_state(qotd_state_head, instance)) == NULL)
		return (ENXIO);

	ASSERT(qsp->instance == instance);

	if (otyp != OTYP_CHR)
		return (EINVAL);

	return (0);
}

/*ARGSUSED*/
static int
qotd_close(dev_t dev, int flag, int otyp, cred_t *credp)
{
	struct qotd_state *qsp;
	int instance = getminor(dev);

	if ((qsp = ddi_get_soft_state(qotd_state_head, instance)) == NULL)
		return (ENXIO);

	ASSERT(qsp->instance == instance);

	if (otyp != OTYP_CHR)
		return (EINVAL);

	return (0);
}

/*ARGSUSED*/
static int
qotd_read(dev_t dev, struct uio *uiop, cred_t *credp)
{
	struct qotd_state *qsp;
	int instance = getminor(dev);

	if ((qsp = ddi_get_soft_state(qotd_state_head, instance)) == NULL)
		return (ENXIO);

	ASSERT(qsp->instance == instance);

	return (uiomove((void *)qotd, min(uiop->uio_resid, strlen(qotd)),
	    UIO_READ, uiop));
}
