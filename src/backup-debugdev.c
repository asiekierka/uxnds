
static void
stack_diff(Stack *old, Stack *new, char *title)
{
	size_t i;
	printf("%6s: ", title);
	for(i = 0;; ++i) {
		if(i < old->ptr) {
			if(i < new->ptr) {
				if(old->dat[i] == new->dat[i]) {
					printf(" \033[0m%02x", new->dat[i]);
				} else {
					printf(" \033[0;31m%02x\033[33;1m%02x", old->dat[i], new->dat[i]);
				}
			} else { /* only in old stack */
				printf(" \033[0;31m%02x", old->dat[i]);
			}
		} else {
			if(i < new->ptr) { /* only in new stack */
				printf(" \033[33;1m%02x", new->dat[i]);
			} else { /* in neither stack, end of loop */
				break;
			}
		}
	}
	printf("\033[0m\n");
}

static void
memory_diff(Uint8 *old, Uint8 *new, size_t start, size_t end)
{
	size_t i, j;
	for(i = start; i < end; i += 0x10) {
		int changes = 0;
		for(j = i; j < i + 0x10; ++j) {
			if(old[j] != new[j]) {
				changes = 1;
				break;
			}
		}
		if(!changes) continue;
		printf("0x%04lx:  ", i);
		for(j = i; j < i + 0x10; ++j) {
			printf("\033[%sm%02x", old[j] == new[j] ? "0" : "33;1", new[j]);
			if(j % 2) putchar(' ');
		}
		printf("  ");
		for(j = i; j < i + 0x10; ++j) {
			printf("\033[%sm%c", old[j] == new[j] ? "0" : "33;1", (new[j] < ' ' || new[j] > '~') ? '.' : new[j]);
		}
		printf("\033[0m\n");
	}
}

Uint8
debug_poke(Uxn *u, Uint16 ptr, Uint8 b0, Uint8 b1)
{
	size_t i;
	(void)ptr;
	switch(b0) {
	case 0x08: /* stack */
		printf("pc %04x working stack:", u->ram.ptr);
		for(i = 0; i < u->wst.ptr; ++i) {
			printf(" %02x", u->wst.dat[i]);
		}
		printf(", return stack: ");
		for(i = 0; i < u->rst.ptr; ++i) {
			printf(" %02x", u->rst.dat[i]);
		}
		printf("\n");
		if(b1 && b1 != u->wst.ptr) {
			printf("length %d failed to match %d!\n", b1, u->wst.ptr);
			exit(1);
		}
		break;
	case 0x09: /* snapshot */
		if(u->snapshot != NULL) {
			if(!(b1 & 0x01)) {
				stack_diff(&u->snapshot->wst, &u->wst, "work");
			}
			if(!(b1 & 0x02)) {
				stack_diff(&u->snapshot->rst, &u->rst, "return");
			}
			if(!(b1 & 0x04)) {
				memory_diff(u->snapshot->ram.dat, u->ram.dat, 0, PAGE_DEVICE);
				memory_diff(u->snapshot->ram.dat, u->ram.dat, PAGE_DEVICE + 0x0100, 0x10000);
			}
		}
		{
			int want_snapshot = !(b1 & 0x80);
			if(want_snapshot) {
				if(u->snapshot == NULL) {
					u->snapshot = malloc(sizeof(*u));
				}
				for(i = 0; i < sizeof(*u); ++i) {
					((char *)u->snapshot)[i] = ((char *)u)[i];
				}
			}
			printf("pc 0x%04x snapshot%s taken\n", u->counter, want_snapshot ? "" : " not");
		}
		break;
	case 0x0a: /* exit */
		printf("Exited after 0x%04x cycles.\n", u->counter);
		exit(b1);
		break;
	case 0x0f: /* test mode */
		u->test_mode = b1;
		printf("Test mode is now 0x%02x: ", u->test_mode);
		if(b1 & 0x01) {
			printf("BRK resets stacks to zero length");
		} else {
			printf("all test mode features disabled");
		}
		printf("\n");
		break;
	}
	fflush(stdout);
	return b1;
}