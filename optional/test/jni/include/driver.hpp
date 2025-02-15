#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include "comm.h"
#include <stdio.h>
#include <memory.h>
#include <unistd.h>
#define CMD_GET_CHEAT_HANDLE 14
class c_driver
{
private:
	int fd;
	pid_t pid;

	

public:
	c_driver()
	{
		int result=-1;
		prctl(0xdeadbeef, CMD_GET_CHEAT_HANDLE, 0, 0, &result);
		fd = result;
		if (fd == -1)
		{
			printf("[-] open driver failed\n");
		}
	}

	~c_driver()
	{
		if (fd > 0)
			close(fd);
	}

	void initialize(pid_t pid)
	{
		this->pid = pid;
	}

	bool read(uintptr_t addr, void *buffer, size_t size)
	{
		COPY_MEMORY cm;

		cm.pid = this->pid;
		cm.addr = addr;
		cm.buffer = buffer;
		cm.size = size;
		cm.offsets_count = 0;
		fflush(stdout);
		if (ioctl(fd, OP_READ_MEM, &cm) != 0)
		{
			return false;
		}
		return true;
	}
	bool read_with_offsets(uintptr_t addr, void *buffer, size_t size,size_t offsets_count,uintptr_t offsets[])
	{
		COPY_MEMORY cm;

		cm.pid = this->pid;
		cm.addr = addr;
		cm.buffer = buffer;
		cm.size = size;
		cm.offsets_count = offsets_count;
		memcpy(cm.offsets,offsets,sizeof(uintptr_t)*offsets_count);
		if (ioctl(fd, OP_READ_MEM, &cm) != 0)
		{
			return false;
		}
		return true;
	}

	bool write(uintptr_t addr, void *buffer, size_t size)
	{
		COPY_MEMORY cm;

		cm.pid = this->pid;
		cm.addr = addr;
		cm.buffer = buffer;
		cm.size = size;

		if (ioctl(fd, OP_WRITE_MEM, &cm) != 0)
		{
			return false;
		}
		return true;
	}

	template <typename T>
	T read(uintptr_t addr)
	{
		T res;
		if (this->read(addr, &res, sizeof(T)))
			return res;
		return {};
	}
	template <typename T>
	T read_with_offsets(uintptr_t addr,size_t offsets_count,uintptr_t offsets[])
	{
		T res;
		if (this->read_with_offsets(addr, &res, sizeof(T),offsets_count,offsets))
			return res;
		return {};
	}

	template <typename T>
	bool write(uintptr_t addr, T value)
	{
		return this->write(addr, &value, sizeof(T));
	}

	uintptr_t get_module_base(char *name)
	{
		MODULE_BASE mb;
		char buf[0x100];
		strcpy(buf, name);
		mb.pid = this->pid;
		mb.name = buf;

		if (ioctl(fd, OP_MODULE_BASE, &mb) != 0)
		{
			return 0;
		}
		return mb.base;
	}
};

static c_driver *driver = new c_driver();
