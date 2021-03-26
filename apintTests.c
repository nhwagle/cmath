/*
 * Unit tests for arbitrary-precision integer data type
 *
 * These tests are by no means comprehensive.  You will need to
 * add more tests of your own!  In particular, make sure that
 * you have tests for more challenging situations, such as
 *
 * - large values
 * - adding/subtracting/comparing values with different lengths
 * - special cases (carries when adding, borrows when subtracting, etc.)
 * - etc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "apint.h"
#include "tctest.h"

typedef struct {
	ApInt *ap0;
	ApInt *ap1;
	ApInt *ap110660361;
	ApInt *max1;
	ApInt *minus1;
	/* add additional fields of test fixture */
	ApInt *negative;
	ApInt *ap0neg;
	ApInt *max2;
    ApInt *trail0;
    ApInt *mid0;
    ApInt *twoblocks;
    ApInt *shift;

} TestObjs;

TestObjs *setup(void);
void cleanup(TestObjs *objs);

void testCreateFromU64(TestObjs *objs);
void testHighestBitSet(TestObjs *objs);
void testCompare(TestObjs *objs);
void testFormatAsHex(TestObjs *objs);
void testAdd(TestObjs *objs);
void testSub(TestObjs *objs);
/* add more test function prototypes */
void testNegate(TestObjs *objs);
void testShift(TestObjs *objs);
void testCreateFromHex(TestObjs *objs);


int main(int argc, char **argv) {
	TEST_INIT();

	if (argc > 1) {
		/*
		 * name of specific test case to execute was provided
		 * as a command line argument
		 */
		tctest_testname_to_execute = argv[1];
	}

	TEST(testCreateFromU64);
	TEST(testHighestBitSet);
	TEST(testCompare);
	TEST(testFormatAsHex);
	TEST(testAdd);
    TEST(testSub);
	/* use TEST macro to execute more test functions */
	TEST(testNegate);
    TEST(testShift);
    TEST(testCreateFromHex);

	TEST_FINI();
}

TestObjs *setup(void) {
	TestObjs *objs = malloc(sizeof(TestObjs));
	objs->ap0 = apint_create_from_u64(0UL);
	objs->ap1 = apint_create_from_u64(1UL);
	objs->ap110660361 = apint_create_from_u64(110660361UL);
	objs->max1 = apint_create_from_u64(0xFFFFFFFFFFFFFFFFUL);
	objs->minus1 = apint_negate(objs->ap1);
	/* additional members of test fixture */
	objs->negative = apint_negate(objs->ap1);
	objs->ap0neg = apint_negate(objs->ap0);
	objs->max2 = apint_create_from_hex("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFA");
    objs->trail0 = apint_create_from_hex("10000000000000000");
    objs->mid0 = apint_create_from_hex("10000000000100000");
    objs->twoblocks = apint_create_from_hex("ffffffffffffffffffffffffffffffff");
    objs->shift = apint_create_from_hex("80008001800080018000800180008001");
    
	return objs;
}

void cleanup(TestObjs *objs) {
	apint_destroy(objs->ap0);
	apint_destroy(objs->ap1);
	apint_destroy(objs->ap110660361);
	apint_destroy(objs->max1);
	apint_destroy(objs->minus1);
	/* destroy additional members of test fixture */
	apint_destroy(objs->negative);
	apint_destroy(objs->ap0neg);
	apint_destroy(objs->max2);
    apint_destroy(objs->trail0);
    apint_destroy(objs->mid0);
    apint_destroy(objs->twoblocks);
    apint_destroy(objs->shift);
	free(objs);
}

void testCreateFromU64(TestObjs *objs) {
	ASSERT(0UL == apint_get_bits(objs->ap0, 0));
	ASSERT(objs->ap0->data[0] == 0);
	ASSERT(objs->ap0->len == 1);
	ASSERT(objs->ap0->flags == 0);
	ASSERT(1UL == apint_get_bits(objs->ap1, 0));
	ASSERT(110660361UL == apint_get_bits(objs->ap110660361, 0));
	ASSERT(0xFFFFFFFFFFFFFFFFUL == apint_get_bits(objs->max1, 0)); 
	ASSERT(1UL == apint_get_bits(objs->negative, 0));
	ASSERT(0UL == apint_get_bits(objs->ap0neg, 0));
}

void testHighestBitSet(TestObjs *objs) {
	ASSERT(-1 == apint_highest_bit_set(objs->ap0));
	ASSERT(0 == apint_highest_bit_set(objs->ap1));
	ASSERT(26 == apint_highest_bit_set(objs->ap110660361));
	ASSERT(63 == apint_highest_bit_set(objs->max1));
}

void testCompare(TestObjs *objs) {
	/* 1 > 0 */
	ASSERT(apint_compare(objs->ap1, objs->ap0) > 0);
	/* 0 < 1 */
	ASSERT(apint_compare(objs->ap0, objs->ap1) < 0);
	/* 110660361 > 0 */
	ASSERT(apint_compare(objs->ap110660361, objs->ap0) > 0);
	/* 110660361 > 1 */
	ASSERT(apint_compare(objs->ap110660361, objs->ap1) > 0);
	/* 0 < 110660361 */
	ASSERT(apint_compare(objs->ap0, objs->ap110660361) < 0);
	/* 1 < 110660361 */
	ASSERT(apint_compare(objs->ap1, objs->ap110660361) < 0);
}

void testFormatAsHex(TestObjs *objs) {
	char *s;
    ApInt *a;

	ASSERT(0 == strcmp("0", (s = apint_format_as_hex(objs->ap0))));
	free(s);

	ASSERT(0 == strcmp("1", (s = apint_format_as_hex(objs->ap1))));
	free(s);

	ASSERT(0 == strcmp("6988b09", (s = apint_format_as_hex(objs->ap110660361))));
	free(s);

	ASSERT(0 == strcmp("ffffffffffffffff", (s = apint_format_as_hex(objs->max1))));
	free(s);

	ASSERT(0 == strcmp("fffffffffffffffffffffffffffffffa", (s = apint_format_as_hex(objs->max2))));
	free(s);	

    a = apint_negate(objs->ap1);
	ASSERT(0 == strcmp("-1", (s = apint_format_as_hex(a))));
    apint_destroy(a);
	free(s);
    
    a = apint_negate(objs->max1);
    ASSERT(0 == strcmp("-ffffffffffffffff", (s = apint_format_as_hex(a))));
    apint_destroy(a);
	free(s);
    
    ASSERT(0 == strcmp("10000000000000000", (s = apint_format_as_hex(objs->trail0))));
	free(s);
    
    a = apint_negate(objs->trail0);
    ASSERT(0 == strcmp("-10000000000000000", (s = apint_format_as_hex(a))));
	free(s);
    apint_destroy(a);
    
    ASSERT(0 == strcmp("10000000000100000", (s = apint_format_as_hex(objs->mid0))));
	free(s);
}

void testAdd(TestObjs *objs) {
	ApInt *sum;
	char *s;

	/* 0 + 0 = 0 */
	sum = apint_add(objs->ap0, objs->ap0);
	ASSERT(apint_get_bits(sum, 0) == 0);
	ASSERT(0 == strcmp("0", (s = apint_format_as_hex(sum))));
	apint_destroy(sum);
	free(s);

	/* 1 + 0 = 1 */
	sum = apint_add(objs->ap1, objs->ap0);
	ASSERT(apint_get_bits(sum, 0) == 1);
	ASSERT(0 == strcmp("1", (s = apint_format_as_hex(sum))));
	apint_destroy(sum);
	free(s);

	/* 1 + 1 = 2 */
	sum = apint_add(objs->ap1, objs->ap1);
	ASSERT(apint_get_bits(sum, 0) == 2);
	ASSERT(0 == strcmp("2", (s = apint_format_as_hex(sum))));
	apint_destroy(sum);
	free(s);

	/* 110660361 + 1 = 110660362 */
	sum = apint_add(objs->ap110660361, objs->ap1);
	ASSERT(apint_get_bits(sum, 0) == 110660362);
	ASSERT(0 == strcmp("6988b0a", (s = apint_format_as_hex(sum))));
	apint_destroy(sum);
	free(s);

	// FFFFFFFFFFFFFFFF FFFFFFFFFFFFFFFA + 1 = FFFFFFFFFFFFFFFF FFFFFFFFFFFFFFFB
	sum = apint_add(objs->max2, objs->ap1);
	ASSERT(0 == strcmp("fffffffffffffffffffffffffffffffb", (s = apint_format_as_hex(sum))));
	apint_destroy(sum);
	free(s);

	/* FFFFFFFFFFFFFFFF + 1 = 10000000000000000 */

	sum = apint_add(objs->max1, objs->ap1);
	ASSERT(0 == strcmp("10000000000000000", (s = apint_format_as_hex(sum))));
	apint_destroy(sum);
	free(s);
    
    /* Every add case for ifs */
    ApInt *n = apint_negate(objs->twoblocks);
    ApInt *two = apint_add(objs->ap1, objs->ap1);
    ApInt *ntwo = apint_negate(two);
    
    sum = apint_add(n, ntwo);
    s = apint_format_as_hex(sum);
	ASSERT(0 == strcmp("-100000000000000000000000000000001", s));
	apint_destroy(sum);
	free(s);
    
    sum = apint_add(ntwo, n);
    s = apint_format_as_hex(sum);
	ASSERT(0 == strcmp("-100000000000000000000000000000001", s));
	apint_destroy(sum);
	free(s);
    
    sum = apint_add(objs->twoblocks, two);
    s = apint_format_as_hex(sum);
	ASSERT(0 == strcmp("100000000000000000000000000000001", s));
	apint_destroy(sum);
	free(s);
    
    sum = apint_add(two, objs->twoblocks);
    s = apint_format_as_hex(sum);
	ASSERT(0 == strcmp("100000000000000000000000000000001", s));
	apint_destroy(sum);
	free(s);
    
    sum = apint_add(n, two);
    s = apint_format_as_hex(sum);
	ASSERT(0 == strcmp("-fffffffffffffffffffffffffffffffd", s));
	apint_destroy(sum);
	free(s);
    
    sum = apint_add(ntwo, objs->twoblocks);
    s = apint_format_as_hex(sum);
	ASSERT(0 == strcmp("fffffffffffffffffffffffffffffffd", s));
	apint_destroy(sum);
	free(s);
    
    sum = apint_add(objs->twoblocks, ntwo);
    s = apint_format_as_hex(sum);
	ASSERT(0 == strcmp("fffffffffffffffffffffffffffffffd", s));
	apint_destroy(sum);
	free(s); 
    
    sum = apint_add(two, n);
    s = apint_format_as_hex(sum);
	ASSERT(0 == strcmp("-fffffffffffffffffffffffffffffffd", s));
	apint_destroy(sum);
	free(s);
    
    apint_destroy(n);
    apint_destroy(two);
    apint_destroy(ntwo);
}

void testSub(TestObjs *objs) {
	ApInt *a, *b;
	ApInt *diff;
	char *s;

	/* subtracting 1 from ffffffffffffffff is fffffffffffffffe */
	diff = apint_sub(objs->max1, objs->ap1);
	ASSERT(apint_get_bits(diff, 0) == 18446744073709551614UL);
	ASSERT(0 == strcmp("fffffffffffffffe", (s = apint_format_as_hex(diff))));
	apint_destroy(diff);
	free(s);

	/* subtracting 0 from 1 is 1 */
	diff = apint_sub(objs->ap1, objs->ap0);
	ASSERT(apint_get_bits(diff, 0) == 1);
	ASSERT(0 == strcmp("1", (s = apint_format_as_hex(diff))));
	ASSERT(0 == apint_compare(diff, objs->ap1));
	apint_destroy(diff);
	free(s);

	/* subtracting 1 from 1 is 0 */
	diff = apint_sub(objs->ap1, objs->ap1);
    s = apint_format_as_hex(diff);
	ASSERT(apint_get_bits(diff, 0) == 0UL);
	ASSERT(0 == strcmp("0", (s)));
	ASSERT(0 == apint_compare(diff, objs->ap0));
	apint_destroy(diff);
	free(s);

	/* subtracting 1 from 0 is -1 */
	diff = apint_sub(objs->ap0, objs->ap1);
	ASSERT(apint_get_bits(diff, 0) == 1UL); //not as helpful since get bits ignores sign
	ASSERT(0 == (strcmp("-1", (s = apint_format_as_hex(diff)))));
	ASSERT(0 == apint_compare(diff, objs->minus1));
	apint_destroy(diff);
	free(s);

	/* test involving larger values */
	
	a = apint_create_from_hex("7e35207519b6b06429378631ca460905c19537644f31dc50114e9dc90bb4e4ebc43cfebe6b86d");
	b = apint_create_from_hex("9fa0fb165441ade7cb8b17c3ab3653465e09e8078e09631ec8f6fe3a5b301dc");
	diff = apint_sub(a, b);
	ASSERT(0 == strcmp("7e35207519b6afc4883c6fdd8898213a367d73b918de95f20766963b0251c622cd3ec4633b691",
		(s = apint_format_as_hex(diff))));
	apint_destroy(diff);
	apint_destroy(b);
	apint_destroy(a);
	free(s);
	

	// test involving larger values (with a negative difference) 
	a = apint_create_from_hex("9fa0fb165441ade7cb8b17c3ab3653465e09e8078e09631ec8f6fe3a5b301dc");
	b = apint_create_from_hex("7e35207519b6b06429378631ca460905c19537644f31dc50114e9dc90bb4e4ebc43cfebe6b86d");
	diff = apint_sub(a, b);
	ASSERT(0 == strcmp("-7e35207519b6afc4883c6fdd8898213a367d73b918de95f20766963b0251c622cd3ec4633b691",
	(s = apint_format_as_hex(diff))));
	apint_destroy(diff);
	apint_destroy(b);
	apint_destroy(a);
	free(s);
	
}

void testNegate(TestObjs *objs) {
    ApInt *a;
	ASSERT(apint_get_bits(objs->minus1, 0) == 1UL);
	ASSERT(apint_is_negative(objs->minus1) == 1);
	ASSERT(objs->minus1->flags == 1);
    a = apint_negate(objs->ap0);
	ASSERT(apint_get_bits(a, 0) == 0);
    apint_destroy(a);
	ASSERT(apint_is_negative(objs->negative) == 1);
}

void testShift(TestObjs *objs){
    ApInt *a;
    char *s;
    
    // Our understanding of the bonus was that each uint64 block in a apint data array would be shifted left n digits as if the binary representation was concatenated as one representation. This means bits shifted left would shift into their subsequent blocks, as shown in the examples. We couldn't pass the gradescope tests, but commented the function to explain our process.
    
    // 0 --> 0
    a = apint_lshift(objs->ap0);
    ASSERT(apint_get_bits(a, 0) == 0);
    apint_destroy(a);
    
    // 1 --> 10
    a = apint_lshift(objs->ap1);
    ASSERT(apint_get_bits(a, 0) == 2);
    apint_destroy(a);
    
    // 1111111111111111111111111111111111111111111111111111111111111111 --> 1111111111111111111111111111111111111111111111111111111111111110
    uint64_t x = 18446744073709551614UL;
    a = apint_lshift(objs->max1);
    ASSERT(apint_get_bits(a, 0) == x);
    apint_destroy(a);
    // 11111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111010 --> 11111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111110100
    x = 18446744073709551604UL;
    a = apint_lshift(objs->max2);
    ASSERT(apint_get_bits(a, 0) == x);
    x = objs->max1->data[0];
    ASSERT(apint_get_bits(a,1) == x);
    apint_destroy(a);
    
    a = apint_lshift(objs->shift);
    s = apint_format_as_hex(a);
    ASSERT(0 == strcmp("10003000100030001000300010002",s));
    apint_destroy(a);
    free(s);
    
    
    a = apint_lshift_n(objs->shift, 4);
    s = apint_format_as_hex(a);
    ASSERT(0 == strcmp("80018000800180008001800080010",s));
    apint_destroy(a);
    free(s);
    
}

void testCreateFromHex(TestObjs *objs){

    
    ASSERT(18446744073709551610UL == apint_get_bits(objs->max2, 0));
    ASSERT(18446744073709551615UL == apint_get_bits(objs->max2, 1));
    
    ASSERT(0UL == apint_get_bits(objs->trail0, 0));
    ASSERT(1UL == apint_get_bits(objs->trail0, 1));
    
    ASSERT(1048576UL == apint_get_bits(objs->mid0, 0));
    ASSERT(1UL == apint_get_bits(objs->mid0, 1));
    
    ASSERT(apint_get_bits(objs->twoblocks, 1) == apint_get_bits(objs->twoblocks, 0));
}
