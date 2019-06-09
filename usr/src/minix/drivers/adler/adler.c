#include <minix/drivers.h>
#include <minix/chardriver.h>
#include <stdio.h>
#include <stdlib.h>
#include <minix/ds.h>

#define ADLER_SIZE 1024
#define ADLER_N 8

/*
 * Adler32 algorithm function prototypes.
 */
static void adler_reset();
static u32_t adler_get();
static void adler_count(const unsigned char *buf, size_t buf_length);

/*
 * Function prototypes for the adler driver.
 */
static int adler_open(devminor_t minor, int access, endpoint_t user_endpt);
static int adler_close(devminor_t minor);
static ssize_t adler_read(devminor_t minor, u64_t position, endpoint_t endpt,
    cp_grant_id_t grant, size_t size, int flags, cdev_id_t id);
static ssize_t adler_write(devminor_t minor, u64_t position, endpoint_t endpt,
    cp_grant_id_t grant, size_t size, int flags, cdev_id_t id);

/* SEF functions and variables. */
static void sef_local_startup(void);
static int sef_cb_init(int type, sef_init_info_t *info);
static int sef_cb_lu_state_save(int);
static int lu_state_restore(void);

/* Entry points to the adler driver. */
static struct chardriver adler_tab =
{
    .cdr_open	= adler_open,
    .cdr_close	= adler_close,
    .cdr_read	= adler_read,
    .cdr_write  = adler_write,
};

/* State variables. */
static u32_t A;
static u32_t B;

static void adler_reset()
{
    A = 1;
    B = 0;
}

static u32_t adler_get()
{
    return (B << 16) + A;
}

static void adler_count(const unsigned char *buf, size_t buf_length)
{
    while (buf_length--) {
        A = (A + *(buf++)) % 65521;
        B = (B + A) % 65521;
    }
}

static int adler_open(devminor_t UNUSED(minor), int UNUSED(access),
    endpoint_t UNUSED(user_endpt))
{
    return OK;
}

static int adler_close(devminor_t UNUSED(minor))
{
    return OK;
}

static ssize_t adler_read(devminor_t UNUSED(minor), u64_t UNUSED(position),
    endpoint_t endpt, cp_grant_id_t grant, size_t size, int UNUSED(flags),
    cdev_id_t UNUSED(id))
{
    int ret;
    char hash[ADLER_N + 1];

    if (size < ADLER_N) return EINVAL;

    snprintf(hash, ADLER_N + 1, "%08x", adler_get());
    if ((ret = sys_safecopyto(endpt, grant, 0, (vir_bytes) hash, ADLER_N)) != OK)
        return ret;

    adler_reset();
    return ADLER_N;
}

static ssize_t adler_write(devminor_t UNUSED(minor), u64_t UNUSED(position),
    endpoint_t endpt, cp_grant_id_t grant, size_t size, int UNUSED(flags),
    cdev_id_t UNUSED(id))
{
    int ret;
    unsigned char buf[ADLER_SIZE + 1];

    size = MIN(size, ADLER_SIZE);
    if ((ret = sys_safecopyfrom(endpt, grant, 0, (vir_bytes) buf, size)) != OK)
        return ret;

    adler_count(buf, size);
    /* Return the number of bytes read. */
    return size;
}

static int sef_cb_lu_state_save(int UNUSED(state)) {
/* Save the state. */
    ds_publish_u32("adler_A", A, DSF_OVERWRITE);
    ds_publish_u32("adler_B", B, DSF_OVERWRITE);

    return OK;
}

static int lu_state_restore() {
/* Restore the state. */
    ds_retrieve_u32("adler_A", &A);
    ds_delete_u32("adler_A");

    ds_retrieve_u32("adler_B", &B);
    ds_delete_u32("adler_B");

    return OK;
}

static void sef_local_startup()
{
    /*
     * Register init callbacks. Use the same function for all event types
     */
    sef_setcb_init_fresh(sef_cb_init);
    sef_setcb_init_lu(sef_cb_init);
    sef_setcb_init_restart(sef_cb_init);

    /*
     * Register live update callbacks.
     */
    /* - Agree to update immediately when LU is requested in a valid state. */
    sef_setcb_lu_prepare(sef_cb_lu_prepare_always_ready);
    /* - Support live update starting from any standard state. */
    sef_setcb_lu_state_isvalid(sef_cb_lu_state_isvalid_standard);
    /* - Register a custom routine to save the state. */
    sef_setcb_lu_state_save(sef_cb_lu_state_save);

    /* Let SEF perform startup. */
    sef_startup();
}

static int sef_cb_init(int type, sef_init_info_t *UNUSED(info))
{
/* Initialize the adler driver. */
    int do_announce_driver = TRUE;

    adler_reset();
    switch(type) {
        case SEF_INIT_FRESH:
        break;

        case SEF_INIT_LU:
            /* Restore the state. */
            lu_state_restore();
            do_announce_driver = FALSE;
        break;

        case SEF_INIT_RESTART:
        break;
    }

    /* Announce we are up when necessary. */
    if (do_announce_driver) {
        chardriver_announce();
    }

    /* Initialization completed successfully. */
    return OK;
}

int main(void)
{
    /*
     * Perform initialization.
     */
    sef_local_startup();

    /*
     * Run the main loop.
     */
    chardriver_task(&adler_tab);
    return OK;
}

