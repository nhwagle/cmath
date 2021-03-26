/*
 * CSF Assignment 1
 * Arbitrary-precision integer data type
 * Function implementations
 */

#include <stdio.h> //for testing
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "apint.h"
#include <math.h>

ApInt *apint_create_from_u64(uint64_t val) { //ms1
    ApInt *ap = (ApInt*) malloc(sizeof(ApInt));
    ap->len = 1;
    ap->flags = 0; // a uint64_t will always be non-negative
    ap->data = (uint64_t*)malloc(ap->len*sizeof(uint64_t));
    ap->data[0] = val;
	return ap;
}

int find_max(int a, int b){ // helper function to find max of two numbers
    if(a > b){
        return a;
    } else{
        return b;
    }
}

uint64_t convert_hex_to_dec(char *hex){ // hex refers to an (at max) 16 character/64 bit representation
    uint64_t dec = strtoul(hex, NULL, 16);
    return dec;
}

ApInt *apint_create_from_hex(const char *hex) {
    //assumes len(hex) > 0 --> need test
    ApInt *ap = (ApInt*) malloc(sizeof(ApInt));
    int i = 0; // value used to keep track of start of array
    // identify sign of hex
    ap->flags = 0;
    if(hex[i] == '-'){
        ap->flags = 1;
        i++;
    }
    // ignores leading 0s
    int hexlen = strlen(hex);
    while(i < hexlen && hex[i] == '0'){
        i++;
    }
    // checks if hex = 0
    if(i == hexlen){
        ap->len = 1; // 0 is length 1
        ap->data = (uint64_t*)malloc(ap->len*sizeof(uint64_t));
        ap->data[0] = 0UL;
    } else { // hex > 0
        int blocks = (hexlen - i) / 16; // length of data/number of uint64_t blocks based on number of 64 bit blocks
        if((hexlen-i)%16 > 0) { // adjusts block count based on remainder
            blocks++;
        }
        ap->len = blocks;
        ap->data = (uint64_t*)malloc(ap->len*sizeof(uint64_t)); // allocate enough memory for the blocks
        int end = hexlen-1; // keeps track of the last index
        for(int j = 0; j < blocks; j++){ // fills data array block by block
            int start = end - 16 + 1; // keeps track of the start of a 16 character representation
            start = find_max(start, i);
            int numchars = end - start + 1; // number of characters in a block - most often will be 16, unless block < 64 bit
            char *hex16 = (char*)malloc(numchars*sizeof(char)+1); // allocate memory to store chars of one block
            for(int k = 0; k < numchars; k++){ // fill hex16 with 64 bits of hex
               hex16[k] = hex[start]; 
               start++;
            }
            hex16[numchars] = '\0';
            ap->data[j] = convert_hex_to_dec(hex16);
            free(hex16);
            end = end - numchars;
        }
    } 
	return ap;
}

void apint_destroy(ApInt *ap) {
	free(ap->data);
    ap->data = NULL;
    free(ap);
    ap = NULL;
}

int apint_is_zero(const ApInt *ap) { //ms1
    if(ap->len == 1 && ap->data[0] == 0){ // checks zero
        return 1;
    } else{
        return 0;
    }
}

int apint_is_negative(const ApInt *ap) { //ms1
    if(ap->flags == 1 && !(apint_is_zero(ap))){ // checks negative, ignores -0
        return 1;
    } else {
        return 0;
    }
}

uint64_t apint_get_bits(const ApInt *ap, unsigned n) { 
    if (n < ap->len) { // checks if n is in range of apint
        return ap->data[n];
    } 
    return 0L;
}

int apint_highest_bit_set(const ApInt *ap) { 
    if(ap->len == 1 && ap->data[0] == 0){ // case 0
        return -1;
    } else{
        uint64_t x = ap->data[ap->len-1];
        int soln = 64*(ap->len-1); // keeps track of preceding uint64_t's in data
        int i = -1;
        while (x != 0){ // shift bits left until 0
            x = x >> 1;
            i++;
        }
        soln += i;
        return soln;
    }
}

char *apint_format_as_hex(const ApInt *ap) {
    char* s = malloc(sizeof(char) * (ap->len*16)+1);
    s[0] = '\0';
    for (int i = ap->len-1; i > -1; i--) {
        char temp[17];
        sprintf(temp, "%lx", ap->data[i]);
        int templen = strlen(temp);
        int aplenind = ap->len-1;
        if(templen < 16 && ap->len > 1 && i < aplenind){ // checks if leading zeros need to be added to the hex representatiion of a uint64 within data
            int zero_pad = 16-templen;
            char* newtemp = malloc(sizeof(char)*17);
            for(int j = 0; j < zero_pad; j++){ // pads hex with zeros
                newtemp[j] = '0';
            }
            newtemp[zero_pad] = '\0';
            strcat(newtemp,temp);
            strcat(s, newtemp);
            free(newtemp);
        } else{
            strcat(s, temp);
        }
    } 
    if (ap->flags == 1) { // adjusts hex for negative cases
        char* s2 = malloc(sizeof(char) * (ap->len*16+2));
        s2[0] = '-';
        s2[1] = '\0';
        strcat(s2, s);
        free(s);
        return s2;
    } else {
        return s;
    }
    
}

ApInt *apint_negate(const ApInt *ap) { 
	ApInt *ap_neg = (ApInt*)malloc(sizeof(ApInt));
    
    // This checks if ap is negative or zero
    if(ap->flags == 1 || (ap->len == 1 && ap->data[0] == 0)){
        ap_neg->flags = 0;
    } else{
        ap_neg->flags = 1;
    }
    ap_neg->len = ap->len;
    ap_neg->data = (uint64_t*)malloc((ap_neg->len)*sizeof(uint64_t));
    for(uint32_t i=0; i < ap_neg->len; i++){
        ap_neg->data[i] = ap->data[i];
    }
    
	return ap_neg;
}

// helper to add magnitudes to two apint instances
// |a| > |b|
ApInt *addition(const ApInt *a, const ApInt *b, ApInt *sum){
    uint64_t *data = (uint64_t*)malloc((a->len)*sizeof(uint64_t));
    uint64_t carry = 0;
    sum->len = a->len;
    for (uint i = 0; i < a->len; i++) { // calculates sum block by block
        uint64_t tempsum = a->data[i] + apint_get_bits(b, i) + carry;
        if (tempsum < a->data[i] || tempsum < apint_get_bits(b, i)) { // detects overflow
            carry = 1;
        } else { // no overflow
            carry = 0;
        }
        data[i] = tempsum;
    }
    
    if (carry > 0) { // incorporates remaining carryover into sum
        data = (uint64_t*) realloc(data, (sum->len+1)*sizeof(uint64_t));
        sum->len++;
        data[sum->len-1] = carry;
    }
    sum->data = data;
    /*
    if (carry > 0) {
        uint64_t *data2 = (uint64_t*)malloc((sum->len+1)*sizeof(uint64_t));
        for(uint i = 0; i < sum->len; i++){
            data2[i] = data[i];
        }
        data[sum->len] = carry;
        sum->len++;
        free(data);
        sum->data = data2;
    } else{
        sum->data = data;
    }*/
    
    
    return sum;
}

// helper to subtract magnitudes to two apint instances
// |a| > |b|
ApInt *subtraction(const ApInt *a, const ApInt *b, ApInt *diff){
    uint64_t *data = (uint64_t*)malloc((a->len)*sizeof(uint64_t));
    
    // set to zero
    for(uint i = 0; i < a->len; i++){
        data[i] = 0UL;
    }
    
    uint64_t borrow = 0;

    for (uint i = 0; i < a->len; i++) { // calculate different block by block
        uint64_t tempdiff = a->data[i] - apint_get_bits(b,i) - borrow;
        if (apint_get_bits(b,i) > a->data[i]) { // detect overflow
            borrow = 1; 
        }
        else if (borrow == 1 && a->data[i] == 0) { // detect overflow
            borrow = 1;
        }
        else { // no overflow
            borrow = 0;
        }
        data[i] = tempdiff;
        
    }
    diff->data = data;
    
    // readjust length to remove blank blocks in data
    diff->len = a->len;
    for(int i = a->len-1; i > -1; i--){
        if(data[i] == 0){
            diff->len --;
        }
    }
    if(diff->len == 0){ //adjust length in special case of diff = 0
        diff->len = 1;
    }
    return diff;
}

ApInt *apint_add(const ApInt *a, const ApInt *b) { //ms1
    ApInt *sum = (ApInt*)malloc(sizeof(ApInt));
    if (a->flags == b->flags) { // same signs
        sum->flags = a->flags;
        if(a->flags == 1){ // both apints are negative
            ApInt *aneg = apint_negate(a);
            ApInt *bneg = apint_negate(b);
            if(apint_compare(aneg, bneg) == 1){ // |a| > |b|
                sum = addition(a,b,sum);
            } else{ // |a| <= |b|
                sum = addition(b,a,sum);
            }
            apint_destroy(aneg);
            apint_destroy(bneg);
        } else{
            if(apint_compare(a,b) == 1){ // |a| > |b|
                sum = addition(a,b,sum);
            } else{ // |a| <= |b|
                sum = addition(b,a,sum);
            }
        }
    } else{ //diff signs
        if(a->flags == 1){ //a is negative
            ApInt *aneg = apint_negate(a);
            if(apint_compare(aneg, b) == 1){ // |a| > |b|
                sum->flags = a->flags;
                sum = subtraction(a, b, sum);
            } else{ // |a| <= |b|
                sum->flags = b->flags;
                sum = subtraction(b, a, sum);
                if(apint_is_zero(sum)){
                    sum->flags = 0;
                }
            }
            apint_destroy(aneg);
        } else{ // b is negative
            ApInt *bneg = apint_negate(b);
            if(apint_compare(a, bneg) == 1){ // |a| > |b|
                sum->flags = a->flags;
                sum = subtraction(a, b, sum);
            } else{ // |a| <= |b|
                sum->flags = b->flags;
                sum = subtraction(b, a, sum);
                if(apint_is_zero(sum)){
                    sum->flags = 0;
                }
            }
            apint_destroy(bneg);
        }
    }
	return sum;
}

ApInt *apint_sub(const ApInt *a, const ApInt *b) {
    ApInt *negative_b = apint_negate(b);
    ApInt *sum = apint_add(a, negative_b);
    apint_destroy(negative_b);
    return sum;
}

int apint_compare(const ApInt *left, const ApInt *right) {
    uint32_t flagl = left->flags;
    uint32_t flagr = right->flags;
    if(flagl == flagr){
        if(flagl == 0){ //both pos
            if(left->len > right->len){ //l bigger
                return 1;
            } else if(right->len > left->len){ //r bigger
                return -1;
            } else{ // equal length
                for(int i = left->len-1; i > -1; i--){ // compares apints block by block
                    if(left->data[i] > right->data[i]){
                        return 1;
                    } else if(right->data[i] > left->data[i]){
                        return -1;
                    }
                }
                return 0;
            }
        
        } else{ // both neg
            if(left->len > right->len){ //l bigger negative
                return -1;
            } else if(right->len > left->len){ //r bigger negative
                return 1;
            } else{ // equal length
                for(int i = left->len-1; i > -1; i--){ // compares apints block by block
                    if(left->data[i] > right->data[i]){
                        return -1;
                    } else if(right->data[i] > left->data[i]){
                        return 1;
                    }
                }
                return 0;
            }
        }
    } else if(flagl < flagr){ // left is non-neg(flag=0), right is neg(flag=1)
        return 1;
    } else { // right is non-neg, left is negative
        return -1;
    }
}

ApInt *apint_lshift(ApInt *ap){
    return apint_lshift_n(ap, 1);
}
ApInt *apint_lshift_n(ApInt *ap, unsigned n){
    ApInt *apshift = (ApInt*) malloc(sizeof(ApInt));
    apshift->len = ap->len;
    apshift->flags = ap->flags; // shift won't affect flags
    apshift->data = (uint64_t*)malloc(apshift->len*sizeof(uint64_t));
    uint64_t cur = ap->data[ap->len - 1];
    cur = cur << n; // shift most significant bits left
    
    int i = apshift->len - 1; // keep track of current uint64 block
    while(i > 0){ // loop through each block
        uint64_t prev = ap->data[i-1]; // keep track of previous uint64 block
        int shiftforprev = 64 - n; // calculate right shift for prev
        uint64_t firstn = prev >> shiftforprev; // shift prev right 64-n times to 
        apshift->data[i] = cur + firstn;
        cur = prev;
        cur = cur << n;
        i--;
    }
    apshift->data[i] = cur;
    /* implementation of shifting each data[i] left 
    for(uint32_t i = 0; i < apshift->len; i++){
        apshift->data[i] = ap->data[i] << n;
    }*/
	return apshift;
}
