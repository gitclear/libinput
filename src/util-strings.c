/*
 * Copyright © 2008 Kristian Høgsberg
 * Copyright © 2013-2015 Red Hat, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "config.h"

#include "util-strings.h"

/**
 * Return the next word in a string pointed to by state before the first
 * separator character. Call repeatedly to tokenize a whole string.
 *
 * @param state Current state
 * @param len String length of the word returned
 * @param separators List of separator characters
 *
 * @return The first word in *state, NOT null-terminated
 */
static const char *
next_word(const char **state, size_t *len, const char *separators)
{
	const char *next = *state;
	size_t l;

	if (!*next)
		return NULL;

	next += strspn(next, separators);
	if (!*next) {
		*state = next;
		return NULL;
	}

	l = strcspn(next, separators);
	*state = next + l;
	*len = l;

	return next;
}

/**
 * Return a null-terminated string array with the tokens in the input
 * string, e.g. "one two\tthree" with a separator list of " \t" will return
 * an array [ "one", "two", "three", NULL ].
 *
 * Use strv_free() to free the array.
 *
 * @param in Input string
 * @param separators List of separator characters
 *
 * @return A null-terminated string array or NULL on errors
 */
char **
strv_from_string(const char *in, const char *separators)
{
	const char *s, *word;
	char **strv = NULL;
	int nelems = 0, idx;
	size_t l;

	assert(in != NULL);

	s = in;
	while (next_word(&s, &l, separators) != NULL)
	       nelems++;

	if (nelems == 0)
		return NULL;

	nelems++; /* NULL-terminated */
	strv = zalloc(nelems * sizeof *strv);

	idx = 0;

	s = in;
	while ((word = next_word(&s, &l, separators)) != NULL) {
		char *copy = strndup(word, l);
		if (!copy) {
			strv_free(strv);
			return NULL;
		}

		strv[idx++] = copy;
	}

	return strv;
}

/**
 * Return a newly allocated string with all elements joined by the
 * joiner, same as Python's string.join() basically.
 * A strv of ["one", "two", "three", NULL] with a joiner of ", " results
 * in "one, two, three".
 *
 * An empty strv ([NULL]) returns NULL, same for passing NULL as either
 * argument.
 *
 * @param strv Input string arrray
 * @param joiner Joiner between the elements in the final string
 *
 * @return A null-terminated string joining all elements
 */
char *
strv_join(char **strv, const char *joiner)
{
	char **s;
	char *str;
	size_t slen = 0;
	size_t count = 0;

	if (!strv || !joiner)
		return NULL;

	if (strv[0] == NULL)
		return NULL;

	for (s = strv, count = 0; *s; s++, count++) {
		slen += strlen(*s);
	}

	assert(slen < 1000);
	assert(strlen(joiner) < 1000);
	assert(count > 0);
	assert(count < 100);

	slen += (count - 1) * strlen(joiner);

	str = zalloc(slen + 1); /* trailing \0 */
	for (s = strv; *s; s++) {
		strcat(str, *s);
		--count;
		if (count > 0)
			strcat(str, joiner);
	}

	return str;
}
