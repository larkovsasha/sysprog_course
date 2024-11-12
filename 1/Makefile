GCC_FLAGS = -Wextra -Werror -Wall -Wno-gnu-folding-constant
HEAP_HELP = -ldl -rdynamic

all: libcoro.c solution.c
	gcc $(GCC_FLAGS) $(HEAP_HELP) libcoro.c solution.c qsort.c vector.c heap_help/heap_help.c
heap_help: libcoro.c solution.c
	gcc  $(HEAP_HELP) libcoro.c solution.c qsort.c vector.c heap_help/heap_help.c
clean:
	rm a.out
