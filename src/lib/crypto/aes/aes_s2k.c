/* Insert MIT copyright here.  */

#include "k5-int.h"
#include "dk.h"
#include "aes_s2k.h"

krb5_error_code
krb5int_aes_string_to_key(const struct krb5_enc_provider *enc,
			  const krb5_data *string,
			  const krb5_data *salt,
			  const krb5_data *params,
			  krb5_keyblock *key)
{
    unsigned long iter_count;
    krb5_data out;
    static const krb5_data usage = { KV5M_DATA, 8, "kerberos" };
    krb5_error_code err;

    if (params) {
	unsigned char *p = (unsigned char *) params->data;
	if (params->length != 4)
	    return KRB5_ERR_BAD_S2K_PARAMS;
	iter_count = ((p[0] << 24) | (p[1] << 16) | (p[2] <<  8) | (p[3]));
	if (iter_count == 0) {
	    iter_count = (1L << 16) << 16;
	    if (((iter_count >> 16) >> 16) != 1)
		return KRB5_ERR_BAD_S2K_PARAMS;
	}
    } else
	iter_count = 0xb000L;

    /* This is not a protocol specification constraint; this is an
       implementation limit, which should eventually be controlled by
       a config file.  */
    if (iter_count >= 0x1000000L)
	return KRB5_ERR_BAD_S2K_PARAMS;

    /*
     * Dense key space, no parity bits or anything, so take a shortcut
     * and use the key contents buffer for the generated bytes.
     */
    out.data = (char *) key->contents;
    out.length = key->length;
    if (out.length != 16 && out.length != 32)
	return KRB5_CRYPTO_INTERNAL;

    err = krb5int_pbkdf2_hmac_sha1 (&out, iter_count, string, salt);
    if (err) {
	memset(out.data, 0, out.length);
	return err;
    }

    err = krb5_derive_key (enc, key, key, &usage);
    if (err) {
	memset(out.data, 0, out.length);
	return err;
    }
    return 0;
}
