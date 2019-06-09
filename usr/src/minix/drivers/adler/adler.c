#include <minix/drivers.h>
#include <minix/chardriver.h>
#include <stdio.h>
#include <stdlib.h>
#include <minix/ds.h>

#define ADLER_SIZE 1024
#define ADLER_MESSAGE "ADLER32!\n"

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

/** State variable to count the number of times the device has been opened.
 * Note that this is not the regular type of open counter: it never decreases.
 */
static int open_counter;
static char buffer[ADLER_SIZE + 1]; // +1 for \0 for strlen
static uint32_t A;
static uint32_t B;

void adler_reset()
{
    A = 1;
    B = 0;
}

uint32_t adler_get()
{
    uint32_t hash = (B << 16) + A;
    adler_reset();
    return hash;
}

uint32_t adler_count(const char* buf, size_t buf_length)
{
    while( buf_length-- ) {
        A = (A + *( buf++ )) % 65521;
        B = (B + A) % 65521;
    }
}

static int adler_open(devminor_t UNUSED(minor), int UNUSED(access),
    endpoint_t UNUSED(user_endpt))
{
    printf("adler_open(). Called %d time(s).\n", ++open_counter);
    return OK;
}

static int adler_close(devminor_t UNUSED(minor))
{
    printf("adler_close()\n");
    return OK;
}

static ssize_t adler_read(devminor_t UNUSED(minor), u64_t position,
    endpoint_t endpt, cp_grant_id_t grant, size_t size, int UNUSED(flags),
    cdev_id_t UNUSED(id))
{
    u64_t dev_size;
    char *ptr;
    int ret;
    char *buf = buffer;

    printf("adler_read()\n");

    /* This is the total size of our device. */
    dev_size = (u64_t) ADLER_SIZE;

    /* Check for EOF, and possibly limit the read size. */
    if (position >= dev_size) return 0;		/* EOF */
    if (position + size > dev_size)
        size = (size_t)(dev_size - position);	/* limit size */

    /* Copy the requested part to the caller. */
    ptr = buf + (size_t)position;
    if ((ret = sys_safecopyto(endpt, grant, 0, (vir_bytes) ptr, size)) != OK)
        return ret;

    /* Return the number of bytes read. */
    return size;
}

static ssize_t adler_write(devminor_t UNUSED(minor), u64_t position,
    endpoint_t endpt, cp_grant_id_t grant, size_t size, int UNUSED(flags),
    cdev_id_t UNUSED(id))
{
    u64_t dev_size;
    char *ptr;
    int ret;
    char *buf = buffer;

    /* This is the total size of our device. */
    dev_size = (u64_t) ADLER_SIZE;

    /* Check for EOF, and possibly limit the read size. */
    if (position >= dev_size) return 0;		/* EOF */
    if (position + size > dev_size)
        size = (size_t)(dev_size - position);	/* limit size */

    /* Copy the requested part to the caller. */
    ptr = buf + (size_t)position;
    if ((ret = sys_safecopyfrom(endpt, grant, 0, (vir_bytes) ptr, size)) != OK)
        return ret;

    /* Return the number of bytes read. */
    return size;
}

static int sef_cb_lu_state_save(int UNUSED(state)) {
/* Save the state. */
    ds_publish_u32("adler_counter", open_counter, DSF_OVERWRITE);
    ds_publish_mem("adler_buffer", buffer, ADLER_SIZE, DSF_OVERWRITE);
    return OK;
}

static int lu_state_restore() {
/* Restore the state. */
    u32_t value;

    ds_retrieve_u32("adler_counter", &value);
    ds_delete_u32("adler_counter");
    open_counter = (int) value;

    size_t length = ADLER_SIZE;
    ds_retrieve_mem("adler_buffer", buffer, &length);
    ds_delete_mem("adler_buffer");

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

    memset(buffer, (int) 'a', ADLER_SIZE);
    buffer[ADLER_SIZE] = '\0';
    open_counter = 0;
    adler_reset();
    switch(type) {
        case SEF_INIT_FRESH:
            printf("%s", ADLER_MESSAGE);
        break;

        case SEF_INIT_LU:
            /* Restore the state. */
            lu_state_restore();
            do_announce_driver = FALSE;

            printf("%sHey, I'm a new version!\n", ADLER_MESSAGE);
        break;

        case SEF_INIT_RESTART:
            printf("%sHey, I've just been restarted!\n", ADLER_MESSAGE);
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

