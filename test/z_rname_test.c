#include <assert.h>
#include "zenoh/rname.h"
#include "zenoh/types.h"

int main(int argc, char **argv) {
  Z_UNUSED_ARG_2(argc, argv);  
  assert(   zn_rname_intersect("/", "/"));
  assert(   zn_rname_intersect("/a", "/a"));
  assert(   zn_rname_intersect("/a/", "/a"));
  assert(   zn_rname_intersect("/a", "/a/"));
  assert(   zn_rname_intersect("/a/b", "/a/b"));
  assert(   zn_rname_intersect("/*", "/abc"));
  assert(   zn_rname_intersect("/*", "/abc/"));
  assert(   zn_rname_intersect("/*/", "/abc"));
  assert( ! zn_rname_intersect("/*", "/"));
  assert( ! zn_rname_intersect("/*", "xxx"));
  assert(   zn_rname_intersect("/ab*", "/abcd"));
  assert(   zn_rname_intersect("/ab*d", "/abcd"));
  assert(   zn_rname_intersect("/ab*", "/ab"));
  assert( ! zn_rname_intersect("/ab/*", "/ab"));
  assert(   zn_rname_intersect("/a/*/c/*/e", "/a/b/c/d/e"));
  assert(   zn_rname_intersect("/a/*b/c/*d/e", "/a/xb/c/xd/e"));
  assert( ! zn_rname_intersect("/a/*/c/*/e", "/a/c/e"));
  assert( ! zn_rname_intersect("/a/*/c/*/e", "/a/b/c/d/x/e"));
  assert( ! zn_rname_intersect("/ab*cd", "/abxxcxxd"));
  assert(   zn_rname_intersect("/ab*cd", "/abxxcxxcd"));
  assert( ! zn_rname_intersect("/ab*cd", "/abxxcxxcdx"));
  assert(   zn_rname_intersect("/**", "/abc"));
  assert(   zn_rname_intersect("/**", "/a/b/c"));
  assert(   zn_rname_intersect("/**", "/a/b/c/"));
  assert(   zn_rname_intersect("/**/", "/a/b/c"));
  assert(   zn_rname_intersect("/**/", "/"));
  assert(   zn_rname_intersect("/ab/**", "/ab"));
  assert(   zn_rname_intersect("/**/xyz", "/a/b/xyz/d/e/f/xyz"));
  assert( ! zn_rname_intersect("/**/xyz*xyz", "/a/b/xyz/d/e/f/xyz"));
  assert(   zn_rname_intersect("/a/**/c/**/e", "/a/b/b/b/c/d/d/d/e"));
  assert(   zn_rname_intersect("/a/**/c/**/e", "/a/c/e"));
  assert(   zn_rname_intersect("/a/**/c/*/e/*", "/a/b/b/b/c/d/d/c/d/e/f"));
  assert( ! zn_rname_intersect("/a/**/c/*/e/*", "/a/b/b/b/c/d/d/c/d/d/e/f"));
  assert( ! zn_rname_intersect("/ab*cd", "/abxxcxxcdx"));
  assert(   zn_rname_intersect("/x/abc", "/x/abc"));
  assert( ! zn_rname_intersect("/x/abc", "/abc"));
  assert(   zn_rname_intersect("/x/*", "/x/abc"));
  assert( ! zn_rname_intersect("/x/*", "/abc"));
  assert( ! zn_rname_intersect("/*", "/x/abc"));
  assert(   zn_rname_intersect("/x/*", "/x/abc*"));
  assert(   zn_rname_intersect("/x/*abc", "/x/abc*"));
  assert(   zn_rname_intersect("/x/a*", "/x/abc*"));
  assert(   zn_rname_intersect("/x/a*de", "/x/abc*de"));
  assert(   zn_rname_intersect("/x/a*d*e", "/x/a*e"));
  assert(   zn_rname_intersect("/x/a*d*e", "/x/a*c*e"));
  assert(   zn_rname_intersect("/x/a*d*e", "/x/ade"));
  assert( ! zn_rname_intersect("/x/c*", "/x/abc*"));
  assert( ! zn_rname_intersect("/x/*d", "/x/*e"));

  return 0;
}
