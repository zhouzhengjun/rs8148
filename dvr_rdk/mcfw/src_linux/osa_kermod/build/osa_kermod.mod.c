#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
 .arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x5beb8438, "module_layout" },
	{ 0xad15349a, "cdev_del" },
	{ 0xdb63d8fa, "kmalloc_caches" },
	{ 0x83d70683, "edma_start" },
	{ 0x12da5bb2, "__kmalloc" },
	{ 0x8dc8a2dd, "cdev_init" },
	{ 0xfbc74f64, "__copy_from_user" },
	{ 0x67c2fa54, "__copy_to_user" },
	{ 0xa31e44ba, "edma_free_channel" },
	{ 0x61e1850a, "edma_write_slot" },
	{ 0xf1e0b260, "edma_set_transfer_params" },
	{ 0xcaddbd7e, "edma_set_dest_index" },
	{ 0x85737519, "edma_read_slot" },
	{ 0xb0bb9c02, "down_interruptible" },
	{ 0x7485e15e, "unregister_chrdev_region" },
	{ 0x9bda4bb4, "edma_set_src" },
	{ 0xf6288e02, "__init_waitqueue_head" },
	{ 0x9276ce28, "edma_set_dest" },
	{ 0xe707d823, "__aeabi_uidiv" },
	{ 0xfa2a45e, "__memzero" },
	{ 0x3635439, "edma_stop" },
	{ 0xea147363, "printk" },
	{ 0xec6a4d04, "wait_for_completion_interruptible" },
	{ 0x8b00761c, "cdev_add" },
	{ 0x82cb91ae, "kmem_cache_alloc" },
	{ 0xe9ce8b95, "omap_ioremap" },
	{ 0x37a0cba, "kfree" },
	{ 0xa9822f7b, "remap_pfn_range" },
	{ 0xfefb6077, "edma_alloc_channel" },
	{ 0x8cf51d15, "up" },
	{ 0xf7271948, "edma_set_src_index" },
	{ 0xefd6cf06, "__aeabi_unwind_cpp_pr0" },
	{ 0x60f71cfa, "complete" },
	{ 0x29537c9e, "alloc_chrdev_region" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

