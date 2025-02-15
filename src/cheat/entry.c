
#include "linux/anon_inodes.h"
#include <linux/file.h>
#include <linux/module.h>
#include "comm.h"
#include "memory.h"
#include "process.h"

int cheat_open(struct inode *node, struct file *file)
{
	return 0;
}

int cheat_close(struct inode *node, struct file *file)
{
	return 0;
}

long cheat_ioctl(struct file *const file, unsigned int const cmd,
		 unsigned long const arg)
{
	static COPY_MEMORY cm;
	static MODULE_BASE mb;
	static char name[0x100] = { 0 };

	switch (cmd) {
	case OP_READ_MEM: {
		if (copy_from_user(&cm, (void __user *)arg, sizeof(cm)) != 0) {
			return ERROR_TOUCH_ARGS_FAILED;
		}
		if (read_process_memory(cm.pid, cm.addr, cm.buffer, cm.size) ==
		    false) {
			return ERROR_READ_MEM_FAILED;
		}
		break;
	}
	case OP_WRITE_MEM: {
		if (copy_from_user(&cm, (void __user *)arg, sizeof(cm)) != 0) {
			return ERROR_TOUCH_ARGS_FAILED;
		}
		if (write_process_memory(cm.pid, cm.addr, cm.buffer, cm.size) ==
		    false) {
			return ERROR_WRITE_MEM_FAILED;
		}
		break;
	}
	case OP_MODULE_BASE: {
		if (copy_from_user(&mb, (void __user *)arg, sizeof(mb)) != 0 ||
		    copy_from_user(name, (void __user *)mb.name,
				   sizeof(name) - 1) != 0) {
			return ERROR_TOUCH_ARGS_FAILED;
		}
		mb.base = get_module_base(mb.pid, name);
		if (copy_to_user((void __user *)arg, &mb, sizeof(mb)) != 0) {
			return ERROR_GET_MODULE_BASE_FAILED;
		}
		break;
	}
	default:
		return ERROR_UNKNOWN_OP;
	}

	return OP_SUCCESS;
}

struct file_operations cheat_fops = {
	.owner = THIS_MODULE,
	.open = cheat_open,
	.release = cheat_close,
	.unlocked_ioctl = cheat_ioctl,
};

int get_cheat_tool_handle(void)
{
	int fd = anon_inode_getfd("/data/adb/ksud", &cheat_fops, NULL, O_RDWR);
	return fd;
}
