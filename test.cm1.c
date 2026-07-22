#include "test2.cm1.c"
#include <stdio.h>

struct z {
   int e;
   int a;
   int b;
   int c;
   int d;
   int t;
} zgvar[1], *xx;

typedef struct {
   int e;
   int a;
   int b;
   int c;
   int d;
   int t;
} w, *r;
w waz;

typedef int xyz, *p;
typedef struct g o, *oo;
typedef oo *ooo;
typedef ooo *oooo;
o *uu;

int k;

void helloworld();
void hello(int32_t a);
void test2(int a, int d, w x);

int test3();

struct z *test4(struct z *n) {
   hello((*n).e);
   n->e = 91;
   return n;
}

struct z test5() {
   struct z n;
   n.t = 888 * 99;
   return n;
}

struct z test6() {
   struct z n;
   n.e = 7;
   n.t = 3;
   // printf("n.e = %u, n.t = %u\n", n.e, n.t);
   return n;
}

void report(char* name) {
   printf("hello %s\n", name);
}

struct z test(int x, struct z *j) {
   report("Galileo");
   printf("Lajara %s hello world\n", "galileo");
   // (*j).t = 55;
   // hello(k);
   // k = 6;
   // hello(k);
   // hello(zgvar.e);
   // zgvar.e = 78;
   // hello(zgvar.e);
   for (int i = 0; i < 10; i++) {
      hello(i + 1.1f * i);
   }
   struct z n;
   n.e = 10;
   if (&n) {
      hello(8);
   }
   while (0) {
      switch (n.e & 3) {
         case 0:
            hello(0);
            break;
         case 1:
            hello(1);
            break;
         case 2:
            hello(2);
            break;
         case 3:
            hello(3);
            break;
      }
      n.e--;
   }
   struct z k;
   k = n;
   hello(k.e);
   return zgvar[1];
   // n.t = 1234;
   // return n;
   // int y = {};
   // hello(y);
   // y = 1;
   // hello(y);
   // hello(x);
   // hello(test3());
   // hello(x = 1 + 1 / 1);
   // hello(x);
   // hello(5 - 1);
   // hello(5 * 1);
   // hello(5 / 2);
   // hello(5 % 2);
   // hello(5 << 2);
   // hello(5 >> 1);
   // hello(5 & 1);
   // hello(5 ^ 1);
   // hello(5 | 8);
   // hello(0);
   // x = 9;
   // hello(x);
   // x += 9;
   // hello(x);
   // x -= 9;
   // hello(x);
   // x *= 3;
   // hello(x);
   // x /= 3;
   // hello(x);
   // x %= 5;
   // hello(x);
   // x <<= 1;
   // hello(x);
   // x >>= 1;
   // hello(x);
   // x &= 1;
   // hello(x);
   // x ^= 1;
   // hello(x);
   // x |= 2;
   // hello(x);
   // w k;
   // test2(0, 1, k);
}

int test3() {
   return 7;
}

void test2(int b, int c, w x) {
   hello(99);
   hello(b);
   hello(c);
   hello(100);
   hello(x.t);
   hello(x.e);
   hello(101);
   hello(x.e);
   x.e = 9999;
   hello(x.e);
}

int seven() {
   return 7;
}

int three(int val) {
   return 3 * val;
}

int add(int a, int b) {
   printf("%d + %d = %d\n", a, b, a + b);
   return a + b;
}

void hello(int a);
void hello(int a) {
   printf("hello: %u\n", a);
}

void helloworld() {
   printf("hello world2\n");
}

int run() {
   printf("hello world\n");
   struct z i;
   struct z j = test(10, &i);
   printf("hello world: %d\n", i.t);
   printf("hello world: %d\n", j.t);
   return 0;
}

int main() {
   k = 9;
   zgvar[0].e = 87;
   cm1_init("out/test.cm1");
   run();
   printf("zgvar.q = %d\n", zgvar[0].e);
   return 0;
}
