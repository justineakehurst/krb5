/*
 * lib/krb5/krb/copy_princ.c
 *
 * Copyright 1990 by the Massachusetts Institute of Technology.
 * All Rights Reserved.
 *
 * Export of this software from the United States of America may
 *   require a specific license from the United States Government.
 *   It is the responsibility of any person or organization contemplating
 *   export to obtain such a license before exporting.
 * 
 * WITHIN THAT CONSTRAINT, permission to use, copy, modify, and
 * distribute this software and its documentation for any purpose and
 * without fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright notice and
 * this permission notice appear in supporting documentation, and that
 * the name of M.I.T. not be used in advertising or publicity pertaining
 * to distribution of the software without specific, written prior
 * permission.  M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is" without express
 * or implied warranty.
 * 
 *
 * krb5_copy_principal()
 */

#include "k5-int.h"

/*
 * Copy a principal structure, with fresh allocation.
 */
krb5_error_code INTERFACE
krb5_copy_principal(context, inprinc, outprinc)
    krb5_context context;
    krb5_const_principal inprinc;
    krb5_principal *outprinc;
{
    register krb5_principal tempprinc;
    register int i, nelems;

    tempprinc = (krb5_principal)malloc(sizeof(krb5_principal_data));

    if (tempprinc == 0)
	return ENOMEM;

    *tempprinc = *inprinc;	/* Copy all of the non-allocated pieces */

    nelems = krb5_princ_size(context, inprinc);
    tempprinc->data = malloc(nelems * sizeof(krb5_data));

    if (tempprinc->data == 0) {
	free((char *)tempprinc);
	return ENOMEM;
    }

    for (i = 0; i < nelems; i++) {
	int len = krb5_princ_component(context, inprinc, i)->length;
	krb5_princ_component(context, tempprinc, i)->length = len;
	if ((krb5_princ_component(context, tempprinc, i)->data = malloc(len)) == 0) {
	    while (--i >= 0)
		free(krb5_princ_component(context, tempprinc, i)->data);
	    free (tempprinc->data);
	    free (tempprinc);
	    return ENOMEM;
	}
	memcpy(krb5_princ_component(context, tempprinc, i)->data,
	       krb5_princ_component(context, inprinc, i)->data, len);
    }

    tempprinc->realm.data =
	    malloc(tempprinc->realm.length = inprinc->realm.length);
    if (!tempprinc->realm.data) {
	    for (i = 0; i < nelems; i++)
		    free(krb5_princ_component(context, tempprinc, i)->data);
	    free(tempprinc->data);
	    free(tempprinc);
	    return ENOMEM;
    }
    memcpy(tempprinc->realm.data, inprinc->realm.data, inprinc->realm.length);
    
    *outprinc = tempprinc;
    return 0;
}
