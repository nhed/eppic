/*
 * Copyright 2000 Silicon Graphics, Inc. All rights reserved.
 * Copyright 2002-2012 Luc Chouinard. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include "eppic.h"
#include "eppic.tab.h"
/*
    This utility generates a operator function table for the base type.
    Each combinaison of operand type and operation needs a dedicated
    function. We use a table defined here in to generate an indirect table
    that if indexed (from within eppic_op.c) using :

    value_t * (func)(value_t *v1, value_t *v2) = table[type1][type2][op];
*/
static struct opss {
    char *str;
    char *acro;
    int code;
} opstbl[] = {
    { "+", "ADD", ADD },
    { "-", "SUB", SUB },
    { "/", "DIV", DIV },
    { "*", "MUL", MUL },
    { "^", "XOR", XOR },
    { "%", "MOD", MOD },
    { "|", "OR",  OR  },
    { "&", "AND", AND },
    { "<<", "SHL", SHL },
    { ">>", "SHR", SHR },
    { "==", "EQ", EQ }, /* most be first bool */
    { ">", "GT", GT },
    { "<", "LT", LT },
    { ">=", "GE", GE },
    { "<=", "LE", LE },
    { "!=", "NE", NE },
};

static char *typtbl[] = { "sc", "uc", "ss", "us", "sl", "ul", "sll", "ull" };

#define NOPS (sizeof(opstbl)/sizeof(opstbl[0]))
#define NTYPS (sizeof(typtbl)/sizeof(typtbl[0]))

int
main()
{
int i,j,k;

    printf("\
#include \"eppic.h\"\n\
#include \"eppic.tab.h\"\n\
/**************************************************************\n\
 This file is generated by a program.\n\
 Check out and modify libeppic/mkbaseop.c instead !\n\
***************************************************************/\n");


    /* create all the functions for all combinaison */
    for(i=0;i<NTYPS;i++) {
    
        for(j=0; j<NTYPS;j++) {

            int bool=0;

            for(k=0;k<NOPS;k++) {

                if(opstbl[k].code==EQ) bool++;


                if(!bool) {

                    printf(""
"static void \n"
"op_%s_%s_%s(value_t *v1,value_t *v2,value_t *ret)\n"
"{\n"
"   ret->v.%s = v1->v.%s %s v2->v.%s;\n"
"   ret->type.type=%s->type.type;\n"
"   ret->type.idx=%s->type.idx;\n"
"   ret->type.size=%s->type.size;\n"
"}\n", 
                    opstbl[k].acro, 
                    typtbl[i], 
                    typtbl[j], 
                    j>=i?typtbl[j]:typtbl[i], 
                    typtbl[i], 
                    opstbl[k].str,
                    typtbl[j],
                    j>=i?"v2":"v1",
                    j>=i?"v2":"v1",
                    j>=i?"v2":"v1");

                } else {

                    printf(""
"static void \n"
"op_%s_%s_%s(value_t *v1,value_t *v2,value_t *ret)\n"
"{\n"
"   ret->v.%s = ( v1->v.%s %s v2->v.%s );\n"
"   ret->type.type=V_BASE;\n"
"   ret->type.idx=B_UL;\n"
"   ret->type.size=4;\n"
"}\n",
                    opstbl[k].acro, 
                    typtbl[i], 
                    typtbl[j], 
                    "ul",
                    typtbl[i], 
                    opstbl[k].str,
                    typtbl[j]);
                }

            }

        }

    }

    /* create the array from within which the runtime functions
       will indexed to get a function pointer */

    printf("void (*opfuncs[%u][%u][%u])()={\n", NTYPS, NTYPS, NOPS);

    for(i=0;i<NTYPS;i++) {
    
        for(j=0; j<NTYPS;j++) {

            printf("\t");

            for(k=0;k<NOPS;k++) {

                if(!k%6) printf("\n\t");
                printf("op_%s_%s_%s, ", opstbl[k].acro, typtbl[i], typtbl[j]);

            }
            printf("\n");

        }

    }
    printf("};\n");

    /* output a ops lut */
    printf("\nstatic int opslut[%u]={\n", NOPS);

    for(i=0;i<NOPS;i++) {

        printf("%s, ", opstbl[i].acro);

    }
    printf("};\n");

    /* output the main op execution function */
    printf("\n\
void\n\
eppic_baseop(int op, value_t *v1, value_t *v2, value_t *ret)\n\
{\n\
int i;\n\
\n\
    for(i=0;i<%u;i++) {\n\
\n\
        if(opslut[i]==op) break;\n\
\n\
    }\n\
    if(i==%u) eppic_error(\"Oops!ops!\");\n\
    (opfuncs[v1->type.idx][v2->type.idx][i])(v1,v2,ret);\n\
}\n", NOPS, NOPS);
    exit(0);
}
