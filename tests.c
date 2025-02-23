// Copyright 2023, Midnight Blue.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <inttypes.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "hurdle.h"
#include "tea1.h"
#include "tea2.h"
#include "tea3.h"
#include "taa1.h"

#define TEST_VECTORS_SETUP_EX2(tag,elts,invoke,cmp,print_expected,print_computed,...) { \
    printf ("Testing %s...%*c", (tag), (int)(40 - strlen(tag)), ' '); \
    struct { \
        elts \
    } astVectors[] = __VA_ARGS__; \
    uint8_t bSuccess = 1; \
    for (int i = 0; i < sizeof(astVectors)/sizeof(astVectors[0]); i++) { \
        invoke; \
        if ((cmp) != 0) { \
            if (bSuccess) { \
                printf("[\x1b[31mFAIL\x1b[0m]\n"); \
            } \
            printf("  expected:"); \
            print_expected \
            printf(", computed:"); \
            print_computed \
            printf("\n"); \
            bSuccess = 0; \
        } \
    } \
    if (bSuccess) { \
        printf("[\x1b[32m OK \x1b[0m]\n"); \
    } \
}

#define TEST_VECTORS_SETUP_EX(tag,elts,invoke,cmp,elex,elcomp,...) \
    TEST_VECTORS_SETUP_EX2(tag, \
        elts, \
        invoke, \
        cmp, \
        for (int j = 0; j < sizeof(astVectors[i].elex); j++) { \
            printf(" %02x", astVectors[i].elex[j]); \
        }, \
        for (int j = 0; j < sizeof(astVectors[i].elcomp); j++) { \
            printf(" %02x", astVectors[i].elcomp[j]); \
        }, \
        __VA_ARGS__ \
    )

#define TEST_VECTORS_SETUP(tag,lenout,elts,invoke,...) \
    TEST_VECTORS_SETUP_EX( \
        tag, \
        elts \
        uint8_t abOutput[(lenout)]; \
        uint8_t abComputed[(lenout)];, \
        invoke, \
        memcmp(astVectors[i].abOutput, astVectors[i].abComputed, (lenout)), \
        abOutput, abComputed, \
        __VA_ARGS__ \
    )

#define TEST_VECTORS_INVERTIBLE_EX(tag1,tag2,elex,elinv,elts,invoke,invert,cmpex,cmpinv,...) { \
    struct { \
        elts \
    } __dummy[0]; \
\
    TEST_VECTORS_SETUP_EX( \
        tag1, \
        elts \
        uint8_t abComputed[sizeof(__dummy[0].elex)];, \
        invoke, \
        cmpex, \
        elex, abComputed, \
        __VA_ARGS__ \
    ) \
    TEST_VECTORS_SETUP_EX( \
        tag2, \
        elts \
        uint8_t abComputed[sizeof(__dummy[0].elinv)];, \
        invert, \
        cmpinv, \
        elinv, abComputed, \
        __VA_ARGS__ \
    ) \
}

#define TEST_VECTORS_INVERTIBLE(tag1,tag2,elex,elinv,elts,invoke,invert,...) \
    TEST_VECTORS_INVERTIBLE_EX(tag1,tag2,elex,elinv,elts,invoke,invert, \
        memcmp(astVectors[i].elex, astVectors[i].abComputed, sizeof(astVectors[i].elex)), \
        memcmp(astVectors[i].elinv, astVectors[i].abComputed, sizeof(astVectors[i].elinv)), \
        __VA_ARGS__ \
    )

#define TEST_VECTORS_1_ARGUMENT(tag,len0,lenout,fun,...) \
    TEST_VECTORS_SETUP(tag, lenout, \
        uint8_t abInput0[len0];, \
        fun(astVectors[i].abInput0, astVectors[i].abComputed), \
        __VA_ARGS__ \
    )

#define TEST_VECTORS_2_ARGUMENTS(tag,len0,len1,lenout,fun,...) \
    TEST_VECTORS_SETUP(tag, lenout, \
        uint8_t abInput0[len0]; \
        uint8_t abInput1[len1];, \
        fun(astVectors[i].abInput0, astVectors[i].abInput1, astVectors[i].abComputed), \
        __VA_ARGS__ \
    )

#define TEST_VECTORS_3_ARGUMENTS(tag,len0,len1,len2,lenout,fun,...) \
    TEST_VECTORS_SETUP(tag, lenout, \
        uint8_t abInput0[len0]; \
        uint8_t abInput1[len1]; \
        uint8_t abInput2[len2];, \
        fun(astVectors[i].abInput0, astVectors[i].abInput1, astVectors[i].abInput2, astVectors[i].abComputed), \
        __VA_ARGS__ \
    )

#define TEST_VECTORS_4_ARGUMENTS(tag,len0,len1,len2,len3,lenout,fun,...) \
    TEST_VECTORS_SETUP(tag, lenout, \
        uint8_t abInput0[len0]; \
        uint8_t abInput1[len1]; \
        uint8_t abInput2[len2]; \
        uint8_t abInput3[len3];, \
        fun(astVectors[i].abInput0, astVectors[i].abInput1, astVectors[i].abInput2, astVectors[i].abInput3, astVectors[i].abComputed), \
        __VA_ARGS__ \
    )

void test_transform_80_to_120_alt() {
    TEST_VECTORS_1_ARGUMENT("transform_80_to_120_alt", 10, 15, transform_80_to_120_alt, {
            { { 0x45,0x67,0x89,0xAB,0xCD,0xEF,0xAA,0xBB,0xCC,0xDD },{ 0x45,0x67,0x22,0x89,0xAB,0x22,0xCD,0xEF,0x22,0xAA,0xBB,0x11,0xCC,0xDD,0x11 } },
            { { 0xE8,0xD4,0x7C,0xA2,0xED,0xAE,0x0C,0xFB,0xF0,0xBA },{ 0xE8,0xD4,0x3C,0x7C,0xA2,0xDE,0xED,0xAE,0x43,0x0C,0xFB,0xF7,0xF0,0xBA,0x4A } }, 
            { { 0xC5,0x61,0x75,0xBF,0xBE,0x35,0x68,0x91,0xE2,0x29 },{ 0xC5,0x61,0xA4,0x75,0xBF,0xCA,0xBE,0x35,0x8B,0x68,0x91,0xF9,0xE2,0x29,0xCB } }, 
            { { 0x06,0x96,0xE8,0x3D,0x16,0x60,0x89,0x89,0x2E,0xC8 },{ 0x06,0x96,0x90,0xE8,0x3D,0xD5,0x16,0x60,0x76,0x89,0x89,0x00,0x2E,0xC8,0xE6 } }, 
            { { 0x4A,0x1E,0x24,0x93,0x04,0xEF,0xC2,0xF9,0x5C,0xBC },{ 0x4A,0x1E,0x54,0x24,0x93,0xB7,0x04,0xEF,0xEB,0xC2,0xF9,0x3B,0x5C,0xBC,0xE0 } },
            { { 0xFF,0xDB,0xFB,0xCD,0x89,0x0C,0xF7,0x9D,0x22,0x4E },{ 0xFF,0xDB,0x24,0xFB,0xCD,0x36,0x89,0x0C,0x85,0xF7,0x9D,0x6A,0x22,0x4E,0x6C } }
        }
    );
}

void test_transform_88_to_120() {
    TEST_VECTORS_1_ARGUMENT("transform_88_to_120", 11, 15, transform_88_to_120, {
            { { 0x23,0x45,0x67,0x89,0xAB,0xCD,0xEF,0xAA,0xBB,0xCC,0xDD },{ 0x23,0x45,0x66,0x67,0x89,0xAB,0x45,0xCD,0xEF,0xAA,0x88,0xBB,0xCC,0xDD,0xAA } },
            { { 0xF8,0xE8,0xD4,0x7C,0xA2,0xED,0xAE,0x0C,0xFB,0xF0,0xBA },{ 0xF8,0xE8,0x10,0xD4,0x7C,0xA2,0x0A,0xED,0xAE,0x0C,0x4F,0xFB,0xF0,0xBA,0xB1 } },
            { { 0x41,0xC5,0x61,0x75,0xBF,0xBE,0x35,0x68,0x91,0xE2,0x29 },{ 0x41,0xC5,0x84,0x61,0x75,0xBF,0xAB,0xBE,0x35,0x68,0xE3,0x91,0xE2,0x29,0x5A } },
            { { 0x3E,0x06,0x96,0xE8,0x3D,0x16,0x60,0x89,0x89,0x2E,0xC8 },{ 0x3E,0x06,0x38,0x96,0xE8,0x3D,0x43,0x16,0x60,0x89,0xFF,0x89,0x2E,0xC8,0x6F } },
            { { 0x38,0x4A,0x1E,0x24,0x93,0x04,0xEF,0xC2,0xF9,0x5C,0xBC },{ 0x38,0x4A,0x72,0x1E,0x24,0x93,0xA9,0x04,0xEF,0xC2,0x29,0xF9,0x5C,0xBC,0x19 } },
            { { 0x87,0xFF,0xDB,0xFB,0xCD,0x89,0x0C,0xF7,0x9D,0x22,0x4E },{ 0x87,0xFF,0x78,0xDB,0xFB,0xCD,0xED,0x89,0x0C,0xF7,0x72,0x9D,0x22,0x4E,0xF1 } }
        }
    );
}

void test_transform_80_to_128() {
    TEST_VECTORS_1_ARGUMENT("transform_80_to_128", 10, 16, transform_80_to_128, {
            { { 0x45,0x67,0x89,0xAB,0xCD,0xEF,0xAA,0xBB,0xCC,0xDD },{ 0xBC,0x22,0x45,0xDD,0x33,0x67,0xCC,0x44,0x89,0xBB,0x55,0xAB,0xAA,0xBC,0xCD,0xEF } }, 
            { { 0xE8,0xD4,0x7C,0xA2,0xED,0xAE,0x0C,0xFB,0xF0,0xBA },{ 0x24,0xA2,0xE8,0xBA,0xC4,0xD4,0xF0,0x77,0x7C,0xFB,0xAE,0xA2,0x0C,0x9B,0xED,0xAE } }, 
            { { 0xC5,0x61,0x75,0xBF,0xBE,0x35,0x68,0x91,0xE2,0x29 },{ 0x7F,0xEE,0xC5,0x29,0x43,0x61,0xE2,0x06,0x75,0x91,0x27,0xBF,0x68,0xF3,0xBE,0x35 } }, 
            { { 0x06,0x96,0xE8,0x3D,0x16,0x60,0x89,0x89,0x2E,0xC8 },{ 0xCB,0xCE,0x06,0xC8,0xC4,0x96,0x2E,0x71,0xE8,0x89,0xC6,0x3D,0x89,0x76,0x16,0x60 } }, 
            { { 0x4A,0x1E,0x24,0x93,0x04,0xEF,0xC2,0xF9,0x5C,0xBC },{ 0xC7,0x06,0x4A,0xBC,0x7A,0x1E,0x5C,0x1D,0x24,0xF9,0x55,0x93,0xC2,0xF3,0x04,0xEF } }, 
            { { 0xFF,0xDB,0xFB,0xCD,0x89,0x0C,0xF7,0x9D,0x22,0x4E },{ 0x79,0x4D,0xFF,0x4E,0xFD,0xDB,0x22,0x98,0xFB,0x9D,0xC4,0xCD,0xF7,0x95,0x89,0x0C } }
        }
    );
}

void test_transform_120_to_88() {
    TEST_VECTORS_1_ARGUMENT("transform_120_to_88", 15, 11, transform_120_to_88, {
            { { 0x23,0x45,0x67,0x89,0xAB,0xCD,0xEF,0xAA,0xBB,0xCC,0xDD,0xBC,0x22,0x45,0xDD },{ 0x23,0x45,0x89,0xAB,0xCD,0xAA,0xBB,0xCC,0xBC,0x22,0x45 } },
            { { 0xF8,0xE8,0xD4,0x7C,0xA2,0xED,0xAE,0x0C,0xFB,0xF0,0xBA,0xE8,0x3D,0x16,0x60 },{ 0xF8,0xE8,0x7C,0xA2,0xED,0x0C,0xFB,0xF0,0xE8,0x3D,0x16 } },
            { { 0x41,0xC5,0x61,0x75,0xBF,0xBE,0x35,0x68,0x91,0xE2,0x29,0xF7,0x9D,0x22,0x4E },{ 0x41,0xC5,0x75,0xBF,0xBE,0x68,0x91,0xE2,0xF7,0x9D,0x22 } },
            { { 0x3E,0x06,0x96,0xE8,0x3D,0x16,0x60,0x89,0x89,0x2E,0xC8,0x57,0x87,0xFF,0xDB },{ 0x3E,0x06,0xE8,0x3D,0x16,0x89,0x89,0x2E,0x57,0x87,0xFF } },
            { { 0x38,0x4A,0x1E,0x24,0x93,0x04,0xEF,0xC2,0xF9,0x5C,0xBC,0x89,0xAB,0xCD,0xEF },{ 0x38,0x4A,0x24,0x93,0x04,0xC2,0xF9,0x5C,0x89,0xAB,0xCD } },
            { { 0x87,0xFF,0xDB,0xFB,0xCD,0x89,0x0C,0xF7,0x9D,0x22,0x4E,0x0C,0xFB,0xF0,0xBA },{ 0x87,0xFF,0xFB,0xCD,0x89,0xF7,0x9D,0x22,0x0C,0xFB,0xF0 } }
        }
    );
}

void test_transform_80_to_128_alt() {
    TEST_VECTORS_1_ARGUMENT("transform_80_to_128_alt", 10, 16, transform_80_to_128_alt, {
            { { 0x45,0x67,0x89,0xAB,0xCD,0xEF,0xAA,0xBB,0xCC,0xDD },{ 0x45,0x67,0x22,0x89,0xAB,0x22,0xCD,0xEF,0x22,0xAA,0xBB,0x11,0xCC,0xDD,0x11,0x88 } },
            { { 0xE8,0xD4,0x7C,0xA2,0xED,0xAE,0x0C,0xFB,0xF0,0xBA },{ 0xE8,0xD4,0x3C,0x7C,0xA2,0xDE,0xED,0xAE,0x43,0x0C,0xFB,0xF7,0xF0,0xBA,0x4A,0x9E } },
            { { 0xC5,0x61,0x75,0xBF,0xBE,0x35,0x68,0x91,0xE2,0x29 },{ 0xC5,0x61,0xA4,0x75,0xBF,0xCA,0xBE,0x35,0x8B,0x68,0x91,0xF9,0xE2,0x29,0xCB,0xBD } },
            { { 0x06,0x96,0xE8,0x3D,0x16,0x60,0x89,0x89,0x2E,0xC8 },{ 0x06,0x96,0x90,0xE8,0x3D,0xD5,0x16,0x60,0x76,0x89,0x89,0x00,0x2E,0xC8,0xE6,0xC1 } },
            { { 0x4A,0x1E,0x24,0x93,0x04,0xEF,0xC2,0xF9,0x5C,0xBC },{ 0x4A,0x1E,0x54,0x24,0x93,0xB7,0x04,0xEF,0xEB,0xC2,0xF9,0x3B,0x5C,0xBC,0xE0,0x11 } },
            { { 0xFF,0xDB,0xFB,0xCD,0x89,0x0C,0xF7,0x9D,0x22,0x4E },{ 0xFF,0xDB,0x24,0xFB,0xCD,0x36,0x89,0x0C,0x85,0xF7,0x9D,0x6A,0x22,0x4E,0x6C,0xB5 } }
        }
    );
}

void test_tb4() {
    TEST_VECTORS_2_ARGUMENTS("tb4", 10, 10, 10, tb4, {
            { { 0x01,0x23,0x45,0x67,0x89,0xAB,0xCD,0xEF,0xAA,0xBB }, { 0x01,0x23,0x45,0x67,0x89,0xAB,0xCD,0xEF,0xAA,0xBB }, { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00  } },
            { { 0xBD,0xF8,0xE8,0xD4,0x7C,0xA2,0xED,0xAE,0x0C,0xFB }, { 0x56,0x3B,0x92,0xC2,0xA2,0x27,0x5A,0x0F,0x61,0x13 }, { 0xEB,0xC3,0x7A,0x16,0xDE,0x85,0xB7,0xA1,0x6D,0xE8  } },
            { { 0x8A,0x41,0xC5,0x61,0x75,0xBF,0xBE,0x35,0x68,0x91 }, { 0x2D,0xCA,0xB8,0x83,0xAA,0xC7,0x09,0xEB,0x45,0x66 }, { 0xA7,0x8B,0x7D,0xE2,0xDF,0x78,0xB7,0xDE,0x2D,0xF7  } },
            { { 0xBA,0x3E,0x06,0x96,0xE8,0x3D,0x16,0x60,0x89,0x89 }, { 0x9A,0x87,0xD3,0x69,0x9D,0x42,0xCB,0x3F,0x7E,0xDE }, { 0x20,0xB9,0xD5,0xFF,0x75,0x7F,0xDD,0x5F,0xF7,0x57  } }
        }
    );
}

void test_tb5() {
    TEST_VECTORS_4_ARGUMENTS("tb5", 2, 2, 1, 10, 10, tb5, {
            { { 0x02,0xBC },{ 0x1D,0xCC }, { 0x05 }, { 0x01,0x23,0x45,0x67,0x89,0xAB,0xCD,0xEF,0xAA,0xBB }, { 0x76,0x13,0xEA,0x62,0xA2,0x6A,0x87,0x1F,0xF8,0x07 } },
            { { 0x0D,0xE8 },{ 0x3A,0xF0 }, { 0x16 }, { 0xBD,0xF8,0xE8,0xD4,0x7C,0xA2,0xED,0xAE,0x0C,0xFB }, { 0x56,0x3B,0x92,0xC2,0xA2,0x27,0x5A,0x0F,0x61,0x13 } },
            { { 0x0D,0xF7 },{ 0x29,0xE2 }, { 0x22 }, { 0x8A,0x41,0xC5,0x61,0x75,0xBF,0xBE,0x35,0x68,0x91 }, { 0x2D,0xCA,0xB8,0x83,0xAA,0xC7,0x09,0xEB,0x45,0x66 } },
            { { 0x07,0x57 },{ 0x08,0x2E }, { 0x3F }, { 0xBA,0x3E,0x06,0x96,0xE8,0x3D,0x16,0x60,0x89,0x89 }, { 0x9A,0x87,0xD3,0x69,0x9D,0x42,0xCB,0x3F,0x7E,0xDE } }
        }
    );
}

void test_tb6() {
    TEST_VECTORS_3_ARGUMENTS("tb6", 10, 2, 3, 10, tb6, {
            { { 0x01,0x23,0x45,0x67,0x89,0xAB,0xCD,0xEF,0xAA,0xBB },{ 0x02,0xBC },{ 0x1D,0xCC,0x05}, { 0x2A,0xE2,0x99,0xA7,0xDB,0x17,0xD0,0x23,0xAF,0xBE } },
            { { 0xBD,0xF8,0xE8,0xD4,0x7C,0xA2,0xED,0xAE,0x0C,0xFB },{ 0x0D,0xE8 },{ 0x3A,0xF0,0x16}, { 0x63,0x7B,0x47,0xD5,0x11,0x4A,0xD7,0x5E,0x1A,0xED } },
            { { 0x8A,0x41,0xC5,0x61,0x75,0xBF,0xBE,0x35,0x68,0x91 },{ 0x0D,0xF7 },{ 0x29,0xE2,0x22}, { 0x55,0x33,0x5B,0x43,0x58,0x48,0x97,0xD7,0x4A,0xB3 } },
            { { 0xBA,0x3E,0x06,0x96,0xE8,0x3D,0x16,0x60,0x89,0x89 },{ 0x07,0x57 },{ 0x08,0x2E,0x3F}, { 0xCF,0x4E,0x84,0x75,0x1F,0x6A,0x1E,0x4E,0xB6,0xB6 } }
        }
    );
}

void test_tb7() {
    TEST_VECTORS_1_ARGUMENT("tb7", 12, 16, tb7, {
            { { 0x01,0x23,0x45,0x67,0x89,0xAB,0xCD,0xEF,0xAA,0xBB,0x02,0xBC },{ 0x01,0x23,0x45,0x67,0x67,0x89,0xAB,0x45,0xCD,0xEF,0xAA,0x88,0xBB,0x02,0xBC,0x05 } },
            { { 0xBD,0xF8,0xE8,0xD4,0x7C,0xA2,0xED,0xAE,0x0C,0xFB,0x0D,0xE8 },{ 0xBD,0xF8,0xE8,0xAD,0xD4,0x7C,0xA2,0x0A,0xED,0xAE,0x0C,0x4F,0xFB,0x0D,0xE8,0x1E } },
            { { 0x8A,0x41,0xC5,0x61,0x75,0xBF,0xBE,0x35,0x68,0x91,0x0D,0xF7 },{ 0x8A,0x41,0xC5,0x0E,0x61,0x75,0xBF,0xAB,0xBE,0x35,0x68,0xE3,0x91,0x0D,0xF7,0x6B } },
            { { 0xBA,0x3E,0x06,0x96,0xE8,0x3D,0x16,0x60,0x89,0x89,0x07,0x57 },{ 0xBA,0x3E,0x06,0x82,0x96,0xE8,0x3D,0x43,0x16,0x60,0x89,0xFF,0x89,0x07,0x57,0xD9 } }
        }
    );
}

void test_ta11_ta41() {
    TEST_VECTORS_2_ARGUMENTS("ta11 (aka ta41)", 16, 10, 16, ta11_ta41, {
            {
                { 0x77,0xe7,0x9f,0xee,0x7f,0xc6,0x54,0xdc,0x65,0x44,0x64,0x4f,0xdf,0x47,0x68,0x15 },
                { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 },
                { 0x9C,0x84,0x51,0xA3,0x56,0x95,0xD3,0x3C,0x30,0x94,0x37,0x12,0x02,0x48,0x54,0x53 }
            }, {
                { 0xc6,0x2e,0x22,0x85,0x03,0x40,0xbc,0xeb,0x55,0x52,0x22,0x28,0x60,0x17,0x3d,0x7e },
                { 0x56,0x5a,0x72,0xd6,0x3c,0xce,0xed,0x0b,0x6f,0x30 },
                { 0x77,0xBC,0x47,0xF6,0x5C,0x87,0xC1,0xE7,0x49,0xB7,0x4F,0xDE,0xA6,0xB5,0x46,0x61 }
            }, {
                { 0x4e,0xbb,0x68,0x9d,0x87,0x4a,0xd6,0x41,0x79,0x05,0xc0,0xed,0xaa,0x3f,0x90,0xec },
                { 0x93,0x5e,0x49,0xfc,0xdc,0xbb,0x47,0x58,0x19,0x55 },
                { 0x48,0x9C,0x79,0xEA,0x05,0x2F,0xDE,0xFA,0x90,0x2A,0x83,0x3F,0x26,0xCF,0x12,0x7C }
            }, {
                { 0x67,0xfb,0x13,0x4d,0xd7,0x9c,0x7d,0x77,0xf5,0x2a,0x5d,0xce,0xf2,0x3d,0xe6,0xfd },
                { 0xb8,0x24,0xff,0xb1,0x37,0xa4,0xef,0x87,0xe0,0x7a },
                { 0xB7,0x14,0x21,0xBA,0x11,0xCF,0xD5,0x4A,0xD6,0xC4,0xD2,0x57,0x92,0x5A,0x53,0xB2 }
            }
        }
    );
}
 
void test_ta21() {
    TEST_VECTORS_2_ARGUMENTS("ta21", 16, 10, 16, ta21, {
            {
                { 0x77,0xe7,0x9f,0xee,0x7f,0xc6,0x54,0xdc,0x65,0x44,0x64,0x4f,0xdf,0x47,0x68,0x15 },
                { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 },
                { 0x9C,0x84,0x51,0xA3,0x56,0x95,0xD3,0x3C,0x30,0x94,0x37,0x12,0x02,0x48,0x54,0x53 }
            }, {
                { 0xc6,0x2e,0x22,0x85,0x03,0x40,0xbc,0xeb,0x55,0x52,0x22,0x28,0x60,0x17,0x3d,0x7e },
                { 0x56,0x5a,0x72,0xd6,0x3c,0xce,0xed,0x0b,0x6f,0x30 },
                { 0xFC,0xFA,0xF4,0x55,0x92,0xDF,0xC6,0x5D,0x8A,0x1F,0x5C,0x45,0xDC,0xA2,0x93,0xDA }
            }, {
                { 0x4e,0xbb,0x68,0x9d,0x87,0x4a,0xd6,0x41,0x79,0x05,0xc0,0xed,0xaa,0x3f,0x90,0xec },
                { 0x93,0x5e,0x49,0xfc,0xdc,0xbb,0x47,0x58,0x19,0x55 },
                { 0x5E,0x4C,0x24,0x1E,0x21,0x91,0x5A,0x48,0x07,0x05,0x29,0x42,0xAF,0x14,0xAC,0xCD }
            }, {
                { 0x67,0xfb,0x13,0x4d,0xd7,0x9c,0x7d,0x77,0xf5,0x2a,0x5d,0xce,0xf2,0x3d,0xe6,0xfd },
                { 0xb8,0x24,0xff,0xb1,0x37,0xa4,0xef,0x87,0xe0,0x7a },
                { 0xAD,0x31,0x0A,0xEF,0x61,0xB0,0x6B,0x2A,0x6C,0x83,0x30,0xC6,0x14,0x5B,0x7F,0xEE }
            }
        }
    );
}

void test_ta31() {
    TEST_VECTORS_INVERTIBLE_EX("ta31","ta32",abSealed,abUnsealed,
        uint8_t abUnsealed[10];
        uint8_t abCckId[2];
        uint8_t abDck[10];
        uint8_t abSealed[15];
        uint8_t bMf;,
        ta31(astVectors[i].abUnsealed, astVectors[i].abCckId, astVectors[i].abDck, astVectors[i].abComputed);,
        ta32(astVectors[i].abSealed, astVectors[i].abCckId, astVectors[i].abDck, astVectors[i].abComputed, &astVectors[i].bMf);,
        memcmp(astVectors[i].abSealed, astVectors[i].abComputed, sizeof(astVectors[i].abSealed)),
        memcmp(astVectors[i].abUnsealed, astVectors[i].abComputed, sizeof(astVectors[i].abUnsealed)) | astVectors[i].bMf, {
            {
                { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 }, { 0x6b,0x18 },
                { 0x5f,0xb0,0x44,0x2f,0x4b,0x5e,0xe2,0xf0,0xea,0x91 },
                { 0xA3,0x48,0x85,0xFC,0x27,0x7D,0x8D,0x96,0x11,0xD4,0x0E,0x22,0x40,0x0A,0x14 }
            }, {
                { 0x7d,0x33,0x96,0x8e,0xf5,0xb5,0x34,0x3d,0x3d,0x0e }, { 0xb2,0xe3 },
                { 0x08,0x4a,0xc4,0x80,0x91,0x56,0xce,0x54,0x4c,0xb2 },
                { 0xBB,0xE6,0xF1,0xBC,0x44,0x85,0x0B,0xC7,0x35,0x3C,0xBB,0xB4,0xAF,0x5B,0x39 }
            }, {
                { 0x39,0xec,0x0e,0x26,0xf2,0x27,0xef,0xf7,0x64,0x8f }, { 0xa0,0x86 },
                { 0xfd,0xaf,0xc2,0x1c,0xc8,0x65,0xa7,0xa0,0x85,0xec },
                { 0xBB,0x07,0xEB,0xB6,0x78,0x17,0xA2,0xC0,0xEF,0xA9,0xE3,0x3C,0xBC,0x7B,0x7B }
            }, {
                { 0xd6,0x07,0x5e,0x8e,0x93,0xd9,0x50,0x14,0xc3,0x75 },{ 0x22,0x86 },
                { 0xfc,0x4c,0xa2,0x29,0x7f,0xba,0xf0,0x96,0x2e,0x5b },
                { 0x18,0xE0,0x90,0x00,0x4A,0xC3,0xBD,0x2C,0xC1,0x45,0x3A,0x33,0x8F,0x16,0xD3 }
            }
        }
    );
}

void test_ta51() {
    TEST_VECTORS_INVERTIBLE_EX("ta51","ta52",abSealed,abUnsealed,
        uint8_t abUnsealed[10];
        uint8_t abVn[2];
        uint8_t abKey[16];
        uint8_t bKeyN;
        uint8_t abSealed[15];
        uint8_t bMf;
        uint8_t bComputedKeyN;,
        ta51(astVectors[i].abUnsealed, astVectors[i].abVn, astVectors[i].abKey, &astVectors[i].bKeyN, astVectors[i].abComputed);,
        ta52(astVectors[i].abSealed, astVectors[i].abKey, astVectors[i].abVn, astVectors[i].abComputed, &astVectors[i].bMf, &astVectors[i].bComputedKeyN);,
        memcmp(astVectors[i].abSealed, astVectors[i].abComputed, sizeof(astVectors[i].abSealed)),
        memcmp(astVectors[i].abUnsealed, astVectors[i].abComputed, sizeof(astVectors[i].abUnsealed)) | astVectors[i].bMf | (astVectors[i].bKeyN - astVectors[i].bComputedKeyN), {
            {
                { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 },{ 0x0f,0x6d },
                { 0x77,0xe7,0x9f,0xee,0x7f,0xc6,0x54,0xdc,0x65,0x44,0x64,0x4f,0xdf,0x47,0x68,0x15 }, 0x0f,
                { 0x08, 0x3D, 0x05, 0xA7, 0x8E, 0x86, 0xFD, 0x5F, 0x46, 0xD6, 0x2B, 0x28, 0x42, 0x2B, 0x0B }
            }, {
                { 0x56,0x5a,0x72,0xd6,0x3c,0xce,0xed,0x0b,0x6f,0x30 },{ 0x79,0x0a },
                { 0xc6,0x2e,0x22,0x85,0x03,0x40,0xbc,0xeb,0x55,0x52,0x22,0x28,0x60,0x17,0x3d,0x7e }, 0x1b,
                { 0x90, 0xB1, 0xEF, 0x3A, 0xCE, 0x5C, 0xAD, 0x1A, 0x87, 0x2A, 0x75, 0x39, 0xBC, 0xCA, 0xA4 }
            }, {
                { 0x93,0x5e,0x49,0xfc,0xdc,0xbb,0x47,0x58,0x19,0x55 },{ 0x60,0xab },
                { 0x4e,0xbb,0x68,0x9d,0x87,0x4a,0xd6,0x41,0x79,0x05,0xc0,0xed,0xaa,0x3f,0x90,0xec }, 0x13,
                { 0x2D, 0xDA, 0x81, 0xF9, 0x9C, 0xA3, 0x1C, 0x3E, 0xD8, 0xE6, 0xDE, 0x31, 0xF1, 0x6D, 0x58 }
            }, {
                { 0xb8,0x24,0xff,0xb1,0x37,0xa4,0xef,0x87,0xe0,0x7a },{ 0x4c,0xad },
                { 0x67,0xfb,0x13,0x4d,0xd7,0x9c,0x7d,0x77,0xf5,0x2a,0x5d,0xce,0xf2,0x3d,0xe6,0xfd }, 0x03,
                { 0x3E, 0x7C, 0x8E, 0x73, 0x3B, 0xC1, 0x33, 0xA7, 0x0D, 0x27, 0x83, 0x97, 0x43, 0x50, 0x30 }
            }
        }
    );
}

void test_ta71() {
    TEST_VECTORS_2_ARGUMENTS("ta71", 10, 10, 10, ta71, {
            {
                { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 },
                { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 },
                { 0x32,0x14,0xCD,0x6B,0xC0,0x48,0x8C,0xDC,0x46,0x76 }
            }, {
                { 0x01,0x23,0x45,0x67,0x89,0xAB,0xCD,0xEF,0xAA,0xBB },
                { 0xBD,0xF8,0xE8,0xD4,0x7C,0xA2,0xED,0xAE,0x0C,0xFB },
                { 0x90,0xC6,0x03,0x7D,0x99,0xDB,0x19,0x7C,0xAF,0x27 }
            }, {
                { 0x8A,0x41,0xC5,0x61,0x75,0xBF,0xBE,0x35,0x68,0x91 },
                { 0xBA,0x3E,0x06,0x96,0xE8,0x3D,0x16,0x60,0x89,0x89 },
                { 0x50,0x86,0x5D,0x57,0xF9,0xB3,0xD6,0x1B,0x90,0x99 }
            }
        }
    );
}

void test_ta81() {
    TEST_VECTORS_INVERTIBLE_EX("ta81","ta82",abSealedGck,abUnsealedGck,
        uint8_t abUnsealedGck[10];
        uint8_t abGckVn[2];
        uint8_t abGckN[2];
        uint8_t abKey[16];
        uint8_t abSealedGck[15];
        uint8_t bMf;
        uint8_t abComputedGckN[2];,
        ta81(astVectors[i].abUnsealedGck, astVectors[i].abGckVn, astVectors[i].abGckN, astVectors[i].abKey, astVectors[i].abComputed);,
        ta82(astVectors[i].abSealedGck, astVectors[i].abGckVn, astVectors[i].abKey, astVectors[i].abComputed, &astVectors[i].bMf, astVectors[i].abComputedGckN);,
        memcmp(astVectors[i].abSealedGck, astVectors[i].abComputed, sizeof(astVectors[i].abSealedGck)),
            memcmp(astVectors[i].abUnsealedGck, astVectors[i].abComputed, sizeof(astVectors[i].abUnsealedGck)) | \
            astVectors[i].bMf | \
            memcmp(astVectors[i].abGckN, astVectors[i].abComputedGckN, sizeof(astVectors[i].abGckN)), {
            {
                { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 },{ 0xa3,0x97 },{ 0x1f,0x5d },
                { 0x63,0x59,0x38,0xac,0xe8,0x1f,0x6b,0x67,0x82,0xa6,0xfa,0x46,0xae,0x4f,0x7f,0x69 },
                { 0x1c,0xcf,0x9c,0x6b,0xa8,0x5c,0x5b,0x45,0x60,0xf9,0xcf,0x5c,0x63,0xb0,0xdc }
            }, {
                { 0x63,0x09,0x5a,0xeb,0x26,0xf0,0xe7,0x26,0x28,0xd0 },{ 0x8f,0xbc },{ 0xe3,0x63 },
                { 0x55,0x36,0xeb,0x46,0xa6,0x02,0xe7,0x80,0x49,0x29,0x9d,0x82,0xab,0x64,0x80,0x45 },
                { 0xc9,0x35,0x95,0xa5,0x4e,0xcd,0x34,0xf9,0xb5,0x9a,0xfb,0x3a,0x0d,0x59,0x05 }
            }, {
                { 0x10,0x6a,0x40,0xc6,0xf7,0xb0,0x89,0xb7,0xac,0x41 },{ 0xbe,0xc4 },{ 0x12,0xa6 },
                { 0x08,0xf6,0x17,0x4e,0xc8,0x18,0xd2,0xa2,0x14,0xf1,0xd7,0x53,0x4d,0x31,0x9f,0x6c },
                { 0x03,0x52,0x0f,0xe9,0x75,0x7b,0x66,0x83,0x0b,0x36,0xb1,0x3b,0x7c,0xe5,0xb4 }
            }, {
                { 0xc3,0xc6,0x82,0xda,0xd8,0xaa,0x33,0xa2,0xce,0x6b },{ 0x1f,0x8b },{ 0xce,0x89 },
                { 0xc2,0x71,0x13,0x63,0xd3,0x6d,0x4f,0xbf,0xc9,0xf5,0xcf,0x73,0xb8,0xb4,0xdd,0x35 },
                { 0xc8,0x6a,0x1d,0x03,0x2c,0x97,0xbb,0x86,0x7a,0x30,0x2f,0x35,0xdf,0x7e,0x93 }
            }
        }
    );
}

void test_ta91() {
    TEST_VECTORS_INVERTIBLE_EX("ta91","ta92",abSealedGsko,abUnsealedGsko,
        uint8_t abUnsealedGsko[12];
        uint8_t abGskoVn[2];
        uint8_t abKey[16];
        uint8_t abSealedGsko[15];
        uint8_t bMf;,
        ta91(astVectors[i].abUnsealedGsko, astVectors[i].abGskoVn, astVectors[i].abKey, astVectors[i].abComputed);,
        ta92(astVectors[i].abSealedGsko, astVectors[i].abGskoVn, astVectors[i].abKey, astVectors[i].abComputed, &astVectors[i].bMf);,
        memcmp(astVectors[i].abSealedGsko, astVectors[i].abComputed, sizeof(astVectors[i].abSealedGsko)),
        memcmp(astVectors[i].abUnsealedGsko, astVectors[i].abComputed, sizeof(astVectors[i].abUnsealedGsko)) | astVectors[i].bMf, {
            {
                { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xb0,0xdc },{ 0xa3,0x97 },
                { 0x63,0x59,0x38,0xac,0xe8,0x1f,0x6b,0x67,0x82,0xa6,0xfa,0x46,0xae,0x4f,0x7f,0x69 },
                { 0x1c,0xcf,0x9c,0x6b,0xa8,0x5c,0x5b,0xc7,0xe0,0x7b,0x04,0x21,0xf2,0xc3,0x8a }
            }, {
                { 0x63,0x09,0x5a,0xeb,0x26,0xf0,0xe7,0x26,0x28,0xd0,0x59,0x05 },{ 0x8f,0xbc },
                { 0x55,0x36,0xeb,0x46,0xa6,0x02,0xe7,0x80,0x49,0x29,0x9d,0x82,0xab,0x64,0x80,0x45 },
                { 0xc9,0x35,0x95,0xa5,0x4e,0xcd,0x34,0x16,0x13,0xd0,0xcf,0x87,0x03,0xeb,0xac }
            }, {
                { 0x10,0x6a,0x40,0xc6,0xf7,0xb0,0x89,0xb7,0xac,0x41,0xe5,0xb4 },{ 0xbe,0xc4 },
                { 0x08,0xf6,0x17,0x4e,0xc8,0x18,0xd2,0xa2,0x14,0xf1,0xd7,0x53,0x4d,0x31,0x9f,0x6c },
                { 0x03,0x52,0x0f,0xe9,0x75,0x7b,0x66,0x9d,0x93,0xf6,0x57,0x5f,0xee,0x6f,0x9c }
            }, {
                { 0xc3,0xc6,0x82,0xda,0xd8,0xaa,0x33,0xa2,0xce,0x6b,0x7e,0x93 },{ 0x1f,0x8b },
                { 0xc2,0x71,0x13,0x63,0xd3,0x6d,0x4f,0xbf,0xc9,0xf5,0xcf,0x73,0xb8,0xb4,0xdd,0x35 },
                { 0xc8,0x6a,0x1d,0x03,0x2c,0x97,0xbb,0x94,0xb0,0x01,0x8f,0xe5,0x50,0x7b,0xa7 }
            }
        }
    );
}

typedef struct {
    uint8_t abPlaintext[8];
    uint8_t abKey[16];
    uint8_t abCiphertext[8];
} HurdleTestVector;


void test_HURDLE() {
    TEST_VECTORS_INVERTIBLE("HURDLE_encrypt","HURDLE_decrypt",abCiphertext,abPlaintext,
        uint8_t abPlaintext[8];
        uint8_t abKey[16];
        uint8_t abCiphertext[8];,
        HURDLE_CTX stCipher;
        HURDLE_set_key(astVectors[i].abKey, &stCipher);
        HURDLE_encrypt(astVectors[i].abComputed, astVectors[i].abPlaintext, &stCipher, HURDLE_ENCRYPT);,
        HURDLE_CTX stCipher;
        HURDLE_set_key(astVectors[i].abKey, &stCipher);
        HURDLE_encrypt(astVectors[i].abComputed, astVectors[i].abCiphertext, &stCipher, HURDLE_DECRYPT);, {
            {
                { 0xca, 0xfe, 0xba, 0xbe, 0xde, 0xad, 0xbe, 0xef },
                { 0xab, 0xcd, 0xef, 0x12, 0xc0, 0x01, 0xf0, 0x0d, 0xde, 0xad, 0xbe, 0xef, 0xca, 0xfe, 0xba, 0xbe},
                { 0x4b, 0xf1, 0x55, 0x08, 0x81, 0x2e, 0x06, 0xf0 },
            }, {
                { 0x22, 0x22, 0x66, 0x66, 0x22, 0x22, 0xee, 0xee },
                { 0x99, 0x99, 0x00, 0x99, 0x99, 0x11, 0x88, 0x99, 0x22, 0x77, 0x99, 0x33, 0x66, 0x99, 0x44, 0x55 },
                { 0xb4, 0xda, 0x66, 0x98, 0xd3, 0x6b, 0x16, 0x52 }
            }
        }
    );
}

void test_TEA1() {
    TEST_VECTORS_SETUP("TEA1", 10,
        uint32_t dwFrameNumbers;
        uint8_t abKey[10];,
        tea1(astVectors[i].dwFrameNumbers, astVectors[i].abKey, 10, astVectors[i].abComputed), {
            { 0x11111111, { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 }, { 0xd3,0x3f,0xd8,0xa6,0x05,0xa0,0xa1,0xbb,0x90,0x23 } },
            { 0x01234567, { 0xA7,0x98,0x39,0xE4,0xBA,0x88,0xEE,0x54,0xA0,0x29 }, { 0x1d,0xec,0x9c,0x7e,0xc6,0x22,0x3d,0x87,0xc2,0xcc } }
        }
    );
}

void test_TEA2() {
    TEST_VECTORS_SETUP("TEA2", 10,
        uint32_t dwIv;
        uint8_t abKey[10];,
        tea2(astVectors[i].dwIv, astVectors[i].abKey, 10, astVectors[i].abComputed), {
            { 0x12345678, { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 }, { 0xA7,0x98,0x39,0xE4,0xBA,0x88,0xEE,0x54,0xA0,0x29 } },
            { 0x12345678, { 0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xAA }, { 0x64,0x70,0x4E,0xA9,0xD7,0xDC,0x25,0x60,0x81,0x39 } }
        }
    );
}

void test_TEA3() {
    TEST_VECTORS_SETUP("TEA3", 10,
        uint32_t dwFrameNumbers;
        uint8_t abKey[10];,
        tea3(astVectors[i].dwFrameNumbers, astVectors[i].abKey, 10, astVectors[i].abComputed), {
            { 0x11111111, { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 }, { 0x06,0xA6,0x58,0x8C,0x5D,0x9A,0x99,0x6D,0xD2,0x5E } },
            { 0x01234567, { 0xA7,0x98,0x39,0xE4,0xBA,0x88,0xEE,0x54,0xA0,0x29 }, { 0x02,0x49,0x1e,0xf5,0x57,0xc5,0x1c,0x17,0x73,0x0c } }
        }
    );
}

int main() {
    
    test_transform_80_to_120_alt();
    test_transform_88_to_120();
    test_transform_80_to_128();
    test_transform_120_to_88();
    test_transform_80_to_128_alt();
    
    test_ta11_ta41();
    test_ta21();
    test_ta31();
    test_ta51();
    test_ta71();
    test_ta81();
    test_ta91();

    test_tb4();
    test_tb5();
    test_tb6();
    test_tb7();

    test_HURDLE();
    test_TEA1();
    test_TEA2();
    test_TEA3();
}
