/**
 * This overlay adds OCF specific virtual attributes.
 *
 * Virtual Attribute list:
 *     ocfMail: uid@ocf.berkeley.edu into
 */
#include "portable.h"

#include <stdio.h>

#include <ac/string.h>
#include <ac/socket.h>

#include "slap.h"
#include "config.h"

// https://github.com/ocf/ocflib/blob/555dd97e22858396a311ef2addfe8ff747128b16/ocflib/account/validators.py#L337
#define USERNAME_MAX 16
#define OCF_MAIL_SUFFIX "@ocf.berkeley.edu"
#define OCF_MAIL_LEN sizeof(OCF_MAIL_SUFFIX) - 1

// Name of "virtual" attribute.
static const char *ATTR_OCFEMAIL = "ocfEmail";

// Names of source attributes.
static const char *ATTR_UID = "uid";
static const char *SOURCE_ATTRS = "uid";

static AttributeDescription *uid_ad;
static AttributeDescription *ocfemail_ad;

static slap_overinst ocfvirt;

static struct berval *read_attr(Entry *entry, AttributeDescription *attr_desc) {
    const Attribute *attr = attr_find(entry->e_attrs, attr_desc);

    return attr ? &attr->a_vals[0] : NULL;
}

/**
 * If client has specified attributes to retrieve and the list contains
 * ATTR_OCFEMAIL, add SOURCE_ATTRS to the list.
 *
 * This is needed for e.g. ldap backend, otherwise the source attributes
 * would not be available for this overlay. However, there's no code to
 * remove extra attributes after processing by this overlay, which is not
 * very nice, but whatever...
 */
static int ocfvirt_search(Operation *op, SlapReply *rs) {
    // Is this our own internal search? Ignore it.
    if (op->o_no_schema_check) {
        return SLAP_CB_CONTINUE;
    }

    // If there are some requested attributes.
    if (op->ors_attrs) {
        // If requested attributes contains ocfEmail.
        if (an_find(op->ors_attrs, &ocfemail_ad->ad_cname)) {
            // Append needed source attributes to it.
            str2anlist(op->ors_attrs, SOURCE_ATTRS, ",");
        }
    }
    return SLAP_CB_CONTINUE;
}

/**
 * Add virtual attribute ATTR_OCFEMAIL to the search response.
 * This function is invoked on every return from LDAP to the client.
 */
static int ocfvirt_response(Operation *op, SlapReply *rs) {
    char ocfemail_virt[OCF_MAIL_LEN + USERNAME_MAX + 1];

    // Do nothing if we are connected as admin; admin "view"
    // is not altered in any way.
    if (be_isroot(op)) {
        return SLAP_CB_CONTINUE;
    }

    // Do nothing if not in the search operation. "Search" is in LDAP what
    // you would usually call "retrieve" or something.
    // TODO: support for compare op.
    if (rs->sr_type != REP_SEARCH) {
        return SLAP_CB_CONTINUE;
    }

    // The usual setup you'll see in any overlay's _response function.
    const slap_overinst *on = (slap_overinst *)op->o_bd->bd_info;
    op->o_bd->bd_info = (BackendInfo *)on->on_info;

    // The current entry may live in a cache, so don't modify it directly.
    // Make a copy and work with that instead.
    (void)rs_entry2modifiable(op, rs, on);
    const Entry *entry = rs->sr_entry;

    struct berval *uid_bv = read_attr(entry, uid_ad);
    // If attribute "uid" not set, do nothing.
    if (!uid_bv) {
        return SLAP_CB_CONTINUE;
    }

    // uid_bv->bv_val may or may not be NULL terminated, and lber-types(3) not
    // clear whether bv_len includes the NULL terminator. Let's just be extra
    // safe and also handle the case where it does.
    if (uid_bv->bv_len > USERNAME_MAX &&
        !(uid_bv->bv_len == USERNAME_MAX + 1 &&
          uid_bv->bv_val[USERNAME_MAX] == '\0')) {
        return SLAP_CB_CONTINUE;
    }

    struct berval ocfemail_bv;
    *ocfemail_virt = '\0';
    strncat(ocfemail_virt, uid_bv->bv_val, uid_bv->bv_len);
    strcat(ocfemail_virt, OCF_MAIL_SUFFIX);
    (void)ber_str2bv(ocfemail_virt, uid_bv->bv_len + OCF_MAIL_LEN, 0,
                     &ocfemail_bv);

    // Add/replace attribute ATTR_OCFEMAIL in the entry.
    (void)attr_delete(&entry->e_attrs, ocfemail_ad);
    (void)attr_merge_normalize_one(entry, ocfemail_ad, &ocfemail_bv,
                                   op->o_tmpmemctx);
    return SLAP_CB_CONTINUE;
}

static int ocfvirt_db_destroy(BackendDB *be, ConfigReply *cr) {
    return LDAP_SUCCESS;
}

static int ocfvirt_db_init(BackendDB *be, ConfigReply *cr) {
    const char *err_msg;

    if (slap_str2ad(ATTR_UID, &uid_ad, &err_msg) != LDAP_SUCCESS) {
        Debug(LDAP_DEBUG_ANY, "uid: attribute '%s': %s.\n", ATTR_UID, err_msg,
              0);
        return -1;
    }
    if (slap_str2ad(ATTR_OCFEMAIL, &ocfemail_ad, &err_msg) != LDAP_SUCCESS) {
        Debug(LDAP_DEBUG_ANY, "ocfEmail: attribute '%s': %s.\n", ATTR_OCFEMAIL,
              err_msg, 0);
        return -1;
    }
    return LDAP_SUCCESS;
}

/**
 * The easily recognizable init function, similarly done in all projects
 * providing dynamic modules functionality.
 *
 */
int ocfvirt_initialize() {
    // Register name and callbacks.
    ocfvirt.on_bi.bi_type = "ocfvirt";
    ocfvirt.on_bi.bi_db_init = ocfvirt_db_init;
    ocfvirt.on_bi.bi_db_destroy = ocfvirt_db_destroy;
    ocfvirt.on_bi.bi_op_search = ocfvirt_search;
    ocfvirt.on_response = ocfvirt_response;

    return overlay_register(&ocfvirt);
}

int init_module(int argc, char *argv[]) { return ocfvirt_initialize(); }
