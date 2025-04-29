#include <assert.h>
#include <memory.h>

void *worker_malloc(const size_t size, uint64_t *stat)
{
	assert(stat);

	void *p = malloc(size);
	if (!p)
		return NULL;
	*stat = *stat + 1;
	return p;
}

void worker_free(void *p, uint64_t *stat)
{
	assert(p);
	assert(stat);
	free(p);
	*stat = *stat + 1;
}