/***
  This file is part of systemd.

  Copyright 2010 Lennart Poettering

  systemd is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation; either version 2.1 of the License, or
  (at your option) any later version.

  systemd is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with systemd; If not, see <http://www.gnu.org/licenses/>.
***/

#include "alloc-util.h"
#include "escape.h"
#include "macro.h"

static void test_cescape(void) {
        _cleanup_free_ char *escaped;

        assert_se(escaped = cescape("abc\\\"\b\f\n\r\t\v\a\003\177\234\313"));
        assert_se(streq(escaped, "abc\\\\\\\"\\b\\f\\n\\r\\t\\v\\a\\003\\177\\234\\313"));
}

static void test_cunescape(void) {
        _cleanup_free_ char *unescaped;

        assert_se(cunescape("abc\\\\\\\"\\b\\f\\a\\n\\r\\t\\v\\003\\177\\234\\313\\000\\x00", 0, &unescaped) < 0);
        assert_se(cunescape("abc\\\\\\\"\\b\\f\\a\\n\\r\\t\\v\\003\\177\\234\\313\\000\\x00", UNESCAPE_RELAX, &unescaped) >= 0);
        assert_se(streq_ptr(unescaped, "abc\\\"\b\f\a\n\r\t\v\003\177\234\313\\000\\x00"));
        unescaped = mfree(unescaped);

        /* incomplete sequences */
        assert_se(cunescape("\\x0", 0, &unescaped) < 0);
        assert_se(cunescape("\\x0", UNESCAPE_RELAX, &unescaped) >= 0);
        assert_se(streq_ptr(unescaped, "\\x0"));
        unescaped = mfree(unescaped);

        assert_se(cunescape("\\x", 0, &unescaped) < 0);
        assert_se(cunescape("\\x", UNESCAPE_RELAX, &unescaped) >= 0);
        assert_se(streq_ptr(unescaped, "\\x"));
        unescaped = mfree(unescaped);

        assert_se(cunescape("\\", 0, &unescaped) < 0);
        assert_se(cunescape("\\", UNESCAPE_RELAX, &unescaped) >= 0);
        assert_se(streq_ptr(unescaped, "\\"));
        unescaped = mfree(unescaped);

        assert_se(cunescape("\\11", 0, &unescaped) < 0);
        assert_se(cunescape("\\11", UNESCAPE_RELAX, &unescaped) >= 0);
        assert_se(streq_ptr(unescaped, "\\11"));
        unescaped = mfree(unescaped);

        assert_se(cunescape("\\1", 0, &unescaped) < 0);
        assert_se(cunescape("\\1", UNESCAPE_RELAX, &unescaped) >= 0);
        assert_se(streq_ptr(unescaped, "\\1"));
        unescaped = mfree(unescaped);

        assert_se(cunescape("\\u0000", 0, &unescaped) < 0);
        assert_se(cunescape("\\u00DF\\U000000df\\u03a0\\U00000041", UNESCAPE_RELAX, &unescaped) >= 0);
        assert_se(streq_ptr(unescaped, "ßßΠA"));
        unescaped = mfree(unescaped);

        assert_se(cunescape("\\073", 0, &unescaped) >= 0);
        assert_se(streq_ptr(unescaped, ";"));
}

static void test_shell_escape_one(const char *s, const char *bad, const char *expected) {
        _cleanup_free_ char *r;

        assert_se(r = shell_escape(s, bad));
        assert_se(streq_ptr(r, expected));
}

static void test_shell_escape(void) {
        test_shell_escape_one("", "", "");
        test_shell_escape_one("\\", "", "\\\\");
        test_shell_escape_one("foobar", "", "foobar");
        test_shell_escape_one("foobar", "o", "f\\o\\obar");
        test_shell_escape_one("foo:bar,baz", ",:", "foo\\:bar\\,baz");
}

static void test_shell_maybe_quote_one(const char *s, const char *expected) {
        _cleanup_free_ char *r;

        assert_se(r = shell_maybe_quote(s));
        assert_se(streq(r, expected));
}

static void test_shell_maybe_quote(void) {

        test_shell_maybe_quote_one("", "");
        test_shell_maybe_quote_one("\\", "\"\\\\\"");
        test_shell_maybe_quote_one("\"", "\"\\\"\"");
        test_shell_maybe_quote_one("foobar", "foobar");
        test_shell_maybe_quote_one("foo bar", "\"foo bar\"");
        test_shell_maybe_quote_one("foo \"bar\" waldo", "\"foo \\\"bar\\\" waldo\"");
        test_shell_maybe_quote_one("foo$bar", "\"foo\\$bar\"");
}

int main(int argc, char *argv[]) {
        test_cescape();
        test_cunescape();
        test_shell_escape();
        test_shell_maybe_quote();

        return 0;
}
