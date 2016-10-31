/* network.c - Loopback demo: IPV6 + UDP */

/*
 * Copyright (c) 2015 Intel Corporation.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <zephyr.h>

#if defined(CONFIG_STDOUT_CONSOLE)
#include <stdio.h>
#define PRINT           printf
#else
#include <misc/printk.h>
#define PRINT           printk
#endif

#include <net/ip_buf.h>
#include <net/net_core.h>
#include <net/net_socket.h>

#include <net_driver_loopback.h>

/* Longer packet sending works only if fragmentation is supported
 * by network stack.
 */
/* Generated by http://www.lipsum.com/
 * 2 paragraphs, 185 words, 1231 bytes of Lorem Ipsum
 * The main() will add one null byte at the end so the maximum
 * length for the data to send is 1232 bytes.
 */
static const char *text =
	"Lorem ipsum dolor sit amet, consectetur adipiscing elit. Etiam "
	"congue non neque vel tempor. In id porta nibh, ut cursus tortor. "
	"Morbi eleifend tristique vehicula. Nunc vitae risus mauris. "
	"Praesent vel imperdiet dolor, et ultricies nibh. Aliquam erat "
	"volutpat. Maecenas pellentesque dolor vitae dictum tincidunt. "
	"Fusce vel nibh nec leo tristique auctor eu a massa. Nam et tellus "
	"ac tortor sollicitudin semper vitae nec tortor. Aliquam nec lacus "
	"velit. Maecenas ornare ullamcorper justo non auctor. Donec "
	"aliquam feugiat turpis, quis elementum sem rutrum ut. Sed eu "
	"ullamcorper libero, ut suscipit magna."
	"\n"
	"Donec vehicula magna ut varius aliquam. Ut vitae commodo nulla, "
	"quis ornare dolor. Nulla tortor sem, venenatis eu iaculis id, "
	"commodo ut massa. Sed est lorem, euismod vitae enim sed, "
	"hendrerit gravida felis. Donec eros lacus, auctor ut ultricies "
	"eget, lobortis quis nisl. Aliquam sit amet blandit eros. "
	"Interdum et malesuada fames ac ante ipsum primis in faucibus. "
	"Quisque egestas nisl leo, sed consectetur leo ornare eu. "
	"Suspendisse vitae urna vel purus maximus finibus. Proin sed "
	"sollicitudin turpis. Mauris interdum neque eu tellus "
	"pellentesque, id fringilla nisi fermentum. Suspendisse gravida "
	"pharetra sodales orci aliquam.";

/* Specify delay between greetings (in ms); compute equivalent in ticks */
#define SLEEPTIME  1000
#define SLEEPTICKS (SLEEPTIME * sys_clock_ticks_per_sec / 1000)

#define STACKSIZE 2000
static char receiver_stack[STACKSIZE];
static char sender_stack[STACKSIZE];

static struct net_addr any_addr;
static struct net_addr loopback_addr;

static int sent;
static int received;

static nano_thread_id_t sender_id;
static nano_thread_id_t receiver_id;

static int failure;
static int data_len;

/* How many packets to send/receive */
#if defined(CONFIG_NETWORK_LOOPBACK_TEST_COUNT)
#define TEST_COUNT CONFIG_NETWORK_LOOPBACK_TEST_COUNT
#else
#define TEST_COUNT 0
#endif
static unsigned long count = TEST_COUNT;

int eval_rcvd_data(char *rcvd_buf, int rcvd_len)
{
	int rc = 0;

	if (data_len != rcvd_len) {
		rc = -1;
		PRINT("Received %d bytes but was sent %d bytes\n",
		      rcvd_len, data_len);
	} else {
		/* Data integrity */
		rc = memcmp(text, rcvd_buf, data_len-1);
		if (rc != 0) {
			PRINT("Sent and received data does not match.\n");
			PRINT("Sent: %.*s\n", data_len, text);
			PRINT("Received: %.*s\n",
			      data_len, rcvd_buf);
		}
	}
	return rc;
}

void fiber_receiver(void)
{
	struct net_context *ctx;
	struct net_buf *buf;
	char *rcvd_buf;
	int rcvd_len;

	ctx = net_context_get(IPPROTO_UDP,
			      &any_addr, 0,
			      &loopback_addr, 4242);
	if (!ctx) {
		PRINT("%s: Cannot get network context\n", __func__);
		return;
	}

	while (!failure) {
		/* Fiber blocks until something is ready to be read */
		buf = net_receive(ctx, TICKS_UNLIMITED);
		if (buf) {
			/* Application level data and its length */
			rcvd_buf = ip_buf_appdata(buf);
			rcvd_len = ip_buf_appdatalen(buf);

			PRINT("[%d] %s: Received: %d bytes\n",
			      received, __func__, rcvd_len);

			if (eval_rcvd_data(rcvd_buf, rcvd_len) != 0) {
				PRINT("[%d] %s: net_receive failed!\n",
				      received, __func__);
				failure = 1;
			}
			ip_buf_unref(buf);
			received++;
		}
		fiber_wakeup(sender_id);
		fiber_sleep(SLEEPTICKS);
		if (count && (count < received)) {
			break;
		}
	}
}

void prepare_to_send(struct net_buf *buf, size_t *len)
{
	char *ptr;
	int text_len;

	text_len = strlen(text);
	*len = sys_rand32_get() % text_len;

	/* net_buf_add: returns a pointer to the current tail of
	 * buf->data before adding n bytes.
	 * Adding 0 bytes just allows us to get a pointer to the
	 * tail without affecting buf->len.
	 */
	ptr = net_buf_add(buf, 0);
	memcpy(ptr, text, *len);
	net_buf_add(buf, *len);
	/* We need to know where the text finishes to add the
	 * end-of-line character.
	 */
	ptr = net_buf_add(buf, 1);
	*ptr = '\0';

	*len += 1;
}

void fiber_sender(void)
{
	struct net_context *ctx;
	struct net_buf *buf;
	uint16_t sent_len;
	size_t len;
	int header_size;

	ctx = net_context_get(IPPROTO_UDP,
			      &loopback_addr, 4242,
			      &any_addr, 0);
	if (!ctx) {
		PRINT("Cannot get network context\n");
		return;
	}

	while (!failure) {
		buf = ip_buf_get_tx(ctx);
		if (buf) {
			prepare_to_send(buf, &len);
			sent_len = buf->len;
			header_size = ip_buf_reserve(buf);
			data_len = sent_len - header_size;

			PRINT("[%d] %s: App data: %d bytes, IPv6+UDP: %d bytes, "
			      "Total packet size: %d bytes\n",
			      sent, __func__, len, header_size, sent_len);

			if (net_send(buf) < 0) {
				PRINT("[%d] %s: net_send failed!\n",
				      sent, __func__);
				failure = 1;
			}
			ip_buf_unref(buf);
			sent++;
		}
		fiber_wakeup(receiver_id);
		fiber_sleep(SLEEPTICKS);
		if (sent != received) {
			failure = 1;
		}
		if (count && (count < sent)) {
			break;
		}

	}

	if (failure) {
		PRINT("TEST FAILED\n");
	} else {
		PRINT("TEST PASSED\n");
	}
}

void main(void)
{
	struct in6_addr in6addr_any = IN6ADDR_ANY_INIT;            /* ::  */
	struct in6_addr in6addr_loopback = IN6ADDR_LOOPBACK_INIT;  /* ::1 */

	PRINT("%s: running network loopback test\n", __func__);

	sys_rand32_init();

	net_init();
	net_driver_loopback_init();

	any_addr.in6_addr = in6addr_any;
	any_addr.family = AF_INET6;

	loopback_addr.in6_addr = in6addr_loopback;
	loopback_addr.family = AF_INET6;

	receiver_id = task_fiber_start(receiver_stack, STACKSIZE,
				       (nano_fiber_entry_t)fiber_receiver,
				       0, 0, 7, 0);

	sender_id = task_fiber_start(sender_stack, STACKSIZE,
				     (nano_fiber_entry_t)fiber_sender,
				     0, 0, 7, 0);
}