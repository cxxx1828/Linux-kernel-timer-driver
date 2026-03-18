#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xa87151c7, "module_layout" },
	{ 0x8181429f, "platform_driver_unregister" },
	{ 0xb9a12f50, "__platform_driver_register" },
	{ 0x5f754e5a, "memset" },
	{ 0xae353d77, "arm_copy_from_user" },
	{ 0x2eadb111, "misc_register" },
	{ 0xe2544082, "devm_kasprintf" },
	{ 0xc3756dde, "devm_request_threaded_irq" },
	{ 0xaf055cdc, "platform_get_irq" },
	{ 0xe346f67a, "__mutex_init" },
	{ 0x5bbe49f4, "__init_waitqueue_head" },
	{ 0x40bff132, "of_property_read_variable_u32_array" },
	{ 0xd26a07d, "__pm_runtime_resume" },
	{ 0xc6f1ef7d, "pm_runtime_enable" },
	{ 0x34d84551, "devm_ioremap_resource" },
	{ 0x47491625, "platform_get_resource" },
	{ 0x22a32fe1, "devm_kmalloc" },
	{ 0x3dcf1ffa, "__wake_up" },
	{ 0x8f678b07, "__stack_chk_guard" },
	{ 0x86332725, "__stack_chk_fail" },
	{ 0x51a910c0, "arm_copy_to_user" },
	{ 0x67ea780, "mutex_unlock" },
	{ 0x97255bdf, "strlen" },
	{ 0xc271c3be, "mutex_lock" },
	{ 0x49970de8, "finish_wait" },
	{ 0x1000e51, "schedule" },
	{ 0x6578533e, "prepare_to_wait" },
	{ 0xad73041f, "autoremove_wake_function" },
	{ 0x6110406, "__pm_runtime_disable" },
	{ 0xf523617, "misc_deregister" },
	{ 0xb1ad28e0, "__gnu_mcount_nc" },
};

MODULE_INFO(depends, "");

MODULE_ALIAS("of:N*T*Clinux,timer");
MODULE_ALIAS("of:N*T*Clinux,timerC*");

MODULE_INFO(srcversion, "2E0F96D8156FF3A618149DB");
