#ifndef CLI_H
#define CLI_H


#include "conf_file_read.h"
#include "conf_file_create.h"


#define FAIL_WITH_ERROR( CODE , MESSAGE , ... ) \
    {                                           \
        fprintf(stderr, "\n" MESSAGE "\n", __VA_ARGS__ );\
        exit( CODE );                           \
    }


#define PRINT_OPTION_INFO( OPTION_TEXT , OPTION_DESCRIPTION ) \
    fprintf(stderr,"    "   OPTION_TEXT   "\n        "   OPTION_DESCRIPTION   "\n");


#define ARGV_MATCH( INDEX , TEXT ) (strcmp(argv[ INDEX ] , TEXT ) == 0)


void usage_info(int argc, char **argv)
{
    fprintf(stderr,"usage:\n" );
    PRINT_OPTION_INFO( "fsqlf [<input_file>] [<output_file>] [options]"
        , "Read from <input_file> and write formatted output to <output_file> (use std I/O if missing)\n"
        "        If there are overlaping options set, then the last one (overflapping setting) wins.\n"
        "        e.g. If config file is set 2 times, then from 1st file use only configs that don't exist in the 2nd file.");
    PRINT_OPTION_INFO( "fsqlf --create-config-file" , "(Re)create '"CONFIG_FILE"' config file.");
    fprintf(stderr,"options:\n");
    PRINT_OPTION_INFO( "-i <input_file>" , "Use <input_file> as input");
    PRINT_OPTION_INFO( "-o <output_file>" , "Use  <output_file> as output");
    PRINT_OPTION_INFO( "--config-file <config_file>"                , "Read configuration from <config_file>");
    PRINT_OPTION_INFO( "--select-comma-newline (after|before|none)" , "New lines for each item in SELECT clause");
    PRINT_OPTION_INFO( "--select-newline-after <digit>"             , "Put <digit> new lines right after SELECT keyword");
    PRINT_OPTION_INFO( "--newline-or-before <digit>"                , "Put <digit> new lines before OR keyword");
    PRINT_OPTION_INFO( "--newline-or-after <digit>"                 , "Put <digit> new lines before OR keyword");
    PRINT_OPTION_INFO( "--newline-and-before <digit>"               , "Put <digit> new lines before AND keyword");
    PRINT_OPTION_INFO( "--newline-and-after <digit>"                , "Put <digit> new lines before AND keyword");
    PRINT_OPTION_INFO( "--newline-major-sections <digit>"           , "Put <digit> new lines before major sections (FROM, JOIN, WHERE)");
    PRINT_OPTION_INFO( "--keyword-case (upper|lower|initcap|none)"  , "Convert all keywords to UPPER, lower, or Initcap case, or not to convert case at all");
    PRINT_OPTION_INFO( "--keyword-text (original|default)"          , "Use original or programs default text for the keyword, when there are several alternatives");
    PRINT_OPTION_INFO( "--debug (none|state|match|paranthesis)"     , "Print info for debuging.  To have different kinds of debug output, use more than once");
    PRINT_OPTION_INFO( "--help, -h"                                 , "Show this help.");
}


// Get argument and convert it to integer
int get_int_arg(int i, int argc, char **argv)
{
    // TODO:
    // actualy check whether argument is a number.
    // `atoi` has no defined behaviour when parse does not succeed
    // (but usualy returns 0)
    if( i >= argc || !isdigit(argv[i][0]) )
    {
        FAIL_WITH_ERROR(1,"Missing or invalid value for option : %s", argv[i-1]);
    }
    return atoi(argv[i]);
}


void read_cli_options(int argc, char **argv)
{
    int i;
    if(argc == 1) return; // use stdin and stdout

    if( argc == 2 && strcmp(argv[1],"--create-config-file") == 0) {
        if(create_config_file(CONFIG_FILE) != 0)
            exit(1);
        else {
            fprintf(stderr,"File '%s' (re)created.\n", CONFIG_FILE);
            exit(0);
        }
    }

    for(i=1;i<argc;i++)
    {
        if( argv[i][0] != '-')
        {
            if(yyin == stdin){   //try to openinig INPUT file
                if(  !(yyin = fopen(argv[1],"r"))  ) FAIL_WITH_ERROR(1,"Error opening input file: %s", argv[i]);
            }
            else if(yyout == stdout){   //try to openinig OUTPUT file (only if INPUT file is set)
                if(  !(yyout=fopen(argv[2],"w+"))  ) FAIL_WITH_ERROR(1,"Error opening output file: %s", argv[i]);
            }
        } else if( ARGV_MATCH(i,"-i") ){
            if( ++i >= argc) FAIL_WITH_ERROR(1,"Missing value for option : %s", argv[i-1]);
	    if(  !(yyin=fopen(argv[i],"r"))  ) FAIL_WITH_ERROR(1,"Error opening input file: %s", argv[i]);
        } else if( ARGV_MATCH(i,"-o") ){
            if( ++i >= argc) FAIL_WITH_ERROR(1,"Missing value for option : %s", argv[i-1]);
	    if(  !(yyout=fopen(argv[i],"w+"))  ) FAIL_WITH_ERROR(1,"Error opening output file: %s", argv[i]);
        } else if( ARGV_MATCH(i,"--config-file") )
        {
            if( ++i >= argc) FAIL_WITH_ERROR(1,"Missing value for option : %s", argv[i-1]);
            if( read_conf_file(argv[i]) == READ_FAILED )
            {
                FAIL_WITH_ERROR(1,"Error reading configureation file: %s", argv[i]);
            }
        } else if( ARGV_MATCH(i,"--select-comma-newline") )
        {
            if( ++i >= argc) FAIL_WITH_ERROR(1,"Missing value for option : %s", argv[i-1]);
            if(strcmp(argv[i],"after") == 0) {
                kw_comma.before.new_line = 0;
                kw_comma.after.new_line  = 1;
            } else if(strcmp(argv[i],"before") == 0) {
                kw_comma.before.new_line = 1;
                kw_comma.after.new_line  = 0;
            } else if(strcmp(argv[i],"none") == 0) {
                kw_comma.before.new_line = 0;
                kw_comma.after.new_line  = 0;
            }
        } else if( ARGV_MATCH(i,"--keyword-case") )
        {
            if( ++i >= argc) FAIL_WITH_ERROR(1,"Missing value for option : %s", argv[i-1]);
            if(strcmp(argv[i],"none") == 0) {
                set_case(CASE_none);
            } else if(strcmp(argv[i],"upper") == 0) {
                set_case(CASE_UPPER);
            } else if(strcmp(argv[i],"lower") == 0) {
                set_case(CASE_lower);
            } else if(strcmp(argv[i],"initcap") == 0) {
                set_case(CASE_Initcap);
            }
        } else if( ARGV_MATCH(i,"--keyword-text") )
        {
            if( ++i >= argc) FAIL_WITH_ERROR(1,"Missing value for option : %s", argv[i-1]);
            if(strcmp(argv[i],"original") == 0) {
                set_text_original(1);
            } else if(strcmp(argv[i],"default") == 0) {
                set_text_original(0);
            }
        } else if( ARGV_MATCH(i,"--select-newline-after") )
        {
            kw_select.after.new_line = get_int_arg(++i, argc, argv);
        } else if( ARGV_MATCH(i,"--newline-or-before") )
        {
            kw_or.before.new_line = get_int_arg(++i, argc, argv);
        } else if( ARGV_MATCH(i,"--newline-or-after") )
        {
            kw_or.after.new_line = get_int_arg(++i, argc, argv);
        } else if( ARGV_MATCH(i,"--newline-and-before") )
        {
            kw_and.before.new_line = get_int_arg(++i, argc, argv);
        } else if( ARGV_MATCH(i,"--newline-and-after") )
        {
            kw_and.after.new_line = get_int_arg(++i, argc, argv);
        } else if( ARGV_MATCH(i,"--newline-major-sections") )
        {
            int new_line_count = get_int_arg(++i, argc, argv);
            kw_from.before.new_line = new_line_count;
            kw_where.before.new_line = new_line_count;
            kw_inner_join.before.new_line = new_line_count;
            kw_left_join.before.new_line  = new_line_count;
            kw_right_join.before.new_line = new_line_count;
            kw_full_join.before.new_line  = new_line_count;
            kw_cross_join.before.new_line = new_line_count;
        } else if( ARGV_MATCH(i,"--debug") )
        {
            if( ++i >= argc ) FAIL_WITH_ERROR(1,"Missing or invalid value for option : %s", argv[i-1]);
            if     ( ARGV_MATCH(i,"none")         ) debug_level |= DEBUGNONE;
            else if( ARGV_MATCH(i,"state")        ) debug_level |= DEBUGSTATES;
            else if( ARGV_MATCH(i,"match")        ) debug_level |= DEBUGMATCHES;
            else if( ARGV_MATCH(i,"paranthesis")  ) debug_level |= DEBUGPARCOUNTS;
            else FAIL_WITH_ERROR(1,"Missing or invalid value for option : %s", argv[i-1]);

        } else if( strcmp(argv[i],"--help") == 0 || strcmp(argv[i],"-h") == 0)
        {
            usage_info(argc, argv);
            exit(0);
        } else FAIL_WITH_ERROR(1,"Option `%s' is not recognised or used incorrectly.\nTry `%s --help' for more information\n", argv[i], argv[0]);
    }
}



#endif

