.PHONY = all clean fclean re

NAME = webserv

SRCDIR = src
SRC = $(wildcard $(SRCDIR)/*.cpp)

OBJ = $(SRC:.cpp=.o)

CC = c++

CFLAG = -Wall -Wextra -Werror -std=c++98 -pedantic-errors -I. -g3 -fsanitize=address
# CFLAG = -Wall -Wextra -Werror -std=c++98 -pedantic-errors -I.

all: .all_check

.all_check: $(OBJ)
	@$(CC) $(CFLAG) -o $(NAME) $^
	@touch $@

clean:
	@rm -rf $(OBJ)

fclean:
	@make clean
	@rm -rf .all_check
	@rm -rf $(NAME)

re:
	@make fclean
	@make all

$(SRCDIR)/%.o: $(SRCDIR)/%.cpp
	$(CC) $(CFLAG) -c $< -o $@
