#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libcoro.h"
#include "qsort.h"
#include <dirent.h>
#include <getopt.h>
#include <time.h>

#include <unistd.h>

#include "vector.h"


struct my_context {
	char *name;
	int num_files;
	int *index_next_file;
	char **file_names;
	struct vector *vectors;
	long long time_per_coroutine;
	long long time;
	struct timespec prev_time;
};

static struct my_context *
my_context_new(const char *name, int num_files, int * index_next_file, char ** file_names, struct vector *vectors, long long time_per_coroutine)
{
	struct my_context *ctx = malloc(sizeof(*ctx));
	ctx->name = strdup(name);
	ctx->num_files = num_files;
	ctx->index_next_file = index_next_file;
	ctx->file_names = file_names;
	ctx->vectors = vectors;
	ctx->time_per_coroutine = time_per_coroutine;
	return ctx;
}

static void
my_context_delete(struct my_context *ctx)
{
	free(ctx->name);
	free(ctx);
}
// ======TIME======
static void
start_timer(struct my_context *ctx) {
	clock_gettime(CLOCK_MONOTONIC, &ctx->prev_time);
}

static void
stop_timer(struct my_context *ctx)
{
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	ctx->time += (t.tv_sec - ctx->prev_time.tv_sec) * 1e9;
	ctx->time += t.tv_nsec - ctx->prev_time.tv_nsec;
}

static bool
is_time_over(struct my_context *ctx)
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    long elapsed_time = (t.tv_sec - ctx->prev_time.tv_sec) * 1e9 + (t.tv_nsec - ctx->prev_time.tv_nsec);
    return elapsed_time >= ctx->time_per_coroutine;
}



static int
coroutine_func_f(void *context)
{
	struct coro *this = coro_this();
	struct my_context *ctx = context;
	char *name = ctx->name;
	int num_files = ctx->num_files;
	char **file_names = ctx->file_names;
	struct vector *vectors = ctx->vectors;
	int *index_next_file = ctx->index_next_file;

	start_timer(ctx);
	printf("Started: %s\n", name);

	for (;*index_next_file < num_files;) {
		int index = *index_next_file;
		printf("%s: switch count %lld. File name: %s\n", name, coro_switch_count(this), file_names[index]);
		++*index_next_file;

		FILE * fp = fopen(file_names[index], "r");
		if(!fp) {return -1;}
		int num;
		while (!feof(fp)) {
			fscanf(fp, "%d ", &num);
			vector_push_back(&vectors[index], num);
		}
		quick_sort( vectors[index].arr, 0, vectors->length-1);
		fclose(fp);

		fp = fopen(file_names[index], "w");
		if (!fp) {return -1;}
		for (int i = 0; i < vectors[index].length; ++i) {
			fprintf(fp, "%d ", vectors[index].arr[i]);
		}
		fclose(fp);

		if (is_time_over(ctx)) {
			stop_timer(ctx);
			printf("%s: yield\n", name);
			coro_yield();
			start_timer(ctx);
		}

	}
	stop_timer(ctx);

	printf("%s: work time %lld\n", name, ctx->time);

	my_context_delete(ctx);
	return 0;
}
int main(int argc, char **argv) {
	int num_taken_options = 0;
	int number_coroutines = 10;
	long long latency = 5000;
	int opt;
	while((opt = getopt(argc, argv, "l:n:")) != -1)
	{
		switch(opt)
		{
			case 'l':
				latency = atoi(optarg);
				num_taken_options += 2;
				break;
			case 'n':
				number_coroutines = atoi(optarg);
				num_taken_options += 2;
				break;
			default:
				break;
		}
	}
	latency *= 1000;

	int num_files = argc - 1 - num_taken_options;
	int  next_file = 0;
	struct vector *vectors = malloc(num_files * sizeof(struct vector));

	for (int i = 0; i < num_files; ++i) {
		vector_init(&vectors[i]);
	}

	coro_sched_init();
	for (int i = 0; i < number_coroutines; ++i) {
		char name[16];
		sprintf(name, "coro_%d", i);

		coro_new(coroutine_func_f, my_context_new(name, num_files, &next_file, &argv[num_taken_options+1], vectors, latency/num_files));
	}

	struct coro *c;
	while ((c = coro_sched_wait()) != NULL) {
		/*
		 * Each 'wait' returns a finished coroutine with which you can
		 * do anything you want. Like check its exit status, for
		 * example. Don't forget to free the coroutine afterwards.
		 */
		printf("Finished %d\n", coro_status(c));
		coro_delete(c);
	}

	// ======MERGING======
	FILE * fp = fopen("out.txt", "w");
	if(!fp) {return -1;}
	int *indexes = malloc(sizeof(int) * num_files);
	for (int i = 0; i < num_files; ++i) {
		indexes[i] = 0;
	}
	printf("\n");
	for(;;) {
		int index_min = -1;
		int num_processed_vectors = 0;
		for (int i = 0; i < num_files; ++i) {
			if(indexes[i] >= vectors[i].length) {
				++num_processed_vectors;
				continue;
			}
			if(index_min == -1 || vectors[i].arr[indexes[i]] < vectors[index_min].arr[indexes[index_min]])
				index_min = i;
		}
		if(num_processed_vectors == num_files)
			break;
		fprintf(fp, "%d ", vectors[index_min].arr[indexes[index_min]]);
		++indexes[index_min];
	}
	fclose(fp);
	free(indexes);

	for (int i = 0; i < num_files; ++i) {
		vector_delete(&vectors[i]);
	}
	free(vectors);
}

