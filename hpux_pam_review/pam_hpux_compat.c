 cat > pam_hpux_compat.c <<'EOF'
#define _GNU_SOURCE
#include <security/pam_modules.h>
#include <security/pam_appl.h>
#include <security/pam_ext.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <shadow.h>
#include <crypt.h>
#include <syslog.h>

/* Read current shadow hash for user (PAM runs as root, so this should succeed) */
static int get_shadow_hash(pam_handle_t *pamh, const char *user, const char **out_hash) {
    if (!user) return PAM_AUTHINFO_UNAVAIL;
    errno = 0;
    struct spwd *sp = getspnam(user);
    if (!sp || !sp->sp_pwdp || sp->sp_pwdp[0] == '\0') {
        pam_syslog(pamh, LOG_ERR, "hpux_compat: unable to read shadow for %s (errno=%d)", user, errno);
        return PAM_AUTHINFO_UNAVAIL;
    }
    *out_hash = sp->sp_pwdp;
    return PAM_SUCCESS;
}

PAM_EXTERN int pam_sm_authenticate(pam_handle_t *pamh, int flags, int argc, const char **argv) {
    (void)flags; (void)argc; (void)argv;

    const char *user = NULL;
    int rv = pam_get_user(pamh, &user, NULL);
    if (rv != PAM_SUCCESS || !user) return PAM_AUTHINFO_UNAVAIL;

    /* get the password (authtok) from PAM; if missing, this will prompt */
    const char *pass = NULL;
    rv = pam_get_authtok(pamh, PAM_AUTHTOK, &pass, NULL);
    if (rv != PAM_SUCCESS || !pass) return PAM_AUTHINFO_UNAVAIL;

    /* fetch stored hash (provides algo+salt) */
    const char *hash = NULL;
    rv = get_shadow_hash(pamh, user, &hash);
    if (rv != PAM_SUCCESS) return rv;

    /* truncate to first 8 chars like HP-UX */
    char trunc[9];
    size_t n = strlen(pass);
    if (n > 8) n = 8;
    memcpy(trunc, pass, n);
    trunc[n] = '\0';

    /* compute hash using same salt/alg as stored */
    struct crypt_data cd;
    memset(&cd, 0, sizeof(cd));
    char *calc = crypt_r(trunc, hash, &cd);
    if (!calc) {
        pam_syslog(pamh, LOG_ERR, "hpux_compat: crypt_r failed: %s", strerror(errno));
        return PAM_AUTH_ERR;
    }

    return (strcmp(calc, hash) == 0) ? PAM_SUCCESS : PAM_AUTH_ERR;
}

PAM_EXTERN int pam_sm_setcred(pam_handle_t *pamh, int flags, int argc, const char **argv) {
    (void)pamh; (void)flags; (void)argc; (void)argv;
    return PAM_SUCCESS;
}
EOF