#include <rtthread.h>
#include <dfs_posix.h>

#define TEST_FN		"/test.dat"

static char test_data[120], buffer[120];
int readwrite()
{
	int fd;
	int index, length;

	rt_kprintf("begin test read/write.\n");

	fd = open(TEST_FN, O_WRONLY | O_CREAT | O_TRUNC, 0);
	if (fd < 0)
	{
		rt_kprintf("open file for write failed\n");
		return 0;
	}

	for (index = 0; index < sizeof(test_data); index ++)
	{
		test_data[index] = index + 27;
	}

	length = write(fd, test_data, sizeof(test_data));
	if (length != sizeof(test_data))
	{
		rt_kprintf("write data failed\n");
		close(fd);
		return 0;
	}

	close(fd);

	fd = open(TEST_FN, O_WRONLY | O_CREAT | O_APPEND, 0);
	if (fd < 0)
	{
		rt_kprintf("open file for append write failed\n");
		return 0;
	}

	length = write(fd, test_data, sizeof(test_data));
	if (length != sizeof(test_data))
	{
		rt_kprintf("append write data failed\n");
		close(fd);
		return 0;
	}

	close(fd);

	fd = open(TEST_FN, O_RDONLY, 0);
	if (fd < 0)
	{
		rt_kprintf("check: open file for read failed\n");
		return 0;
	}

	length = read(fd, buffer, sizeof(buffer));
	if (length != sizeof(buffer))
	{
		rt_kprintf("check: read file failed\n");
		close(fd);
		return 0;
	}

	for (index = 0; index < sizeof(test_data); index ++)
	{
		if (test_data[index] != buffer[index])
		{
			rt_kprintf("check: check data failed at %d\n", index);
			close(fd);
			return 0;
		}
	}

	length = read(fd, buffer, sizeof(buffer));
	if (length != sizeof(buffer))
	{
		rt_kprintf("check: read file failed\n");
		close(fd);
		return 0;
	}

	for (index = 0; index < sizeof(test_data); index ++)
	{
		if (test_data[index] != buffer[index])
		{
			rt_kprintf("check: check data failed at %d\n", index);
			close(fd);
			return 0;
		}
	}

	close(fd);
	rt_kprintf("read/write done.\n");
	return 1;
}

