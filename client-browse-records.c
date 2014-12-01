/***
  This file is modified from client-browse-services.c which is part of avahi.

  avahi is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation; either version 2.1 of the
  License, or (at your option) any later version.

  avahi is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General
  Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with avahi; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
  USA.
***/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>

#include <avahi-client/client.h>
#include <avahi-client/lookup.h>

#include <avahi-common/simple-watch.h>
#include <avahi-common/malloc.h>
#include <avahi-common/error.h>

static AvahiSimplePoll *simple_poll = NULL;

static void browse_callback(
    AvahiRecordBrowser *b,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiBrowserEvent event,
    const char *name,
    uint16_t clazz,
	uint16_t type,
    const void *rdata,
	size_t size,
    AVAHI_GCC_UNUSED AvahiLookupResultFlags flags,
    void* userdata) {

    AvahiClient *c = userdata;
    assert(b);

    /* Called whenever a new services becomes available on the LAN or is removed from the LAN */

    switch (event) {
        case AVAHI_BROWSER_FAILURE:

            fprintf(stderr, "(Browser) %s\n", avahi_strerror(avahi_client_errno(avahi_record_browser_get_client(b))));
            avahi_simple_poll_quit(simple_poll);
            return;

        case AVAHI_BROWSER_NEW:
            fprintf(stderr, "(Browser) NEW: record '%s' of type '%s' clazz '%s'\n", name, type, clazz);
            break;

        case AVAHI_BROWSER_REMOVE:
            fprintf(stderr, "(Browser) REMOVE: record '%s' of type '%s' clazz '%s'\n", name, type, clazz);
            break;

        case AVAHI_BROWSER_ALL_FOR_NOW:
        case AVAHI_BROWSER_CACHE_EXHAUSTED:
            fprintf(stderr, "(Browser) %s\n", event == AVAHI_BROWSER_CACHE_EXHAUSTED ? "CACHE_EXHAUSTED" : "ALL_FOR_NOW");
            break;
    }
}

static void client_callback(AvahiClient *c, AvahiClientState state, AVAHI_GCC_UNUSED void * userdata) {
    assert(c);

    /* Called whenever the client or server state changes */

    if (state == AVAHI_CLIENT_FAILURE) {
        fprintf(stderr, "Server connection failure: %s\n", avahi_strerror(avahi_client_errno(c)));
        avahi_simple_poll_quit(simple_poll);
    }
}

int main(AVAHI_GCC_UNUSED int argc, AVAHI_GCC_UNUSED char*argv[]) {
    AvahiClient *client = NULL;
    AvahiRecordBrowser *sb = NULL;
    int error;
    int ret = 1;

    /* Allocate main loop object */
    if (!(simple_poll = avahi_simple_poll_new())) {
        fprintf(stderr, "Failed to create simple poll object.\n");
        goto fail;
    }

    /* Allocate a new client */
    client = avahi_client_new(avahi_simple_poll_get(simple_poll), 0, client_callback, NULL, &error);

    /* Check wether creating the client object succeeded */
    if (!client) {
        fprintf(stderr, "Failed to create client: %s\n", avahi_strerror(error));
        goto fail;
    }

    /* Create the record browser */
	/*avahi_record_browser_new	(	AvahiClient * 	client,
									AvahiIfIndex 	interface,
									AvahiProtocol 	protocol,
									const char * 	name,
									uint16_t 	clazz,
									uint16_t 	type,
									AvahiLookupFlags 	flags,
									AvahiRecordBrowserCallback 	callback,
									void * 	userdata 
								)	*/
    if (!(sb = avahi_record_browser_new(client, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, "Brother DCP-8011DN", AVAHI_DNS_CLASS_IN, AVAHI_DNS_TYPE_A, 0, browse_callback, client))) {
        fprintf(stderr, "Failed to create record browser: %s\n", avahi_strerror(avahi_client_errno(client)));
        goto fail;
    }

    /* Run the main loop */
    avahi_simple_poll_loop(simple_poll);

    ret = 0;

fail:

    /* Cleanup things */
    if (sb)
        avahi_record_browser_free(sb);

    if (client)
        avahi_client_free(client);

    if (simple_poll)
        avahi_simple_poll_free(simple_poll);

    return ret;
}
