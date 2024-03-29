# ---------------------------------------------------------------------------- #
# The purpose of the Makefile is to provide a tool which optimizes the build   #
# of a project, The common problems are bad management of dependencies, relink #
# build, uncompiled modified sources...                                        #
#                                                                              #
# ---------------------------------------------------------------------------- #
#                                                                              #
# USAGE                                                                        #
#                                                                              #
# ---------------------------------------------------------------------------- #
# First, the user must configure its environment settings.                     #
# - see 'PROJECT CONFIGURATION' to configure project directories               #
# - see 'EXTERNAL TOOLS SETTINGS' to setup default programs to use             #
#                                                                              #
# Second, configure the sources.                                               #
# - see 'TARGET SETUP' to set the name of the target and the sources           #
#                                                                              #
# Third, configure the build options.                                          #
# - see 'PROJECT COMPILATION' to setup prepocessor, flags and libraries        #
#                                                                              #
# Fourth, setup the linking rule.                                              #
# - see 'PUBLIC RULES' to modify the $(NAME) rule                              #
#                                                                              #
# ---------------------------------------------------------------------------- #
# The project must compile at this step.                                       #
# ---------------------------------------------------------------------------- #
#                                                                              #
# To add custom rules, the concerned section is 'PUBLIC RULES'. Be sure to     #
# keep at least the rules all, $(NAME), libs, clean, fclean and re.            #
#                                                                              #
# ---------------------------------------------------------------------------- #
#                                                                              #
#                               /!\ WARNING /!\                                #
#                                                                              #
# The sections commented with '/!\' are critical, and must not be modified.    #
#                                                                              #
# ---------------------------------------------------------------------------- #


# ---------------------------------------------------------------------------- #
#                                                                              #
# TARGET SETUP                                                                 #
#                                                                              #
# ---------------------------------------------------------------------------- #
# - The 'NAME' variable must contain the expected name of the output target.   #
# - The 'SRCS' variable must contain the list of the source files without the  #
# base prefix of the directory.                                                #
# ---------------------------------------------------------------------------- #

NAME	=	programlib.a

# ---------------------------------------------------------------------------- #

SRCS	=	\
			logger/logger/logger_init.c						\
			logger/logger/logger_close.c					\
			logger/display/logger_fatal.c					\
			logger/display/logger_error.c					\
			logger/display/logger_warn.c					\
			logger/display/logger_success.c				\
			logger/display/logger_info.c					\
			logger/display/logger_debug.c					\
			logger/display/logger_trace.c					\
			logger/utils/logger_get_time.c				\
			logger/utils/logger_init_open_file.c	\
			networking/tftp/header.c							\
			networking/tftp/tftp_server.c					\
			networking/tftp/tftp_client_handle.c	\
			networking/http/http_server.c					\
			networking/http/http_request.c					\
			networking/checksum.c 								\
			networking/server.c 									\
			data_structures/lists/linked_list.c		\
			data_structures/lists/queue.c					\
			data_structures/common/node.c					\
			data_structures/trees/binary_search_tree.c \
			data_structures/dictionary/entry.c	  \
			data_structures/dictionary/dictionary.c    \
			systems/files.c												\
			systems/thread_pool.c									\
			utils/helper.c 												\
			database/db.c													\
			http/controller/user_controller.c		  \
			http/controller/group_controller.c		\
			http/controller/directory_controller.c\
			http/controller/file_controller.c			\
			http/helper/helper.c									\
			model/user.c  												\
			model/session.c												\
			model/group.c													\
			model/directory.c											\
			model/file.c													\
			# main.c																\

# ---------------------------------------------------------------------------- #
# PROJECT CONFIGURATION                                                        #
# ---------------------------------------------------------------------------- #
# - The 'DIR*' variables describe all directories of the project.              #
# ---------------------------------------------------------------------------- #

DIRSRC	=	src
DIRINC	=	includes
DIRLIB	=	libs
DIRTST	=	tests
DIRMAIN = main
DIROBJ	=	.objs
DIRDEP	=	.deps


# ---------------------------------------------------------------------------- #
# EXTERNAL TOOLS SETTINGS                                                      #
# ---------------------------------------------------------------------------- #
# - Set the default external programs.                                         #
# ---------------------------------------------------------------------------- #

CC			=	gcc
AR			=	ar
ARFLAGS	=	rc
RM			=	rm -f

# ---------------------------------------------------------------------------- #
# PROJECT COMPILATION                                                          #
# ---------------------------------------------------------------------------- #
# - The 'LDFLAGS' tells the linker where to find external libraries (-L flag). #
# - The 'LDLIBS' tells the linker the prefix of external libraries (-l flag).  #
# - The 'CPPFLAGS' tells the compiler where to find preprocessors (-I flag).   #
# - The 'CFLAGS' configures the compiler options.                              #
# ---------------------------------------------------------------------------- #

LIBS		=	\

LDLIBS		=	\
					  -lm               \
						-lpthread				  \
						-l:sqlite3.a      \

LDFLAGS		=	\
				-L $(DIRLIB)					\

CPPFLAGS	=	\
				-I $(DIRINC)					\

CFLAGS		=	\
				-Wall -Wextra -Werror	\

# ---------------------------------------------------------------------------- #
# /!\ COMPILATION RULES /!\                                                    #
# ---------------------------------------------------------------------------- #

DEPFLAGS	=	\
				-MT $@ -MMD -MP -MF $(DIRDEP)/$*.Td	\

COMPILE.c	=	$(CC) $(DEPFLAGS) $(CFLAGS) $(CPPFLAGS) -c
POSTCOMPILE	=	mv -f $(DIRDEP)/$*.Td $(DIRDEP)/$*.d

# ---------------------------------------------------------------------------- #
# /!\ SOURCES NORMALIZATION /!\                                                #
# ---------------------------------------------------------------------------- #

SRC	=	$(addprefix $(DIRSRC)/, $(SRCS))
OBJ	=	$(addprefix $(DIROBJ)/, $(SRCS:.c=.o))

$(DIROBJ)	:
	@mkdir -p $(DIROBJ)

$(DIRDEP)	:
	@mkdir -p $(DIRDEP)

# ---------------------------------------------------------------------------- #
# PUBLIC RULES                                                                 #
# ---------------------------------------------------------------------------- #
# The rules must contain at least :                                            #
# - all        make libs and target                                            #
# - $(NAME)    make binaries and target                                        #
# - libs       build static libraries                                          #
# - clean      remove binaries                                                 #
# - fclean     remove binaries and target                                      #
# - fcleanlibs apply fclean rule on libraries                                  #
# - re         remove binaries, target and libraries and build the target      #
#                                                                              #
# To compile a static library, the $(NAME) rule should be :                    #
#     '$(AR) $(ARFLAGS) $(NAME) $(OBJ)'                                        #
#     'ranlib $(NAME)'                                                         #
#                                                                              #
# To compile a C binary, the $(NAME) rule should be :                          #
#     '$(CC) $(OBJ) -o $(NAME) $(LDFLAGS) $(LDLIBS)'                           #
# ---------------------------------------------------------------------------- #

all: $(NAME)
	@printf "\033[32m[ %s ]\033[0m %s\n" "$(NAME)" "finish to build $(NAME)"

$(NAME): $(DIROBJ) $(DIRDEP) $(OBJ) $(LIBS)
	@printf "\033[32m[ %s ]\033[0m %s\n" "$(NAME)" "link objects..."
	@$(AR) $(ARFLAGS) $(NAME) $(OBJ)
	@ranlib $(NAME)
libs		:

fcleanlibs	:

clean		:
	@printf "\033[32m[ %s ]\033[0m %s\n" "$(NAME)" "remove objects..."
	@$(RM) -r $(DIROBJ)
	@printf "\033[32m[ %s ]\033[0m %s\n" "$(NAME)" "remove dependencies..."
	@$(RM) -r $(DIRDEP)

fclean		:	clean
	@printf "\033[32m[ %s ]\033[0m %s\n" "$(NAME)" "remove target..."
	@$(RM) $(NAME)

re			:	fcleanlibs fclean all

# ---------------------------------------------------------------------------- #
# CUSTOM RULES                                                                 #
# ---------------------------------------------------------------------------- #
http: all
	@printf "\033[32m[ http.out ]\033[0m %s\n" "Compiling http server..."
	@$(CC) $(CFLAGS) -c $(DIRMAIN)/http.c -o http.o -I $(DIRINC)
	@$(CC) http.o $(NAME) $(LDFLAGS) $(LDLIBS) -o http.out

tftp: all
	@printf "\033[32m[ tftp.out ]\033[0m %s\n" "Compiling tftp server..."
	@$(CC) $(CFLAGS) -c $(DIRMAIN)/tftp.c -o tftp.o -I $(DIRINC)
	@$(CC) tftp.o $(NAME) -o tftp.out

# ---------------------------------------------------------------------------- #
# /!\ PRIVATE RULES /!\                                                        #
# ---------------------------------------------------------------------------- #
$(DIROBJ)/%.o	:	$($(DIRSRC)/%.c
$(DIROBJ)/%.o	:	$(DIRSRC)/%.c $(DIRDEP)/%.d
	@mkdir -p $(dir $@)
	@mkdir -p $(dir $(word 2,$^))
	@printf "\033[32m[ %s ]\033[0m %s\n" "$(NAME)" "compiling $<..."
	@$(COMPILE.c) $(OUTPUT_OPTION) $<
	@$(POSTCOMPILE)

$(DIRDEP)/%.d	:	;
.PRECIOUS		:	$(DIRDEP)/%.d

-include $(patsubst %,$(DIRDEP)/%.d,$(basename $(SRCS)))

# ---------------------------------------------------------------------------- #

.PHONY	:	all clean fclean re $(DIROBJ)/%.o $(DIRDEP)/%.d libs

# ---------------------------------------------------------------------------- #
