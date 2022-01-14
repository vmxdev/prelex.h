#ifndef prelex_h_included
#define prelex_h_included

#include <stddef.h>
#include <stdlib.h>

#define PRELEX_ARITY     256
#define PRELEX_UNDEFINED (-1)
#define PRELEX_ANY       (-1)

typedef struct prelex prelex;
typedef int prelex_callback(prelex *p, int c, int from, int to, void *user);

struct prelex_node
{
	int next[PRELEX_ARITY];

	/* action after exit */
	prelex_callback *cb;
};

struct prelex
{
	struct prelex_node *nodes;
	int n;
};

static void
prelex_node_init(struct prelex_node *n, int next)
{
	int i;

	for (i=0; i<PRELEX_ARITY; i++) {
		n->next[i] = next;
	}
	n->cb = NULL;
}

static prelex *
prelex_new(int n)
{
	prelex *p;
	int i;

	p = malloc(sizeof(prelex));
	if (!p) {
		goto p_malloc_fail;
	}
	if (n > 0) {
		p->nodes = malloc(n * sizeof(struct prelex_node));
		if (!p->nodes) {
			goto n_malloc_fail;
		}
	} else {
		p->nodes = NULL;
	}
	p->n = n;

	/* init nodes */
	for (i=0; i<n; i++) {
		prelex_node_init(&p->nodes[i], PRELEX_UNDEFINED);
	}

	return p;

	/* errors */
n_malloc_fail:
	free(p);

p_malloc_fail:
	return NULL;
}


static int
prelex_node_next(prelex *p, int c, int node, int next)
{
	if (((node + 1) > p->n) || ((next + 1) > p->n)) {
		return 0;
	}

	if ((node < 0) || (next < 0)) {
		return 0;
	}

	if (c == PRELEX_ANY) {
		/* special case, fill all the subnodes */
		prelex_node_init(&p->nodes[node], next);
		return 1;
	}

	if ((c < 0) || (c > PRELEX_ARITY)) {
		return 0;
	}

	p->nodes[node].next[c] = next;
	return 1;
}

static int
prelex_cb_set(prelex *p, int node, prelex_callback *cb)
{
	if (((node + 1) > p->n) || (node < 0)) {
		return 0;
	}

	p->nodes[node].cb = cb;
	return 1;
}

static int
prelex_parse(prelex *p, int *state, const char *text, size_t size, void *user)
{
	size_t i;

	for (i=0; i<size; i++) {
		int c = text[i];
		int next_node = p->nodes[*state].next[c];

		if (next_node == PRELEX_UNDEFINED) {
			return 0;
		}

		if (p->nodes[next_node].cb) {
			int rc;

			rc = p->nodes[next_node].cb(p, c, *state, next_node,
				user);

			if (!rc) {
				/* callback returned 0, stop parsing */
				return 0;
			}
		}

		*state = next_node;
	}
	return 1;
}

#endif

