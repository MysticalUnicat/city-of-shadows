%token_type {struct Token}
%token_destructor {string_free($$.string);}
%token INVALID.
%token WAITING.
%token END_OF_INPUT.
%extra_argument {struct Extra * extra}
