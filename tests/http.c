#include <stdio.h>
#include <string.h>

#include "prelex.h"

enum HTTP_STATES
{
	/* first line */
	METHOD,
	FIRST_SPACE_AFTER_METHOD,
	SPACES_AFTER_METHOD,

	/* request target with optional parameters after question sign */
	TARGET,
	TARGET_QUESTION,
	TARGET_PARAM,
	TARGET_EQ,
	TARGET_VAL,
	TARGET_AMP,

	/* HTTP 0.9 allows cr/lf right after target */
	CR_AFTER_TARGET,
	LF_AFTER_TARGET,

	FIRST_SPACE_AFTER_TARGET,
	SPACES_AFTER_TARGET,
	VERSION,
	CR_AFTER_VERSION,
	LF_AFTER_VERSION,

	/* header fields */
	HEADER_KEY,
	SPACE_AFTER_KEY,
	HEADER_COLON,
	SPACE_AFTER_COLON,
	HEADER_VAL,
	CR_AFTER_OPT,
	LF_AFTER_OPT,

	STATE_ERROR, /* ?? */

	STATE_LAST
};

struct http_request
{
	char *method;
	char *target;
	char *version;
};

static int
method_sym_callback(prelex *p, int c, void *user)
{
	(void)p;
	(void)user;

	printf("method symbol: '%c'\n", c);
	return 1;
}

static int
method_callback(prelex *p, int c, void *user)
{
	(void)p;
	(void)user;
	(void)c;

	printf("method complete\n");
	return 1;
}


static int
target_sym_callback(prelex *p, int c, void *user)
{
	(void)p;
	(void)user;

	printf("target symbol: '%c'\n", c);
	return 1;
}

static int
target_callback(prelex *p, int c, void *user)
{
	(void)p;
	(void)user;
	(void)c;

	printf("target complete\n");
	return 1;
}

static int
ver_sym_callback(prelex *p, int c, void *user)
{
	(void)p;
	(void)user;

	printf("version symbol: '%c'\n", c);
	return 1;
}

static int
ver_callback(prelex *p, int c, void *user)
{
	(void)p;
	(void)user;
	(void)c;

	printf("version complete\n");
	return 1;
}

static int
key_sym_callback(prelex *p, int c, void *user)
{
	(void)p;
	(void)user;

	printf("key symbol: '%c'\n", c);
	return 1;
}


int
main()
{
	char text[] = "GET  /target   HTTP/1.0\r\n\r\n";
	char text2[] = "POST   /target2\r\nazaza: uuuu\r\n\r\n";
	prelex *p;
	int state;
	int rc;

	p = prelex_new(STATE_LAST);
	if (!p) {
		return 1;
	}

	/* method */
	prelex_node_next(p, PRELEX_ANY, METHOD, METHOD);
	PRELEX_NODE_NEXT_CT(p, isspace, METHOD, FIRST_SPACE_AFTER_METHOD);

	prelex_cb_set(p, METHOD, &method_sym_callback);

	/* space after method */
	prelex_node_next(p, PRELEX_ANY, FIRST_SPACE_AFTER_METHOD, TARGET);
	PRELEX_NODE_NEXT_CT(p, isspace, FIRST_SPACE_AFTER_METHOD,
		SPACES_AFTER_METHOD);

	prelex_cb_set(p, FIRST_SPACE_AFTER_METHOD, &method_callback);

	prelex_node_next(p, PRELEX_ANY, SPACES_AFTER_METHOD, TARGET);
	PRELEX_NODE_NEXT_CT(p, isspace, SPACES_AFTER_METHOD,
		SPACES_AFTER_METHOD);


	/* target */
	prelex_node_next(p, PRELEX_ANY, TARGET, TARGET);
	PRELEX_NODE_NEXT_CT(p, isspace, TARGET, FIRST_SPACE_AFTER_TARGET);

	prelex_cb_set(p, TARGET, &target_sym_callback);

	/* cr/lf after target */
	prelex_node_next(p, '\r', TARGET, CR_AFTER_TARGET);
	prelex_node_next(p, '\n', CR_AFTER_TARGET, LF_AFTER_TARGET);
	/* single '\n' too */
	prelex_node_next(p, '\n', TARGET, LF_AFTER_TARGET);

	prelex_cb_set(p, LF_AFTER_TARGET, &target_callback);



	/* space after target */
	prelex_node_next(p, PRELEX_ANY, FIRST_SPACE_AFTER_TARGET, VERSION);
	PRELEX_NODE_NEXT_CT(p, isspace, FIRST_SPACE_AFTER_TARGET,
		SPACES_AFTER_TARGET);

	prelex_cb_set(p, FIRST_SPACE_AFTER_TARGET, &target_callback);

	prelex_node_next(p, PRELEX_ANY, SPACES_AFTER_TARGET, VERSION);
	PRELEX_NODE_NEXT_CT(p, isspace, SPACES_AFTER_TARGET,
		SPACES_AFTER_TARGET);


	/* version */
	prelex_node_next(p, PRELEX_ANY, VERSION, VERSION);

	prelex_cb_set(p, VERSION, &ver_sym_callback);

	/* cr/lf after version */
	prelex_node_next(p, '\r', VERSION, CR_AFTER_VERSION);
	prelex_node_next(p, '\n', CR_AFTER_VERSION, LF_AFTER_VERSION);
	/* single '\n' */
	prelex_node_next(p, '\n', VERSION, LF_AFTER_VERSION);

	prelex_cb_set(p, LF_AFTER_VERSION, &ver_callback);


	/* \r\n after first line */
	prelex_node_next(p, '\r', LF_AFTER_TARGET, CR_AFTER_OPT);
	prelex_node_next(p, '\n', CR_AFTER_OPT, LF_AFTER_OPT);

	prelex_node_next(p, '\n', LF_AFTER_TARGET, LF_AFTER_OPT);

	prelex_node_next(p, '\r', LF_AFTER_VERSION, CR_AFTER_OPT);
	prelex_node_next(p, '\n', LF_AFTER_VERSION, LF_AFTER_OPT);


	/* options */
	prelex_node_next(p, PRELEX_ANY, LF_AFTER_TARGET, HEADER_KEY);
	prelex_node_next(p, PRELEX_ANY, LF_AFTER_VERSION, HEADER_KEY);

	prelex_node_next(p, PRELEX_ANY, HEADER_KEY, HEADER_KEY);
	PRELEX_NODE_NEXT_CT(p, isspace, HEADER_KEY, SPACE_AFTER_KEY);

	prelex_cb_set(p, HEADER_KEY, &key_sym_callback);

	/* set initial state */
	state = METHOD;
	/* parse */
	rc = prelex_parse(p, &state, text, strlen(text), NULL);
	printf("rc == %d, state == %d\n\n", rc, state);

	state = METHOD;
	rc = prelex_parse(p, &state, text2, strlen(text2), NULL);
	printf("rc2 == %d, state2 == %d\n", rc, state);

	return 0;
}

