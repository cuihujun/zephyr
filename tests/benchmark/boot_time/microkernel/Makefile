CONF_basic_minuteia 	= ia32/prj.conf
CONF_quark_d2000_crb   	= quark_d2000/quark_d2000_prj.conf
CONF_quark_se_devboard	= quark_se/quark_se_prj.conf
CONF_qemu_x86     	= ia32/prj_expert_test.conf
CONF_qemu_x86_iamcu	= ia32/prj_expert_test.conf
CONF_minnowboard     	= ia32/prj_expert_test.conf
CONF_galileo        	= ia32_pci/prj.conf
CONF_arduino_101	= quark_se/quark_se_prj.conf

MDEF_FILE = prj.mdef
KERNEL_TYPE = micro
BOARD ?= qemu_x86
CONF_FILE = ${CONF_${BOARD}}

include ${ZEPHYR_BASE}/Makefile.inc
