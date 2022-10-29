#ifndef _yy_defines_h_
#define _yy_defines_h_

#define WORD 257
#define NOTOKEN 258
#define GREAT 259
#define NEWLINE 260
#define GGREAT 261
#define LESS 262
#define LLESS 263
#define PIPE 264
#define AMPERSAND 265
#define GREATAMPERCAND 266
#define GGREATAMPERCAND 267
#ifdef YYSTYPE
#undef  YYSTYPE_IS_DECLARED
#define YYSTYPE_IS_DECLARED 1
#endif
#ifndef YYSTYPE_IS_DECLARED
#define YYSTYPE_IS_DECLARED 1
typedef union YYSTYPE	{
		char   *string_val;
        } YYSTYPE;
#endif /* !YYSTYPE_IS_DECLARED */
extern YYSTYPE yylval;

#endif /* _yy_defines_h_ */
